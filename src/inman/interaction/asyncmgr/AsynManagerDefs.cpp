#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/asyncmgr/AsynManagerDefs.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc   {
namespace inman  {
namespace interaction {

/* ************************************************************************** *
 * class AsynWorkerManagerAC implementation:
 * ************************************************************************** */
/* NOTE: all PRIVATE/PROTECTED methods must be called with _mgrSync locked !  *
 * ************************************************************************** */
//Note: this call blocks creation of new workers.
void AsynWorkerManagerAC::dumpWorkers(std::string & dump)
{
  bool restoreState = false;
  if (_runState > cmStopping) {
    _prevState = _runState;
    _runState = cmStopping;  //temporary prevent the creation of new workers
    restoreState = true;
  }

  dump.clear();
  if (_wrkReg.Count()) {
    WorkersMap::const_iterator it = _wrkReg.begin();
    do {
      if (it->_value.get()) {
        WorkerGuard wGrd = it->_value;
        {
          ReverseMutexGuard rGrd(_mgrSync);
          wGrd->wrkLogState(dump);
          wGrd.release(); //worker may be destoyed at this point!
        }
      }
      dump += ", ";
    } while (!(++it).isEnd()); //_wrkReg specific!
  }
  if (restoreState)
    _runState = _prevState;
}

//Notifies all monitored worker that they must abort.
//Returns number of active worker remains.
//NOTE: _runState must not be equal to cmRunning upon entry, 
//     so no new workers may be inserted into workers registry.
WorkerID AsynWorkerManagerAC::abortWorkers(const char * use_reason/* = NULL*/)
{
  if (_wrkReg.Count()) { //abort all monitored workers
    if (use_reason) {
      smsc_log_error(_logger, "%s: aborting %u workers, reason: %s", _logId, _wrkReg.Count(), use_reason);
    } else {
      smsc_log_info(_logger, "%s: aborting %u workers ..", _logId, _wrkReg.Count());
    }

    WorkersMap::iterator it = _wrkReg.begin();
    do {
      if (it->_value.get()) {
        WorkerGuard wGrd = it->_value;
        {
          ReverseMutexGuard rGrd(_mgrSync);
          wGrd->wrkAbort(use_reason); //may call workerDone()
          wGrd.release(); //worker may be destoyed at this point!
        }
      }
    } while (!(++it).isEnd()); //_wrkReg specific!
  }
  return _wrkReg.Count();
}

// ------------------------------------------
// -- WorkerManagerIface interface methods:
// ------------------------------------------
void AsynWorkerManagerAC::workerDone(const WorkerIface & p_worker)
{
  //NOTE: worker object destruction may lasts rather long time,
  //      so perform it while _mgrSync is unlocked.
  WorkerID      wrkId = p_worker.wrkId();
  WorkerGuard   tGrd;
  {
    MutexGuard grd(_mgrSync);
    WorkerGuard * wGrd = _wrkReg.GetPtr(wrkId);
    if (!wGrd || !wGrd->get()) {
      smsc_log_warn(_logger, "%s: uncontrolled Worker[%u] reported completion",
                     _logId, wrkId);
      return;
    }
    tGrd = *wGrd;
    _wrkReg.Delete(wrkId);
    _mgrSync.notify();
  }
  //worker may be destoyed at this point.
  onWorkerRelease(tGrd);
}

//Creates a monitored worker guarding object (prevents worker from being deleted).
WorkerGuard AsynWorkerManagerAC::getWorkerGuard(const WorkerIface & p_worker) const /*throw()*/
{
  MutexGuard grd(_mgrSync);
  WorkerGuard * wGrd = _wrkReg.GetPtr(p_worker.wrkId());
  if (!wGrd || !wGrd->get()) {
    smsc_log_warn(_logger, "%s: uncontrolled Worker[%u] requested guard creation",
                  _logId, p_worker.wrkId());
    return WorkerGuard();
  }
  return *wGrd;
}

// ------------------------------------------
// PUBLIC methods:
// ------------------------------------------
void AsynWorkerManagerAC::startWorkers(void)
{
  MutexGuard grd(_mgrSync);
  _runState = cmRunning;
}

void AsynWorkerManagerAC::stopWorkers(bool do_wait, const char * stop_reason/* = NULL*/)
{
  MutexGuard grd(_mgrSync);
  if (_runState > cmStopping)
    _prevState = _runState = cmStopping; //no new worker will be created
  if (do_wait && abortWorkers(stop_reason)) {
    while (_wrkReg.Count())
      _mgrSync.wait(100); //Note: workers reported completion, may have deferred refs releasing.
  }
}

} //interaction
} //inman
} //smsc

