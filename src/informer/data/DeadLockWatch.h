#ifndef _INFORMER_DEADLOCKWATCH_H
#define _INFORMER_DEADLOCKWATCH_H

#include <string>
#include <vector>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace eyeline {
namespace informer {

class DeadLockWatcher;

class DeadLockWatch
{
public:
    DeadLockWatch( DeadLockWatcher& watcher,
                   const char*      category,
                   const char*      name );

    ~DeadLockWatch();

    inline void ping() { ++serial_; }

    bool isAlive() {
        const int next = serial_;
        const bool res = (next != prevSerial_);
        prevSerial_ = next;
        return res;
    }

    const char* getCategory() const { return category_.c_str(); }
    const char* getName() const { return name_.c_str(); }

private:
    DeadLockWatcher&  watcher_;
    const std::string category_;
    const std::string name_;
    volatile int      serial_;
    int               prevSerial_;
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
