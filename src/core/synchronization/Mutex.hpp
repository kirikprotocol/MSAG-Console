/* ************************************************************************** *
 * Synchronization primitive(s): Mutex
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_MUTEX_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_SYNCHRONIZATION_MUTEX_HPP__

#include <pthread.h>
#include "core/synchronization/MutexGuard.hpp"

#ifdef CHECKCONTENTION
#include <stdio.h>
#endif

namespace smsc {
namespace core {
namespace synchronization {

class Condition;

class Mutex {
protected:
    friend class Condition;

    pthread_mutex_t mutex;
    pthread_t       ltid;
#ifdef CHECKCONTENTION
    const char*        file;
    const int          line;
    volatile pthread_t wasid;

    static unsigned contentionLimit;
#endif

    Mutex(const Mutex&);
    void operator=(const Mutex&);

#ifdef CHECKCONTENTION
    void updateThreadId(void) { wasid = ltid = pthread_self(); }
#else
    void updateThreadId(void) { ltid = pthread_self(); }
#endif

public:
#ifdef CHECKCONTENTION
    static void setContentionLimit( unsigned usec ) throw() {
        contentionLimit = usec;
    }

    /// NOTE: please use CPPFLAGS += -DCHECKCONTENTION=NUSEC and
    /// special macro INITMUTEX(name) to have mutex contention check enabled.
    Mutex( const char* fl = 0, int ln = 0) : file(fl), line(ln), wasid(pthread_t(-1))
    {
        pthread_mutex_init(&mutex, NULL);
    }
#else
    Mutex()
    {
        pthread_mutex_init(&mutex, NULL);
    }
#endif
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }

    // NOTE: even if CHECKCONTENTION is on, it is possible to disable
    // checking by supplying line=0 in ctor.
    void Lock()
    {
#ifdef CHECKCONTENTION
        if (line && file) {
            if (!pthread_mutex_trylock(&mutex)) {
                updateThreadId();
                return;
            }
            const unsigned was = unsigned(wasid);
            timeval ts,te;
            gettimeofday(&ts,0);
            pthread_mutex_lock(&mutex);
            gettimeofday(&te,0);
            const unsigned waslocked =
                unsigned(te.tv_sec - ts.tv_sec)*1000000 +
                unsigned(te.tv_usec) - unsigned(ts.tv_usec);
            if (waslocked > contentionLimit) {
                fprintf(stderr,"%s:%d contented by %u for %u usec\n",file,line,was,waslocked);
            }
        } else {
            pthread_mutex_lock(&mutex);
        }
#else
        pthread_mutex_lock(&mutex);
#endif
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

#ifdef CHECKCONTENTION
#define INITMUTEX(nm) nm(__FILE__,__LINE__)
#else
#define INITMUTEX(nm) nm()
#endif

typedef MutexGuardTmpl<Mutex> MutexGuard;
typedef MutexTryGuard_T<Mutex> MutexTryGuard;
typedef ReverseMutexGuard_T<Mutex> ReverseMutexGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_MUTEX_HPP__ */

