/* ************************************************************************** *
 * POSIX Synchronization primitive(s): Statefulll Event
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_EVENT_HPP__
#ident "@(#)$Id$"
#define __CORE_SYNCHRONIZATION_EVENT_HPP__

#include <pthread.h>
#include <sys/time.h>
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace core {
namespace synchronization {

class Event {
protected:
  pthread_cond_t    event;
  mutable Mutex     mutex;
  bool              signaled;

public:
    Event() : signaled(false)
    {
        pthread_cond_init(&event, NULL);
    }
    ~Event()
    {
        pthread_cond_destroy(&event);
    }
    int Wait()
    {
        mutex.Lock();
        if (signaled) {
            signaled = false;
            mutex.Unlock();
            return 0;
        }
        int retval = pthread_cond_wait(&event, &mutex.mutex);
        signaled = false;
        mutex.Unlock();
        return retval;
    }
    int Wait(int timeout_sec)
    {
        mutex.Lock();
        if (signaled) {
            signaled = false;
            mutex.Unlock();
            return 0;
        }

        struct timespec tv = {0,0}; //time with nanosecs
        //Note: on Solaris requires -D__EXTENSIONS__
        clock_gettime(CLOCK_REALTIME, &tv);
        tv.tv_sec += timeout_sec/1000;
        tv.tv_nsec += (timeout_sec % 1000)*1000000L;
        if (tv.tv_nsec > 1000000000L) {
            ++tv.tv_sec;
            tv.tv_nsec -= 1000000000L;
        }

        int retval = pthread_cond_timedwait(&event, &mutex.mutex, &tv);
        signaled = false;
        mutex.Unlock();
        return retval;
    }
    void Signal()
    {
        MutexGuard tmp(mutex);
        signaled = true;
        pthread_cond_signal(&event);
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
        pthread_cond_broadcast(&event);
    }
};

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_EVENT_HPP__ */

