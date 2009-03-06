#ifndef _SCAG_PVSS_CORE_SERVER_WORKERTHREAD_H
#define _SCAG_PVSS_CORE_SERVER_WORKERTHREAD_H

#include "scag/util/WatchedThreadedTask.h"
#include "ContextQueue.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerCore;

class WorkerThread : public util::WatchedThreadedTask
{
protected:
    WorkerThread( ServerCore& core, const char* logCategory );

public:
    /// return attached queue
    inline ContextQueue& getQueue() { return queue_; }

    // virtual ~WorkerThread();

    // virtual void shutdown();

protected:
    ContextQueue          queue_;
    ServerCore&           core_;
    smsc::logger::Logger* log_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_WORKERTHREAD_H */
