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

protected:
    State getState() const {
        return state_;
    }
    void setState( State state ) {
        state_ = state;
    }

    virtual void setResponse(Response* response) throw (PvssException)
    {
        if ( state == NEW && response != 0 ) {
            state = PROCESSED;
            response->setSeqNum(getSeqNum());
            Context::setResponse(response);
            core->contextProcessed(*this);
        } else {
            throw PvssException(PvssException::BAD_RESPONSE,"Response is null or was already processed");
        }
    }

};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H */
