#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <exception>
#include <signal.h>
#include <time.h>

#include "core/threads/ThreadPool.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc {
namespace core {
namespace threads {
/* ************************************************************************** *
 * class ThreadPool::PooledThread implementation:
 * ************************************************************************** */
ThreadPool::PooledThread::PooledThread(ThreadPool * new_owner, Logger * tp_log/* = NULL*/)
  : Thread(), _tpLogger(tp_log), owner(new_owner), task(NULL)
{
  if (!_tpLogger)
    _tpLogger = Logger::getInstance(ThreadPool::_dflt_log_category);
}

int ThreadPool::PooledThread::Execute()
{
  //establish proper thread signal's handling
  {
    sigset_t sigMask;
    sigfillset(&sigMask);
    sigdelset(&sigMask, SIGUSR1);
    sigdelset(&sigMask, SIGPROF);
    if (pthread_sigmask(SIG_SETMASK, &sigMask, NULL) != 0) {
      smsc_log_error(_tpLogger, "Thread[%lu](%p): failed to set thread signal mask!", getThrId(), this);
    }
  }

  smsc_log_debug(_tpLogger, "Thread[%lu](%p): ready for tasks", getThrId(), this);
  if (!task)
      owner->releaseThread(this);
  for(;;)
  {
    smsc_log_debug(_tpLogger, "Thread[%lu](%p): waiting for tasks", getThrId(), this);
    taskEvent.Wait();
    smsc_log_debug(_tpLogger, "Thread[%lu](%p): got a task(%s)", getThrId(), this,
                   task ? task->taskName() : "NOTASK");
    if (!task)
      return 0;
    try {
      task->Execute();
    } catch(const std::exception& e) {
      smsc_log_error(_tpLogger, "Thread[%lu](%p): exception in task(%s): %s", getThrId(), this,
                     task->taskName(), e.what());
    } catch(...) {
      smsc_log_error(_tpLogger, "Thread[%lu](%p): exception in task(%s): <unknown>", getThrId(), this,
                     task->taskName());
    }
    smsc_log_debug(_tpLogger, "Thread[%lu](%p): task(%s) is finished", getThrId(), this,
                   task->taskName());
    owner->releaseThread(this);
  }
}

/* ************************************************************************** *
 * class ThreadPool implementation:
 * ************************************************************************** */
const char * ThreadPool::_dflt_log_category = "tp";

ThreadPool::ThreadPool(Logger * use_log/* = NULL*/)
  : _tpLogger(use_log), defaultStackSize(4096*1024), maxThreads(256)
{
  if (!_tpLogger)
    _tpLogger = Logger::getInstance(ThreadPool::_dflt_log_category);
}

static void disp(int sig)
{
//  __trace__("got user signal");
}

ThreadPool::~ThreadPool()
{
  shutdown();
  smsc_log_debug(_tpLogger, "ThreadPool(%p) is destroyed", this);
}

bool ThreadPool::isRunning(void) const
{
  MutexGuard grd(lock);
  return (usedThreads.Count() || freeThreads.Count());
}

int ThreadPool::getPendingTasksCount() const
{
  MutexGuard mg(lock);
  return pendingTasks.Count();
}

//Returns number of threads executing tasks currently.
int ThreadPool::getActiveThreads(void) const
{
  MutexGuard mg(lock);
  return usedThreads.Count();
}

void ThreadPool::stopNotify()
{
  MutexGuard  grd(lock);
  smsc_log_debug(_tpLogger, "ThreadPool(%p): stopping "
                "pendingTasks: %d, threads: used %d, idle %d", this,
                 pendingTasks.Count(), usedThreads.Count(), freeThreads.Count());

  for (int i=0; i<usedThreads.Count(); i++)
  {
    if (!usedThreads[i].destructing)
      usedThreads[i].ptr->stopTask();
  }
}

extern "C" typedef void (*SignalHandler)(int);


void ThreadPool::shutdown(TimeSlice::UnitType_e time_unit, long use_tmo)
{
  TimeSlice timeout(use_tmo ? use_tmo : 1, use_tmo ? time_unit : TimeSlice::tuSecs);
  struct timespec maxTime = timeout.adjust2Nano();

  MutexGuard  grd(lock);
  smsc_log_debug(_tpLogger, "ThreadPool(%p): shutdowning "
                "pendingTasks: %d, threads: used %d, idle %d", this,
                 pendingTasks.Count(), usedThreads.Count(), freeThreads.Count());

  //delete pending task first
  for (int i=0; i<pendingTasks.Count(); ++i)
  {
    if (pendingTasks[i]->delOnCompletion())
      delete pendingTasks[i];
    else
      pendingTasks[i]->onRelease();
  }
  pendingTasks.Empty();

  if (!usedThreads.Count() && !freeThreads.Count())
    return;

  //stop tasks of currently active threads
  for (int i=0; i < usedThreads.Count(); ++i)
  {
    if (!usedThreads[i].destructing)
      usedThreads[i].ptr->stopTask();
  }
  smsc_log_debug(_tpLogger, "ThreadPool(%p): all tasks are notified", this);

  //wait either last released thread signal or first timeout expiration
  if (usedThreads.Count()) {
    TimeSlice firstTmo;
    if (TimeSlice(10, TimeSlice::tuSecs) < timeout) {
      firstTmo = TimeSlice(1, TimeSlice::tuSecs);
    } else if (timeout < TimeSlice(10, TimeSlice::tuMSecs)) {
      firstTmo = TimeSlice(1, TimeSlice::tuMSecs);
    } else
      firstTmo = timeout/10;

    lock.wait(firstTmo);
  }

   //check for unexpectedly lasting threads
  if (usedThreads.Count())
  {
    sigset(SIGUSR1,(SignalHandler)disp);
    do {
      smsc_log_debug(_tpLogger, "ThreadPool(%p): waiting for %d tasks to complete",
                     this, usedThreads.Count());
      //send a signal to lasting thread in order to awake it if it's blocked
      for (int i=0; i<usedThreads.Count(); ++i)
      {
        if (!usedThreads[i].destructing)
        {
          smsc_log_warn(_tpLogger, "ThreadPool(%p): unfinished tasks(%s)",
                         this, usedThreads[i].ptr->taskName());
          usedThreads[i].ptr->stopTask();
          usedThreads[i].ptr->Kill(SIGUSR1);
        }
      }
      //wait up to maximum allowed time
      lock.wait(maxTime);
      if (!use_tmo) //shift maximum allowed time
        maxTime = timeout.adjust2Nano();
      /**/
    } while (!use_tmo && usedThreads.Count());
  }
#ifndef LEAKTRACE
  if (usedThreads.Count())
      abort(); //generate core dump
#endif

  //finish unused threads
  for(int i=0; i<freeThreads.Count(); ++i)
  {
    freeThreads[i]->assignTask(NULL);
    freeThreads[i]->processTask();
    freeThreads[i]->WaitFor();
    delete freeThreads[i];
  }
  freeThreads.Clean();
}

//Returns number of successfully created threads
int ThreadPool::preCreateThreads(int req_count)
{
  MutexGuard  grd(lock);
  int iniCnt = usedThreads.Count() + freeThreads.Count();
  int n = req_count - iniCnt;
  if (n < 0)
    return req_count;

  smsc_log_debug(_tpLogger, "ThreadPool(%p): attempting "
                 "to create %d threads (idle: %d, used: %d)",
                 this, n, freeThreads.Count(), usedThreads.Count());
    
  usedThreads.SetSize(req_count); //enlarge array of active threads
  for(int i = 0; i < n ; ++i) {
    PooledThread * nThr = allcThread();
    if (!nThr)
      return (iniCnt += i);
  }
  return req_count;
}

bool ThreadPool::startTask(ThreadedTask* task)
{
  MutexGuard  grd(lock);
  PooledThread* t;
  if (freeThreads.Count() > 0) {
    freeThreads.Pop(t);
    smsc_log_debug(_tpLogger, "ThreadPool(%p): assigning task(%s) to idle Thread[%lu](%p)",
                   this, task->taskName(), t->getThrId(), t);
    t->assignTask(task);
    t->processTask();
    usedThreads.Push(ThreadInfo(t));
  } else {
    if (usedThreads.Count() == maxThreads) {
      smsc_log_debug(_tpLogger, "ThreadPool(%p): assigning task(%s) to pending queue",
                     this, task->taskName());
      pendingTasks.Push(task);
    } else {
      if (!(t = allcThread(task)))
        return false;
      smsc_log_debug(_tpLogger, "ThreadPool(%p): assigning task(%s) to new Thread[%lu](%p)",
                     this, task->taskName(), t->getThrId(), t);
      t->processTask();
    }
  }
  return true;
}

bool ThreadPool::startTask(ThreadedTask* task, bool delOnCompletion)
{
  task->setDelOnCompletion(delOnCompletion);
  return startTask(task);
}

void ThreadPool::releaseThread(PooledThread * thread)
{
  smsc_log_debug(_tpLogger, "ThreadPool(%p): releasing Thread[%lu](%p)",
                 this, thread->getThrId(), thread);
  int   i = 0;
  ThreadedTask * pTask = 0;
  {
    MutexGuard  grd(lock);
    //NOTE: no check for return value, because of in case of
    //findUsed() failure only core dump analyzis will help :)
    findUsed(thread, i);

    //destroy task if necessary
    pTask = thread->releaseTask();
    if (pTask && pTask->delOnCompletion()) {
      usedThreads[i].destructing = true;
      {
        ReverseMutexGuard rgrd(lock);
        delete pTask; //may lasts rather long time
        pTask = 0;
      }
      findUsed(thread, i);
      usedThreads[i].destructing = false;
    }
    //assign next task from queue of pending ones
    if (pendingTasks.Count() > 0) {
      ThreadedTask * t;
      pendingTasks.Shift(t);
      smsc_log_debug(_tpLogger, "ThreadPool(%p): assigning task(%s) to idle Thread[%lu](%p)",
                     this, t->taskName(), thread->getThrId(), thread);
      thread->assignTask(t);
      thread->processTask();
    } else {
      usedThreads.Delete(i,1);
      freeThreads.Push(thread);
      smsc_log_debug(_tpLogger, "ThreadPool(%p): "
                    "pendingTasks: %d, threads: used %d, idle %d", this,
                     pendingTasks.Count(), usedThreads.Count(), freeThreads.Count());
    }
    if (!usedThreads.Count()) //awake shutdown()
      lock.notify();
  }

  if (pTask && !pTask->delOnCompletion()) {
    try {
      pTask->onRelease();
    } catch (const std::exception& e ) {
      smsc_log_error(_tpLogger, "ThreadPool(%p): task(%s) onRelease() exception: %s",
                     this, pTask->taskName(), e.what());
    } catch (...) {
      smsc_log_error(_tpLogger, "ThreadPool(%p): task(%s) onRelease() exception: <unknown>",
                     this, pTask->taskName());
    }
  }
}

bool ThreadPool::findUsed(PooledThread * thread, int & idx)
{
  for (int i = 0; i < usedThreads.Count(); ++i) {
    if (usedThreads[i].ptr == thread) {
      idx = i; return true;
    }
  }
  return false;
}

ThreadPool::PooledThread * ThreadPool::allcThread(ThreadedTask * use_task/* = NULL*/)
{
  PooledThread * res = NULL;
  std::auto_ptr<PooledThread> nThr(new PooledThread(this, _tpLogger));

  nThr->assignTask(use_task);
  nThr->Start(defaultStackSize);
  if (nThr->getThrId()) {
    usedThreads.Push(ThreadInfo(res = nThr.release()));
    smsc_log_debug(_tpLogger, "ThreadPool(%p): created Thread[%lu](%p)",
                   this, res->getThrId(), res);
  } else {
    smsc_log_error(_tpLogger, "ThreadPool(%p): failed to create thread #%d",
                   this, usedThreads.Count() + freeThreads.Count() + 1);
  }
  return res;
}



}//threads
}//core
}//smsc
