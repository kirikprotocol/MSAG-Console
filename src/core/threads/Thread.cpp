#include "Thread.hpp"

namespace smsc{
namespace core{
namespace threads{


#ifdef _WIN32
static DWORD WINAPI ThreadRunner(LPVOID obj)
{
  return ((Thread*)obj)->Execute();
}
#else
static void* ThreadRunner(void* obj)
{
  return (void*)((Thread*)obj)->Execute();
}
#endif

Thread::Thread()
{
}

void Thread::Start()
{
#ifdef _WIN32
  thread=CreateThread(NULL,0,ThreadRunner,this,0,&threadid);
#else
  thr_create(NULL,0,ThreadRunner,this,0,&thread);
#endif
}

void Thread::Start(int stacksize)
{
#ifdef _WIN32
  thread=CreateThread(NULL,stacksize,ThreadRunner,this,0,&threadid);
#else
  thr_create(NULL,stacksize,ThreadRunner,this,0,&thread);
#endif
}

Thread::~Thread()
{
  WaitFor();
#ifdef _WIN32
  CloseHandle(thread);
#else
  //thr_destroy(thread);
#endif
}

int Thread::WaitFor()
{
#ifdef _WIN32
  return WaitForSingleObject(thread,INFINITE);
#else
  return thr_join(thread,NULL,NULL);
#endif
}


};//threads
};//core
};//smsc
