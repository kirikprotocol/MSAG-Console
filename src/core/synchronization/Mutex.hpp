/* ************************************************************************** *
 * Synchronization primitive(s): Mutex
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_MUTEX_HPP__
#ident "@(#)$Id$"
#define __CORE_SYNCHRONIZATION_MUTEX_HPP__

#include <pthread.h>
#include "core/synchronization/MutexGuard.hpp"

namespace smsc {
namespace core {
namespace synchronization {

class Condition;

class Mutex {
protected:
    friend class Condition;

    pthread_mutex_t mutex;
    pthread_t       ltid;

    Mutex(const Mutex&);
    void operator=(const Mutex&);

    void updateThreadId(void) { ltid = pthread_self(); }

public:
    Mutex()
    {
        pthread_mutex_init(&mutex, NULL);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }

    void Lock()
    {
        pthread_mutex_lock(&mutex);
        updateThreadId();
    }
    void Unlock()
    {
        ltid = (pthread_t)-1;
        pthread_mutex_unlock(&mutex);
    }
    bool TryLock()
    {
        if (!pthread_mutex_trylock(&mutex)) {
            updateThreadId();
            return true;
        }
        return false; 
    }
};

typedef MutexGuardTmpl<Mutex> MutexGuard;
typedef MutexTryGuard_T<Mutex> MutexTryGuard;
typedef ReverseMutexGuard_T<Mutex> ReverseMutexGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_MUTEX_HPP__ */

