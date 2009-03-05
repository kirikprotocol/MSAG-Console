#ifndef _SCAG_PVSS_CORE_SERVER_DISPATCHER_H
#define _SCAG_PVSS_CORE_SERVER_DISPATCHER_H

#include "ContextQueue.h"
#include "scag/util/WatchedThreadedTask.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class SyncDispatcher
{
public:
    virtual ~SyncDispatcher() {}
    virtual unsigned getIndex(Request& request) = 0;
    virtual Server::SyncLogic* getSyncLogic(unsigned index) = 0;
};

class AsyncDispatcher
{
public:
    virtual ~AsyncDispatcher() {}
    virtual unsigned getIndex(Request& request) = 0;
    virtual Server::AsyncLogic* getAsyncLogic(unsigned index) = 0;
};

class Dispatcher : public util::WatchedThreadedTask
{
public:
    virtual int Execute();

    virtual const char* taskName() { return "pvss.dspch"; }

    inline ContextQueue& getQueue() { return queue_; }
    inline ContextQueue& getRespQueue() { return respQueue_; }

private:
    ContextQueue queue_;
    ContextQueue respQueue_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_DISPATCHER_H */
