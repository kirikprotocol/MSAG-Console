#include <errno.h>
#include "util/Exception.hpp"
#include "HttpLogger.h"
#include "HttpAcceptor.h"
#include "HttpContext.h"
#include "Managers.h"

namespace scag { namespace transport { namespace http
{
using smsc::util::Exception;

void ScagTaskManager::fitQueueLimit() {
    while (waitQueueShrinkage) {
        http_log_warn("ScagTaskManager queue overlimited");
        queMon.wait();
    }
}

HttpAcceptor::HttpAcceptor(HttpManager& m) : manager(m)
{
}

int HttpAcceptor::Execute()
{
    Socket *user_socket;
    Socket master_socket;

    try {
        if (master_socket.InitServer(host, port, 0, 0) == -1) {          
            http_log_error("Failed to init HttpAcceptor master socket");
            throw Exception("Socket::InitServer() failed");
        }
        if (master_socket.StartServer() == -1) {
            http_log_error("Failed to start HttpAcceptor master socket");
            throw Exception("Socket::StartServer() failed");
        }
    }
    catch(...) {
        throw;
    }

    //master_socket.setNonBlocking(1);
    //smsc_log_debug(httpLogger, "HttpAcceptor started (smsc) %d", httpLogger->isDebugEnabled());
    http_log_debug("HttpAcceptor started");

    while (!isStopping) {
    manager.scags.fitQueueLimit();
    
        user_socket = master_socket.Accept();

        if (!user_socket) {
            http_log_error("Failed to accept, error: %s", strerror(errno));
            break;
        }

        HttpContext *cx = new HttpContext(user_socket);
        http_log_info("Accepted: context %p, socket %p", cx, user_socket);
        manager.readers.process(cx);
    }

    master_socket.Close();

    if (!isStopping)
        throw Exception("Accept() failed");

    http_log_debug("HttpAcceptor quit");

    return 0;
}

const char* HttpAcceptor::taskName()
{
    return "HttpAcceptor";
}

void HttpAcceptor::shutdown()
{
    isStopping = true;

    accMon.Lock();
    accMon.notify();
    accMon.Unlock();
}

void HttpAcceptor::init(const char *_host, int _port)
{
    HttpAcceptor::host = _host;
    HttpAcceptor::port = _port;
    isStopping = false;

    Start();
}

}}}
