#ifndef _SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H
#define _SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H

#include "scag/pvss/api/core/Context.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerCore;
class Worker;

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
    ServerContext( Request* req, PvssSocket& channel ) :
    Context(req), worker_(0), state_(NEW), socket_(channel) {}

    State getState() const {
        return state_;
    }
    void setState( State state ) {
        state_ = state;
    }

    virtual void setResponse(Response* response) throw (PvssException);

    virtual void setError(const std::string& msg) throw(PvssException);

    virtual void setWorker( Worker& worker ) {
        worker_ = &worker;
    }

private:
    Worker*     worker_;
    State       state_;
    PvssSocket& socket_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H */
