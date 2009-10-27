#ifndef _SCAG_PROTOTYPES_INFOSME_SENDER_H
#define _SCAG_PROTOTYPES_INFOSME_SENDER_H

#include <vector>
#include "logger/Logger.h"
#include "ScoredList.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Region;
class Message;
class TaskDispatcher;

/// sender class
class Sender
{
private:
    friend class ScoredList< Sender >;
    
public:
    Sender( TaskDispatcher& disp );
    ~Sender();

    void addRegion( Region* conn );
    unsigned connectorCount();
    // int send( unsigned deltaTime, Message& msg );

    void dumpStatistics( std::string& s );

    // return a number ms to sleep until a connector becomes ready
    // if 0, then regionId will contain the regionId of ready connector.
    // unsigned hasReadyRegion( unsigned deltaTime, unsigned& regionId );
    // void suspendRegion( unsigned deltaTime, unsigned regionId );

    unsigned send( unsigned deltaTime, unsigned sleepTime );

private:
    // RegionList::iterator findRegion( unsigned regionId );
    // void resort( RegionList::iterator iter );

    // to satisfy contract for ScoredList
    typedef Region ScoredObjType;
    void scoredObjToString( std::string& s, Region& c );
    unsigned scoredObjIsReady( unsigned deltaTime, Region& c );
    int processScoredObj( unsigned deltaTime, Region& c );

private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::Mutex lock_;
    ScoredList< Sender > scoredList_;
    Region*              default_;
    TaskDispatcher*      dispatcher_;
};

}
}
}

#endif
