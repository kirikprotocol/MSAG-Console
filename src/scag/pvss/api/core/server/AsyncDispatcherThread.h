#ifndef _SCAG_PVSS_CORE_SERVER_ASYNCDISPATCHERTHREAD_H
#define _SCAG_PVSS_CORE_SERVER_ASYNCDISPATCHERTHREAD_H

#include "WorkerThread.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class AsyncDispatcher;

class AsyncDispatcherThread : public WorkerThread
{
public:
    AsyncDispatcherThread( AsyncDispatcher& dispatcher );

    ~AsyncDispatcherThread();

    virtual int Execute();

    virtual const char* taskName() { return "pvss.dspch"; }

    virtual void shutdown();
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_ASYNCDISPATCHERTHREAD_H */
