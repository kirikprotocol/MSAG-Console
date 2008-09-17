#ifndef __CORE_THREADS_THREADPOOL_HPP__
#define __CORE_THREADS_THREADPOOL_HPP__
#ident "$Id$"

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
  PooledThread(ThreadPool* newowner):Thread(),owner(newowner),task(NULL) {}

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

  ThreadedTask* releaseTask(void)
  {
      ThreadedTask * tmp = task;
      task = NULL;
      return tmp;
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

  void startTask(ThreadedTask* task, bool delOnCompletion = true);

  void releaseThread(PooledThread* thread);

  void stopNotify();
  void shutdown(uint32_t timeout = 180);

  int getPendingTasksCount()
  {
    MutexGuard mg(lock);
    return pendingTasks.Count();
  }

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
//  MemoryManager mm;
  struct ThreadInfo {
      PooledThread * ptr;
      bool          destructing; //threaded task is releasing its resources
      ThreadInfo(PooledThread * use_ptr = NULL)
        : ptr(use_ptr), destructing(false)
      { }
  };
  typedef Array<PooledThread *> ThreadsArray;
  typedef Array<ThreadInfo>     ThreadInfoArray;
  typedef Array<ThreadedTask*>  TasksArray;
  ThreadsArray      freeThreads;
  ThreadInfoArray   usedThreads;
  TasksArray        pendingTasks;
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
  bool findUsed(PooledThread* thread, int & idx)
  {
    for (int i = 0; i < usedThreads.Count(); i++) {
        if (usedThreads[i].ptr == thread) {
            idx = i; return true;
        }
    }
    return false;
  }

};//ThreadPool

}//threads
}//core
}//smsc

#endif
