#include "InfoSmeMessageSender.h"
#include "SmscConnector.h"
#include "util/config/region/RegionFinder.hpp"
#include "util/config/ConfString.h"
#include "system/status.h"

namespace smsc {
namespace infosme {

using smsc::util::config::ConfString;

InfoSmeMessageSender::InfoSmeMessageSender( TaskProcessor& processor ) :
log_(smsc::logger::Logger::getInstance("msgsend")),
processor_(processor),
defaultConnector_(0),
started_(false)
{}


InfoSmeMessageSender::~InfoSmeMessageSender()
{
    ConnectorIterator it = connectors_.getIterator();
    SmscConnector* connector;
    char *key = 0;
    while(it.Next(key, connector)) {
        if (connector) {
            delete connector;
        }
    }
}


void InfoSmeMessageSender::start()
{
    if ( ! regionsConfig_.get() ) {
        throw smsc::util::config::ConfigException("regions not loaded, use reloadSmscAndRegions");
    }
    if ( started_ ) return;
    started_ = true;

    ConnectorIterator it = connectors_.getIterator();
    SmscConnector* connector;
    char *key = 0;
    while(it.Next(key, connector)) {
        if (connector) {
            connector->start();
        }
    }
}


void InfoSmeMessageSender::stop() 
{
    if ( ! started_ ) return;
    ConnectorIterator it = connectors_.getIterator();
    SmscConnector* connector;
    char *key = 0;
    while(it.Next(key, connector)) {
        if (connector) {
            connector->stop();
        }
    }
    started_ = false;
}


uint32_t InfoSmeMessageSender::sendSms(const std::string& org,const std::string& dst,const std::string& txt,bool flash)
{
    smsc::sms::Address parsedAddr(dst.c_str());
    const smsc::util::config::region::Region* foundRegion = smsc::util::config::region::RegionFinder::getInstance().findRegionByAddress(parsedAddr.toString());
    if ( !foundRegion ) {
      smsc_log_warn(log_, "sendSms: can't find region for abonent '%s'", dst.c_str());
      return smsc::system::Status::NOROUTE;
    }
    smsc_log_debug(log_, "sendSms: telephone number '%s' matches to mask for region with id '%s'", dst.c_str(), foundRegion->getId().c_str());

    SmscConnector* connector = getSmscConnector(foundRegion->getId());

    return connector->sendSms(org,dst,txt,flash);
}


SmscConnector* InfoSmeMessageSender::getSmscConnector( const string& regionId )
{
    const string* smscId = regions_.GetPtr(regionId.c_str());
    if (!smscId) {
        smsc_log_debug(log_, "SMSC id for region '%s' not set. Default SMSC '%s' will be used.",
                       regionId.c_str(), defaultConnector_->getSmscId().c_str() );
        return defaultConnector_;
    }
    SmscConnector** connector = connectors_.GetPtr(smscId->c_str());
    if (connector) {
        smsc_log_debug(log_, "SMSC id='%s' will be used for region '%s'.",smscId->c_str(), regionId.c_str());
        return *connector;
    }
    smsc_log_warn(log_, "SMSC connector id='%s' for region '%s' not found. Default SMSC '%s' will be used.",
                  smscId->c_str(), regionId.c_str(), defaultConnector_->getSmscId().c_str());
    return defaultConnector_;
}


void InfoSmeMessageSender::reloadSmscAndRegions( Manager& manager )
{
    const std::string csn("InfoSme.SMSCConnectors");
    ConfigView ccv(manager,csn.c_str());

    // get the default connector id
    const std::string defId = ConfString(ccv.getString("default", "default SMSC Connector id not set")).str();

    // create connectors
    std::auto_ptr<smsc::util::config::CStrSet> connNames(ccv.getShortSectionNames());
    defaultConnector_ = 0;
    {
        // delete connectors that are not found in new set
        std::set< std::string > badConnectors;
        std::vector< SmscConnector* > badConns;
        SmscConnector* connector;
        char *key = 0;
        for ( ConnectorIterator it = connectors_.getIterator(); it.Next(key,connector); ) {
            if ( connNames->find(key) != connNames->end() ) {
                badConnectors.insert(key);
                if ( connector ) {
                    connector->stop();
                    badConns.push_back( connector );
                }
            }
        }
        for ( std::set< std::string >::const_iterator i = badConnectors.begin();
              i != badConnectors.end();
              ++i ) {
            connectors_.Delete( i->c_str() );
            smsc_log_debug(log_,"smsc connector %s is removed from connector list",i->c_str());
        }
        for ( std::vector< SmscConnector* >::const_iterator i = badConns.begin();
              i != badConns.end();
              ++i ) {
            delete *i;
        }
    }

    const bool oldStarted = started_;
    stop();

    // add new connectors
    for ( smsc::util::config::CStrSet::iterator i = connNames.get()->begin();
          i != connNames.get()->end(); ++i ) {
        const std::string sectName( csn + "." + *i );
        ConfigView cview(manager,sectName.c_str());
        bool perftest = false;
        try {
            perftest = manager.getBool("InfoSme.performanceTest");
        } catch (...) {
        }
        SmscConnector::SmscConfig cfg = SmscConnector::readSmeConfig(cview);
        SmscConnector* conn = addConnector(cfg,*i,perftest);
        if ( *i == defId ) {
            defaultConnector_ = conn;
        }
    }
    if ( !defaultConnector_ ) {
        throw ConfigException("the section for default SMSC Connector is not found");
    } else {
        smsc_log_debug(log_,"the default smsc is %s", defaultConnector_->getSmscId().c_str() );
    }

    // delete all regions-to-smsc mapping
    smsc_log_debug(log_,"resetting all region mapping in region finder");
    smsc::util::config::region::RegionFinder::getInstance().unsafeReset();
    regions_.Empty();

    // regions
    ConfigView tpConfig(manager,"InfoSme");
    ConfString route_xml_file(tpConfig.getString("route_config_filename"));
    ConfString regions_xml_file(tpConfig.getString("regions_config_filename"));
    regionsConfig_.reset(new smsc::util::config::region::RegionsConfig(regions_xml_file.c_str()));
    smsc::util::config::region::RegionsConfig::status st = regionsConfig_->load();
    if ( st == smsc::util::config::region::RegionsConfig::success )
        smsc_log_info(log_, "config file %s has been loaded successful", regions_xml_file.c_str());
    else
        throw smsc::util::config::ConfigException("can't load config file %s", regions_xml_file.c_str());

    smsc::util::config::route::RouteConfig routeConfig;
    if ( routeConfig.load(route_xml_file.c_str()) == smsc::util::config::route::RouteConfig::success )
        smsc_log_info(log_, "config file %s has been loaded successful", route_xml_file.c_str());
    else
        throw smsc::util::config::ConfigException("can't load config file %s", route_xml_file.c_str());

    smsc::util::config::region::Region* region;
    smsc::util::config::region::RegionsConfig::RegionsIterator regsIter = regionsConfig_->getIterator();
    while (regsIter.fetchNext(region) == smsc::util::config::region::RegionsConfig::success) {
        try {
            region->expandSubjectRefs(routeConfig);
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"Problem in configuration of region '%s'",region->getId().c_str());
            throw ConfigException("Region '%s' misconfigured, exc: %s",region->getId().c_str(),e.what());
        }
        smsc::util::config::region::Region::MasksIterator maskIter = region->getMasksIterator();
        std::string addressMask;
        while(maskIter.fetchNext(addressMask)) {
            smsc_log_debug(log_,"registering mask '%s' for region %s/'%s'",
                           addressMask.c_str(),
                           region->getId().c_str(),
                           region->getName().c_str());
            smsc::util::config::region::RegionFinder::getInstance().registerAddressMask(addressMask, region);
        }
        smsc_log_debug(log_,"region %s/'%s' maps to SMSC '%s'",
                       region->getId().c_str(),
                       region->getName().c_str(),
                       region->getInfosmeSmscId().c_str());
        addRegionMapping( region->getId(), region->getInfosmeSmscId() );
    }
    const smsc::util::config::region::RegionDefault& regdef = regionsConfig_->getDefaultRegion();
    smsc_log_debug(log_,"registering a default region %s/'%s'",
                   regdef.getId().c_str(), regdef.getName().c_str() );
    smsc::util::config::region::RegionFinder::getInstance().registerDefaultRegion(&regdef);

