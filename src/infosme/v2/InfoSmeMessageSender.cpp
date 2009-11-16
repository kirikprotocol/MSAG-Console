#include "InfoSmeMessageSender.h"
#include "SmscConnector.h"
#include "RegionSender.h"
#include "ConfString.h"
#include "util/config/region/RegionFinder.hpp"
#include "system/status.h"

namespace smsc {
namespace infosme {

InfoSmeMessageSender::InfoSmeMessageSender( TaskDispatcher& disp ) :
log_(smsc::logger::Logger::getInstance("msgsend")),
dispatcher_(&disp),
defaultConnector_(0),
started_(false),
scoredList_(*this,10000,log_)
{}


InfoSmeMessageSender::~InfoSmeMessageSender()
{
    MutexGuard mg(lock_);
    scoredList_.clear();
    defaultConnector_ = 0;
    {
        int key;
        RegionSender* sender;
        for ( SenderIterator i = senders_.First(); i.Next(key,sender); ) {
            if (sender) {
                dispatcher_->delRegion(sender->getId());
                delete sender;
            }
        }
        senders_.Empty();
    }
    {
        char *key = 0;
        SmscConnector* connector;
        ConnectorIterator it = connectors_.getIterator();
        while(it.Next(key, connector)) {
            if (connector) {
                delete connector;
            }
        }
    }
    regionsConfig_.reset(0);
    regionFinder_.reset(0);
    smsc_log_debug(log_,"dtor");
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
            connector->Start();
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
    MutexGuard mg(lock_);
    smsc::sms::Address parsedAddr(dst.c_str());
    const smsc::util::config::region::Region* foundRegion = smsc::util::config::region::RegionFinder::getInstance().findRegionByAddress(parsedAddr.toString());
    if ( !foundRegion ) {
        smsc_log_warn(log_, "sendSms: can't find region for abonent '%s'", dst.c_str());
        return smsc::system::Status::NOROUTE;
    }
    smsc_log_debug(log_, "sendSms: telephone number '%s' matches to mask for region with id '%s'", dst.c_str(), foundRegion->getId().c_str());
    int regId;
    try {
        regId = Message::stringToRegionId(foundRegion->getId());
    } catch (...) {
        smsc_log_warn(log_, "region id is wrong '%s'", foundRegion->getId().c_str());
        return smsc::system::Status::NOROUTE;
    }
    RegionSender** ptr = senders_.GetPtr(regId);
    if ( !ptr ) {
        smsc_log_warn(log_, "region id=%d is not found", regId);
        return smsc::system::Status::NOROUTE;
    }
    return (*ptr)->getSmscConnector().sendSms(org,dst,txt,flash);
}


/*
SmscConnector* InfoSmeMessageSender::getSmscConnector( const uint32_t regionId )
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
 */


void InfoSmeMessageSender::init( TaskProcessor& processor,
                                 ConfigView* config )
{
    if ( ! config ) {
        throw smsc::util::config::ConfigException("config is null");
    }

    ConfString routeFile(config->getString("route_config_filename"));
    ConfString regionFile(config->getString("regions_config_filename"));
    std::auto_ptr< smsc::util::config::ConfigView > ccv(config->getSubConfig("SMSCConnectors"));
    ConfString defConn(ccv->getString("default","default SMSC id not found"));
    std::auto_ptr< smsc::util::config::CStrSet > connNames(ccv->getShortSectionNames());
    if ( connNames->find(defConn.str()) == connNames->end() ) {
        throw smsc::util::config::ConfigException("default SMSC does not match any section");
    }

    smsc_log_info(log_,"init");
    MutexGuard mg(lock_);

    // --- full cleanup
    {
        smsc_log_debug(log_,"full cleanup");
        defaultConnector_ = 0;
        if ( !regionFinder_.get() ) regionFinder_.reset(new smsc::util::config::region::RegionFinder());
        else regionFinder_->unsafeReset();
        scoredList_.clear();
        regionsConfig_.reset( new smsc::util::config::region::RegionsConfig(regionFile.str().c_str()) );
        {
            int key;
            RegionSender* sender;
            for ( SenderIterator i = senders_.First(); i.Next(key,sender); ) {
                if (sender) {
                    dispatcher_->delRegion(sender->getId());
                    delete sender;
                }
            }
            senders_.Empty();
        }

        // remove connectors
        std::vector< std::string > badConnNames;
        std::vector< SmscConnector* > badConns;
        char* key = 0;
        SmscConnector* connector;
        for ( ConnectorIterator i = connectors_.getIterator(); i.Next(key,connector); ) {
            if ( connNames->find(key) == connNames->end() ) {
                // not found
                badConnNames.push_back(key);
                if ( connector ) {
                    // connector->stop();
                    badConns.push_back(connector);
                }
            }
        }
        for ( std::vector< std::string >::const_iterator i = badConnNames.begin();
              i != badConnNames.end();
              ++i ) {
            connectors_.Delete(i->c_str());
        }
        for ( std::vector< SmscConnector* >::const_iterator i = badConns.begin();
              i != badConns.end();
              ++i ) {
            smsc_log_info(log_,"destroying SMSC connector '%s'",(*i)->getSmscId().c_str() );
            delete *i;
        }

    } // cleanup


    // --- connectors
    for ( smsc::util::config::CStrSet::iterator i = connNames->begin();
          i != connNames->end(); ++i ) {
        std::auto_ptr< smsc::util::config::ConfigView > sect(ccv->getSubConfig(i->c_str()));
        smsc::sme::SmeConfig cfg( SmscConnector::readSmeConfig(*sect.get()) );
        // adding a connector
        SmscConnector** ptr = connectors_.GetPtr(i->c_str());
        SmscConnector* p = 0;
        if ( ptr ) {
            p = *ptr;
        } else {
            smsc_log_info(log_,"creating a new smsc connector '%s'",i->c_str());
            p = new SmscConnector(processor,*i);
            connectors_.Insert( i->c_str(), p );
        }
        smsc_log_info(log_,"updating config for smsc '%s'",i->c_str());
        p->updateConfig(cfg);
        if ( !ptr ) {
            // need start
            p->Start(); 
        }
        if ( *i == defConn.str() ) {
            defaultConnector_ = p;
        }
    }
    if ( defaultConnector_ == 0 ) {
        throw smsc::util::config::ConfigException("default SMSC is not set");
    }


    // --- regions
    smsc::util::config::region::RegionsConfig::status st = regionsConfig_->load();
    if ( st != smsc::util::config::region::RegionsConfig::success ) {
        throw smsc::util::config::ConfigException("regionFile exc: %s", regionFile.str().c_str() );
    }
    smsc_log_info(log_,"file %s has been loaded", regionFile.str().c_str() );

    smsc::util::config::route::RouteConfig routeConfig;
    if ( routeConfig.load(routeFile.str().c_str()) != smsc::util::config::route::RouteConfig::success ) {
        throw smsc::util::config::ConfigException("routeFile exc: %s", routeFile.str().c_str() );
    }
    smsc_log_info(log_,"file %s has been loaded", routeFile.str().c_str() );

    smsc::util::config::region::Region* region;
    for ( smsc::util::config::region::RegionsConfig::RegionsIterator iter = regionsConfig_->getIterator();
          iter.fetchNext(region) == smsc::util::config::region::RegionsConfig::success;
          ) {

        try {
            region->expandSubjectRefs( routeConfig );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"region %s/'%s' exc: %s",
                           region->getId().c_str(),
                           region->getName().c_str(),e.what());
            throw smsc::util::config::ConfigException("region %s/'%s' exc: %s",
                                                      region->getId().c_str(),
                                                      region->getName().c_str(),e.what());
        }

        std::string addressMask;
        for ( smsc::util::config::region::Region::MasksIterator maskIter = region->getMasksIterator();
              maskIter.fetchNext(addressMask); ) {
            smsc_log_debug(log_,"region %s/'%s' registering mask '%s'",
                           region->getId().c_str(),
                           region->getName().c_str(),
                           addressMask.c_str());
            regionFinder_->registerAddressMask(addressMask,region);
        }
        smsc_log_debug(log_,"region %s/'%s' maps to SMSC '%s'",
                       region->getId().c_str(),
                       region->getName().c_str(),
                       region->getInfosmeSmscId().c_str());
        createRegionSender( region );

    }

