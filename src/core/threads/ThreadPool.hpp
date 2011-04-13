#ifndef __CORE_THREADS_THREADPOOL_HPP__
#ifndef __GNUC__
#ident "$Id$"
#endif
#define __CORE_THREADS_THREADPOOL_HPP__

#include "logger/Logger.h"
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

namespace smsc {
namespace core {
namespace threads {

using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;

class ThreadPool {
protected:
  class PooledThread : public Thread {
  private:
    Logger * _tpLogger; //threadPool logger

  protected:
    Event         taskEvent;
    ThreadPool *  owner;
    ThreadedTask* task;

  public:
    explicit PooledThread(ThreadPool * new_owner, Logger * tp_log = NULL);
    virtual ~PooledThread()
    { }

    // -- Thread interface methods
    virtual int Execute();

    void assignTask(ThreadedTask* newtask)
    {
        task=newtask;
        if (task) { task->onThreadPoolStartTask(); }
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
  };

  friend class PooledThread;
  void releaseThread(PooledThread* thread);

public:
  static const char * _dflt_log_category; //"tp"

  explicit ThreadPool(Logger * use_log = NULL);
  ~ThreadPool();

  //Returns number of successfully created threads
  int preCreateThreads(int req_count);

  void setDefaultStackSize(int size)
  {
    defaultStackSize=size;
  }
  void setMaxThreads(int count)
  {
    maxThreads=count;
  }

  //Returns false if task cann't be started due to resource limitation
  bool startTask(ThreadedTask* task);
  bool startTask(ThreadedTask* task, bool delOnCompletion);
  //
  void stopNotify();
  //
  void shutdown(TimeSlice::UnitType_e time_unit, long use_tmo);
  void shutdown(uint32_t timeout_secs = 180)
  {
    shutdown(TimeSlice::tuSecs, timeout_secs);
  }

  //Returns true if there is at least one thread is started.
  bool isRunning(void) const;
  //
  int getPendingTasksCount(void) const;
  //Returns number of threads executing tasks currently.
  int getActiveThreads(void) const;

protected:
  EventMonitor & getSync(void) const { return lock; }

private:
  struct ThreadInfo {
    PooledThread *  ptr;
    bool            destructing; //threaded task is releasing its resources

    explicit ThreadInfo(PooledThread * use_ptr = NULL)
      : ptr(use_ptr), destructing(false)
    { }
  };
  typedef smsc::core::buffers::Array<PooledThread *> ThreadsArray;
  typedef smsc::core::buffers::Array<ThreadInfo>     ThreadInfoArray;
  typedef smsc::core::buffers::Array<ThreadedTask*>  TasksArray;

  mutable EventMonitor  lock;
  Logger *              _tpLogger; //threadPool logger
  int                   defaultStackSize;
  int                   maxThreads;
  ThreadsArray          freeThreads;
  ThreadInfoArray       usedThreads;
  TasksArray            pendingTasks;

  bool findUsed(PooledThread * thread, int & idx);
  PooledThread * allcThread(ThreadedTask * task = NULL);

};//ThreadPool

}//threads
}//core
}//smsc

#endif /* __CORE_THREADS_THREADPOOL_HPP__ */

