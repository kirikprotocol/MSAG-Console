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
#ifndef _WIN32
  int Kill(int sig)
  {
    return thr_kill(thread,sig);
  }
#endif
  int getRetCode(){return retcode;}
  void setRetCode(int rc){retcode=rc;}

protected:
  int retcode;
#ifdef _WIN32
  HANDLE thread;
#else
  thread_t thread;
#endif
};//Thread

};//threads
};//core
};//smsc

#endif
