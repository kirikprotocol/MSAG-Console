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
            smsc_log_error(logger, "%s failed to accept, error: %s", taskName(), strerror(errno));
            break;
        }
        HttpContext *cx = new HttpContext(user_socket, httpsOptions);
        smsc_log_info(logger, "%s accepted: context %p, socket %p", taskName(), cx, user_socket);
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

    smsc_log_debug(logger, "%s quit", taskName());

    return isStopping == false;
}

const char* HttpAcceptor::taskName()
{
    return (httpsOptions == NULL) ? "HttpAcceptor" : "HttpsAcceptor";
}

void HttpAcceptor::shutdown()
{
    isStopping = true;

    masterSocket.Close();
    manager.scags.looseQueueLimit();
    WaitFor();
}

void HttpAcceptor::init(const char *host, int port, HttpsOptions& options)
{
    isStopping = false;
    httpsOptions = new HttpsOptions(options); //separate copy for each acceptor

    logger = Logger::getInstance((httpsOptions == NULL) ? "http.acceptor" : "https.acceptor");

    try {
        if (masterSocket.InitServer(host, port, 0, 0) == -1) {          
            smsc_log_error(logger, "%s failed to init master socket", taskName());
            throw Exception("Socket::InitServer() failed");
        }
        smsc_log_info(logger, "%s masterSocket init: host %s, port %d", taskName(), host, port);
        if (masterSocket.StartServer() == -1) {
            smsc_log_error(logger, "%s failed to start master socket", taskName());
            throw Exception("Socket::StartServer() failed");
        }
        smsc_log_info(logger, "%s masterSocket StartServer Ok", taskName());
    }
    catch(...) {
        throw;
    }

    Start();
}

}}}
