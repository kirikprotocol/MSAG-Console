#include "InfoSmeMessageSender.h"
#include "SmscConnector.h"
#include "RegionSender.h"
#include "util/config/ConfString.h"
#include "util/config/region/RegionFinder.hpp"
#include "system/status.h"

namespace smsc {
namespace infosme {

using namespace smsc::util::config;

InfoSmeMessageSender::InfoSmeMessageSender( TaskDispatcher& disp ) :
log_(smsc::logger::Logger::getInstance("msgsend")),
dispatcher_(&disp),
#ifdef INTHASH_USAGE_CHECKING
senders_(SMSCFILELINE),
#endif
defaultConnector_(0),
started_(false),
scoredList_(*this,10000,log_)
{
    smsc_log_debug(log_,"ctor");
}


InfoSmeMessageSender::~InfoSmeMessageSender()
{
    smsc_log_debug(log_,"dtor");
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
    smsc_log_debug(log_,"dtor finished");
}


void InfoSmeMessageSender::start()
{
    if ( ! regionsConfig_.get() ) {
        throw ConfigException("regions not loaded, use reloadSmscAndRegions");
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
    MutexGuard mg(lock_);
    smsc::sms::Address parsedAddr(dst.c_str());
    int regId;
    try {
        regId = findRegionByAddress(parsedAddr.toString().c_str());
    } catch (...) {
        smsc_log_warn(log_, "region id is wrong for address '%s'", parsedAddr.toString().c_str());
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
    smsc_log_info(log_,"init");
    if ( ! config ) {
        throw ConfigException("config is null");
    }

    bool perftest = false;
    try {
        perftest = config->getBool("performanceTest");
    } catch (...) {
    }

    ConfString routeFile(config->getString("route_config_filename"));
    ConfString regionFile(config->getString("regions_config_filename"));
    std::auto_ptr< ConfigView > ccv(config->getSubConfig("SMSCConnectors"));
    ConfString defConn(ccv->getString("default","default SMSC id not found"));
    std::auto_ptr< CStrSet > connNames(ccv->getShortSectionNames());
    if ( connNames->find(defConn.str()) == connNames->end() ) {
        throw ConfigException("default SMSC does not match any section");
    }

    MutexGuard mg(lock_);

    // --- full cleanup
    {
        smsc_log_debug(log_,"full cleanup");
        defaultConnector_ = 0;
        if ( !regionFinder_.get() ) regionFinder_.reset(new region::RegionFinder());
        else regionFinder_->unsafeReset();
        scoredList_.clear();
        regionsConfig_.reset( new region::RegionsConfig(regionFile.str().c_str()) );
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
    for ( CStrSet::iterator i = connNames->begin();
          i != connNames->end(); ++i ) {
        std::auto_ptr< ConfigView > sect(ccv->getSubConfig(i->c_str()));
        smsc::sme::SmeConfig cfg( SmscConnector::readSmeConfig(*sect.get()) );
        // adding a connector
        SmscConnector** ptr = connectors_.GetPtr(i->c_str());
        SmscConnector* p = 0;
        if ( ptr ) {
            p = *ptr;
        } else {
            smsc_log_info(log_,"creating a new smsc connector '%s'",i->c_str());
            p = new SmscConnector(processor,*i,perftest);
            connectors_.Insert( i->c_str(), p );
        }
        smsc_log_info(log_,"updating config for smsc '%s'",i->c_str());
        p->updateConfig(cfg);
        if ( !ptr ) {
            // need start
            p->start();
        }
        if ( *i == defConn.str() ) {
            defaultConnector_ = p;
        }
    }
    if ( defaultConnector_ == 0 ) {
        throw ConfigException("default SMSC is not set");
    }


    // --- regions
    region::RegionsConfig::status st = regionsConfig_->load();
    if ( st != region::RegionsConfig::success ) {
        throw ConfigException("regionFile exc: %s", regionFile.str().c_str() );
    }
    smsc_log_info(log_,"file %s has been loaded", regionFile.str().c_str() );

    route::RouteConfig routeConfig;
    if ( routeConfig.load(routeFile.str().c_str()) != route::RouteConfig::success ) {
        throw ConfigException("routeFile exc: %s", routeFile.str().c_str() );
    }
    smsc_log_info(log_,"file %s has been loaded", routeFile.str().c_str() );

    region::Region* region;
    for ( region::RegionsConfig::RegionsIterator iter = regionsConfig_->getIterator();
          iter.fetchNext(region) == region::RegionsConfig::success;
          ) {

        try {
            region->expandSubjectRefs( routeConfig );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"region %s/'%s' exc: %s",
                           region->getId().c_str(),
                           region->getName().c_str(),e.what());
            throw ConfigException("region %s/'%s' exc: %s",
                                                      region->getId().c_str(),
                                                      region->getName().c_str(),e.what());
        }

        std::string addressMask;
        for ( region::Region::MasksIterator maskIter = region->getMasksIterator();
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

    const region::RegionDefault& regdef = regionsConfig_->getDefaultRegion();
    smsc_log_debug(log_,"registering a default region %s/'%s'",
                   regdef.getId().c_str(), regdef.getName().c_str() );
    regionFinder_->registerDefaultRegion(&regdef);
    createRegionSender( &regdef );

    smsc_log_info(log_,"init finished");
}


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
    const region::Region* foundRegion =
        regionFinder_->findRegionByAddress( addr );
    if ( ! foundRegion ) return -2;
    return Message::stringToRegionId(foundRegion->getId());
}


void InfoSmeMessageSender::createRegionSender( const region::Region* region )
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
    SmscConnector* conn;
    if ( ptr ) {
        conn = *ptr;
    } else {
        smsc_log_warn(log_,"smsc connector '%s' is not found for region %d",
                      smscId,regId);
        conn = defaultConnector_;
    }
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
