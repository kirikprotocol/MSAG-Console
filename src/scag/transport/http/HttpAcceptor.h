#ifndef SCAG_TRANSPORT_HTTP_ACCEPTOR
#define SCAG_TRANSPORT_HTTP_ACCEPTOR

#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"

namespace scag { namespace transport { namespace http
{
using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

class HttpManager;

class HttpAcceptor : public Thread {
public:
    HttpAcceptor(HttpManager& m);

    virtual int Execute();
    virtual const char* taskName();

    void shutdown();
    void init(const char *host, int port);

protected:
    HttpManager &manager;    
    Socket masterSocket;
    Logger *logger;

private:
    bool isStopping;  
};

}}}

#endif // SCAG_TRANSPORT_HTTP_ACCEPTOR
