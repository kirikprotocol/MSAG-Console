/* ************************************************************************* *
 * 
 * 
 * ************************************************************************* */
#ifndef __SMSC_TASK_SCHEDULER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_TASK_SCHEDULER_HPP

#include <map>
#include <list>

#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "inman/services/scheduler/TaskSchedulerDefs.hpp"
#include "logger/Logger.h"
#include "util/strlcpy.h"   // for strlcpy on linux

namespace smsc {
namespace util {

using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

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
        { cleanUp(); }

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
    bool releaseTask(TaskMap::iterator qIt);
    //NOTE: _sync must be locked upon entry
    void cleanUpReleased(void);

    //-- Thread interface methods
    int Execute(void);

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

    virtual ~TaskSchedulerAC();

    const char * Name(void) const { return _logId; }

    // ---------------------------------------
    // -- TaskSchedulerITF interface methods
    // ---------------------------------------
    bool Start(void);
    //Stops scheduler: no new task will start, sigAbort is send to existing ones.
    //If timeout is not zero, then method blocks until scheduler thread will be completed.
    void Stop(unsigned timeOut_ms = _SHUTDOWN_TIMEOUT);

    //-- ***************************************************************
    //-- TaskSchedulerITF interface methods
    //-- ***************************************************************

    //Registers and schedules task. TaskId == 0 means failure.
    TaskId StartTask(ScheduledTaskAC * use_task, TaskRefereeITF * use_ref = NULL);
    //Enqueues signal for task scheduling, returns false if signal cann't be
    //scheduled for task (unknown id, scheduler is stopp[ed/ing], etc).
    //Note:
    // 1) not all signals accepts cmd_dat argument, rcBadArg is returned in that case
    // 2) in case of failure it's a caller responsibility to utilize cmd_dat
    SchedulerRC SignalTask(TaskId task_id, PGSignal cmd = sigProc, UtilizableObjITF * cmd_dat = NULL);
    //Attempts to immediately abort given task
    SchedulerRC AbortTask(TaskId task_id);
    //Sets referee for task, returns rcBadArg if other referee is already set
    SchedulerRC RefTask(TaskId task_id, TaskRefereeITF * use_ref);
    //Cancels referee for task, returns false if given referee is already
    //targeted by task for reporting
    bool   UnrefTask(TaskId task_id, TaskRefereeITF * use_ref);
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

        bool hasProcData(void) const
        {
            return !procData.empty();
        }
        void addProcData(UtilizableObjITF * use_dat)
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

        // --------------------------------
        // -- TaskDataAC interface methods
        // --------------------------------
        //Cleans extra resources associated eith task: postponed sigProc data
        void cleanUp(void)
        {
            while (!procData.empty()) {
              UtilizableObjITF * pObj = procData.front();
              if (pObj)
                  pObj->Utilize();
              procData.pop_front();
            }
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

