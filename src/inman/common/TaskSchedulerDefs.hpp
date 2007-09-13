#pragma ident "$Id$"
/* ************************************************************************* *
 * 
 * 
 * ************************************************************************* */
#ifndef __SMSC_TASK_SCHEDULER_DEFS_HPP
#define __SMSC_TASK_SCHEDULER_DEFS_HPP

namespace smsc {
namespace util {

typedef unsigned long   TaskId;
typedef std::string     ScheduleCriterion;

class ScheduledTaskAC;
class TaskRefereeITF;

class TaskSchedulerITF {
public:
    enum SchedulerType {
        schedMT = 0, //parallel mode: all tasks are executed simultaneously
        schedSEQ     //sequential mode: tasks with same criterion are executed sequentially
    };
    enum PGSignal {  //task scheduling commands
        sigNone    = 0, //do not reschedule task
        sigSuspend,     //move to suspended tasks queue for awaiting further task signal
        sigProc,        //execute task (call ScheduledTaskAC::Process())
        sigRelease,     //release task
        sigAbort        //abort task (call ScheduledTaskAC::Process() with abort flag set)
                        //highest priority signal, cancels all other signals for task
    };

    virtual TaskId StartTask(ScheduledTaskAC * use_task, TaskRefereeITF * use_ref = NULL) = 0;
    //Enqueue signal for task scheduling
    virtual void   SignalTask(TaskId task_id, PGSignal cmd = sigProc) = 0;
    //Cancels referee for task, returns true if given referee is already targeted by task
    virtual bool   UnrefTask(TaskId task_id, TaskRefereeITF * use_ref) = 0;
    //Report task to referee if it's active
    virtual void   ReportTask(TaskId task_id) = 0;

    static const char * nmPGSignal(PGSignal cmd)
    {
        switch (cmd) {
        case sigSuspend:    return "sigSuspend";
        case sigProc:       return "sigProc";
        case sigRelease:    return "sigRelease";
        case sigAbort:      return "sigAbort";
        }
        return "sigNone";
    }
};

class TaskRefereeITF {
public:
    virtual void onTaskReport(TaskSchedulerITF * sched, ScheduledTaskAC * task) = 0;
};

class TaskSchedulerFactoryITF {
public:
    virtual TaskSchedulerITF * getScheduler(TaskSchedulerITF::SchedulerType sched_type) = 0;
};


/* ScheduledTaskAC FSM:
                       - > - 
                      |     V
                      ^     |
       ------         --------
      | Idle | > - - |  Cont  | - < - 
       ------         --------       |
                        V            ^
        ------          |        ---------
       | Done | < - - < + > - > | Suspend |
        ------                   ---------
          ^                          V
          |                          |
           - - - < - - - - - < - - --
*/
class ScheduledTaskAC {
public:
    enum PGState {  //task processing graph states
        pgIdle = 0,
        pgCont,     //task continues its processing (further task signal possible)
        pgSuspend,  //task suspends its processing (further task signal possible)
        pgDone      //task is completed
    };

private:
    bool _doDel;

protected:
    TaskId  _Id;
    PGState _fsmState;
    TaskSchedulerITF * _Owner;
    ScheduleCriterion _Criterion;

public:
    ScheduledTaskAC(bool del_on_completion = true)
        : _doDel(del_on_completion), _Id(0), _fsmState(pgIdle), _Owner(0)
    { }
    virtual ~ScheduledTaskAC()
    { }
    
    static const char * nmPGState(enum PGState state)
    {
        switch (state) {
        case pgDone:    return "pgDone";
        case pgSuspend: return "pgSuspend";
        case pgCont:    return "pgCont";
        }
        return "pgIdle";
    }

    inline TaskId Id(void) const                { return _Id; }
    inline bool DelOnCompletion(void) const     { return _doDel; }
    //Criterion for assigning task to one of scheduling queues
    inline const ScheduleCriterion & Criterion(void) const { return _Criterion; }

    inline void Signal(TaskSchedulerITF::PGSignal req_signal)
                                                    { _Owner->SignalTask(_Id, req_signal); }
    inline void UnrefBy(TaskRefereeITF * use_ref)   { _Owner->UnrefTask(_Id, use_ref); }
    inline void Report(void)                        { _Owner->ReportTask(_Id); }


    //-- ScheduledTaskAC interface methods
    virtual const char * TaskName(void) const = 0;
    //NOTE: if succesor overwrites this method it MUST call this one.
    virtual void Init(TaskId use_id, TaskSchedulerITF * owner)
                                                { _Id = use_id; _Owner = owner; }
    //if do_abort is set, the only PGState the task is expected to switch to is pgDone
    virtual PGState Process(bool do_abort = false) = 0;
};

} //util
} //smsc
#endif /* __SMSC_TASK_SCHEDULER_DEFS_HPP */

