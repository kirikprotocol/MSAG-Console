#ifndef __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__
#define __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <thread.h>
#include <synch.h>
#endif
#include "Mutex.hpp"

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
    WaitForSingleObject(hEvent,INFINITE);
    Lock();
    return 0;
  }
  int wait(int timeout)
  {
    return WaitForSingleObject(hEvent,INFINITE);
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
    cond_init(&event,USYNC_THREAD,NULL);
  }
  ~EventMonitor()
  {
    cond_destroy(&event);
  }
  int wait()
  {
    return cond_wait(&event,&mutex);
  }
  int wait(cond_t* cnd)
  {
    return cond_wait(cnd,&mutex);
  }
  int wait(int timeout)
  {
    timestruc_t tv;
    clock_gettime(CLOCK_REALTIME,&tv);
    tv.tv_sec+=timeout/1000;
    tv.tv_nsec+=(timeout%1000)*1000000L;
    if(tv.tv_nsec>1000000000L)
    {
      tv.tv_sec++;
      tv.tv_nsec-=1000000000L;
    }
    return cond_timedwait(&event,&mutex,&tv);
  }
  void notify()
  {
    cond_signal(&event);
  }
  void notify(cond_t* cnd)
  {
    cond_signal(cnd);
  }
  void notifyAll()
  {
    cond_broadcast(&event);
  }
protected:
  cond_t event;
#endif
};//EventMonitor

};//synchronization
};//core
};//smsc


#endif

