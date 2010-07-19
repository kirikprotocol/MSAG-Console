/* ************************************************************************** *
 * Synchronization primitive(s): Statefulll Event
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_EVENT_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_SYNCHRONIZATION_EVENT_HPP__

#include "core/synchronization/Condition.hpp"

namespace smsc {
namespace core {
namespace synchronization {

class Event {
protected:
    Condition       condVar;
    mutable Mutex   mutex;
    bool            signaled;

    Event(const Event &);
    void operator=(const Event &);

    enum TGTTime { ttNone = 0, ttTimeout, ttAbsTime };
    int condWait(TGTTime tgt_time, const void * use_time = 0)
    {
        mutex.Lock();
        if (signaled) {
            signaled = false;
            mutex.Unlock();
            return 0;
        }
        int retval;
        if (tgt_time == ttNone)
            retval = condVar.WaitOn(mutex);
        else if (tgt_time == ttTimeout)
            retval = condVar.WaitOn(mutex, *(const TimeSlice *)use_time);
        else //ttAbsTime
            retval = condVar.WaitOn(mutex, *(const struct timespec *)use_time);

        signaled = false;
        mutex.Unlock();
        return retval;
    }

public:
    Event() : signaled(false)
    { }
    ~Event()
    { }

    int Wait()
    {
        return condWait(ttNone);
    }
    int Wait(const TimeSlice & use_timeout)
    {
        return condWait(ttTimeout, &use_timeout);
    }
    int Wait(const struct timespec & abs_time)
    {
        return condWait(ttAbsTime, &abs_time);
    }

    //this one is kept for compatibility issue
    int Wait(int timeout_msec) //timeout unit: millisecs
    {
        return Wait(TimeSlice(timeout_msec, TimeSlice::tuMSecs));
    }

    void Signal()
    {
        MutexGuard tmp(mutex);
        signaled = true;
        condVar.Signal();
    }
    bool isSignaled() const
    {
        MutexGuard tmp(mutex);
        return signaled;
    }
    void SignalAll()
    {
        MutexGuard tmp(mutex);
        signaled = true;
        condVar.SignalAll();
    }
};

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_EVENT_HPP__ */

