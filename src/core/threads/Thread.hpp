#ifndef __CORE_THREADS_THREAD_HPP__
#define __CORE_THREADS_THREAD_HPP__

#ifdef _WIN32
#include <windows.h>
#else
#include <thread.h>
#endif

namespace smsc{
namespace core{
namespace threads{

class Thread{
public:

  Thread();
  virtual ~Thread();

  virtual int Execute()=0;
  void Start();
  void Start(int stacksize);
  int WaitFor();

protected:
#ifdef _WIN32
  HANDLE thread;
  DWORD threadid;
#else
  thread_t thread;
#endif
};//Thread

};//threads
};//core
};//smsc

#endif
