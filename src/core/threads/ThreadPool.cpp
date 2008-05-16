static const char ident[] = "$Id$";

#include "ThreadPool.hpp"
#include <exception>
#include <signal.h>
#include <time.h>

namespace smsc{
namespace core{
namespace threads{

/*
void ThreadedTask::getMemoryInfo(int& rawheapsize,int& blocksheapquantum)
{
  rawheapsize=0;//MM_DEFAULT_RAWHEAP_SIZE;
  blocksheapquantum=0;//MM_BLOCKS_HEAP_QUANTUM;
};
*/


int PooledThread::Execute()
{
  int rawheapsize;
  int blocksheapquantum;
  sigset_t set;
  sigemptyset(&set);
  for(int i=1;i<=37;i++)if(i!=16 && i!=SIGPROF)sigaddset(&set,i);

  if(pthread_sigmask(SIG_SETMASK,&set,NULL)!=0)
  {
    __warning__("failed to set thread signal mask!");
  };

  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("tp");

  smsc_log_debug(log,"Pooled thread %p ready for tasks",this);
  if (!task)
      owner->releaseThread(this);
  for(;;)
  {
    trace2("Thread %p waiting for task",this);
    taskEvent.Wait();
    trace2("Thread %p got a task",this);
    if(task==NULL)return 0;
    /*
    task->getMemoryInfo(rawheapsize,blocksheapquantum);
    if(rawheapsize!=0 || blocksheapquantum!=0)
    {
      task->assignHeap(
        owner->getMemoryHeap(
          task->taskName(),
          rawheapsize,
          blocksheapquantum
        )
      );
    }
    */
    try{
      task->Execute();
      if (!task->delOnCompletion())
          task->onRelease();
    }
    catch(std::exception& e)
    {
      smsc_log_warn(log,"Exception in task %s:%s",task->taskName(),e.what());
    }
    catch(...)
    {
      smsc_log_warn(log,"Unknown exception in task:%s",task->taskName());
    }
    smsc_log_info(log,"Execution of task %s finished",task->taskName());
    //task->releaseHeap();
    owner->releaseThread(this);
  }
}


ThreadPool::ThreadPool()
{
  defaultStackSize=4096*1024;
  maxThreads=256;
}

static void disp(int sig)
{
//  __trace__("got user signal");
}

ThreadPool::~ThreadPool()
{
  shutdown();
  __trace__("ThreadPool destroyed");
}

void ThreadPool::stopNotify()
{
  Lock();
  __trace__("stopping notify tasks");
  for (int i=0; i<usedThreads.Count(); i++) {
      if (!usedThreads[i].destructing)
          usedThreads[i].ptr->stopTask();
  }
  Unlock();
}

extern "C" typedef void (*SignalHandler)(int);

void ThreadPool::shutdown(uint32_t timeout)
{
  sigset(SIGUSR1,(SignalHandler)disp);
  Lock();
  if(usedThreads.Count()==0 && freeThreads.Count()==0)
  {
    Unlock();
    return;
  }
  __trace__("stopping tasks");
  for(int i=0;i<pendingTasks.Count();i++)
  {
    if (pendingTasks[i]->delOnCompletion())
    {
      delete pendingTasks[i];
    }else
    {
      pendingTasks[i]->onRelease();
    }
  }
  pendingTasks.Empty();

  for (int i=0; i < usedThreads.Count(); i++)
  {
      if (!usedThreads[i].destructing)
          usedThreads[i].ptr->stopTask();
  }
  __trace__("all tasks are notified");
  Unlock();
#ifdef linux
      typedef timespec timestruc_t;
#endif
  timestruc_t tv={0,1000000};
  nanosleep(&tv,0);
  time_t sdstart=time(NULL);
  for(;;)
  {
    Lock();
    trace2("Waiting when all threads will be finished:%d",usedThreads.Count());
    if(usedThreads.Count()==0)
    {
      Unlock();
      break;
    }
    for(int i=0;i<usedThreads.Count();i++)
    {
        if (!usedThreads[i].destructing) {
          __warning2__("Unfinished task:%s",usedThreads[i].ptr->taskName());
          usedThreads[i].ptr->stopTask();
          usedThreads[i].ptr->Kill(16);
        }
    }
    Unlock();
#ifndef LEAKTRACE
    if(timeout && time(NULL)-sdstart>timeout)abort();
#endif
    Wait();
  }
  Lock();
  for(int i=0;i<freeThreads.Count();i++)
  {
    freeThreads[i]->assignTask(NULL);
    freeThreads[i]->processTask();
    freeThreads[i]->WaitFor();
    delete freeThreads[i];
  }
  freeThreads.Clean();
  Unlock();
}

MemoryHeap* ThreadPool::getMemoryHeap(const char* taskname,int rawheapsize,int blocksheapquantum)
{
  return 0;//mm.acquireHeap(taskname,rawheapsize,blocksheapquantum);
}

void ThreadPool::preCreateThreads(int count)
{
  trace2("COUNT:%d",count);
  int n=count-usedThreads.Count()-freeThreads.Count();
  trace2("Attempting to create %d threads(%d/%d)",n,freeThreads.Count(),usedThreads.Count());
  //mm.preallocateHeaps(count);
  Lock();
  usedThreads.SetSize(count);
  int i;
  for(i=0;i<n;i++)
  {
    trace2("Creating thread:%d",i);
    usedThreads.Push(ThreadInfo(new PooledThread(this)));
    usedThreads[-1].ptr->Start(defaultStackSize);
  }
  Unlock();
}

void ThreadPool::startTask(ThreadedTask* task, bool delOnCompletion/* = true*/)
{
  Lock();
  task->setDelOnCompletion(delOnCompletion);
  PooledThread* t;
  if(freeThreads.Count()>0)
  {
    __trace__("use free thread for new task");
    freeThreads.Pop(t);
    t->assignTask(task);
    t->processTask();
    usedThreads.Push(ThreadInfo(t));
  }else
  {
    if(usedThreads.Count()==maxThreads)
    {
      __trace__("pending task");
      pendingTasks.Push(task);
    }else
    {
      __trace__("creating new thread for task");
      t=new PooledThread(this);
      t->assignTask(task);
      t->Start(defaultStackSize);
      t->processTask();
      usedThreads.Push(ThreadInfo(t));
    }
  }
  Unlock();
}

void ThreadPool::releaseThread(PooledThread* thread)
{
  trace2("Releasing thread %8p",thread);
  int   i = 0;

  Lock();
  //NOTE: no check for return value, because of in case of
  //findUsed() failure only core dump analyzis will help :)
  findUsed(thread, i);

  //destroy task if necessary
  ThreadedTask* pTask = thread->releaseTask();
  if (pTask && pTask->delOnCompletion()) {
      usedThreads[i].destructing = true;
      Unlock();
      delete pTask; //may lasts rather long time
      Lock();
      findUsed(thread, i);
      usedThreads[i].destructing = false;
  }
  //assign next task from queue of pending ones
  trace2("Pending tasks:%d",pendingTasks.Count());
  if (pendingTasks.Count() > 0) {
      ThreadedTask * t;
      pendingTasks.Shift(t);
      thread->assignTask(t);
      thread->processTask();
  } else {
      usedThreads.Delete(i,1);
      freeThreads.Push(thread);
  }
  Unlock();
}


}//threads
}//core
}//smsc
