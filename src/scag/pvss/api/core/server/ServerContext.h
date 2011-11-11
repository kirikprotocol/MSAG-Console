#ifndef _SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H
#define _SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H

#include "core/synchronization/Mutex.hpp"
#include "scag/pvss/api/core/Context.h"

namespace scag2 {
namespace pvss {
namespace core {

class PvssSocketBase;

namespace server {

class ServerCore;
class Worker;
class ContextQueue;

/// base server context (may be new or old)
class ServerContext : public Context
{
    friend class eyeline::informer::EmbedRefPtr< ServerContext >;
public:
    enum State {
            NEW = 1,
            PROCESSED,
            SENT,
            FAILED
    };

public:
    ServerContext( Request* req ) :
    Context(req), state_(NEW), respQueue_(0) {}

    State getState() const {
        return state_;
    }
    void setState( State state ) {
        state_ = state;
    }

    virtual void setResponse(Response* response) /* throw (PvssException) */ ;

    void setError( const std::string& msg, Response::Type type ) /* throw(PvssException) */ ;

    /// where to return notification on response delivery
    virtual void setRespQueue( ContextQueue& respQueue ) {
        respQueue_ = &respQueue;
    }

    ContextQueue* getRespQueue() {
        return respQueue_;
    }

    /// used for hashing context on socket in servercore
    virtual PvssSocketBase* getSocket() = 0;

    /// send response
    virtual void sendResponse() /* throw (PvssException) */  = 0;

private:
    smsc::core::synchronization::Mutex setRespMutex_;
    State                              state_;
    ContextQueue*                      respQueue_;
    // PvssSocket& socket_;
};

typedef eyeline::informer::EmbedRefPtr< ServerContext > ServerContextPtr;

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCONTEXT_H */
