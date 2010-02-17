#ifndef _SCAG_PVSS_CORE_SERVER_CONTEXTQUEUE_H
#define _SCAG_PVSS_CORE_SERVER_CONTEXTQUEUE_H

#include <memory>
#include "core/buffers/CyclicQueue.hpp"
#include "scag/pvss/common/PvssException.h"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerContext;

/**
 * Context queue keeps server contexts.
 * The queue is fully stopped when !queue.isStarted() and getContext() returns 0.
 */
class ContextQueue
{
public:
    /// NOTE: queueLimit is used for requests only.
    ContextQueue( int queueLimit );
    
    /// notify queue that a new request is received.
    void requestReceived(std::auto_ptr<ServerContext>& context) /* throw (PvssException) */ ;

    /// notify queue that response is send/failed
    void reportResponse(std::auto_ptr<ServerContext>& context);

    /// receive a new context, until either one:
    /// 1. a new context is arrived (it is returned);
    /// 2. the queue is stopped, and all contained contexts are exhausted (0 is returned).
    /// 3. tmo (msec) is expired if it is >0, otherwise it is blocking
    ServerContext* getContext( int tmo = 0 );

    /// fast check for request queue w/o locking
    inline int getSize() const { return queues_[1].Count(); }

    /// check if queue is started.
    inline bool isStarted() const { return started_; }

    /// return true if it could have request (i.e. acceptRequests==true or request queue is not empty)
    bool couldHaveRequests() const;

    /// startup the queue.
    void startup();

    /// notify the queue that requests are not allowed anymore.
    void stop();

    /// notify the queue that it is shutdown.
    void shutdown();

private:
    typedef smsc::core::buffers::CyclicQueue< ServerContext* >  QueueType;

private:
    smsc::logger::Logger*       log_;
    mutable smsc::core::synchronization::EventMonitor queueMon_;
    bool                        acceptRequests_; // only if started
    bool                        started_;
    int                         queueLimit_;
    QueueType                   queues_[2]; // 0 - response, 1 - request
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_CONTEXTQUEUE_H */
