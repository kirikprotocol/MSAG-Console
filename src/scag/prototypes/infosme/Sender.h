#ifndef _SCAG_PROTOTYPES_INFOSME_SENDER_H
#define _SCAG_PROTOTYPES_INFOSME_SENDER_H

#include <vector>
#include "logger/Logger.h"
#include "ScoredList.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Connector;
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

    void addConnector( Connector* conn );
    unsigned connectorCount();
    // int send( unsigned deltaTime, Message& msg );

    void dumpStatistics( std::string& s );

    // return a number ms to sleep until a connector becomes ready
    // if 0, then regionId will contain the regionId of ready connector.
    // unsigned hasReadyConnector( unsigned deltaTime, unsigned& regionId );
    // void suspendConnector( unsigned deltaTime, unsigned regionId );

    unsigned send( unsigned deltaTime, unsigned sleepTime );

private:
    // ConnectorList::iterator findConnector( unsigned regionId );
    // void resort( ConnectorList::iterator iter );

    // to satisfy contract for ScoredList
    typedef Connector ScoredObjType;
    void scoredObjToString( std::string& s, Connector& c );
    unsigned scoredObjIsReady( unsigned deltaTime, Connector& c );
    int processScoredObj( unsigned deltaTime, Connector& c );

private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::Mutex lock_;
    ScoredList< Sender > scoredList_;
    Connector*           default_;
    TaskDispatcher*      dispatcher_;
};

}
}
}

#endif
