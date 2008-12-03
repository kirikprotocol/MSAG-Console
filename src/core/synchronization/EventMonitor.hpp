/* ************************************************************************** *
 * POSIX Synchronization primitive(s): EventMonitor
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__
#ident "@(#)$Id$"
#define __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__

#include <pthread.h>

#include "Mutex.hpp"
#include <sys/time.h>

namespace smsc {
namespace core {
namespace synchronization {

class EventMonitor : public Mutex {
protected:
    pthread_cond_t event;

public:
    EventMonitor()
    {
        pthread_cond_init(&event, NULL);
    }
    ~EventMonitor()
    {
        pthread_cond_destroy(&event);
    }
    int wait()
    {
        return pthread_cond_wait(&event,&mutex);
    }
    int wait(pthread_cond_t* cnd)
    {
        return pthread_cond_wait(cnd,&mutex);
    }
    int wait(pthread_cond_t* cnd, int timeout_sec)
    {
        struct timespec tv = {0,0}; //time with nanosecs
        //Note: on Solaris requires -D__EXTENSIONS__
        clock_gettime(CLOCK_REALTIME, &tv);
        tv.tv_sec += timeout_sec/1000;
        tv.tv_nsec += (timeout_sec % 1000)*1000000L;
        if (tv.tv_nsec > 1000000000L) {
            ++tv.tv_sec;
            tv.tv_nsec -= 1000000000L;
        }
        return pthread_cond_timedwait(cnd, &mutex, &tv);
    }
    int wait(int timeout_sec)
    {
        return wait(&event, timeout_sec);
    }
    void notify()
    {
        pthread_cond_signal(&event);
    }
    void notify(pthread_cond_t* cnd)
    {
        pthread_cond_signal(cnd);
    }
    void notifyAll()
    {
        pthread_cond_broadcast(&event);
    }
};

}//synchronization
}//core
}//smsc


#endif /* __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__ */

