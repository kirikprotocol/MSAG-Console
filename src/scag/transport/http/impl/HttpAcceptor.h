#ifndef SCAG_TRANSPORT_HTTP_IMPL_ACCEPTOR
#define SCAG_TRANSPORT_HTTP_IMPL_ACCEPTOR

#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace transport {
namespace http {

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

class HttpManagerImpl;
class HttpsOptions;

class HttpAcceptor : public Thread 
{
public:
    HttpAcceptor(HttpManagerImpl& m);

    virtual int Execute();
    virtual const char* taskName();

    void shutdown();
    void init(const char *host, int port, HttpsOptions* options=NULL);

protected:
    HttpManagerImpl &manager;    
    Socket masterSocket;
    Logger *logger;

private:
    bool isStopping;
    HttpsOptions* httpsOptions;
};

}}}

#endif // SCAG_TRANSPORT_HTTP_ACCEPTOR
