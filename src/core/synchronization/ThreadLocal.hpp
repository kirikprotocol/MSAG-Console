#ifndef __CORE_SYNCHRONIZATION_THREADLOCAL_HPP__
#define __CORE_SYNCHRONIZATION_THREADLOCAL_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <thread.h>
#include <synch.h>
#endif

namespace smsc{
namespace core{
namespace synchronization{

template <class T>
class ThreadLocal{
public:
  ThreadLocal():initiated(0)
  {
#ifdef _WIN32
    key=TlsAlloc();
#else
    thr_createkey(&key);
#endif
  }
  T* operator->()
  {
    T* object;
#ifdef _WIN32
    object=(T*)TlsGetValue(key);
#else
    object=(T*)thr_get_specific(key);
#endif
    return object;
  }

  void initInstance(T* instance)
  {
    initlock.Lock();
    initiated++;
#ifdef _WIN32
    TlsSetValue(key,instance);
#else
    thr_set_specific(key,instance);
#endif
    initlock.Unlock();
  }

  T* get()
  {
    T* object;
    if(!initiated)
    {
      initlock.Lock();
      if(!initiated)
      {
        initiated++;
        object=initialValue();
        initInstance(object);
      }
      initlock.Unlock();
    }else
    {
#ifdef _WIN32
#else
#endif
    }
    return object;
  }

  virtual T* initialValue()
  {
    return 0;
  }

protected:
  int initiated;
  Mutex initlock;
#ifdef _WIN32
  DWORD key;
#else
  thread_key_t key;
#endif
};//ThreadLocal

};//synchronization
};//core
};//smsc


#endif
