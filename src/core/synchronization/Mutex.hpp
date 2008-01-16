#pragma ident "$Id$"
#ifndef __CORE_SYNCHRONIZATION_MUTEX_HPP__
#define __CORE_SYNCHRONIZATION_MUTEX_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "core/synchronization/MutexGuard.hpp"

namespace smsc{
namespace core{
namespace synchronization{

class Event;

class Mutex{
public:
  Mutex()
  {
#ifdef _WIN32
    mutex=CreateMutex(NULL,FALSE,NULL);
#else
    pthread_mutex_init(&mutex,NULL);
#endif
  }
  ~Mutex()
  {
#ifdef _WIN32
    CloseHandle(mutex);
#else
    pthread_mutex_destroy(&mutex);
#endif
  }
  void Lock()
  {
#ifdef _WIN32
    WaitForSingleObject(mutex,INFINITE);
#else
    pthread_mutex_lock(&mutex);
    ltid=pthread_self();
#endif
  }
  void Unlock()
  {
#ifdef _WIN32
    ReleaseMutex(mutex);
#else
    ltid=-1;
    pthread_mutex_unlock(&mutex);
#endif
  }
  bool TryLock()
  {
#ifdef _WIN32
    return WaitForSingleObject(mutex,1)!=WAIT_TIMEOUT;
#else
    return pthread_mutex_trylock(&mutex)==0;
#endif
  }
#ifndef _WIN32
  //Condition variable should be properly initialized !
  inline int WaitCondition(pthread_cond_t & cond_var)
  {
      return pthread_cond_wait(&cond_var, &mutex);
  }
#endif /* _WIN32 */

protected:
#ifdef _WIN32
  HANDLE mutex;
#else
  pthread_mutex_t mutex;
  pthread_t ltid;
  friend class Event;
#endif
  Mutex(const Mutex&);
  void operator=(const Mutex&);
};//Mutex

typedef MutexGuardTmpl<Mutex> MutexGuard;
typedef MutexTryGuard_T<Mutex> MutexTryGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_MUTEX_HPP__ */

