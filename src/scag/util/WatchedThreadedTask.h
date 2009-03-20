#ifndef _SCAG_UTIL_WATCHEDTHREADEDTASK_H
#define _SCAG_UTIL_WATCHEDTHREADEDTASK_H

#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace scag2 {
namespace util {

class WatchedThreadedTask : public smsc::core::threads::ThreadedTask
{
protected:
    WatchedThreadedTask() {
        isReleased = true;
    }

public:
    virtual void onRelease() {
        MutexGuard mg(releaseMon_);
        ThreadedTask::onRelease();
        releaseMon_.notify();
    }

    void waitUntilReleased() {
        if (isReleased) return;
        MutexGuard mg(releaseMon_);
        while ( !isReleased ) {
            releaseMon_.wait(500);
        }
    }

    void waitUntilStarted() {
        if (!isReleased) return;
        MutexGuard mg(releaseMon_);
        while ( isReleased ) {
            releaseMon_.wait(100);
        }
    }

    bool released() const {
        return isReleased;
    }

    virtual int Execute() {
        isReleased = false;
        return doExecute();
    }

protected:
    virtual int doExecute() = 0;

protected:
    smsc::core::synchronization::EventMonitor releaseMon_;
};

} // namespace util
} // namespace scag2

#endif /* !_SCAG_UTIL_THREADWRAP_H */
