#include <errno.h>
#include "util/Exception.hpp"
#include "HttpAcceptor.h"
#include "scag/transport/http/base/HttpContext.h"
#include "Managers.h"

namespace scag2 { namespace transport { namespace http
{
using smsc::util::Exception;

HttpAcceptor::HttpAcceptor(HttpManagerImpl& m) : manager(m)
{
}

int HttpAcceptor::Execute()
{
    Socket *user_socket;

    smsc_log_debug(logger, "started");

    for (;;) {
        manager.scags.fitQueueLimit();
    
        user_socket = masterSocket.Accept();
        
        if (isStopping)
            break;

        if (!user_socket) {
            smsc_log_error(logger, "failed to accept, error: %s", strerror(errno));
            break;
        }
        HttpContext *cx = new HttpContext(user_socket);
        smsc_log_info(logger, "accepted: context %p, socket %p", cx, user_socket);
        if (manager.isLicenseExpired() || manager.licenseThroughputLimitExceed()) {
          cx->action = SEND_RESPONSE;
          cx->createFakeResponse(503);
          manager.writers.process(cx);
          continue;
        }
        manager.readers.process(cx);
        manager.incLicenseCounter();
    }

    if (user_socket)
        delete user_socket;

    smsc_log_debug(logger, "quit");

    return isStopping == false;
}

const char* HttpAcceptor::taskName()
{
    return "HttpAcceptor";
}

void HttpAcceptor::shutdown()
{
    isStopping = true;

    masterSocket.Close();
    manager.scags.looseQueueLimit();
    WaitFor();
}

void HttpAcceptor::init(const char *host, int port)
{
    isStopping = false;

    logger = Logger::getInstance("scag.http.acceptor");

    try {
        if (masterSocket.InitServer(host, port, 0, 0) == -1) {          
            smsc_log_error(logger, "failed to init master socket");
            throw Exception("Socket::InitServer() failed");
        }
        if (masterSocket.StartServer() == -1) {
            smsc_log_error(logger, "failed to start master socket");
            throw Exception("Socket::StartServer() failed");
        }
    }
    catch(...) {
        throw;
    }

    Start();
}

}}}
