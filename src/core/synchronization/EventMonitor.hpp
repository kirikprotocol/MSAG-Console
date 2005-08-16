#ifndef __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__
#define __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include "Mutex.hpp"
#include <sys/time.h>

namespace smsc{
namespace core{
namespace synchronization{

class EventMonitor:public Mutex{
public:
#ifdef _WIN32
  EventMonitor()
  {
    hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
  }
  ~EventMonitor()
  {
    CloseHandle(hEvent);
  }
  int wait()
  {
    Unlock();
    int res=WaitForSingleObject(hEvent,INFINITE);
    Lock();
    return res;
  }
  int wait(int timeout)
  {
    Unlock();
    int res=WaitForSingleObject(hEvent,timeout);
    Lock();
    return res;
  }
  int notify()
  {
    return SetEvent(hEvent);
  }
protected:
  HANDLE hEvent;
#else
  EventMonitor()
  {
    pthread_cond_init(&event,NULL);
  }
  ~EventMonitor()
  {
    pthread_cond_destroy(&event);
  }
  int wait()
  {
    return pthread_cond_wait(&event,&mutex);
  }
  int wait(pthread_cond_t* cnd)
  {
    return pthread_cond_wait(cnd,&mutex);
  }
  int wait(int timeout)
  {
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
    timestruc_t tv;
    clock_gettime(CLOCK_REALTIME,&tv);
    tv.tv_sec+=timeout/1000;
    tv.tv_nsec+=(timeout%1000)*1000000L;
    if(tv.tv_nsec>1000000000L)
    {
      tv.tv_sec++;
      tv.tv_nsec-=1000000000L;
    }
#endif
    return pthread_cond_timedwait(&event,&mutex,&tv);
  }
  void notify()
  {
    pthread_cond_signal(&event);
  }
  void notify(pthread_cond_t* cnd)
  {
    pthread_cond_signal(cnd);
  }
  void notifyAll()
  {
    pthread_cond_broadcast(&event);
  }
protected:
  pthread_cond_t event;
#endif
};//EventMonitor

}//synchronization
}//core
}//smsc


#endif
