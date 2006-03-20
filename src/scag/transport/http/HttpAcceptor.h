#ifndef SCAG_TRANSPORT_HTTP_ACCEPTOR
#define SCAG_TRANSPORT_HTTP_ACCEPTOR

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace scag { namespace transport { namespace http
{
using smsc::core::threads::Thread;
using smsc::core::synchronization::EventMonitor;

class HttpManager;

class HttpAcceptor : public Thread {
public:
    HttpAcceptor(HttpManager& m);

    virtual int Execute();
    virtual const char* taskName();

    void shutdown();
    void init(const char *host, int port);

protected:
    EventMonitor accMon;
    HttpManager &manager;    

private:
    const char *host;
    int port;
    bool isStopping;  
};

}}}

#endif // SCAG_TRANSPORT_HTTP_ACCEPTOR
