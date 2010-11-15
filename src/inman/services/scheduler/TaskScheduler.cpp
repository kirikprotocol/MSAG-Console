#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/scheduler/TaskScheduler.hpp"

namespace smsc {
namespace util {

using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;

/* ************************************************************************* *
 *  class TaskSchedulerAC implementation
 * ************************************************************************* */
TaskSchedulerAC::~TaskSchedulerAC()
{
  Stop();
  MutexGuard  grd(_sync);
  while (!qPool.empty()) {
    TaskMap::iterator it = qPool.begin();
    auto_ptr_utl<ScheduledTaskAC> pTask(it->second->ptr);
    delete it->second;
    qPool.erase(it);
    {
      ReverseMutexGuard  rGrd(_sync);
      pTask.reset();
    }
  }
}

//NOTE: _sync must be locked upon entry
bool TaskSchedulerAC::releaseTask(TaskMap::iterator qIt)
{
  TaskDataAC * pTask = qIt->second;
  if (pTask->toDelete()) {
    smsc_log_debug(logger, "%s: releasing %s", _logId, pTask->ptr->TaskName());
    auto_ptr_utl<ScheduledTaskAC> pDel(pTask->ptr);
    delete pTask;
    qPool.erase(qIt);
    {
      ReverseMutexGuard  rGrd(_sync);
      pDel.reset();
    }
    return true;
  }
  smsc_log_debug(logger, "%s: postponing release of %s", _logId, pTask->ptr->TaskName());
  return false;
}
//NOTE: _sync must be locked upon entry
void TaskSchedulerAC::cleanUpReleased(void)
{
  if (!qReleased.empty()) {   //Process released tasks
    TaskQueue::iterator rit = qReleased.begin();
    while (rit != qReleased.end()) {
      TaskQueue::iterator cit = rit++;
      TaskMap::iterator qIt = qPool.find(*cit);
      if ((qIt == qPool.end()) || releaseTask(qIt))
        qReleased.erase(cit);
    }
  }
}

//-- Thread interface methods
int TaskSchedulerAC::Execute(void)
{
  bool doAbort = false;
  smsc_log_debug(logger, "%s: started.", _logId);
  _sync.Lock();
  _state = schdRunning;
  _sync.notify(); //if Start() waits for _sync, it will be awaked later
  while (_state != schdStopped) {
    cleanUpReleased();          //Free released tasks
    while (!qSignaled.empty())  //Process tasks signals
      processSignal();

    if (_state == schdStopping) { //no signals can be scheduled!
      if (qPool.empty() || doAbort)
        break;
      if (!doAbort) { //abort remaining tasks
        doAbort = true;
        for (TaskMap::iterator it = qPool.begin(); it != qPool.end(); ++it)
          qSignaled.push_back(TaskSignal(it->first, sigAbort));
        continue;
      }
    }
    _sync.wait(_TIMEOUT_STEP); //unlocks and waits
  }
  _state = schdStopped;
  smsc_log_debug(logger, "%s: stopped, %u tasks pending", _logId, qPool.size());
  _stopped.Signal();
  _sync.Unlock();
  return 0;
}


// ---------------------------------------
// -- TaskSchedulerITF interface methods
// ---------------------------------------
bool TaskSchedulerAC::Start(void)
{
  {
    MutexGuard  tmp(_sync);
    if (_state == schdRunning)
      return true;
  }
  Thread::Start();
  {
    MutexGuard  tmp(_sync);
    if (_state == schdRunning)
      return true;
    _sync.wait(_TIMEOUT_STEP);
    if (_state != schdRunning) {
      smsc_log_fatal(logger, "%s: unable to start scheduler thread", _logId);
      return false;
    }
  }
  return true;
}

//Stops scheduler: no new task will start, sigAbort is send to existing ones.
//If timeout is not zero, then method blocks until scheduler thread will be completed.
void TaskSchedulerAC::Stop(unsigned timeOut_ms/* = _SHUTDOWN_TIMEOUT*/)
{
  {
    MutexGuard  tmp(_sync);
    if (_state == schdStopped)
      return;
    _state = schdStopping;
    _sync.notify();
    if (!timeOut_ms) {
      smsc_log_debug(logger, "%s: stopping ..", _logId);
      return;
    }
    smsc_log_debug(logger, "%s: stopping, timeout = %u ms ..",
                   _logId, timeOut_ms);
  }
  _stopped.Wait(timeOut_ms);
  {
    MutexGuard  tmp(_sync);
    if (_state != schdStopped) {
      _state = schdStopped;
      smsc_log_debug(logger, "%s: shutdown timeout expired, %u tasks pending",
                     _logId, qPool.size());
    }
  }
  Thread::WaitFor();
}

//-- ***************************************************************
//-- TaskSchedulerITF interface methods
//-- ***************************************************************

//Registers and schedules task. TaskId == 0 means failure.
TaskId TaskSchedulerAC::StartTask(ScheduledTaskAC * use_task, TaskRefereeITF * use_ref/* = NULL*/)
{
  MutexGuard  tmp(_sync);
  if (_state != schdRunning)
    return 0;

  TaskId  tId;
  unsigned cnt = _MAX_IDALLOC_ATTEMPT + 1;
  do {
    --cnt;
    tId = ++_lastId ? _lastId : ++_lastId;
  } while ((qPool.find(tId) != qPool.end()) && cnt);
  if (!cnt)   //unable to allocate task ID
    return 0;

  qPool[tId] = initTaskData(use_task, use_ref);
  use_task->Init(tId, this);
  qSignaled.push_back(TaskSignal(tId, sigProc));
  _sync.notify();
  return tId;
}
//Enqueues signal for task scheduling, returns false if signal cann't be
//scheduled for task (unknown id, scheduler is stopp[ed/ing], etc).
//Note:
// 1) not all signals accepts cmd_dat argument, rcBadArg is returned in that case
// 2) in case of failure it's a caller responsibility to utilize cmd_dat
TaskSchedulerAC::SchedulerRC 
TaskSchedulerAC::SignalTask(TaskId task_id, PGSignal cmd/* = sigProc*/,
                                        UtilizableObjITF * cmd_dat/* = NULL*/)
{
  MutexGuard  tmp(_sync);
  if (_state != schdRunning)
    return TaskSchedulerITF::rcSchedNotRunning;

  TaskMap::iterator it = qPool.find(task_id);
  if (it == qPool.end())
    return TaskSchedulerITF::rcUnknownTask;

  qSignaled.push_back(TaskSignal(task_id, cmd, cmd_dat));
  _sync.notify();
  return TaskSchedulerITF::rcOk;
}
//Attempts to immediately abort given task
TaskSchedulerAC::SchedulerRC TaskSchedulerAC::AbortTask(TaskId task_id)
{
  MutexGuard  tmp(_sync);
  if (_state != schdRunning)
    return TaskSchedulerITF::rcSchedNotRunning;

  qSignaled.push_front(TaskSignal(task_id, sigAbort));
  _sync.notify();
  return TaskSchedulerITF::rcOk;
}
//Sets referee for task, returns rcBadArg if other referee is already set
TaskSchedulerAC::SchedulerRC 
TaskSchedulerAC::RefTask(TaskId task_id, TaskRefereeITF * use_ref)
{
  MutexGuard  tmp(_sync);
  TaskMap::iterator it = qPool.find(task_id);
  if (it == qPool.end())
    return TaskSchedulerITF::rcUnknownTask;

  _sync.notify();
  return it->second->setReferee(use_ref) ? TaskSchedulerITF::rcOk
                                          : TaskSchedulerITF::rcBadArg;
}
//Cancels referee for task, returns false if given referee is already
//targeted by task for reporting
bool TaskSchedulerAC::UnrefTask(TaskId task_id, TaskRefereeITF * use_ref)
{
  MutexGuard  tmp(_sync);
  TaskMap::iterator it = qPool.find(task_id);
  if (it == qPool.end())
      return true;

  TaskDataAC * pTask = it->second;
  if (!pTask->checkReferee(use_ref))
      return true;    //task has another referee

  _sync.notify();
  return pTask->cancelReferee();
}


/* ************************************************************************* *
 *  class TaskSchedulerMT implementation
 * ************************************************************************* */
//Processes the first signal from signals queue (schedules associated task)
//NOTE: _sync is locked upon entry and must be locked upon return
void TaskSchedulerMT::processSignal(void)
{
    TaskDataAC * pTask = NULL;
    TaskSignal  tSig = qSignaled.front();
    auto_ptr_utl<UtilizableObjITF>  sDat(tSig.sObj);

    qSignaled.pop_front();
    TaskMap::iterator qIt = qPool.find(tSig.tId);
    if (qIt == qPool.end()) {
        smsc_log_warn(logger, "%s: task[%lu] is unknown or was aborted, signal %s",
                    _logId, tSig.tId, TaskSchedulerITF::nmPGSignal(tSig.sId));
        return;
    }
    pTask = qIt->second;
    TaskRefereeITF * pRef = NULL;
    //check if task should be activated or reported
    TaskAction tAction = signal2TAction(tSig.sId);
    if (tAction == taskReporting)
        pRef = pTask->markReferee();

    if (tAction != taskIgnore) {
        _sync.Unlock();
        //perform actions on task
        smsc_log_debug(logger, "%s: %s %s", _logId, nmTAction(tAction), pTask->ptr->TaskName());
        try {
            ScheduledTaskAC::PGState nState = 
                (tAction == taskReporting) ? pTask->ptr->Report(sDat, pRef)
                : ((tAction == taskAborting) ? pTask->ptr->Abort(sDat)
                                            : pTask->ptr->Process(sDat));
            //select further scheduling action depending on task FSM state
            if (nState == ScheduledTaskAC::pgDone)
                tSig.sId = sigRelease;
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "%s: %s exception: %s", _logId,
                        pTask->ptr->TaskName(), exc.what());
            //keep task in suspended state for easier core file analyzis :)
            tSig.sId = sigSuspend;
        } catch (...) {
            smsc_log_error(logger, "%s: %s exception: %s", _logId,
                        pTask->ptr->TaskName(), "<unknown>");
            //keep task in suspended state for easier core file analyzis :)
            tSig.sId = sigSuspend;
        }
        _sync.Lock();
    }
    if (pRef)   //task reported -> unmark referee
        pTask->unmarkReferee();
    //reschedule according to selected scheduling action
    if (tSig.sId == sigRelease) {
        if (!releaseTask(qIt)) //postponed release
            qReleased.push_back(tSig.tId);
    }
    return;
}

/* ************************************************************************* *
 *  class TaskSchedulerSEQ implementation
 * ************************************************************************* */
//Processes the first signal from signals queue (schedules associated task)
//NOTE: _sync is locked upon entry and must be locked upon return
void TaskSchedulerSEQ::processSignal(void)
{
    TaskDataSEQ * pTask = NULL;
    TaskSignal tSig = qSignaled.front();
    auto_ptr_utl<UtilizableObjITF>  sDat(tSig.sObj);

    qSignaled.pop_front();
    TaskMap::iterator qIt = qPool.find(tSig.tId);
    if (qIt == qPool.end()) {
        smsc_log_warn(logger, "%s: task[%lu] is unknown or was aborted, signal %s",
            _logId, tSig.tId, TaskSchedulerITF::nmPGSignal(tSig.sId));
        return;
    }
    pTask = static_cast<TaskDataSEQ *>(qIt->second);
    TaskRefereeITF * pRef = NULL;
    //check if task should be activated or reported
    TaskAction tAction = signal2TAction(tSig.sId);
    if (tAction == taskReporting)
        pRef = pTask->markReferee();

    if (tSig.sId == sigProc) {
        if (!pTask->pQueue) {
            //assign processing queue (task was just started)
            TaskSchedule::iterator sit = schdMap.find(pTask->ptr->Criterion());
            if (sit == schdMap.end()) {
                //create new scheduling queue
                std::pair<TaskSchedule::iterator, bool> res = schdMap.insert(
                    TaskSchedule::value_type(pTask->ptr->Criterion(), TaskQueue()));
                sit = res.first;
    //            smsc_log_debug(logger, "%s: %s is assigned to queue %s", _logId,
    //                pTask->ptr->TaskName(), pTask->ptr->Criterion().c_str());
            }
            sit->second.push_back(tSig.tId);
            pTask->pQueue = &(sit->second);
        }
        //Note: postpone the sigProc for task that is not at front of queue, it will
        //be automatically activated as moves to front.
        if (tSig.tId != pTask->pQueue->front()) {
            tAction = taskIgnore;
            tSig.sId = sigNone;
            //stores signal argument in task housekeeping data
            pTask->addProcData(tSig.sObj);
            sDat.release();
            smsc_log_debug(logger, "%s: %s is enqueued", _logId, pTask->ptr->TaskName());
        }
    }

    if (tAction != taskIgnore) {
        _sync.Unlock();
        //perform actions on task
        smsc_log_debug(logger, "%s: %s %s", _logId, nmTAction(tAction), pTask->ptr->TaskName());
        try {
            ScheduledTaskAC::PGState nState =
                (tAction == taskReporting) ? pTask->ptr->Report(sDat, pRef)
                : ((tAction == taskAborting) ? pTask->ptr->Abort(sDat)
                                            : pTask->ptr->Process(sDat));
            //select further scheduling action depending on task FSM state
            switch (nState) {
            case ScheduledTaskAC::pgDone:
                tSig.sId = sigRelease; break;
            case ScheduledTaskAC::pgSuspend:
                tSig.sId = sigSuspend; break;
            case ScheduledTaskAC::pgCont:
                tSig.sId = sigProc; break;
            default: 
                tSig.sId = sigNone;
            }
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "%s: %s exception: %s", _logId,
                        pTask->ptr->TaskName(), exc.what());
            //keep task in suspended state for easier core file analyzis :)
            tSig.sId = sigSuspend;
        } catch (...) {
            smsc_log_error(logger, "%s: %s exception: %s", _logId,
                        pTask->ptr->TaskName(), "<unknown>");
            //keep task in suspended state for easier core file analyzis :)
            tSig.sId = sigSuspend;
        }
        _sync.Lock();
    }

