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

class Event{
public:
  Event()
  {
#ifdef _WIN32
    event=CreateEvent(NULL,FALSE,FALSE,NULL);
#else
    cond_init(&event,USYNC_THREAD,NULL);
#endif
  }
  ~Event()
  {
#ifdef _WIN32
    CloseHandle(event);
#else
    cond_destroy(&event);
#endif
  }
  int Wait()
  {
#ifdef _WIN32
    return WaitForSingleObject(event,INFINITE);
#else
    mutex.Lock();
    int retval=cond_wait(&event,&mutex.mutex);
    mutex.Unlock();
    return retval;
#endif
  }
  void Signal()
  {
#ifdef _WIN32
    SetEvent(event);
#else
    cond_signal(&event);
#endif
  }
protected:
#ifdef _WIN32
  HANDLE event;
#else
  cond_t event;
  Mutex mutex;
#endif
};//Event

};//synchronization
};//core
};//smsc


#endif
