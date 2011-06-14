/* ************************************************************************* *
 * Asynchronous Connect Manager: generalizes implementation of 
 * PacketListenerIface for handling asynchronous TCP protocols, serving 
 * each request by separate worker.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCPSRV_CONNECT_MANAGER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCPSRV_CONNECT_MANAGER_HPP

#include "inman/interaction/PckListenerDefs.hpp"
#include "inman/interaction/asyncmgr/AsynManagerDefs.hpp"
#include "inman/interaction/asyncmgr/ConnectsPool.hpp"

namespace smsc {
namespace inman {
namespace interaction {

using smsc::logger::Logger;

//
class ConnectManagerAC : protected AsynWorkerManagerAC
                       , protected smsc::inman::interaction::PacketListenerIface {
public:
  typedef uint32_t ident_type;

  virtual ~ConnectManagerAC()
  {
    stop(true);
  }

  //
  const char * mgrId(void) const { return _mgrId; }

  //
  void bind(const ConnectGuard * use_conn/* = NULL*/);

  //Starts processing of incoming packets by workers.
  void start(void);
  //Stops incoming packets processing. If 'do_wait' argument is set, waits
  //the default shutdown timeout set on Connect, then cancels processing of
  //already accumulated packets and aborts all remaining wrokers.
  void stop(bool do_wait);

private:
  char           _mgrId[sizeof(ident_type)*3 + 1];

protected:
  ConnectGuard  _connGrd;

  ConnectManagerAC(ident_type cm_id, Logger * use_log = NULL)
    : AsynWorkerManagerAC(use_log)
  {
    //Note: successor MUST Initialize _logger if use_log==0
    snprintf(_mgrId, sizeof(_mgrId)-1, "%u", cm_id);
  }

  // ------------------------------------------
  // -- WorkerManagerIface interface methods:
  // ------------------------------------------
  //sends packet, returns false in case of failure
  virtual bool sendPck(const PacketBufferAC & use_pck) const /*throw()*/;
  //returns last connect exception occured while sending data
  virtual bool hasExceptionOnSend(smsc::util::CustomException * p_exc = NULL) const /*throw()*/;

  // -------------------------------------------
  // -- PacketListenerIface interface methods
  // -------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception)*/ = 0;

  //Returns true if listener has processed connect exception so no more
  //listeners should be notified, false - otherwise (in that case exception
  //will be reported to other listeners).
  virtual bool onConnectError(unsigned conn_id,
                              PckAccumulatorIface::Status_e err_status,
                              const smsc::util::CustomException * p_exc = NULL)
    /*throw(std::exception)*/;
};
typedef ConnectManagerAC::ident_type ConnectManagerID;


} //interaction
} //inman
} //smsc
#endif /* __SMSC_INMAN_TCPSRV_CONNECT_MANAGER_HPP */

