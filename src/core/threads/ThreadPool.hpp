#ifndef __CORE_THREADS_THREADPOOL_HPP__
#define __CORE_THREADS_THREADPOOL_HPP__

#include "Thread.hpp"
#include "ThreadedTask.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/Array.hpp"
#ifdef _WIN32
#include <stdio.h>
#else
#include <unistd.h>
#endif

#include "util/debug.h"

namespace smsc{
namespace core{
namespace threads{

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

class ThreadPool;

class PooledThread:public Thread{
public:
  PooledThread(ThreadPool* newowner):Thread(),owner(newowner),task(NULL){}

  virtual int Execute();

  void assignTask(ThreadedTask* newtask)
  {
    task=newtask;
  }
  void processTask()
  {
    taskEvent.Signal();
  }
  void stopTask()
  {
    if(task)task->stop();
  }
  const char* taskName()
  {
    if(task)return task->taskName();else return "";
  }
protected:
  Event taskEvent;
  ThreadPool *owner;
  ThreadedTask* task;
};//PooledThread


class ThreadPool{
public:
  ThreadPool();
  ~ThreadPool();

  void preCreateThreads(int count);
  void setDefaultStackSize(int size)
  {
    defaultStackSize=size;
  }
  void setMaxThreads(int count)
  {
    maxThreads=count;
  }

  void startTask(ThreadedTask* task);

  void releaseThread(PooledThread* thread);

  void shutdown();

  MemoryHeap* getMemoryHeap(const char* taskname,int rawheapsize,int blocksheapquantum);

  void Wait()
  {
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
  }

private:
  Mutex lock;
  MemoryManager mm;
  typedef Array<PooledThread*> ThreadsArray;
  typedef Array<ThreadedTask*> TasksArray;
  ThreadsArray freeThreads;
  ThreadsArray usedThreads;
  TasksArray pendingTasks;
  int defaultStackSize;
  int maxThreads;

  void Lock()
  {
    lock.Lock();
  }
  void Unlock()
  {
    lock.Unlock();
  }

};//ThreadPool

};//threads
};//core
};//smsc

#endif
