#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/scheduler/ICSScheduler.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc {
namespace inman {

/* ************************************************************************* *
 *  class ICSTaskScheduler implementation
 * ************************************************************************* */
void  ICSTaskScheduler::_icsStop(bool do_wait/* = false*/)
{
  unsigned i = 0, iMax = do_wait ? 2 : 1;
  do {
    TSchedRegistry::const_iterator it = tschedReg.begin();
    for (; it != tschedReg.end(); ++it)
      it->second->Stop(!i ? 0 : TaskSchedulerAC::_SHUTDOWN_TIMEOUT);
  } while ((++i) < iMax);
}

// ------------------------------------------
// TaskSchedulerFactoryITF interface methods:
// ------------------------------------------
//Returns started scheduler
TaskSchedulerITF * 
  ICSTaskScheduler::getScheduler(TaskSchedulerITF::SchedulerType sched_type)
{
  {
    MutexGuard grd(_sync);
    TaskSchedulerAC * pSched = tschedReg.find(sched_type);
    if (pSched)
      return pSched;
  }
  //create new scheduler
  std::auto_ptr<TaskSchedulerAC> pShed;
  if (sched_type == TaskSchedulerITF::schedMT)
    pShed.reset(new TaskSchedulerMT(logger));
  else
    pShed.reset(new TaskSchedulerSEQ(logger));

  if (pShed->Start()) {
    MutexGuard grd(_sync);
    tschedReg.insert(sched_type, pShed.get());
    return pShed.release();
  }
  smsc_log_error(logger, "Sched: failed to start %s ..", pShed->Name());
  return NULL;
}

} //inman
} //smsc

