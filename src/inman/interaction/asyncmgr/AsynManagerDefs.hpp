/* ************************************************************************* *
 * Asynchronous workers manager base class: maintains workers registry.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ASYN_WORKER_MANAGER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYN_WORKER_MANAGER_HPP

#include "util/Exception.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "core/buffers/DAHashT.hpp"
#include "core/buffers/DAHashFuncInts.hpp"

#include "inman/interaction/PacketBuffer.hpp"
#include "inman/interaction/asyncmgr/AsynWorkerDefs.hpp"

namespace smsc {
namespace inman {
namespace interaction {

using smsc::core::synchronization::EventMonitor;

// -------------------------------------------------------------------------------
class AsynWorkerManagerAC : public WorkerManagerIface {
public:
  //
  virtual ~AsynWorkerManagerAC()
  {
    //NOTE: it's recommended to call Stop(true) in successor's destructor
  }

  //
  void startWorkers(void);
  //
  void stopWorkers(bool do_wait, const char * stop_reason = NULL);

  // ------------------------------------------
  // -- WorkerManagerIface interface methods:
  // ------------------------------------------
  //sends packet, returns false in case of failure
  virtual bool sendPck(const PacketBufferAC & use_pck) const /*throw()*/ = 0;
  //returns last connect exception occured while sending data
  virtual bool hasExceptionOnSend(smsc::util::CustomException * p_exc = NULL) const /*throw()*/ = 0;
  //notifies manager that worker is complete its processing and should be
  //removed from monitored workers registry.
  //NOTE: worker _sync MUST be unlocked !
  virtual void workerDone(const WorkerIface & p_worker) /*throw()*/;
  //Creates a monitored worker guarding object (prevents worker from being deleted).
  virtual WorkerGuard getWorkerGuard(const WorkerIface & p_worker) const /*throw()*/;

private:
  enum State_e {
    cmIdle = 0
  , cmStopping //no new workers allocated, only existing ones are served for a while
  , cmRunning
  };

  typedef smsc::core::buffers::DAHash_T <
    WorkerID, WorkerGuard, smsc::core::buffers::DAHashSlot_T
  > WorkersMap;

  volatile State_e  _runState;
  volatile State_e  _prevState;
  WorkersMap        _wrkReg; //monitored workers registry

protected:
  static const unsigned _MAX_CONNECT_MGR_NAME = 60;

  mutable EventMonitor  _mgrSync;
  Logger *              _logger;
  //logging prefix, f.ex: "ConnMgr[%u]", should be initialized by successor
  char                  _logId[_MAX_CONNECT_MGR_NAME + 1];
  std::string           _wrkDump;

  // ----------------------------------------------------
  // -- AsynWorkerManagerAC virtual methods:
  // ----------------------------------------------------
  //Performs actions preceeding final worker releasing.
  virtual void onWorkerRelease(WorkerGuard & wrk_grd) { wrk_grd.release(); }
  /* -------------------------------------------------------------- */
  /* NOTE: _mgrSync must be locked prior to calling protected methods */
  /* -------------------------------------------------------------- */
  bool      isRunning(void) const { return _runState == cmRunning; }
  WorkerID  numWorkers(void) const { return _wrkReg.Count(); }

  //Returns empty WorkerGuard if no worker with given id is monitored
  WorkerGuard findWorker(WorkerID w_id) const
  {
    WorkerGuard * wGrd = _wrkReg.GetPtr(w_id);
    return wGrd ? *wGrd : WorkerGuard();
  }
  //
  void activate(void) { _runState = cmRunning; }
  //
  void monitorWorker(const WorkerGuard & w_grd) { _wrkReg.Insert(w_grd->wrkId(), w_grd); }

  //dumps workers state to string
  void dumpWorkers(std::string & dump);
  //Notifies all monitored worker that they must abort.
  //Returns number of active worker remains.
  //NOTE: _runState must not be equal to cmRunning upon entry, 
  //     so no new workers may be inserted into workers registry.
  WorkerID abortWorkers(const char * use_reason = NULL);

  //
  explicit AsynWorkerManagerAC(Logger * use_log = NULL)
    : _runState(cmIdle), _prevState(cmIdle), _logger(use_log)
  {
    _logId[0] = 0;
  }
};

} //interaction
} //inman
} //smsc
#endif /* __SMSC_INMAN_ASYN_WORKER_MANAGER_HPP */

