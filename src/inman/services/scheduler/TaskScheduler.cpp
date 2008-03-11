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
void TaskSchedulerMT::processSignal(bool doAbort/* = false*/)
{
    TaskDataAC * pTask = NULL;
    TaskSignal  tSig = qSignaled.front();
    qSignaled.pop_front();
    TaskMap::iterator qIt = qPool.find(tSig.first);
    if (qIt == qPool.end()) {
        smsc_log_warn(logger, "%s: task[%lu] is not in pool or was aborted, signal %s",
            _logId, tSig.first, TaskSchedulerITF::nmPGSignal(tSig.second));
        return;
    }
    pTask = qIt->second;
    _sync.Unlock();

    //check if task should be activated
    bool reqAbort = (doAbort || (bool)(tSig.second == sigAbort));
    if ((tSig.second == sigProc) || (tSig.second == sigAbort)) {
        std::string nmTask(pTask->ptr->TaskName());
        smsc_log_debug(logger, "%s: %s %s", _logId, reqAbort ?
                    "aborting" : "activating", nmTask.c_str());
        try {
            if (pTask->ptr->Process(reqAbort) == ScheduledTaskAC::pgDone)
                tSig.second = sigRelease;
        } catch (std::exception& exc) {
            smsc_log_error(logger, "%s: %s[%lu] exception: %s", _logId,
                        nmTask.c_str(), tSig.first, exc.what());
            //keep task in suspended state for easier core file analyzis :)
            tSig.second = sigSuspend;
        } catch (...) {
            smsc_log_error(logger, "%s: %s[%lu] exception: %s", _logId,
                        nmTask.c_str(), tSig.first, "<unknown>");
            //keep task in suspended state for easier core file analyzis :)
            tSig.second = sigSuspend;
        }
    }
    //reschedule according to last indicated signal
    _sync.Lock();
    if (tSig.second == sigRelease) {
        if (!releaseTask(qIt)) //postponed release
            qReleased.push_back(tSig.first);
    }
    return;
}

/* ************************************************************************* *
 *  class TaskSchedulerSEQ implementation
 * ************************************************************************* */
//Processes the first signal from signals queue (schedules associated task)
//NOTE: _sync is locked upon entry and must be locked upon return
void TaskSchedulerSEQ::processSignal(bool doAbort/* = false*/)
{
    TaskDataSEQ * pTask = NULL;
    TaskSignal tSig = qSignaled.front();
    qSignaled.pop_front();
    TaskMap::iterator qIt = qPool.find(tSig.first);
    if (qIt == qPool.end()) {
        smsc_log_warn(logger, "%s: task[%lu] is not in pool or was aborted, signal %s",
            _logId, tSig.first, TaskSchedulerITF::nmPGSignal(tSig.second));
        return;
    }
    pTask = static_cast<TaskDataSEQ *>(qIt->second);
    if ((tSig.second == sigProc) && !pTask->pQueue) {
        //assign processing queue (task was just started)
        TaskSchedule::iterator sit = schdMap.find(pTask->ptr->Criterion());
        if (sit == schdMap.end()) {
            std::pair<TaskSchedule::iterator, bool> res = schdMap.insert(
                TaskSchedule::value_type(pTask->ptr->Criterion(), TaskQueue()));
            sit = res.first;
//            smsc_log_debug(logger, "%s: %s is assigned to queue %s", _logId,
//                pTask->ptr->TaskName(), pTask->ptr->Criterion().c_str());
        }
        sit->second.push_back(tSig.first);
        pTask->pQueue = &(sit->second);
    }
    _sync.Unlock();

    //check if task should be activated
    std::string nmTask(pTask->ptr->TaskName());
    bool reqAbort = (doAbort || (bool)(tSig.second == sigAbort));
    if ( ((tSig.second == sigProc)
           && (tSig.first == pTask->pQueue->front()))
        || (tSig.second == sigAbort) ) {
        smsc_log_debug(logger, "%s: %s %s", _logId, reqAbort ?
                        "aborting" : "activating", nmTask.c_str());
        try {
            switch (pTask->ptr->Process(reqAbort)) {
            case ScheduledTaskAC::pgDone:
                tSig.second = sigRelease; break;
            case ScheduledTaskAC::pgSuspend:
                tSig.second = sigSuspend; break;
            default:
                tSig.second = sigNone;
            }
        } catch (std::exception& exc) {
            smsc_log_error(logger, "%s: %s exception: %s", _logId,
                        nmTask.c_str(), exc.what());
            //keep task in suspended state for easier core file analyzis :)
            tSig.second = sigSuspend;
        } catch (...) {
            smsc_log_error(logger, "%s: %s[%lu] exception: %s", _logId,
                        nmTask.c_str(), "<unknown>");
            //keep task in suspended state for easier core file analyzis :)
            tSig.second = sigSuspend;
        }
    }
    _sync.Lock();
    //reschedule according to last indicated signal
    if (tSig.second == sigRelease) {
        Unqueue(qIt);
        if (!releaseTask(qIt)) //postponed release
            qReleased.push_back(tSig.first);
    } else if (tSig.second == sigSuspend) {
        if (!Unqueue(qIt))
            smsc_log_warn(logger, "%s: %s is not in queue, signal %s",
                _logId, nmTask.c_str(), TaskSchedulerITF::nmPGSignal(tSig.second));
        smsc_log_debug(logger, "%s: %s is suspended", _logId, nmTask.c_str());
    }
    return;
}

//Removes task from associated scheduling queue, if given task is the top one,
//prepares next task for activation.
//Returns false if task was not found in any of processing queues
bool TaskSchedulerSEQ::Unqueue(TaskMap::iterator qIt)
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
                    qSignaled.push_back(TaskSignal(schdIt->second.front(), TaskSchedulerITF::sigProc));
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

