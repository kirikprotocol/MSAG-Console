#ifndef _SCAG_PVSS_CORE_SERVER_ASYNCDISPATCHERTHREAD_H
#define _SCAG_PVSS_CORE_SERVER_ASYNCDISPATCHERTHREAD_H

#include "WorkerThread.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class AsyncDispatcher;
class ContextQueue;

class AsyncDispatcherThread : public WorkerThread
{
public:
    AsyncDispatcherThread( ServerCore& core, AsyncDispatcher& dispatcher );

    virtual ~AsyncDispatcherThread();

    virtual int doExecute();

    virtual const char* taskName() { return "pvss.dspch"; }

    /// initialization, startup of async dispatcher.
    void init() throw (PvssException);

    /// The method shutdowns the dispatcher thread and the underlying async logics.
    /// It waits until everything is shutdowned.
    virtual void shutdown();

private:
    AsyncDispatcher* dispatcher_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_ASYNCDISPATCHERTHREAD_H */
