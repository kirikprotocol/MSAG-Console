#ifndef _EYELINE_SMPP_SMPPCONNECTOR_H
#define _EYELINE_SMPP_SMPPCONNECTOR_H

#include <vector>
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/network/Pipe.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "ClientSocket.h"

namespace eyeline {
namespace smpp {

class SocketCloser;
class SocketBinder;

/// smpp server
class SmppConnector : public smsc::core::threads::Thread
{
public:
    SmppConnector( SocketCloser& closer, SocketBinder& binder );
    virtual ~SmppConnector();

    void start();
    void stop();

    void addSocket( ClientSocket& socket );

protected:
    virtual int Execute();

private:
    typedef std::vector< SocketPtr > SocketList;
    typedef smsc::core::buffers::IntHash< SocketPtr > SocketHash;

    smsc::logger::Logger* log_;
    SocketCloser&  socketCloser_;
    SocketBinder&  socketBinder_;

    smsc::core::synchronization::EventMonitor mon_;
    SocketList pendingSockets_;
    SocketHash sockets_;
    bool stopping_;
    smsc::core::network::Pipe wakePipe_;
    smsc::core::network::Multiplexer mul_;
};

}
}

#endif
