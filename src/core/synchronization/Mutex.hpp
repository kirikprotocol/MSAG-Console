#ifndef __CORE_SYNCHRONIZATION_MUTEX_HPP__
#define __CORE_SYNCHRONIZATION_MUTEX_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

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
#endif
  }
  void Unlock()
  {
#ifdef _WIN32
    ReleaseMutex(mutex);
#else
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
protected:
#ifdef _WIN32
  HANDLE mutex;
#else
  pthread_mutex_t mutex;
  friend class Event;
#endif
  Mutex(const Mutex&);
  void operator=(const Mutex&);
};//Mutex

class MutexGuard{
public:
  MutexGuard(Mutex& lock):lock(lock)
  {
    lock.Lock();
  }
  ~MutexGuard()
  {
    lock.Unlock();
  }
protected:
  Mutex& lock;
};

};//synchronization
};//core
};//smsc

#endif
