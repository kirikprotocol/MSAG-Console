#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/asyncmgr/ConnManager.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * class ConnectManagerAC implementation:
 * ************************************************************************** */
//
void ConnectManagerAC::bind(const ConnectGuard * use_conn/* = NULL*/)
{
  MutexGuard  grd(_mgrSync);
  if (!use_conn || !use_conn->get()) {
    if (_connGrd.get()) {
      _connGrd->removeListener(*this);
      _connGrd.release();
    }
  } else {
    if (_connGrd.get())
      _connGrd->removeListener(*this);
    _connGrd = *use_conn;
    _connGrd->addListener(*this);
  }
}

//Starts processing of incoming packets by workers.
void ConnectManagerAC::start(void)
{
  MutexGuard  grd(_mgrSync);
  activate();
  if (_connGrd.get() && !_connGrd->isRunning())
    _connGrd->start();
}
//Stops incoming packets processing. If 'do_wait' argument is set, waits
//the default shutdown timeout set on Connect, then cancels processing of
//already accumulated packets and aborts all remaining wrokers.
void ConnectManagerAC::stop(bool do_wait)
{
  stopWorkers(false);     //no new worker will be created
  if (do_wait) {
    if (_connGrd.get()) {
      _connGrd->stop();   //stop packets acquisition, give a time to process
                          //already acquired packets
    }
    stopWorkers(true);    //abort remaining workers
    bind(NULL);
  }
}


// ------------------------------------------
// -- WorkerManagerIface interface methods:
// ------------------------------------------
//sends packet, returns false in case of failure
bool ConnectManagerAC::sendPck(const PacketBufferAC & use_pck) const /*throw()*/
{
  MutexGuard grd(_mgrSync);
  return _connGrd.get() ? ( _connGrd->sendPck(use_pck) == 0) : false;
}

//returns last connect exception occured while sending data
bool ConnectManagerAC::hasExceptionOnSend(smsc::util::CustomException * p_exc/* = NULL*/) const /*throw()*/
{
  MutexGuard grd(_mgrSync);
  return (_connGrd.get() && _connGrd->hasExceptionOnSend(p_exc));
}


// -------------------------------------------
// -- PacketListenerIface interface methods
// -------------------------------------------

//Returns true if listener has processed connect exception so no more
//listeners should be notified, false - otherwise (in that case exception
//will be reported to other listeners).
bool ConnectManagerAC::onConnectError(unsigned conn_id,
                                      PckAccumulatorIface::Status_e err_status,
                                      const smsc::util::CustomException * p_exc/* = NULL*/)
  /*throw(std::exception)*/
{
  if ((err_status == PckAccumulatorIface::accEOF) && !p_exc) {
    smsc_log_info(_logger, "%s: Connect[%u] is closed", _logId, conn_id);
  } else {
    smsc_log_error(_logger, "%s: Connect[%u] error status(%s): %s", _logId, conn_id,
                   PckAccumulatorIface::nmStatus(err_status), p_exc ? p_exc->what() : "");
  }
  bind(NULL);
  stopWorkers(false, p_exc ? p_exc->what() : NULL);
  return true;
}

} //interaction
} //inman
} //smsc

