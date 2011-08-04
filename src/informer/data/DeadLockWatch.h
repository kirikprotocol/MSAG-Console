#ifndef _INFORMER_DEADLOCKWATCH_H
#define _INFORMER_DEADLOCKWATCH_H

#include <string>
#include <vector>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

class DeadLockWatcher;

class DeadLockWatch
{
public:
    DeadLockWatch( DeadLockWatcher& watcher,
                   const char*      category,
                   const char*      name,
                   timediff_type    period );

    ~DeadLockWatch();

    inline void ping( msgtime_type now ) { lastTime_ = now; }

    /// return 0 if ok, otherwise the number of seconds of last activity
    msgtime_type isAlive( msgtime_type now ) const {
        timediff_type res = timediff_type(now - lastTime_);
        if ( res <= period_ ) return 0;
        return msgtime_type(res);
    }

    const char* getCategory() const { return category_.c_str(); }
    const char* getName() const { return name_.c_str(); }

private:
    DeadLockWatcher&      watcher_;
    const std::string     category_;
    const std::string     name_;
    timediff_type         period_;
    volatile msgtime_type lastTime_;
};


class DeadLockWatcher : protected smsc::core::threads::Thread
{
    friend class DeadLockWatch;
public:
    DeadLockWatcher();
    virtual ~DeadLockWatcher();
    void start();
    void setStopping();

protected:
    virtual int Execute();
    void registerWatch( DeadLockWatch& watch );
    void unregisterWatch( DeadLockWatch& watch );

private:
    typedef std::vector< DeadLockWatch* > WatchList;

    smsc::core::synchronization::EventMonitor mon_;
    WatchList                     watches_;
    bool                          stopping_;
};

} // informer
} // smsc

#endif
