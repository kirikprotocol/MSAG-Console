#include "ThreadPool.hpp"

namespace smsc{
namespace core{
namespace threads{

void ThreadedTask::getMemoryInfo(int& rawheapsize,int& blocksheapquantum)
{
  rawheapsize=MM_DEFAULT_RAWHEAP_SIZE;
  blocksheapquantum=MM_BLOCKS_HEAP_QUANTUM;
};


int PooledThread::Execute()
{
  int rawheapsize;
  int blocksheapquantum;
  trace2("Pooled thread %08X ready for tasks\n",this);
  if(!task)owner->releaseThread(this);
  for(;;)
  {
    trace2("Thread %08X waiting for task\n",this);
    taskEvent.Wait();
    trace2("Thread %08X got a task\n",this);
    if(task==NULL)return 0;
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
    try{
      task->Execute();
    }catch(...)
    {
    }
    trace2("Execution of task %s finished",task->taskName());
    task->releaseHeap();
    delete task;
    task=NULL;
    owner->releaseThread(this);
  }
}


ThreadPool::ThreadPool()
{
  defaultStackSize=4096*1024;
  maxThreads=256;
}

ThreadPool::~ThreadPool()
{
  for(;;)
  {
    Lock();
    trace2("Waiting when all threads will be finished:%d",usedThreads.Count());
    if(usedThreads.Count()==0)
    {
      Unlock();
      break;
    }
    Unlock();
    Wait();
  }
  for(int i=0;i<freeThreads.Count();i++)
  {
    freeThreads[i]->assignTask(NULL);
    freeThreads[i]->processTask();
    delete freeThreads[i];
  }
}

MemoryHeap* ThreadPool::getMemoryHeap(const char* taskname,int rawheapsize,int blocksheapquantum)
{
  return mm.acquireHeap(taskname,rawheapsize,blocksheapquantum);
}

void ThreadPool::preCreateThreads(int count)
{
  trace2("COUNT:%d\n",count);
  int n=count-usedThreads.Count()-freeThreads.Count();
  trace2("Attempting to create %d threads(%d/%d)\n",n,freeThreads.Count(),usedThreads.Count());
  mm.preallocateHeaps(count);
  Lock();
  usedThreads.SetSize(count);
  int i;
  for(i=0;i<n;i++)
  {
    trace2("Creating thread:%d\n",i);
    usedThreads.Push(new PooledThread(this));
    usedThreads[-1]->Start(defaultStackSize);
  }
  Unlock();
}

void ThreadPool::startTask(ThreadedTask* task)
{
  Lock();
  PooledThread* t;
  if(freeThreads.Count()>0)
  {
    trace("use free thread for new task");
    freeThreads.Pop(t);
    t->assignTask(task);
    t->processTask();
    usedThreads.Push(t);
  }else
  {
    if(usedThreads.Count()==maxThreads)
    {
      trace("pending task");
      pendingTasks.Push(task);
    }else
    {
      trace("creating new thread for task");
      t=new PooledThread(this);
      t->assignTask(task);
      t->Start(defaultStackSize);
      t->processTask();
      usedThreads.Push(t);
    }
  }
  Unlock();
}

void ThreadPool::releaseThread(PooledThread* thread)
{
  trace2("Releasing thread %08X",thread);
  Lock();
  int i;
  for(i=0;i<usedThreads.Count();i++)
  {
    if(usedThreads[i]==thread)
    {
      trace2("Pending tasks:%d",pendingTasks.Count());
      if(pendingTasks.Count()>0)
      {
        ThreadedTask *t;
        pendingTasks.Shift(t);
        thread->assignTask(t);
        thread->processTask();
        break;
      }else
      {
        usedThreads.Delete(i,1);
        freeThreads.Push(thread);
      }
      break;
    }
  }

  Unlock();
}


};//threads
};//core
};//smsc
