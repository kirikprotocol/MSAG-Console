#ifndef __CORE_SYNCHRONIZATION_EVENT_HPP__
#define __CORE_SYNCHRONIZATION_EVENT_HPP__

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
    return cond_wait(&event,mutex);
  }
  void notify()
  {
    cond_signal(&event);
  }
  void notifyAll()
  {
    conf_broadcast(&event);
  }
protected:
  cond_t event;
};//EventMonitor

};//synchronization
};//core
};//smsc


#endif
