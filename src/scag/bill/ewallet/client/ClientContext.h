#ifndef SCAG_BILL_EWALLET_CLIENT_CLIENTCONTEXT_H
#define SCAG_BILL_EWALLET_CLIENT_CLIENTCONTEXT_H

#include "scag/bill/ewallet/proto/Context.h"
#include "util/TypeInfo.h"

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

    Client::ResponseHandler* getHandler() {
        CHECKMAGTC;
        return handler_;
    }

    virtual void setError( const Exception& e ) {
        CHECKMAGTC;
        if ( handler_ && getRequest().get() ) {
            handler_->handleError( getRequest(), e );
            getResponse().reset(0);
        }
    }

    virtual void setState( ContextState state ) {
        CHECKMAGTC;
        Context::setState(state);
        if ( handler_ && getRequest().get() ) {
            if ( state == DONE ) {
                handler_->handleResponse( getRequest(), getResponse() );
            } else if ( state == EXPIRED ) {
                setError( Exception( "expired", Status::TIMEOUT ) );
            } else if ( state == FAILED ) {
                setError( Exception( "failed", Status::UNKNOWN ) );
            }
        }
    }

    void makeNonExpirable() {
        CHECKMAGTC;
        creationTime_ = 0;
    }

private:
    DECLMAGTC(ClientContext);
    Client::ResponseHandler* handler_;
};

} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CLIENTCONTEXT_H */
