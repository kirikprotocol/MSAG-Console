/* ************************************************************************* *
 * 
 * 
 * ************************************************************************* */
#ifndef __SMSC_TASK_SCHEDULER_HPP
#ident "@(#)$Id$"
#define __SMSC_TASK_SCHEDULER_HPP

#include <map>
#include <list>

#include "core/threads/Thread.hpp"
using smsc::core::threads::Thread;

#include "core/synchronization/Event.hpp"
using smsc::core::synchronization::Event;

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "inman/services/scheduler/TaskSchedulerDefs.hpp"

#include "util/strlcpy.h"   // for strlcpy on linux

namespace smsc {
namespace util {

class TaskSchedulerAC : public TaskSchedulerITF, Thread {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

protected:
    static const unsigned _MAX_IDALLOC_ATTEMPT = 5;
    static const unsigned _MAX_LOG_ID_LEN = 32;

    enum SchedulerState { schdStopped = 0, schdStopping, schdRunning };
    enum TaskAction { taskIgnore = 0, taskAborting, taskProcessing, taskReporting };

    static const char * nmTAction(TaskAction cmd)
    {
        switch (cmd) {
        case taskAborting:      return "aborting";
        case taskProcessing:    return "processing";
        case taskReporting:     return "reporting";
        default:;
        }
        return "ignoring";
    }

    static TaskAction signal2TAction(PGSignal use_sig)
    {
        switch (use_sig) {
        case TaskSchedulerITF::sigAbort:    return taskAborting;
        case TaskSchedulerITF::sigProc:     return taskProcessing;
        case TaskSchedulerITF::sigReport:   return taskReporting;
        default:;
        }
        return taskIgnore;
    }
    static const char * nmTActionBySignal(PGSignal use_sig)
    {
        return nmTAction(signal2TAction(use_sig));
    }


    // Task housekeeping data
    class TaskDataAC {
    protected:
        //referee data
        TaskRefereeITF * referee;
        bool            targeted;   //referee is targeted by task
        bool            cancelled;  //referee is cancelled

    public:
        ScheduledTaskAC * ptr;      //designated task

        TaskDataAC(ScheduledTaskAC * p_task, TaskRefereeITF * use_ref = NULL)
            : referee(use_ref), targeted(false), cancelled(false), ptr(p_task)
        { }
        virtual ~TaskDataAC()
        { }

        //Cleans extra resources associated with task
        virtual void cleanUp(void) { return; }

        //Checks if task has given referee set up
        bool checkReferee(TaskRefereeITF * use_ref) const
        {
            return referee == use_ref;
        }

        //Sets referee for task, returns false if other referee is already set
        bool setReferee(TaskRefereeITF * use_ref)
        {
            if (referee)
                return false;
            referee = use_ref;
            return true;
        }
        TaskRefereeITF * markReferee(void)
        {
            if (referee && !cancelled) {
                targeted = true;
                return referee;
            }
            return NULL;
        }
        void unmarkReferee(void)
        {
            targeted = false;
            if (cancelled)
                referee = NULL;
        }
        //returns false if given referee is already targeted by task
        bool cancelReferee(void)
        {
            if (targeted)
                cancelled = true;
            else
                referee = NULL;
            return !targeted;
        }

        bool toDelete(void) const { return (!targeted && (!referee || cancelled)); }
    };
    typedef std::map<TaskId, TaskDataAC *> TaskMap;
    struct TaskSignal {
        TaskId      tId;        //task Id
        PGSignal    sId;        //signal id
        UtilizableObjITF *sObj; //signal argument to pass to task

        TaskSignal(TaskId use_tid, PGSignal use_sig, UtilizableObjITF * use_dat = 0)
            : tId(use_tid), sId(use_sig), sObj(use_dat)
        { }
    };
    typedef std::list<TaskSignal> SignalsQueue;
    typedef std::list<TaskId> TaskQueue;

    mutable EventMonitor    _sync;
    volatile SchedulerState _state;
    Event           _stopped;
    TaskId          _lastId;
    TaskMap         qPool;
    SignalsQueue    qSignaled;
    TaskQueue       qReleased;
    Logger *        logger;
    //prefix for logging info
    char            _logId[_MAX_LOG_ID_LEN + 1];

    // ------------------------------------
    // -- TaskSchedulerAC interface methods
    // ------------------------------------
    //Processes the first signal from signals queue (schedules associated task)
    //NOTE: _sync is locked upon entry and must be locked upon return
    virtual void processSignal(void) = 0;
    //initializes task processing data for putting task into pool
    virtual TaskDataAC * initTaskData(ScheduledTaskAC * use_task,
                                        TaskRefereeITF * use_ref = NULL)
    {
        return new TaskDataAC(use_task, use_ref);
    }

