#ifndef __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__
#define __CORE_SYNCHRONIZATION_EVENTMONITOR_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <thread.h>
#include <synch.h>
#include "Mutex.hpp"
#endif

namespace smsc{
namespace core{
namespace synchronization{

class EventMonitor:public Mutex{
public:
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
    tv.tv_sec=time(NULL)+timeout/1000;
    tv.tv_nsec=(timeout%1000)*1000000L;
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
};//EventMonitor

};//synchronization
};//core
};//smsc


#endif