    if ( oldStarted ) start();
}


bool InfoSmeMessageSender::send( Task* task, Message& message )
{
    // get region by message.regionId
    const smsc::util::config::region::Region* region = smsc::util::config::region::RegionFinder::getInstance().getRegionById(message.regionId);
    if ( ! region ) {
        const TaskInfo& info = task->getInfo();
        smsc_log_info(log_,"TaskId=[%d/%s]: msgId=%llx region '%s' is not found, using default",
                      info.uid, info.name.c_str(), message.id, message.regionId.c_str() );
        region = smsc::util::config::region::RegionFinder::getInstance().getDefaultRegion();
        if ( !region ) {
            smsc_log_error(log_,"default region is not found");
            return false;
        }
    }

    SmscConnector* connector = getSmscConnector(region->getId());
    if ( ! connector ) {
        const TaskInfo& info = task->getInfo();
        smsc_log_error(log_,"TaskId=[%d/%s]: msgId=%llx cannot find connector by regionId=%s",
                       info.uid, info.name.c_str(), message.id, region->getId().c_str());
        TaskProcessor::retryMessage(task,message.id);
        return false;
    }
    return connector->send( task, message, region );
}


void InfoSmeMessageSender::processWaitingEvents( time_t tm )
{
    SmscConnector* connector;
    char *key = 0;
    for ( ConnectorIterator it = connectors_.getIterator(); it.Next(key,connector); ) {
        connector->processWaitingEvents(tm);
    }
}


SmscConnector* InfoSmeMessageSender::addConnector( const SmscConnector::SmscConfig& cfg,
                                                   const std::string& smscid,
                                                   bool  performanceTest )
{
    SmscConnector** ptr = connectors_.GetPtr(smscid.c_str());
    SmscConnector* p = 0;
    if ( ptr ) {
        p = *ptr;
        smsc_log_info(log_,"updating config for smsc %s", smscid.c_str());
        p->updateConfig(cfg);
    } else {
        smsc_log_info(log_,"creating a new smsc connector %s", smscid.c_str());
        p = new SmscConnector(processor_,cfg,smscid,performanceTest);
        connectors_.Insert( smscid.c_str(), p );
        if (started_) { p->start(); }
    }
    return p;
}


void InfoSmeMessageSender::addRegionMapping( const std::string& regionId, const std::string& smscIdinput )
{
    if (regions_.Exists(regionId.c_str())) {
        throw ConfigException("Region already exists: '%s'", regionId.c_str());
    }

    const std::string* smscId = &smscIdinput;
    if ( smscId->empty() ) {
        smsc_log_info( log_, "Default SMSC Connector '%s' for region '%s' will be used.",
                       defaultConnector_->getSmscId().c_str(), regionId.c_str());
        smscId = &defaultConnector_->getSmscId();
    }
    if (!connectors_.Exists(smscId->c_str())) {
        smsc_log_info(log_, "SMSC Connector '%s' for region '%s' unknown. Default SMSC Connector '%s' will be used.",
                      smscId->c_str(), regionId.c_str(), defaultConnector_->getSmscId().c_str());
        smscId = &defaultConnector_->getSmscId();
    }

    smsc_log_info(log_, "SMSC Connector '%s' for region '%s' will be used.", smscId->c_str(), regionId.c_str());
    regions_.Insert(regionId.c_str(), *smscId);
}

} // namespace infosme
} // namespace smsc
