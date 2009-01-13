#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/scheduler/TaskScheduler.hpp"

namespace smsc {
namespace util {
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
            //stores signal argument in task housekeeping data
            pTask->addProcData(tSig.sObj);
            sDat.release();
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
bool TaskSchedulerSEQ::Unqueue(TaskMap::iterator & qIt)
{
    TaskDataSEQ * pTask = static_cast<TaskDataSEQ *>(qIt->second);
    TaskSchedule::iterator schdIt = schdMap.find(pTask->ptr->Criterion());
    if (schdIt != schdMap.end()) {
        pTask->pQueue = NULL;
        if (!schdIt->second.empty()) {
            //usually the first element is the mark
            TaskQueue::iterator it = schdIt->second.begin();
            if (*it == pTask->ptr->Id()) {
                schdIt->second.erase(it);
                //prepare next task in queue for activation
                if (!schdIt->second.empty())
                    qSignaled.push_back(TaskSignal(schdIt->second.front(),
                                        TaskSchedulerITF::sigProc, pTask->popProcData()));
                else //destroy empty queue
                    schdMap.erase(schdIt);
                return true;
            }
            //search rest of queue
            for (++it; it != schdIt->second.end(); ++it) {
                if (*it == pTask->ptr->Id()) {
                    schdIt->second.erase(it);
                    return true;
                }
            }
        }
    }
    return false;
}

} //util
} //smsc

