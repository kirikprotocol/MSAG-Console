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
  trace2("Pooled thread ready for task\n");
  owner->releaseThread(this);
  for(;;)
  {
    trace2("Thread waiting for task\n");
    taskEvent.Wait();
    trace2("Thread got a task\n");
    if(task==NULL)return 0;
    task->getMemoryInfo(rawheapsize,blocksheapquantum);
    task->assignHeap(
      owner->getMemoryHeap(
        task->taskName(),
        rawheapsize,
        blocksheapquantum
      )
    );
    task->Execute();
    task->releaseHeap();
    owner->releaseThread(this);
    delete task;
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
    if(usedThreads.Count()==0)
    {
      Unlock();
      break;
    }
    Unlock();
    Wait();
  }
}

MemoryHeap* ThreadPool::getMemoryHeap(const char* taskname,int rawheapsize,int blocksheapquantum)
{
  return mm.acquireHeap(taskname,rawheapsize,blocksheapquantum);
}

void ThreadPool::preCreateThreads(int count)
{
  trace2("Attempting to create %d threads\n",count-usedThreads.Count());
  mm.preallocateHeaps(count);
  Lock();
  usedThreads.SetSize(count);
  int i;
  for(i=0;i<count-(usedThreads.Count()+freeThreads.Count());i++)
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
    freeThreads.Pop(t);
    t->assignTask(task);
    t->processTask();
    usedThreads.Push(t);
  }else
  {
    if(usedThreads.Count()==maxThreads)
    {
      pendingTasks.Push(task);
    }else
    {
      t=new PooledThread(this);
      t->Start(defaultStackSize);
      t->assignTask(task);
      t->processTask();
      usedThreads.Push(t);
    }
  }
  Unlock();
}

void ThreadPool::releaseThread(PooledThread* thread)
{
  Lock();
  int i;
  for(i=0;i<usedThreads.Count();i++)
  {
    if(usedThreads[i]==thread)
    {
      usedThreads.Delete(i);
      freeThreads.Push(thread);
    }
  }
  Unlock();
}


};//threads
};//core
};//smsc
