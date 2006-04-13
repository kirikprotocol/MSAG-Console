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

void ScagTaskManager::looseQueueLimit() {
    waitQueueShrinkage = false;
    queMon.Unlock();
}

HttpAcceptor::HttpAcceptor(HttpManager& m) : manager(m)
{
}

int HttpAcceptor::Execute()
{
    Socket *user_socket;

    http_log_debug("HttpAcceptor started");

    while (!isStopping) 
    {
	manager.scags.fitQueueLimit();
    
        user_socket = masterSocket.Accept();

        if (!user_socket) {
            http_log_error("Failed to accept, error: %s", strerror(errno));
            break;
        }

        HttpContext *cx = new HttpContext(user_socket);
        http_log_info("Accepted: context %p, socket %p", cx, user_socket);
        manager.readers.process(cx);
    }

    http_log_debug("HttpAcceptor quit");

    return isStopping == false;
}

const char* HttpAcceptor::taskName()
{
    return "HttpAcceptor";
}

void HttpAcceptor::shutdown()
{
    isStopping = true;
    manager.scags.looseQueueLimit();
    WaitFor();
}

void HttpAcceptor::init(const char *host, int port)
{
    isStopping = false;

    try {
        if (masterSocket.InitServer(host, port, 0, 0) == -1) {          
            http_log_error("Failed to init HttpAcceptor master socket");
            throw Exception("Socket::InitServer() failed");
        }
        if (masterSocket.StartServer() == -1) {
            http_log_error("Failed to start HttpAcceptor master socket");
            throw Exception("Socket::StartServer() failed");
        }
    }
    catch(...) {
        throw;
    }

    //masterSocket.setNonBlocking(1);

    Start();
}

}}}
