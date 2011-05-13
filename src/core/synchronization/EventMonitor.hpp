/* ************************************************************************** *
 * Synchronization primitive(s): Stateless event monitor
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__

#define EXPLICIT_PTHREAD_API  //support old code in store/ConnectionManager, system/mapio

#include "core/synchronization/Condition.hpp"

namespace smsc {
namespace core {
namespace synchronization {

class EventMonitor : public Mutex {
protected:
    Condition       condVar;

    EventMonitor(const EventMonitor &);
    void operator=(const EventMonitor &);

public:
#ifdef CHECKCONTENTION
    EventMonitor( const char* fileline = 
#if CHECKCONTENTION > 1
                  "unkEvM"
#else
                  0
#endif
                ) : Mutex(fileline)
#else
    EventMonitor()
#endif
    { }
    ~EventMonitor()
    { }

    int wait(void)
    {
        return condVar.WaitOn(*this);
    }
    int wait(const TimeSlice & use_timeout)
    {
        return condVar.WaitOn(*this, use_timeout);
    }
    int wait(const struct timeval & abs_time)
    {
        return condVar.WaitOn(*this, abs_time);
    }
    int wait(const struct timespec & abs_time)
    {
        return condVar.WaitOn(*this, abs_time);
    }
    //this one is kept for compatibility issue
    int wait(int timeout_msec) //timeout unit: millisecs
    {
        return wait(TimeSlice(timeout_msec, TimeSlice::tuMSecs));
    }

    void notify(void)       { condVar.Signal(); }
    void notifyAll(void)    { condVar.SignalAll(); }


    int wait(Condition & use_cond)
    {
        return use_cond.WaitOn(*this);
    }
    int wait(Condition & use_cond, const TimeSlice & use_timeout)
    {
        return use_cond.WaitOn(*this, use_timeout);
    }
    int wait(Condition & use_cond, const struct timeval & abs_time)
    {
        return use_cond.WaitOn(*this, abs_time);
    }
    int wait(Condition & use_cond, const struct timespec & abs_time)
    {
        return use_cond.WaitOn(*this, abs_time);
    }
    //this one is kept for compatibility issue
    int wait(Condition & use_cond, int timeout_msec) //timeout unit: millisecs
    {
        return wait(use_cond, TimeSlice(timeout_msec, TimeSlice::tuMSecs));
    }

#ifdef EXPLICIT_PTHREAD_API
    int wait(pthread_cond_t* cnd)
    {
        int rval = pthread_cond_wait(cnd, &mutex);
        updateThreadId();
        return rval;
    }

    int wait(pthread_cond_t* cnd, int timeout_msec) //timeout unit: millisecs
    {
        TimeSlice tmo(timeout_msec, TimeSlice::tuMSecs);
        struct timespec tv = tmo.adjust2Nano();
        int rval = pthread_cond_timedwait(cnd, &mutex, &tv);
        updateThreadId();
        return rval;
    }

    void notify(pthread_cond_t* cnd)
    {
        pthread_cond_signal(cnd);
    }
#endif /* EXPLICIT_PTHREAD_API */
};

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__ */

