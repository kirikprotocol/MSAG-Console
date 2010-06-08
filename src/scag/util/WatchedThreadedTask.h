#ifndef _SCAG_UTIL_WATCHEDTHREADEDTASK_H
#define _SCAG_UTIL_WATCHEDTHREADEDTASK_H

#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace scag2 {
namespace util {

class WatchedThreadedTask : public smsc::core::threads::ThreadedTask
{
protected:
    WatchedThreadedTask() : isStarted_(false) {
        // isReleased = true;
    }

public:
    virtual void onRelease() {
        stop();
        smsc::core::synchronization::MutexGuard mg(releaseMon_);
        ThreadedTask::onRelease();
        releaseMon_.notify();
    }

    void waitUntilReleased() {
        if (isReleased) return;
        smsc::core::synchronization::MutexGuard mg(releaseMon_);
        while ( !isReleased ) {
            releaseMon_.wait(500);
        }
    }

    void waitUntilStarted() {
        if (isStarted_) return;
        if (stopping()) return;
        smsc::core::synchronization::MutexGuard mg(releaseMon_);
        while ( !isStarted_ ) {
            if ( stopping() ) return;
            releaseMon_.wait(100);
        }
    }

    bool released() const {
        return isReleased;
    }

    bool isStarted() const {
        return isStarted_;
    }

    virtual int Execute() {
        {
            smsc::core::synchronization::MutexGuard mg(releaseMon_);
            isReleased = false;
            isStarted_ = true;
            releaseMon_.notify();
        }
        return doExecute();
    }

protected:
    virtual int doExecute() = 0;

    // should be invoked in dtor
    void checkDestroy() {
        if ( !isReleased && isStarted_ ) {
            waitUntilReleased();
        }
    }

protected:
    bool isStarted_;
    smsc::core::synchronization::EventMonitor releaseMon_;
};

} // namespace util
} // namespace scag2

#endif /* !_SCAG_UTIL_THREADWRAP_H */
