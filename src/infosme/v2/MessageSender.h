#ifndef SMSC_INFOSME2_MESSAGESENDER_H
#define SMSC_INFOSME2_MESSAGESENDER_H

#include <string>

#include "logger/Logger.h"
#include "ScoredList.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/Hash.hpp"

namespace smsc {

namespace util {
namespace config {
class ConfigView;

namespace region {
class RegionsConfig;
class RegionFinder;
class Region;
}
}
}

namespace infosme2 {

class RegionSender;
class TaskDispatcher;
class SmscConnector;

class MessageSender
{
private:
    // typedef Hash<SmscConnector*>::Iterator ConnectorIterator;
    friend class ScoredList< MessageSender >;

public:

    MessageSender( TaskDispatcher& disp );
    ~MessageSender();

    void init( smsc::util::config::ConfigView* config );

    // unsigned send( unsigned curTime, unsigned sleepTime );

    // void start();
    // void stop();

    // virtual uint32_t sendSms(const std::string& org,const std::string& dst,const std::string& txt,bool flash);
    /*
    {
        smsc_log_info(logger, "sendSms do default region!");
        return defaultConnector_->sendSms(org,dst,txt,flash);
    }
     */

    // virtual void reloadSmscAndRegions( Manager& mgr );
    // virtual bool send( Task* task, Message& message );
    // virtual void processWaitingEvents(time_t tm);

    unsigned send( unsigned curTime, unsigned sleepTime );

private:

    void createRegionSender( const smsc::util::config::region::Region* region );

    // virtual SmscConnector* getSmscConnector(const std::string& regionId);
    // SmscConnector* addConnector( const smsc::sme::SmeConfig& cfg, const std::string& smscid );
    // void addRegionMapping( const std::string& regionId, const std::string& smscId );

    typedef RegionSender ScoredObjType;
    typedef smsc::core::buffers::Hash< SmscConnector* >::Iterator ConnectorIterator;
    typedef smsc::core::buffers::IntHash< RegionSender* >::Iterator SenderIterator;

    void scoredObjToString( std::string& s, ScoredObjType& c );
    unsigned scoredObjIsReady( unsigned curTime, ScoredObjType& c );
    int processScoredObj( unsigned curTime, ScoredObjType& c );

private:
    smsc::logger::Logger*       log_;
    TaskDispatcher*             dispatcher_;
    smsc::core::synchronization::Mutex lock_;
    ScoredList< MessageSender > scoredList_;

    // NOTE: we don't use singleton approach for region finder
    std::auto_ptr< smsc::util::config::region::RegionsConfig > regionConfig_;
    std::auto_ptr< smsc::util::config::region::RegionFinder >  regionFinder_;

    smsc::core::buffers::IntHash< RegionSender* >    senders_;     // owned
    SmscConnector*                                   defaultConn_;
    smsc::core::buffers::Hash< SmscConnector* >      connectors_;  // owned
};

} // namespace infosme
} // namespace smsc

#endif /* !INFOSME_INFOSMEMESSAGESENDER_H */
