#ifndef __CORE_SYNCHRONIZATION_MUTEX_HPP__
#define __CORE_SYNCHRONIZATION_MUTEX_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <synch.h>
#endif

namespace smsc{
namespace core{
namespace synchronization{

class Mutex{
public:
  Mutex()
  {
#ifdef _WIN32
    mutex=CreateMutex(NULL,FALSE,NULL);
#else
    init_mutex(&mutex,USYNC_THREAD,NULL);
#endif
  }
  ~Mutex()
  {
#ifdef _WIN32
    CloseHandle(mutex);
#else
    mutex_destroy(mutex);
#endif
  }
  void Lock()
  {
#ifdef _WIN32
    WaitForSingleObject(mutex,INFINITE);
#else
    mutex_lock(&mutex);
#endif
  }
  void Unlock()
  {
#ifdef _WIN32
    ReleaseMutex(mutex);
#else
    mutex_unlock(&mutex);
#endif
  }

private:
#ifdef _WIN32
  HANDLE mutex;
#else
  mutex_t mutex;
#endif
};

};//synchronization
};//core
};//smsc

#endif