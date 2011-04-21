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

/// NOTE: If you want to check contention in you application,
///   please use CPPFLAGS += -DCHECKCONTENTION and
///   special macro INITMUTEX(name) to initialize mutex.
/// Also, you will need to instantiate contentionLimit and
/// implement reportContention() somewhere, e.g. in main().

class Mutex {
protected:
    friend class Condition;

    pthread_mutex_t mutex;
    pthread_t       ltid;
#ifdef CHECKCONTENTION
    const char*        what;
    const char*        wasfrom;
    volatile pthread_t wasid;

    static unsigned contentionLimit;
    static void reportContention( const char* what,
                                  const char* from,
                                  const char* wasfrom,
                                  pthread_t   wholocked,
                                  unsigned  howlongusec ) throw();
#endif

    Mutex(const Mutex&);
    void operator=(const Mutex&);

#ifdef CHECKCONTENTION
    void updateThreadId(const char* from=0) {
        wasid = ltid = pthread_self();
        wasfrom = from;
    }
#else
    void updateThreadId(void) { ltid = pthread_self(); }
#endif

public:
#ifdef CHECKCONTENTION
    static void setContentionLimit( unsigned usec ) throw() {
        contentionLimit = usec;
    }

    Mutex( const char* fileline = 0) : what(fileline), wasid(pthread_t(-1))
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
#ifdef CHECKCONTENTION
    void Lock(const char* from = 0)
    {
        if (what) {
            if (!pthread_mutex_trylock(&mutex)) {
                updateThreadId(from);
                return;
            }
            timeval ts,te;
            gettimeofday(&ts,0);
            pthread_mutex_lock(&mutex);
            gettimeofday(&te,0);
            const unsigned waslocked =
                unsigned(te.tv_sec - ts.tv_sec)*1000000 +
                unsigned(te.tv_usec) - unsigned(ts.tv_usec);
            if (waslocked > contentionLimit) {
                reportContention(what,from,wasfrom,wasid,waslocked);
                // fprintf(stderr,"%s contented by %u for %u usec\n",what,was,waslocked);
            }
        } else {
            pthread_mutex_lock(&mutex);
        }
        updateThreadId(from);
    }
#else
    void Lock()
    {
        pthread_mutex_lock(&mutex);
        updateThreadId();
    }
#endif
    void Unlock()
    {
        ltid = (pthread_t)-1;
        pthread_mutex_unlock(&mutex);
    }
    bool TryLock(const char* from = 0)
    {
        if (!pthread_mutex_trylock(&mutex)) {
            updateThreadId(from);
            return true;
        }
        return false; 
    }
};

#ifdef CHECKCONTENTION
#define INITMUTEXSTRINGIFY(x) #x
#define INITMUTEXTOSTRING(x) INITMUTEXSTRINGIFY(x)
#define WHEREAMI   __FILE__ ":" INITMUTEXTOSTRING(__LINE__)
#define POSTWHERE , __FILE__ ":" INITMUTEXTOSTRING(__LINE__)
#else
#define WHEREAMI
#define POSTWHERE
#endif

typedef MutexGuardTmpl<Mutex> MutexGuard;
typedef MutexTryGuard_T<Mutex> MutexTryGuard;
typedef ReverseMutexGuard_T<Mutex> ReverseMutexGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_MUTEX_HPP__ */

