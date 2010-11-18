/* ************************************************************************* *
 * TaskScheduler: interfaces and definitions.
 * ************************************************************************* */
#ifndef __SMSC_TASK_SCHEDULER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_TASK_SCHEDULER_DEFS_HPP

#include <string>
#include "inman/common/UtlPtr.hpp"

namespace smsc {
namespace inman {

using smsc::util::UtilizableObjITF;
using smsc::util::auto_ptr_utl;

typedef unsigned long   TaskId;
typedef std::string     ScheduleCriterion;

class ScheduledTaskAC;
class TaskRefereeITF;


class TaskSchedulerITF {
protected:
  virtual ~TaskSchedulerITF() //forbid interface destruction
  { }

public:
  enum SchedulerType {
    schedMT = 0, //parallel mode: all tasks are executed simultaneously
    schedSEQ     //sequential mode: tasks with same criterion are executed sequentially
  };
  enum PGSignal {  //task scheduling commands
    sigNone    = 0  //do not reschedule task
    // .. pure scheduling signals
    , sigRelease    //release task
    , sigSuspend    //move to suspended tasks queue for awaiting further signal
    // .. signals, which give control to task,
    //    accepts UtilizableObjITF as an argument for target task
    , sigProc       //execute task (call ScheduledTaskAC::Process())
    , sigReport     //report task to referee (call ScheduledTaskAC::Report())
    , sigAbort      //abort task (call ScheduledTaskAC::Abort()), it's a
                    //highest priority signal, cancels all other signals for task
  };
  enum SchedulerRC {
    rcOk = 0
    , rcUnknownTask
    , rcSchedNotRunning
    , rcBadArg
  };

  static const char * nmPGSignal(PGSignal cmd);
  static const char * nmRCode(SchedulerRC rc);

  //Registers and schedules task. TaskId == 0 means failure.
  virtual TaskId StartTask(ScheduledTaskAC * use_task, TaskRefereeITF * use_ref = NULL) = 0;
  //Enqueues signal for task scheduling, returns false if signal cann't be
  //scheduled for task (unknown id, scheduler is stopp[ed/ing], etc).
  //Note:
  // 1) not all signals accepts cmd_dat argument, rcBadArg is returned in that case
  // 2) in case of failure it's a caller responsibility to utilize cmd_dat
  virtual SchedulerRC  SignalTask(TaskId task_id, PGSignal cmd = sigProc, UtilizableObjITF * cmd_dat = 0) = 0;
  //Attempts to immediately abort given task
  virtual SchedulerRC  AbortTask(TaskId task_id) = 0;

  //Sets referee for task, returns rcBadArg if other referee is already set
  virtual SchedulerRC  RefTask(TaskId task_id, TaskRefereeITF * use_ref) = 0;
  //Cancels referee for task, returns false if given referee is already
  //targeted by task for reporting.
  virtual bool   UnrefTask(TaskId task_id, TaskRefereeITF * use_ref) = 0;
};

class TaskRefereeITF {
protected:
  virtual ~TaskRefereeITF() //forbid interface destruction
  { }

public:
  virtual void onTaskReport(TaskSchedulerITF * sched, const ScheduledTaskAC * task) = 0;
};

class TaskSchedulerFactoryITF {
protected:
  virtual ~TaskSchedulerFactoryITF() //forbid interface destruction
  { }

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
class ScheduledTaskAC : public UtilizableObjITF {
public:
  enum PGState {  //task processing graph states
    pgIdle = 0
    , pgCont     //task continues its processing (further task signals possible)
    , pgSuspend  //task suspends its processing (further task signals possible)
    , pgDone     //task is completed, only sigRelease is expected
  };

protected:
  TaskId              _Id;
  PGState             _fsmState;
  TaskSchedulerITF *  _Owner;
  ScheduleCriterion   _Criterion;

  void Signal(TaskSchedulerITF::PGSignal req_signal) const
  {
    _Owner->SignalTask(_Id, req_signal);
  }

  ScheduledTaskAC() : _Id(0), _fsmState(pgIdle), _Owner(0)
  { }

public:
  virtual ~ScheduledTaskAC()
  { }

  static const char * nmPGState(enum PGState state);
  //
  const char * nmPGState(void) const   { return nmPGState(_fsmState); }
  //
  TaskId Id(void) const                { return _Id; }
  //Criterion for assigning task to one of scheduling queues
  const ScheduleCriterion & Criterion(void) const { return _Criterion; }

  //-- ----------------------------------- --//
  //-- ScheduledTaskAC interface methods
  //-- ----------------------------------- --//
  //NOTE: if succesor overwrites this method it MUST call this one.
  virtual void Init(TaskId use_id, TaskSchedulerITF * owner)
  {
    _Id = use_id; _Owner = owner;
  }
  virtual const char * TaskName(void) const = 0;
  //Called on sigProc, switches task to next state.
  virtual PGState Process(auto_ptr_utl<UtilizableObjITF> & use_dat) = 0;
  //Called on sigAbort, aborts task, the only PGState, the task is expected
  //to switch to is pgDone
  virtual PGState Abort(auto_ptr_utl<UtilizableObjITF> & use_dat) = 0;
  //Called on sigReport, requests task to report to the referee (use_ref != 0)
  //or perform some other reporting actions.
  virtual PGState Report(auto_ptr_utl<UtilizableObjITF> & use_dat, TaskRefereeITF * use_ref = 0) = 0;

  //-- ----------------------------------- --//
  // -- UtilizableObjITF interface methods
  //-- ----------------------------------- --//
  //virtual void    Destroy(void) = 0;
};

} //inman
} //smsc
#endif /* __SMSC_TASK_SCHEDULER_DEFS_HPP */

