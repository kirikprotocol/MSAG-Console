#include "MessageSender.h"
#include "RegionSender.h"
#include "TaskDispatcher.h"
#include "ConfString.h"
#include "SmscConnector.h"
#include "Message.h"

#include "util/config/ConfigView.h"
#include "util/config/region/RegionsConfig.hpp"
#include "util/config/region/RegionFinder.hpp"

namespace smsc {
namespace infosme2 {

MessageSender::MessageSender( TaskDispatcher& disp ) :
log_(smsc::logger::Logger::getInstance("is2.msend")),
dispatcher_(&disp),
scoredList_(*this,10000,log_),
defaultConn_(0)
{
    smsc_log_debug(log_,"ctor");
}


MessageSender::~MessageSender()
{
    MutexGuard mg(lock_);
    scoredList_.clear();
    defaultConn_ = 0;
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
        char* key = 0;
        SmscConnector* connector;
        for ( ConnectorIterator i = connectors_.getIterator(); i.Next(key,connector); ) {
            if (connector) delete connector;
        }
    }
    regionConfig_.reset(0);
    regionFinder_.reset(0);
    smsc_log_debug(log_,"dtor");
}


void MessageSender::init( smsc::util::config::ConfigView* config )
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
        defaultConn_ = 0;
        if ( !regionFinder_.get() ) regionFinder_.reset(new smsc::util::config::region::RegionFinder());
        else regionFinder_->unsafeReset();
        scoredList_.clear();
        regionConfig_.reset( new smsc::util::config::region::RegionsConfig(regionFile.str().c_str()) );
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
            smsc_log_info(log_,"destroying SMSC connector '%s'",(*i)->getId().c_str() );
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
            smsc_log_info(log_,"updating config for smsc '%s'",i->c_str());
            p->updateConfig(cfg);
        } else {
            smsc_log_info(log_,"creating a new smsc connector '%s'",i->c_str());
            p = new SmscConnector(*i,cfg);
            connectors_.Insert( i->c_str(), p );
        }
        if ( *i == defConn.str() ) {
            defaultConn_ = p;
        }
    }
    if ( defaultConn_ == 0 ) {
        throw smsc::util::config::ConfigException("default SMSC is not set");
    }


    // --- regions
    smsc::util::config::region::RegionsConfig::status st = regionConfig_->load();
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
    for ( smsc::util::config::region::RegionsConfig::RegionsIterator iter = regionConfig_->getIterator();
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

    const smsc::util::config::region::RegionDefault& regdef = regionConfig_->getDefaultRegion();
    smsc_log_debug(log_,"registering a default region %s/'%s'",
                   regdef.getId().c_str(), regdef.getName().c_str() );
    regionFinder_->registerDefaultRegion(&regdef);
    createRegionSender( &regdef );

    smsc_log_info(log_,"init finished");
}


unsigned MessageSender::send( unsigned curTime, unsigned sleepTime )
{
    MutexGuard mg(lock_);
    return scoredList_.processOnce(curTime,sleepTime);
}


unsigned MessageSender::scoredObjIsReady( unsigned deltaTime, ScoredObjType& c )
{
    return c.isReady( deltaTime );
}


int MessageSender::processScoredObj( unsigned deltaTime, ScoredObjType& c )
{
    unsigned inc = 1000/c.getBandwidth();
    try {
        unsigned wantToSleep = dispatcher_->processRegion( deltaTime, c );
        smsc_log_debug(log_,"region %d processed, sleep=%u",c.getId(),wantToSleep);
        if ( wantToSleep > 0 ) {
            // all tasks want to sleep
            c.suspend(deltaTime+wantToSleep);
            return -inc;
        }
    } catch ( std::exception& exc ) {
        smsc_log_debug(log_,"region %d sending failed: %s",c.getId(),exc.what());
        c.suspend(deltaTime+1000);
        return -1000;
    }
    return inc;
}


void MessageSender::createRegionSender( const smsc::util::config::region::Region* region )
{
    if ( ! region ) {
        throw smsc::util::config::ConfigException("null region");
    }
    int regId;
    try {
        regId = Message::stringToRegionId(region->getId());
    } catch ( std::exception& e ) {
        throw smsc::util::config::ConfigException("%s",e.what());
    }

    const char* smscId = region->getInfosmeSmscId().c_str();

    if ( senders_.Exist(regId) ) {
        throw smsc::util::config::ConfigException("region %d already exists", regId);
    }

    SmscConnector** ptr = connectors_.GetPtr(smscId);
    SmscConnector* conn = ptr ? *ptr : defaultConn_;
    if ( !conn ) {
        throw smsc::util::config::ConfigException("connector is null for region %d",regId);
    }
    RegionSender* sender = new RegionSender(regId,*region,*conn);
    dispatcher_->addRegion(regId);
    senders_.Insert(regId,sender);
    scoredList_.add( sender );
}


}
}
