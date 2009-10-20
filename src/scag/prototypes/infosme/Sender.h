#ifndef _SCAG_PROTOTYPES_INFOSME_SENDER_H
#define _SCAG_PROTOTYPES_INFOSME_SENDER_H

#include <vector>
#include "logger/Logger.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Connector;
class Message;

/// sender class
class Sender
{
private:
    typedef std::vector< Connector* > ConnectorList;

public:
    Sender();
    ~Sender();
    void addConnector( Connector* conn );
    unsigned connectorCount();
    int send( unsigned deltaTime, Message& msg );
    void dumpStatistics( std::string& s );

    /// return a number ms to sleep until a connector becomes ready
    /// if 0, then regionId will contain the regionId of ready connector.
    unsigned hasReadyConnector( unsigned deltaTime, unsigned& regionId );
    void suspendConnector( unsigned deltaTime, unsigned regionId );

private:
    ConnectorList::iterator findConnector( unsigned regionId );
    void resort( ConnectorList::iterator iter );

private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::Mutex lock_;
    ConnectorList connectors_;
    size_t        default_;
};

}
}
}

#endif
