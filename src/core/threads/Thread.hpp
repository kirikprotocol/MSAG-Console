#ifndef __CORE_THREADS_THREAD_HPP__
#define __CORE_THREADS_THREAD_HPP__

#ifdef _WIN32
#include <windows.h>
#undef Yield
#else
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#endif

namespace smsc{
namespace core{
namespace threads{

class Thread{
public:

  Thread();
  virtual ~Thread();

  virtual int Execute()=0;
  virtual void onStart(){}
  void Start();
  void Start(int stacksize);
  int WaitFor();
#ifndef _WIN32
  int Kill(int sig)
  {
    return pthread_kill(thread,sig);
  }
#endif
  unsigned long getThrId(void) const { return reinterpret_cast<unsigned long>(thread); }
  int getRetCode(){return retcode;}
  void setRetCode(int rc){retcode=rc;}

#ifndef _WIN32
  static void Yield()
  {
    sched_yield();
  }
#else
  static void Yield()
  {
    Sleep(0);
  }
#endif

protected:
  int retcode;
#ifdef _WIN32
  HANDLE thread;
#else
  pthread_t thread;
#endif

#ifdef _WIN32
static void ThreadRunner(void* obj);
#else
static void* ThreadRunner(void* obj);
#endif

};//Thread

}//threads
}//core
}//smsc

#endif
