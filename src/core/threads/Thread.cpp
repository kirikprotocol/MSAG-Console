#ifdef _WIN32
#include <process.h>
#include <windows.h>
#endif

#include "Thread.hpp"

namespace smsc{
namespace core{
namespace threads{


#ifdef _WIN32
void Thread::ThreadRunner(void* obj)
{
  DuplicateHandle(
    GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),
    &((Thread*)obj)->thread,DUPLICATE_SAME_ACCESS,FALSE,DUPLICATE_SAME_ACCESS);
  ((Thread*)obj)->setRetCode(((Thread*)obj)->Execute());
}
#else
void* Thread::ThreadRunner(void* obj)
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
  if ( _beginthread(&Thread::ThreadRunner,0,(void*)this) != -1 )
    while ( !thread ) Sleep(10);
#else
  if(pthread_create(&thread,NULL,&Thread::ThreadRunner,this)!=0)
  {
    thread=0;
  };
#endif
}

void Thread::Start(int stacksize)
{
#ifdef _WIN32
  thread=(HANDLE)_beginthread(&Thread::ThreadRunner,stacksize,(void*)this);
#else
  if(pthread_create(&thread,NULL,&Thread::ThreadRunner,this)!=0)
  {
    thread=0;
  };
#endif
}

Thread::~Thread()
{
  WaitFor();
#ifdef _WIN32
  //_endthread();
  if ( thread ) CloseHandle(thread);
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
  return pthread_join(thread,NULL);
#endif
}


}//threads
}//core
}//smsc
