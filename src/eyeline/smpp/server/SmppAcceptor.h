#ifndef _EYELINE_SMPP_SMPPACCEPTOR_H
#define _EYELINE_SMPP_SMPPACCEPTOR_H

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Pipe.hpp"

namespace eyeline {
namespace smpp {

class SocketBinder;

/// smpp server
class SmppAcceptor : protected smsc::core::threads::Thread
{
public:
    SmppAcceptor( SocketBinder& binder,
                  const char* host, int port,
                  size_t maxrsize, size_t maxwqueue );
    void start();
    void stop();

protected:
    virtual int Execute();

private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::EventMonitor mon_;
    SocketBinder& socketBinder_;
    smsc::core::network::Socket socket_;
    smsc::core::network::Pipe   wakePipe_;
    bool stopping_;
    size_t maxrsize_, maxwqueue_;
};

}
}

#endif