    if (pRef)   //task reported -> unmark referee
        pTask->unmarkReferee();
    //reschedule according to selected scheduling action
    if (tSig.sId == sigProc) {
        if (pTask->hasProcData()) {
            smsc_log_debug(logger, "%s: rescheduling %s, procData(ON)",
                         _logId, pTask->ptr->TaskName());
            tSig.sObj = pTask->popProcData();
            qSignaled.push_back(tSig);
        }
    } else if (tSig.sId == sigRelease) {
        Unqueue(qIt);
        if (!releaseTask(qIt)) //postponed release
            qReleased.push_back(tSig.tId);
    } else if (tSig.sId == sigSuspend) {
        if (!Unqueue(qIt)) {
            smsc_log_warn(logger, "%s: %s is not in queue, signal %s",
                _logId, pTask->ptr->TaskName(), TaskSchedulerITF::nmPGSignal(tSig.sId));
        }
        smsc_log_debug(logger, "%s: %s is suspended", _logId, pTask->ptr->TaskName());
    }
    return;
}

//Removes task from associated scheduling queue, if given task is the top one,
//prepares next task for activation.
//Returns false if task was not found in any of processing queues
bool TaskSchedulerSEQ::Unqueue(TaskMap::iterator & tm_it)
{
    TaskDataSEQ * pTask = static_cast<TaskDataSEQ *>(tm_it->second);
    TaskSchedule::iterator schdIt = schdMap.find(pTask->ptr->Criterion());
    if (schdIt != schdMap.end()) {
        TaskQueue & tQueue = schdIt->second;
        pTask->pQueue = NULL;
        if (!tQueue.empty()) {
            //usually the first element is the mark
            TaskQueue::iterator it = tQueue.begin();
            if (*it == pTask->ptr->Id()) {
                tQueue.erase(it);
                //prepare next task in queue for activation
                while (!tQueue.empty()) {
                  TaskId  tId = tQueue.front();
                  TaskMap::iterator tmIt = qPool.find(tId);
                  if (tmIt == qPool.end()) {
                    smsc_log_warn(logger, "%s: task[%lu] is unknown or was aborted,"
                                          " rescheduling signal %s",
                        _logId, tId, TaskSchedulerITF::nmPGSignal(TaskSchedulerITF::sigProc));
                    tQueue.pop_front();
                  } else {
                    pTask = static_cast<TaskDataSEQ *>(tmIt->second);
                    smsc_log_debug(logger, "%s: rescheduling %s, procData(%s)",
                                   _logId, pTask->ptr->TaskName(),
                                   pTask->hasProcData() ? "ON" : "OFF");
                    qSignaled.push_back(TaskSignal(tId,
                                        TaskSchedulerITF::sigProc, pTask->popProcData()));
                    break;
                  }
                }
                if (tQueue.empty()) //destroy empty queue
                    schdMap.erase(schdIt);
                return true;
            }
            //search rest of queue
            for (++it; it != tQueue.end(); ++it) {
                if (*it == pTask->ptr->Id()) {
                    tQueue.erase(it);
                    return true;
                }
            }
        }
    }
    return false;
}

} //util
} //smsc

