#pragma ident "$Id$"
/* ************************************************************************* *
 * 
 * 
 * ************************************************************************* */
#ifndef __SMSC_TASK_SCHEDULER_HPP
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

namespace smsc {
namespace util {

class TaskSchedulerAC : public TaskSchedulerITF, Thread {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

protected:
    static const unsigned _MAX_IDALLOC_ATTEMPT = 5;
    static const unsigned _MAX_LOG_ID_LEN = 32;
    
    // Task processing data
    class TaskDataAC {
    public:
        ScheduledTaskAC * ptr;
        //referee data
        TaskRefereeITF * referee;
        bool            targeted;   //referee is targeted by task
        bool            cancelled;  //referee is cancelled

        TaskDataAC(ScheduledTaskAC * p_task, TaskRefereeITF * use_ref = NULL)
            : ptr(p_task), referee(use_ref), targeted(false), cancelled(false)
        { }
        virtual ~TaskDataAC()
        { }

        TaskRefereeITF * markReferee(void)
        {
            if (referee && !cancelled) {
                targeted = true;
                return referee;
            }
            return NULL;
        }
        inline void unmarkReferee(void) { targeted = false; }
        //returns true if given referee is already targeted by task
        bool cancelReferee(void)
        {
            if (targeted)
                cancelled = true;
            else
                referee = NULL;
            return targeted;
        }

        inline bool toDelete(void) { return (!targeted && (!referee || cancelled)); }
    };
    typedef std::map<TaskId, TaskDataAC *> TaskMap;
    typedef std::pair<TaskId, PGSignal> TaskSignal;
    typedef std::list<TaskSignal> SignalsQueue;
    typedef std::list<TaskId> TaskQueue;

    typedef enum { schdStopped = 0, schdStopping, schdRunning } SchedulerState;

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
    virtual void processSignal(bool doAbort = false) = 0;
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
            ScheduledTaskAC * pDel = pTask->ptr->DelOnCompletion() ? pTask->ptr : NULL;
            smsc_log_debug(logger, "%s: releasing %s", _logId, pTask->ptr->TaskName());
            delete pTask;
            qPool.erase(qIt);
            if (pDel) {
                _sync.Unlock();
                delete pDel;
                _sync.Lock();
            }
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
        _sync.notify();
        while (_state != schdStopped) {
            cleanUpReleased(); //Free released tasks
            if (!qSignaled.empty()) {   //Process tasks signals
                do {
                    processSignal(doAbort);
                } while (!qSignaled.empty());
            }
            if (_state == schdStopping) {
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
        : _lastId(0), _state(schdStopped), logger(use_log)
    { 
        if (!logger)
            logger = Logger::getInstance("smsc.inman.SchedAC");
        strlcpy(_logId, log_id, sizeof(_logId));
    }

    virtual ~TaskSchedulerAC()
    {
        Stop();
        _sync.Lock();
        if (!qPool.empty()) {
            do {
                TaskMap::iterator it = qPool.begin();
                ScheduledTaskAC * pTask = it->second->ptr->DelOnCompletion() ?
                                                        it->second->ptr : NULL;
                delete it->second;
                qPool.erase(it);
                if (pTask) {
                    _sync.Unlock();
                    delete pTask;
                    _sync.Lock();
                }
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


    //-- TaskSchedulerITF interface methods
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
    //Enqueue signal for task scheduling
    void SignalTask(TaskId task_id, PGSignal cmd = sigProc)
    {
        MutexGuard  tmp(_sync);
        if (cmd == sigAbort)
            qSignaled.push_front(TaskSignal(task_id, cmd));
        else
            qSignaled.push_back(TaskSignal(task_id, cmd));
        _sync.notify();
    }

    //Cancels referee for task, returns true if given referee is already targeted by task
    bool   UnrefTask(TaskId task_id, TaskRefereeITF * use_ref)
    {
        MutexGuard  tmp(_sync);
        TaskMap::iterator it = qPool.find(task_id);
        if (it == qPool.end())
            return false;
        
        TaskDataAC * pTask = it->second;
        if (pTask->referee != use_ref)
            return false;
        _sync.notify();
        return pTask->cancelReferee();
    }
    //Report task to referee if it's active
    void   ReportTask(TaskId task_id)
    {
        TaskDataAC * pTask = NULL;
        TaskRefereeITF * useRef = NULL;
        {
            MutexGuard  tmp(_sync);
            TaskMap::iterator it = qPool.find(task_id);
            if (it == qPool.end())
                return;
            pTask = it->second;
            useRef = pTask->markReferee();
            if (!useRef)
                return;
        }
        useRef->onTaskReport(this, pTask->ptr);
        {
            MutexGuard  tmp(_sync);
            pTask->unmarkReferee();
        }
        return;
    }
};

//shedMT (parallel mode) implementation of scheduler:
//all tasks are executed simultaneously
class TaskSchedulerMT : public TaskSchedulerAC {
protected:
    //-- TaskSchedulerAC interface methods
    void processSignal(bool doAbort = false);

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

    class TaskDataSEQ : public TaskDataAC {
    public:
        TaskQueue *  pQueue;

        TaskDataSEQ(ScheduledTaskAC * p_task, TaskRefereeITF * use_ref = NULL)
            : TaskDataAC(p_task, use_ref), pQueue(0)
        { }
        ~TaskDataSEQ()
        { }
    };

    TaskSchedule     schdMap;

    //Removes task from associated scheduling queue, if given task is the top one,
    //prepares next task for activation.
    //Returns false if task was not found in any of processing queues
    bool Unqueue(TaskMap::iterator qIt);

    //-- TaskSchedulerAC interface methods
    void processSignal(bool doAbort = false);

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

