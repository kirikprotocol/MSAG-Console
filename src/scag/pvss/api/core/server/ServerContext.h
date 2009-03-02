#ifndef _SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H
#define _SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H

#include "scag/pvss/api/core/Context.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class ServerContext : public Context
{
protected:
    enum State {
            NEW = 1,
            PROCESSED,
            SENT,
            FAILED
    };

public:
    ServerContext( Request* req, PvssSocket& channel, ServerCore& core ) :
    Context(req), core_(core), state_(NEW), socket_(channel) {}

    State getState() const {
        return state_;
    }
    void setState( State state ) {
        state_ = state;
    }

    virtual void setResponse(Response* response) throw (PvssException)
    {
        if ( state_ == NEW && response != 0 ) {
            state_ = PROCESSED;
            response->setSeqNum(getSeqNum());
            Context::setResponse(response);
            core_.contextProcessed(*this);
        } else {
            throw PvssException(PvssException::BAD_RESPONSE,"Response is null or was already processed");
        }
    }


    virtual void setError(const std::string& msg) throw(PvssException);
    /*
    {
        setResponse(new ErrorResponse(getSeqNum(),Response::ERROR, message));
    }
     */

private:
    ServerCore& core_;
    State       state_;
    PvssSocket& socket_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H */
