/* ************************************************************************* *
 * AbntDetectorManager: manages abonent contract requests on given Connect
 * in asynchronous mode.
 * ************************************************************************* */
#ifndef __INMAN_ABNT_DETECTOR_MGR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABNT_DETECTOR_MGR_HPP

#include "inman/common/GrdObjPool_T.hpp"

#include "inman/interaction/asyncmgr/ConnManager.hpp"
#include "inman/services/abdtcr/AbntDetector.hpp"

#include "inman/INManErrors.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::INManErrorId;

using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::PckAccumulatorIface;
using smsc::inman::interaction::INPAbntContract; //protocol definition

using smsc::inman::interaction::ConnectManagerID;
using smsc::inman::interaction::WorkerID;

class AbntDetectorManager : protected smsc::inman::interaction::ConnectManagerAC {
private:
  typedef smsc::util::GrdIfaceImplPool_T <
    smsc::inman::interaction::WorkerIface, AbonentDetector, WorkerID
  > WorkerPool;

  const AbonentDetectorCFG &  _cfg;
  const INPAbntContract &     _protoDef;
  WorkerPool                  _wrkPool;
  WorkerID                    _denyCnt;

public: 
  AbntDetectorManager(const AbonentDetectorCFG & use_cfg,
                      const INPAbntContract & proto_def,
                      ConnectManagerID cm_id, Logger * use_log = NULL)
    : smsc::inman::interaction::ConnectManagerAC(cm_id, use_log)
    , _cfg(use_cfg), _protoDef(proto_def), _denyCnt(0)
  {
    if (!_logger)
      _logger = Logger::getInstance("smsc.inman.abdtcr");
    snprintf(_logId, sizeof(_logId)-1, "AbntMgr[%s]", mgrId());
#ifdef __GRD_POOL_DEBUG__
    _wrkPool.debugInit(_logId, _logger);
#endif /* __GRD_POOL_DEBUG__ */
  }
  virtual ~AbntDetectorManager();

  using smsc::inman::interaction::ConnectManagerAC::mgrId;
  using smsc::inman::interaction::ConnectManagerAC::bind;
  using smsc::inman::interaction::ConnectManagerAC::stop;

  //Starts acquisition and processing of incoming packets.
  void start(void);

protected:
  //Composes and sends ContractResult packet (negative)
  //returns zero on success, negative value in case of socket error,
  //positive value in case of serialization error.
  int denyRequest(unsigned dlg_id, INManErrorId::Code_e use_error);

  // -------------------------------------------
  // -- PacketListenerIface interface methods
  // -------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception)*/;
};

} //abdtcr
} //inman
} //smsc

#endif /* __INMAN_ABNT_DETECTOR_MGR_HPP */

