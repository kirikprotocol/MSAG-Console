#ifndef _SCAG_PVSS_CORE_CLIENTCONTEXT_H
#define _SCAG_PVSS_CORE_CLIENTCONTEXT_H

#include "scag/pvss/api/core/Context.h"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/Mutex.hpp"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

/// interface
class ClientContext : public Context
{
public:
    ClientContext( Request* req, Client::ResponseHandler* handle ) :
    Context(req), handler(handle) {}

    virtual void setResponse( Response* resp ) throw (PvssException)
    {
        Context::setResponse(resp);
        if ( handler != 0 ) {
            smsc_log_debug(log_,"notifying handler of response");
            handler->handleResponse(getRequest(),getResponse());
        }
    }
    void setError( const PvssException& exc ) {
        if ( handler != 0 ) {
            smsc_log_debug(log_,"notifying handler of error");
            handler->handleError(exc,getRequest());
        }
    }

private:
    Client::ResponseHandler* handler; // not owned, may be null
};

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CLIENTCONTEXT_H */