    const smsc::util::config::region::RegionDefault& regdef = regionsConfig_->getDefaultRegion();
    smsc_log_debug(log_,"registering a default region %s/'%s'",
                   regdef.getId().c_str(), regdef.getName().c_str() );
    regionFinder_->registerDefaultRegion(&regdef);
    createRegionSender( &regdef );

    smsc_log_info(log_,"init finished");
}

/*
void InfoSmeMessageSender::reloadSmscAndRegions( Manager& manager )
{
    const std::string csn("InfoSme.SMSCConnectors");
    ConfigView ccv(manager,csn.c_str());

    // get the default connector id
    const std::string defId = ccv.getString("default", "default SMSC Connector id not set");

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
        smsc::sme::SmeConfig cfg = SmscConnector::readSmeConfig(ConfigView(manager,sectName.c_str()));
        SmscConnector* conn = addConnector(cfg,*i);
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
    const char* route_xml_file = tpConfig.getString("route_config_filename");
    const char* regions_xml_file = tpConfig.getString("regions_config_filename");
    regionsConfig_.reset(new smsc::util::config::region::RegionsConfig(regions_xml_file));
    smsc::util::config::region::RegionsConfig::status st = regionsConfig_->load();
    if ( st == smsc::util::config::region::RegionsConfig::success )
        smsc_log_info(log_, "config file %s has been loaded successful", regions_xml_file);
    else
        throw smsc::util::config::ConfigException("can't load config file %s", regions_xml_file);

    smsc::util::config::route::RouteConfig routeConfig;
    if ( routeConfig.load(route_xml_file) == smsc::util::config::route::RouteConfig::success )
        smsc_log_info(log_, "config file %s has been loaded successful", route_xml_file);
    else
        throw smsc::util::config::ConfigException("can't load config file %s", route_xml_file);

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
 */


/*
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
 */


unsigned InfoSmeMessageSender::send( unsigned curTime, unsigned sleepTime )
{
    unsigned ret;
    {
        MutexGuard mg(lock_);
        ret = scoredList_.processOnce(curTime,sleepTime);
    }
    smsc_log_debug(log_,"send() result is %u msec",ret);
    return ret;
}


void InfoSmeMessageSender::processWaitingEvents( time_t tm )
{
    SmscConnector* connector;
    char *key = 0;
    for ( ConnectorIterator it = connectors_.getIterator(); it.Next(key,connector); ) {
        connector->processWaitingEvents(tm);
    }
}


int InfoSmeMessageSender::findRegionByAddress( const char* addr )
{
    MutexGuard mg(lock_);
    const smsc::util::config::region::Region* foundRegion =
        regionFinder_->findRegionByAddress( addr );
    if ( ! foundRegion ) return -2;
    return Message::stringToRegionId(foundRegion->getId());
}


/*
SmscConnector* InfoSmeMessageSender::addConnector( const smsc::sme::SmeConfig& cfg, const std::string& smscid )
{
    SmscConnector** ptr = connectors_.GetPtr(smscid.c_str());
    SmscConnector* p = 0;
    if ( ptr ) {
        p = *ptr;
        smsc_log_info(log_,"updating config for smsc %s", smscid.c_str());
        p->updateConfig(cfg);
    } else {
        smsc_log_info(log_,"creating a new smsc connector %s", smscid.c_str());
        p = new SmscConnector(processor_,cfg,smscid);
        connectors_.Insert( smscid.c_str(), p );
        if (started_) { p->Start(); }
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
 */

void InfoSmeMessageSender::createRegionSender( const smsc::util::config::region::Region* region )
{
    if ( ! region ) {
        throw ConfigException("null region");
    }
    int regId;
    try {
        regId = Message::stringToRegionId(region->getId());
    } catch ( std::exception& e ) {
        throw ConfigException("%s",e.what());
    }

    const char* smscId = region->getInfosmeSmscId().c_str();

    if ( senders_.Exist(regId) ) {
        throw ConfigException("region %d already exists", regId);
    }

    SmscConnector** ptr = connectors_.GetPtr(smscId);
    SmscConnector* conn = ptr ? *ptr : defaultConnector_;
    if ( !conn ) {
        throw ConfigException("connector is null for region %d",regId);
    }
    RegionSender* sender = new RegionSender(regId,*region,*conn);
    dispatcher_->addRegion(regId);
    senders_.Insert(regId,sender);
    scoredList_.add( sender );
}


unsigned InfoSmeMessageSender::scoredObjIsReady( unsigned deltaTime, ScoredObjType& c )
{
    const unsigned ret = c.isReady( deltaTime );
    smsc_log_debug(log_,"region %d wants to sleep %u msec",c.getId(),ret);
    return ret;
}


int InfoSmeMessageSender::processScoredObj( unsigned deltaTime, ScoredObjType& c )
{
    unsigned inc = 1000/c.getBandwidth();
    try {
        unsigned wantToSleep = dispatcher_->processRegion( deltaTime, c );
        smsc_log_debug(log_,"region %d processed, sleep=%u",c.getId(),wantToSleep);
        if ( wantToSleep > 0 ) {
            // all tasks want to sleep
            c.suspend(deltaTime + wantToSleep);
            return -inc;
        }
    } catch ( std::exception& exc ) {
        smsc_log_debug(log_,"region %d sending failed: %s",c.getId(),exc.what());
        c.suspend(deltaTime + dispatcher_->sleepTime());
        return -1000;
    }
    return inc;
}

} // namespace infosme
} // namespace smsc