    //NOTE: _sync must be locked upon entry
    bool releaseTask(TaskMap::iterator qIt)
    {
        TaskDataAC * pTask = qIt->second;
        if (pTask->toDelete()) {
            smsc_log_debug(logger, "%s: releasing %s", _logId, pTask->ptr->TaskName());
            ScheduledTaskAC * pDel = pTask->ptr;
            delete pTask;
            qPool.erase(qIt);
            _sync.Unlock();
            pDel->Utilize();
            _sync.Lock();
            return true;
        }
        smsc_log_debug(logger, "%s: postponing release of %s", _logId, pTask->ptr->TaskName());
        return false;
    }
    //NOTE: _sync must be locked upon entry
    void cleanUpReleased(void)
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
    int Execute(void)
    {
        bool doAbort = false;
        smsc_log_debug(logger, "%s: started.", _logId);
        _sync.Lock();
        _state = schdRunning;
        _sync.notify(); //if Start() waits for _sync, it will be awaked later
        while (_state != schdStopped) {
            cleanUpReleased();          //Free released tasks
            if (!qSignaled.empty()) {   //Process tasks signals
                do {
                    processSignal();
                } while (!qSignaled.empty());
            }
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

public:
    static const unsigned _TIMEOUT_STEP = 100; //inactivity timeout, millisecs
    static const unsigned _SHUTDOWN_TIMEOUT = (_TIMEOUT_STEP*4);

    TaskSchedulerAC(const char * log_id, Logger * use_log = NULL)
        : _state(schdStopped), _lastId(0), logger(use_log)
    { 
        if (!logger)
            logger = Logger::getInstance("smsc.util.TSchedAC");
        strlcpy(_logId, log_id, sizeof(_logId));
    }

    virtual ~TaskSchedulerAC()
    {
        Stop();
        _sync.Lock();
        if (!qPool.empty()) {
            do {
                TaskMap::iterator it = qPool.begin();
                ScheduledTaskAC * pTask = it->second->ptr;
                it->second->cleanUp();
                delete it->second;
                qPool.erase(it);
                _sync.Unlock();
                pTask->Utilize();
                _sync.Lock();
            } while (!qPool.empty());
        }
        _sync.Unlock();
    }

    inline const char * Name(void) const { return _logId; }

    // ---------------------------------------
    // -- TaskSchedulerITF interface methods
    // ---------------------------------------
    bool Start(void)
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
    void Stop(unsigned timeOut_ms = _SHUTDOWN_TIMEOUT)
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
    TaskId StartTask(ScheduledTaskAC * use_task, TaskRefereeITF * use_ref = NULL)
    {
        MutexGuard  tmp(_sync);
        if (_state != schdRunning)
            return 0;

        TaskId  tId;
        unsigned cnt = _MAX_IDALLOC_ATTEMPT + 1;
        do {
            --cnt; tId = ++_lastId ? _lastId : ++_lastId;
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
    SchedulerRC SignalTask(TaskId task_id, PGSignal cmd = sigProc, UtilizableObjITF * cmd_dat = 0)
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
    SchedulerRC AbortTask(TaskId task_id)
    {
        MutexGuard  tmp(_sync);
        if (_state != schdRunning)
            return TaskSchedulerITF::rcSchedNotRunning;

        qSignaled.push_front(TaskSignal(task_id, sigAbort));
        _sync.notify();
        return TaskSchedulerITF::rcOk;
    }
    //Sets referee for task, returns rcBadArg if other referee is already set
    SchedulerRC RefTask(TaskId task_id, TaskRefereeITF * use_ref)
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
    bool   UnrefTask(TaskId task_id, TaskRefereeITF * use_ref)
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
};

//shedMT (parallel mode) implementation of scheduler:
//all tasks are executed simultaneously
class TaskSchedulerMT : public TaskSchedulerAC {
protected:
    //-- TaskSchedulerAC interface methods
    void processSignal(void);

public:
    TaskSchedulerMT(Logger * use_log = NULL)
        : TaskSchedulerAC("SchedMT", use_log)
    { }
    ~TaskSchedulerMT()
    { }
};

//shedSEQ (sequential mode) implementation of scheduler:
//tasks with same criterion are executed sequentially
class TaskSchedulerSEQ : public TaskSchedulerAC {
protected:
    typedef std::map<ScheduleCriterion, TaskQueue> TaskSchedule;
    typedef std::list<UtilizableObjITF*> UObjList;

    class TaskDataSEQ : public TaskDataAC {
    protected:
        UObjList    procData;   //list of objects (arguments) of postponed sigProc signals
    public:
        TaskQueue * pQueue;     //assigned scheduling queue

        TaskDataSEQ(ScheduledTaskAC * p_task, TaskRefereeITF * use_ref = NULL)
            : TaskDataAC(p_task, use_ref), pQueue(0)
        { }
        ~TaskDataSEQ()
        { }

        //Cleans extra resources associated eith task: postponed sigProc data
        void cleanUp(void)
        {
            if (!procData.empty()) {
                do {
                    UtilizableObjITF * pObj = procData.front();
                    if (pObj)
                        pObj->Utilize();
                    procData.pop_front();
                } while (!procData.empty());
            }
        }

        inline bool hasProcData(void) const
        {
            return !procData.empty();
        }
        inline void addProcData(UtilizableObjITF * use_dat)
        {
            procData.push_back(use_dat);
        }
        UtilizableObjITF * popProcData(void)
        {
            UtilizableObjITF * pObj = NULL;
            if (!procData.empty()) {
                pObj = procData.front();
                procData.pop_front();
            }
            return pObj;
        }
    };

    TaskSchedule     schdMap;

    //Removes task from associated scheduling queue, if given task is the top one,
    //prepares next task for activation.
    //Returns false if task was not found in any of processing queues
    bool Unqueue(TaskMap::iterator & qIt);

    //-- TaskSchedulerAC interface methods
    void processSignal(void);

    TaskDataAC * initTaskData(ScheduledTaskAC * use_task,
                                        TaskRefereeITF * use_ref = NULL)
    {
        return new TaskDataSEQ(use_task, use_ref);
    }

public:
    TaskSchedulerSEQ(Logger * use_log = NULL)
        : TaskSchedulerAC("SchedSQ", use_log)
    { }
    ~TaskSchedulerSEQ()
    { }
};

} //util
} //smsc
#endif /* __SMSC_TASK_SCHEDULER_HPP */

