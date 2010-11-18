#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/scheduler/TaskSchedulerDefs.hpp"

namespace smsc {
namespace inman {

/* ************************************************************************* *
 *  class TaskSchedulerITF implementation
 * ************************************************************************* */
const char * TaskSchedulerITF::nmPGSignal(TaskSchedulerITF::PGSignal cmd)
{
  switch (cmd) {
  case sigRelease:    return "sigRelease";
  case sigSuspend:    return "sigSuspend";
  case sigProc:       return "sigProc";
  case sigReport:     return "sigReport";
  case sigAbort:      return "sigAbort";
  default:;
  }
  return "sigNone";
}

const char * TaskSchedulerITF::nmRCode(TaskSchedulerITF::SchedulerRC rc)
{
  switch (rc) {
  case rcOk:              return "rcOk";
  case rcUnknownTask:     return "rcUnknownTask";
  case rcSchedNotRunning: return "rcSchedNotRunning";
  case rcBadArg:          return "rcBadArg";
  default:;
  }
  return "rcUndefined";
}

/* ************************************************************************* *
 *  class ScheduledTaskAC implementation
 * ************************************************************************* */
const char * ScheduledTaskAC::nmPGState(enum PGState state)
{
  switch (state) {
  case pgDone:    return "pgDone";
  case pgSuspend: return "pgSuspend";
  case pgCont:    return "pgCont";
  default:;
  }
  return "pgIdle";
}

} //inman
} //smsc

