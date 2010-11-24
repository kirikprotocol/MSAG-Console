#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "ThreadPool.hpp"
#include <exception>
#include <signal.h>
#include <time.h>

namespace smsc{
namespace core{
namespace threads{

int ThreadPool::PooledThread::Execute()
{
  sigset_t set;
  sigemptyset(&set);
  for (int i=1; i<=37; i++)
  {
    if ((i != SIGUSR1) && (i != SIGPROF))
      sigaddset(&set,i);
  }

  if (pthread_sigmask(SIG_SETMASK,&set,NULL) != 0)
  {
    __warning__("failed to set thread signal mask!");
  }

  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("tp");

  smsc_log_debug(log,"Pooled thread %p ready for tasks",this);
  if (!task)
      owner->releaseThread(this);
  for(;;)
  {
    trace2("Thread %p waiting for task",this);
    taskEvent.Wait();
    trace2("Thread %p got a task %s",this,task ? task->taskName() : "NOTASK" );
    if (!task)
      return 0;
    try {
      task->Execute();
    }
    catch(std::exception& e)
    {
      smsc_log_warn(log,"Exception in task %s:%s",task->taskName(),e.what());
    }
    catch(...)
    {
      smsc_log_warn(log,"Unknown exception in task:%s",task->taskName());
    }
    smsc_log_debug(log,"Execution of task %s on thread %u finished",task->taskName(),unsigned(::pthread_self()));
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
  MutexGuard  grd(lock);
  __trace2__("stopping notify tasks, pending:%d, used:%d, free:%d", pendingTasks.Count(), usedThreads.Count(), freeThreads.Count());
  for (int i=0; i<usedThreads.Count(); i++)
  {
    if (!usedThreads[i].destructing)
      usedThreads[i].ptr->stopTask();
  }
}

extern "C" typedef void (*SignalHandler)(int);


void ThreadPool::shutdown(TimeSlice::UnitType_e time_unit, long use_tmo)
{
  sigset(SIGUSR1,(SignalHandler)disp);
  TimeSlice timeout(use_tmo ? use_tmo : 1, use_tmo ? time_unit : TimeSlice::tuSecs);

  TimeSlice firstTmo;
  if (TimeSlice(10, TimeSlice::tuSecs) < timeout) {
    firstTmo = TimeSlice(1, TimeSlice::tuSecs);
  } else if (timeout < TimeSlice(10, TimeSlice::tuMSecs)) {
    firstTmo = TimeSlice(1, TimeSlice::tuMSecs);
  } else
    firstTmo = timeout/10;

  struct timespec maxTime = timeout.adjust2Nano();

  MutexGuard  grd(lock);
  if (!usedThreads.Count() && !freeThreads.Count())
    return;

  __trace2__("stopping tasks, pending:%d, used:%d, free:%d", pendingTasks.Count(), usedThreads.Count(), freeThreads.Count());
  //delete pending task first
  for (int i=0; i<pendingTasks.Count(); ++i)
  {
    if (pendingTasks[i]->delOnCompletion())
      delete pendingTasks[i];
    else
      pendingTasks[i]->onRelease();
  }
  pendingTasks.Empty();

  //stop tasks of currently active threads
  for (int i=0; i < usedThreads.Count(); ++i)
  {
    if (!usedThreads[i].destructing)
      usedThreads[i].ptr->stopTask();
  }
  __trace__("all tasks are notified");
  //wait either last released thread signal or first timeout expiration
  lock.wait(firstTmo);

   //check for unexpectedly lasting threads
  if (usedThreads.Count())
  {
    do {
      //send a signal to lasting thread in order to awake it if it's blocked
      trace2("Waiting when all threads will be finished:%d",usedThreads.Count());
      for (int i=0; i<usedThreads.Count(); ++i)
      {
        if (!usedThreads[i].destructing)
        {
          __warning2__("Unfinished task:%s",usedThreads[i].ptr->taskName());
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

void ThreadPool::preCreateThreads(int count)
{
  trace2("COUNT:%d",count);
  int n=count-usedThreads.Count()-freeThreads.Count();
  if (n < 0)
    return;
  trace2("Attempting to create %d threads(%d/%d)",n,freeThreads.Count(),usedThreads.Count());
  {
    MutexGuard  grd(lock);
    usedThreads.SetSize(count); //enlarge array of active threads
    for(int i=0; i<n ; ++i) {
      trace2("Creating thread:%d",i);
      usedThreads.Push(ThreadInfo(new PooledThread(this)));
      usedThreads[-1].ptr->Start(defaultStackSize);
    }
  }
}

void ThreadPool::startTask(ThreadedTask* task)
{
  MutexGuard  grd(lock);
  PooledThread* t;
  if (freeThreads.Count() > 0) {
    __trace__("use free thread for new task");
    freeThreads.Pop(t);
    t->assignTask(task);
    t->processTask();
    usedThreads.Push(ThreadInfo(t));
  } else {
    if(usedThreads.Count()==maxThreads) {
      __trace2__("pending task %s", task->taskName());
      pendingTasks.Push(task);
    } else {
      __trace__("creating new thread for task");
      t=new PooledThread(this);
      t->assignTask(task);
      t->Start(defaultStackSize);
      t->processTask();
      usedThreads.Push(ThreadInfo(t));
    }
  }
}

void ThreadPool::startTask(ThreadedTask* task, bool delOnCompletion)
{
  task->setDelOnCompletion(delOnCompletion);
  startTask(task);
}

void ThreadPool::releaseThread(PooledThread* thread)
{
  trace2("Releasing thread %8p",thread);
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
    trace2("Pending tasks:%d, used:%d, free:%d",pendingTasks.Count(),usedThreads.Count(),freeThreads.Count());
    if (pendingTasks.Count() > 0) {
      ThreadedTask * t;
      pendingTasks.Shift(t);
      thread->assignTask(t);
      thread->processTask();
    } else {
      usedThreads.Delete(i,1);
      freeThreads.Push(thread);
    }
    if (!usedThreads.Count()) //awake shutdown()
      lock.notify();
  }

  if (pTask && !pTask->delOnCompletion()) {
    try {
      pTask->onRelease();
    } catch (const std::exception& e ) {
      __trace2__("%s::onRelease(): exception: %s", pTask->taskName(), e.what());
    } catch (...) {
      __trace2__("%s::onRelease(): unknown exception", pTask->taskName());
    }
  }
}


}//threads
}//core
}//smsc
