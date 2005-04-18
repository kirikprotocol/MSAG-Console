#ifndef __CORE_SYNCHRONIZATION_EVENT_HPP__
#define __CORE_SYNCHRONIZATION_EVENT_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sys/time.h>
#endif
#include "Mutex.hpp"

namespace smsc{
namespace core{
namespace synchronization{

class Event{
public:
  Event()
  {
#ifdef _WIN32
    event=CreateEvent(NULL,FALSE,FALSE,NULL);
#else
    pthread_cond_init(&event,NULL);
    signaled=0;
#endif
  }
  ~Event()
  {
#ifdef _WIN32
    CloseHandle(event);
#else
    pthread_cond_destroy(&event);
#endif
  }
  int Wait()
  {
#ifdef _WIN32
    return WaitForSingleObject(event,INFINITE);
#else
    mutex.Lock();
    if(signaled)
    {
      signaled=0;
      mutex.Unlock();
      return 0;
    }
    int retval=pthread_cond_wait(&event,&mutex.mutex);
    signaled=0;
    mutex.Unlock();
    return retval;
#endif
  }
  int Wait(int timeout)
  {
#ifdef _WIN32
    return WaitForSingleObject(event,timeout);
#else
    mutex.Lock();
    if(signaled)
    {
      signaled=0;
      mutex.Unlock();
      return 0;
    }
#ifdef linux
    struct timeval now;
    struct timespec tv;
    int retcode;

    gettimeofday(&now,0);
    tv.tv_sec = now.tv_sec + timeout/1000;
    tv.tv_nsec = now.tv_usec * 1000+(timeout%1000)*1000000;

    if(tv.tv_nsec>1000000000L)
    {
      tv.tv_sec++;
      tv.tv_nsec-=1000000000L;
    }

#else
    timestruc_t tv={0,0};
    clock_gettime(CLOCK_REALTIME,&tv);
    tv.tv_sec+=timeout/1000;
    tv.tv_nsec+=(timeout%1000)*1000000L;
    if(tv.tv_nsec>1000000000L)
    {
      tv.tv_sec++;
      tv.tv_nsec-=1000000000L;
    }
#endif
    int retval=pthread_cond_timedwait(&event,&mutex.mutex,&tv);
    signaled=0;
    mutex.Unlock();
    return retval;
#endif
  }
  void Signal()
  {
#ifdef _WIN32
    SetEvent(event);
#else
    mutex.Lock();
    signaled=1;
    pthread_cond_signal(&event);
    mutex.Unlock();
#endif
  }
#ifndef _WIN32
  int isSignaled()
  {
    mutex.Lock();
    int retval=signaled;
    mutex.Unlock();
    return retval;
  }
#endif
#ifndef _WIN32
  void SignalAll()
  {
    mutex.Lock();
    signaled=1;
    pthread_cond_broadcast(&event);
    mutex.Unlock();
  }
#endif
protected:
#ifdef _WIN32
  HANDLE event;
#else
  pthread_cond_t event;
  Mutex mutex;
  int signaled;
#endif
};//Event

}//synchronization
}//core
}//smsc


#endif
