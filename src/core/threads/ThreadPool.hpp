#ifndef __CORE_THREADS_THREADPOOL_HPP__
#ifndef __GNUC__
#ident "$Id$"
#endif
#define __CORE_THREADS_THREADPOOL_HPP__

#include "core/threads/Thread.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"
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

class ThreadPool{
protected:
  class PooledThread : public Thread {
  public:
    PooledThread(ThreadPool* newowner)
      : Thread(), owner(newowner), task(NULL)
    { }

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
      return task ? task->taskName() : "";
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
  };

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
  void startTask(ThreadedTask* task, bool delOnCompletion);

  void releaseThread(PooledThread* thread);

  void stopNotify();
  void shutdown(TimeSlice::UnitType_e time_unit, long use_tmo);
  void shutdown(uint32_t timeout_secs = 180)
  {
    shutdown(TimeSlice::tuSecs, timeout_secs);
  }

  int getPendingTasksCount() const
  {
    MutexGuard mg(lock);
    return pendingTasks.Count();
  }
  //Returns number of threads executing tasks currently.
  int getActiveThreads(void) const
  {
    MutexGuard mg(lock);
    return usedThreads.Count();
  }

protected:
  EventMonitor & getSync(void) const { return lock; }

private:
  mutable EventMonitor lock;

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

#endif /* __CORE_THREADS_THREADPOOL_HPP__ */

