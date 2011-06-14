/* ************************************************************************* *
 * Asynchronous request Worker interface definition.
 * Generalizes asynchronous TCP protocols request handling.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ASYN_CONNECT_WORKER_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYN_CONNECT_WORKER_DEFS

#include "logger/Logger.h"

#include "inman/common/GrdObjPool_T.hpp"
#include "inman/interaction/PacketBuffer.hpp"

namespace smsc {
namespace inman {
namespace interaction {

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
// -------------------------------------------------------------------------------

class WorkerManagerIface; //forward declaration

class WorkerIface {
public:
  typedef uint32_t ident_type;

  ident_type wrkId(void) const { return _wrkId; }
  // ------------------------------------------
  // -- WorkerIface interface methods:
  // ------------------------------------------
  //Initializes worker as it's got from pool.
  virtual void wrkInit(ident_type w_id, WorkerManagerIface * use_mgr, Logger * use_log = NULL) = 0;
  //Prints some information about worker state/status
  virtual void wrkLogState(std::string & use_str) const = 0;
  //
  virtual void wrkAbort(const char * abrt_reason = NULL) = 0;

protected:
  WorkerIface() : _wrkId(0), _wrkMgr(0), _logger(0)
  { }
  virtual ~WorkerIface()
  { }

  ident_type            _wrkId;
  WorkerManagerIface *  _wrkMgr;
  Logger *              _logger;
};
typedef WorkerIface::ident_type WorkerID;
typedef smsc::util::GrdIfaceImplPool_T<WorkerIface, WorkerID> WorkersPoolAC;
typedef WorkersPoolAC::ObjRef WorkerGuard;


class WorkerManagerIface {
protected:
  virtual ~WorkerManagerIface()
  { }

public:
  // ------------------------------------------
  // -- WorkerManagerIface interface methods:
  // ------------------------------------------
  //sends packet, returns false in case of failure
  virtual bool sendPck(const PacketBufferAC & use_pck) const /*throw()*/= 0;
  //returns last connect exception occured while sending data
  virtual bool hasExceptionOnSend(smsc::util::CustomException * p_exc = NULL) const  /*throw()*/= 0;
  //notifies manager that worker is complete its processing and should be
  //removed from monitored workers registry.
  //NOTE: worker _sync MUST be unlocked !
  virtual void workerDone(const WorkerIface & p_worker) /*throw()*/ = 0;
  //Creates a monitored worker guarding object (prevents worker from being deleted).
  virtual WorkerGuard getWorkerGuard(const WorkerIface & p_worker) const /*throw()*/ = 0;
};


} //interaction
} //inman
} //smsc
#endif /* __SMSC_INMAN_ASYN_CONNECT_WORKER_DEFS */

