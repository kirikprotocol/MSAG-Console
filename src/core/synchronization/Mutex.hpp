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
#include <sys/time.h>
#endif

namespace smsc {
namespace core {
namespace synchronization {

class Condition;

/// NOTE: To check contention in you application, you have two options
/// 1. CHECKCONTENTION=YES on make command line;
/// 2. CHECKCONTENTION=FORCE on make command line.
/// Then you may initialize the mutex/eventmonitor under question
/// via special macro MTXWHEREAMI, see the example:
///  Mutex problematicLock( MTXWHEREAMI );
/// Also, you will need to instantiate contentionLimit and
/// implement reportContention() somewhere, e.g. in main().

class Mutex {
protected:
    friend class Condition;

    pthread_mutex_t mutex;
    pthread_t       ltid;
#ifdef CHECKCONTENTION
    const char*          what;
    volatile const char* wasfrom;
    volatile pthread_t   wasid;
    volatile unsigned    lockCount;

    static unsigned contentionLimit;

    /// a method which is invoked to report contention problems.
    /// lock is already acquired.
    ///  @param from is where the Lock() was invoked from (may be NULL);
    ///  @param howlong is the time the lock was not achievable (usec);
    ///  @param oldcount is the old value of lockCount.
    void reportContention( const char* from, unsigned howlong, unsigned oldcount ) const throw();

#endif

    Mutex(const Mutex&);
    void operator=(const Mutex&);

#ifdef CHECKCONTENTION
    void updateThreadId(const char* from=0) {
        wasid = ltid = pthread_self();
        wasfrom = from;
        ++lockCount;
    }
#else
    void updateThreadId(void) { ltid = pthread_self(); }
#endif

public:
#ifdef CHECKCONTENTION
    bool isLocked() const throw() {
        return ltid == pthread_self();
    }


    static void setContentionLimit( unsigned usec ) throw() {
        contentionLimit = usec;
    }

    Mutex( const char* fileline = 
#if CHECKCONTENTION > 1
           "unkMtx"
#else
           0
#endif
         ) : what(fileline), wasfrom(0), wasid(pthread_t(-1)), lockCount(0)
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
            const unsigned oldcount = lockCount;
            timeval ts,te;
            gettimeofday(&ts,0);
            pthread_mutex_lock(&mutex);
            gettimeofday(&te,0);
            const unsigned waslocked =
                unsigned(te.tv_sec - ts.tv_sec)*1000000 +
                unsigned(te.tv_usec) - unsigned(ts.tv_usec);
            if (waslocked > contentionLimit) {
                reportContention(from,waslocked,oldcount);
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
#ifdef CHECKCONTENTION
            updateThreadId(from);
#else
            updateThreadId();
#endif
            return true;
        }
        return false; 
    }
};

#ifdef CHECKCONTENTION
#define INITMUTEXSTRINGIFY(x) #x
#define INITMUTEXTOSTRING(x) INITMUTEXSTRINGIFY(x)
#define MTXWHEREAMI   __FILE__ ":" INITMUTEXTOSTRING(__LINE__)
#define MTXWHEREAMISTATIC  ( __FILE__ ":" INITMUTEXTOSTRING(__LINE__) )
#define MTXWHEREPOST , __FILE__ ":" INITMUTEXTOSTRING(__LINE__)
#define MTXFORCENOCHECK NULL
#else
#define MTXWHEREAMI
#define MTXWHEREAMISTATIC
#define MTXWHEREPOST
#define MTXFORCENOCHECK
#endif

typedef MutexGuardTmpl<Mutex> MutexGuard;
typedef MutexTryGuard_T<Mutex> MutexTryGuard;
typedef ReverseMutexGuard_T<Mutex> ReverseMutexGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_MUTEX_HPP__ */

