/* ************************************************************************* *
 * BillingManager: manages SM/USSD messages billing requests on given
 * Connect in asynchronous mode.
 * ************************************************************************* */
#ifndef __INMAN_BILLING_MANAGER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_BILLING_MANAGER_HPP

#include "inman/interaction/asyncmgr/ConnManager.hpp"
#include "inman/services/smbill/SmBilling.hpp"

namespace smsc   {
namespace inman  {
namespace smbill {

using smsc::inman::interaction::INPBilling;
using smsc::inman::interaction::ConnectManagerID;
using smsc::inman::interaction::WorkerID;

//
class SmBillManager : protected smsc::inman::interaction::ConnectManagerAC {
private:
  typedef smsc::core::buffers::IntrusivePoolOfIfaceImpl_T <
    smsc::inman::interaction::WorkerIface, Billing, WorkerID
  > WorkerPool;

  const SmBillingCFG &  _cfg;
  const INPBilling &    _protoDef;
  WorkerID              _denyCnt;
  WorkerPool            _wrkPool;

public: 
  SmBillManager(const SmBillingCFG & use_cfg, const INPBilling & proto_def,
                ConnectManagerID cm_id, Logger * use_log = NULL)
      : smsc::inman::interaction::ConnectManagerAC(cm_id, use_log)
    , _cfg(use_cfg), _protoDef(proto_def), _denyCnt(0)
  {
    if (!_logger)
      _logger = Logger::getInstance("smsc.inman.smbill");
    snprintf(_logId, sizeof(_logId)-1, "BillMgr[%s]", mgrId());
#ifdef INTRUSIVE_POOL_DEBUG
    _wrkPool.debugInit(_logId, _logger);
#endif /* INTRUSIVE_POOL_DEBUG */
  }
  virtual ~SmBillManager();

  using smsc::inman::interaction::ConnectManagerAC::mgrId;
  using smsc::inman::interaction::ConnectManagerAC::bind;
  using smsc::inman::interaction::ConnectManagerAC::stop;

  //Starts acquisition and processing of incoming packets.
  void start(void);

protected:
  //Composes and sends ChargeSmsResult packet (negative)
  //returns -1 on error, or number of total bytes sent
  int denyRequest(unsigned dlg_id, INManErrorId::Code_e use_error);

  // ----------------------------------------------------
  // -- AsynWorkerManagerAC virtual methods:
  // ----------------------------------------------------
  //Performs actions preceeding final worker releasing.
  virtual void onWorkerRelease(WorkerGuard & wrk_grd);

  // -------------------------------------------
  // -- PacketListenerIface interface methods
  // -------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception)*/;
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_BILLING_MANAGER_HPP */

