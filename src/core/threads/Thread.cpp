#include "Thread.hpp"
#ifdef _WIN32
#include <process.h>
#endif

namespace smsc{
namespace core{
namespace threads{


#ifdef _WIN32
static void ThreadRunner(void* obj)
{
  ((Thread*)obj)->setRetCode(((Thread*)obj)->Execute());
}
#else
static void* ThreadRunner(void* obj)
{
  ((Thread*)obj)->setRetCode(((Thread*)obj)->Execute());
  return (void*)((Thread*)obj)->getRetCode();
}
#endif

Thread::Thread():thread(0)
{
}

void Thread::Start()
{
#ifdef _WIN32
  thread=(HANDLE)_beginthread(ThreadRunner,0,(void*)this);
#else
  if(thr_create(NULL,0,ThreadRunner,this,0,&thread)!=0)
  {
    thread=0;
  };
#endif
}

void Thread::Start(int stacksize)
{
#ifdef _WIN32
  thread=(HANDLE)_beginthread(ThreadRunner,stacksize,(void*)this);
#else
  if(thr_create(NULL,stacksize,ThreadRunner,this,0,&thread)!=0)
  {
    thread=0;
  };
#endif
}

Thread::~Thread()
{
  WaitFor();
#ifdef _WIN32
  _endthread();
#else
  //thr_destroy(thread);
#endif
}

int Thread::WaitFor()
{
  if(!thread)return 0;
#ifdef _WIN32
  return WaitForSingleObject(thread,INFINITE);
#else
  return thr_join(thread,NULL,NULL);
#endif
}


};//threads
};//core
};//smsc
