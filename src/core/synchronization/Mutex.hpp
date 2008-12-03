/* ************************************************************************** *
 * POSIX Synchronization primitive(s): Mutex
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_MUTEX_HPP__
#ident "@(#)$Id$"
#define __CORE_SYNCHRONIZATION_MUTEX_HPP__

#include <pthread.h>
#include "core/synchronization/MutexGuard.hpp"

namespace smsc {
namespace core {
namespace synchronization {

class Event;

class Mutex {
protected:
    friend class Event;

    pthread_mutex_t mutex;
    pthread_t       ltid;

    Mutex(const Mutex&);
    void operator=(const Mutex&);

public:
    Mutex()
    {
        pthread_mutex_init(&mutex,NULL);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }

    void Lock()
    {
        pthread_mutex_lock(&mutex);
        ltid = pthread_self();
    }
    void Unlock()
    {
        ltid=-1;
        pthread_mutex_unlock(&mutex);
    }
    bool TryLock()
    {
        return pthread_mutex_trylock(&mutex) == 0;
    }
    //Condition variable should be properly initialized !
    inline int WaitCondition(pthread_cond_t & cond_var)
    {
        return pthread_cond_wait(&cond_var, &mutex);
    }
};

typedef MutexGuardTmpl<Mutex> MutexGuard;
typedef MutexTryGuard_T<Mutex> MutexTryGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_MUTEX_HPP__ */

