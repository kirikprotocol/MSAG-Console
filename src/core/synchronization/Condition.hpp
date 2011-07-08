/* ************************************************************************** *
 * Synchronization primitive(s): Stateless Event (Condition)
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_CONDITION_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_SYNCHRONIZATION_CONDITION_HPP__

#include "core/synchronization/TimeSlice.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace core {
namespace synchronization {

class Condition {
private:
    pthread_cond_t event;

protected:
    Condition(const Condition &);
    void operator=(const Condition &);

public:
    Condition()
    {
        pthread_cond_init(&event, NULL);
    }
    ~Condition()
    {
        pthread_cond_destroy(&event);
    }

    void Signal(void)
    {
        pthread_cond_signal(&event);
    }
    void SignalAll(void)
    {
        pthread_cond_broadcast(&event);
    }
    //Note: mutex MUST BE LOCKED!
    int WaitOn(Mutex & use_mtx)
    {
        use_mtx.ltid = (pthread_t)-1;
        int rval = pthread_cond_wait(&event, &use_mtx.mutex);
        use_mtx.updateThreadId();
        return rval;
    }
    //Note: mutex MUST BE LOCKED!
    int WaitOn(Mutex & use_mtx, const TimeSlice & use_timeout)
    {
        struct timespec tv = use_timeout.adjust2Nano();
        use_mtx.ltid = (pthread_t)-1;
        int rval = pthread_cond_timedwait(&event, &use_mtx.mutex, &tv);
        use_mtx.updateThreadId();
        return rval;
    }
    //Note: mutex MUST BE LOCKED!
    int WaitOn(Mutex & use_mtx, const struct timeval & abs_time)
    {
        struct timespec tvs;
        tvs.tv_sec = abs_time.tv_sec;
        tvs.tv_nsec = abs_time.tv_usec*1000;
        use_mtx.ltid = (pthread_t)-1;
        int rval = pthread_cond_timedwait(&event, &use_mtx.mutex, &tvs);
        use_mtx.updateThreadId();
        return rval;
    }
    //Note: mutex MUST BE LOCKED!
    int WaitOn(Mutex & use_mtx, const struct timespec & abs_time)
    {
        use_mtx.ltid = (pthread_t)-1;
        int rval = pthread_cond_timedwait(&event, &use_mtx.mutex, &abs_time);
        use_mtx.updateThreadId();
        return rval;
    }
};

} //synchronization
} //core
} //smsc

#endif /* __CORE_SYNCHRONIZATION_CONDITION_HPP__ */

