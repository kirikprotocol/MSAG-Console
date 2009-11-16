#ifndef INFOSME_INFOSMEMESSAGESENDER_H
#define INFOSME_INFOSMEMESSAGESENDER_H

#include <string>
#include "MessageSender.h"
#include "sme/SmppBase.hpp"
#include "util/config/region/RegionsConfig.hpp"
#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"
#include "TaskProcessor.h"
#include "ScoredList.h"

namespace smsc {
namespace util {
namespace config {
namespace region {
class RegionsConfig;
class RegionFinder;
class Region;
}
}
}
}

namespace smsc {
namespace infosme {

class SmscConnector;
class TaskProcessor;
class TaskDispatcher;
class RegionSender;

class InfoSmeMessageSender : public MessageSender
{
    friend class ScoredList< InfoSmeMessageSender >;

public:
    InfoSmeMessageSender( TaskDispatcher& processor );
    virtual void init( TaskProcessor& proc, smsc::util::config::ConfigView* config );

    virtual ~InfoSmeMessageSender();
    void start();
    void stop();

    virtual uint32_t sendSms(const std::string& org,const std::string& dst,const std::string& txt,bool flash);
    /*
    {
        smsc_log_info(logger, "sendSms do default region!");
        return defaultConnector_->sendSms(org,dst,txt,flash);
    }
     */

    // virtual void reloadSmscAndRegions( Manager& mgr );
    // virtual bool send( Task* task, Message& message );

    virtual unsigned send( unsigned deltaTime, unsigned sleepTime );
    virtual void processWaitingEvents( time_t tm );
    virtual int findRegionByAddress( const char* addr );

private:

    void createRegionSender( const smsc::util::config::region::Region* region );

    // virtual SmscConnector* getSmscConnector(const std::string& regionId);
    // SmscConnector* addConnector( const smsc::sme::SmeConfig& cfg, const std::string& smscid );
    // void addRegionMapping( const std::string& regionId, const std::string& smscId );

    typedef RegionSender ScoredObjType;
    typedef Hash<SmscConnector*>::Iterator ConnectorIterator;
    typedef smsc::core::buffers::IntHash< RegionSender* >::Iterator SenderIterator;

    void scoredObjToString( std::string& s, ScoredObjType& c );
    unsigned scoredObjIsReady( unsigned curTime, ScoredObjType& c );
    int processScoredObj( unsigned curTime, ScoredObjType& c );

private:
    smsc::logger::Logger* log_;
    TaskDispatcher*       dispatcher_;
    smsc::core::synchronization::Mutex lock_;
    ScoredList< InfoSmeMessageSender > scoredList_;

    std::auto_ptr< smsc::util::config::region::RegionsConfig > regionsConfig_;
    std::auto_ptr< smsc::util::config::region::RegionFinder >  regionFinder_;

    smsc::core::buffers::IntHash< RegionSender* > senders_;     // owned
    SmscConnector*        defaultConnector_;
    Hash<SmscConnector*>  connectors_;  // owned, all connectors

    bool                  started_;
};

} // namespace infosme
} // namespace smsc

#endif /* !INFOSME_INFOSMEMESSAGESENDER_H */
