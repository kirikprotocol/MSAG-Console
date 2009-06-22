#ifndef SCAG_BILL_EWALLET_CLIENT_CLIENTCONTEXT_H
#define SCAG_BILL_EWALLET_CLIENT_CLIENTCONTEXT_H

#include "scag/bill/ewallet/proto/Context.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace client {

/// an implementation
class ClientContext : public proto::Context
{
public:
    ClientContext( Request* request, Client::ResponseHandler* handler ) :
    proto::Context(request,0), handler_(handler) {}

    Client::ResponseHandler* getHandler() { return handler_; }

    virtual void setError( const Exception& e ) {
        if ( handler_ && getRequest().get() ) {
            handler_->handleError( getRequest(), e );
            getRequest().reset(0);
            getResponse().reset(0);
        }
    }

private:
    Client::ResponseHandler* handler_;
};

} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CLIENTCONTEXT_H */
