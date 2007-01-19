#ifndef __CORE_SYNCHRONIZATION_RECURSIVEMUTEX_HPP__
#define __CORE_SYNCHRONIZATION_RECURSIVEMUTEX_HPP__

#include <pthread.h>

#include "core/synchronization/MutexGuard.hpp"

#include <stdexcept>
namespace smsc{
namespace core{
namespace synchronization{


class Event;

class RecursiveMutex{
public:
  RecursiveMutex()
  {
    if ( pthread_mutexattr_init(&attr) ||
         pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) )
      throw std::runtime_error("RecursiveNutex:: pthread_mutexattr_init was failed");

    pthread_mutex_init(&mutex,&attr);
  }
  ~RecursiveMutex()
  {
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);
  }
  void Lock()
  {
    pthread_mutex_lock(&mutex);
  }
  void Unlock()
  {
    pthread_mutex_unlock(&mutex);
  }
  bool TryLock()
  {
    return pthread_mutex_trylock(&mutex)==0;
  }
protected:
  pthread_mutex_t mutex;
  pthread_mutexattr_t attr;

  RecursiveMutex(const RecursiveMutex&);
  void operator=(const RecursiveMutex&);
};//Mutex

typedef MutexGuardTmpl<RecursiveMutex> RecursiveMutexGuard;

}//synchronization
}//core
}//smsc

#endif
