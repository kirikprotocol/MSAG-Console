/* ************************************************************************** *
 * USSMan client connection manager: 
 *    handles incoming USS requests in asynchronous mode.
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_CONNECT_MANAGER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_CONNECT_MANAGER_HPP

#include "core/buffers/DAHashT.hpp"
#include "inman/interaction/asyncmgr/ConnManager.hpp"

#include "ussman/service/USSReqProcessor.hpp"
#include "ussman/service/USSReqHFunc.hpp"

namespace smsc {
namespace ussman {

using smsc::core::network::Socket;
using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::ConnectManagerID;
using smsc::inman::interaction::WorkerID;
using smsc::ussman::interaction::INPUSSGateway;


class USSConnManager : protected smsc::inman::interaction::ConnectManagerAC {
private:
  typedef smsc::core::buffers::DAHash_T <
    smsc::ussman::comp::USSOperationData, WorkerID, smsc::core::buffers::DAHashSlotPOD_T
  > USSReqRegistry;

  typedef smsc::util::GrdIfaceImplPool_T <
    smsc::inman::interaction::WorkerIface, USSReqProcessor, WorkerID
  > WorkerPool;

  USSConnectCfg         _cfg;
  const INPUSSGateway & _protoDef;
  USSReqRegistry        _reqReg;
  WorkerPool            _wrkPool;
  WorkerID              _denyCnt;

public:
  USSConnManager(const USSConnectCfg & use_cfg, const INPUSSGateway & use_proto,
                 ConnectManagerID cm_id, Logger * use_log = NULL)
    : smsc::inman::interaction::ConnectManagerAC(cm_id, use_log)
    , _cfg(use_cfg), _protoDef(use_proto), _denyCnt(0)
  {
    if (!_logger)
      _logger = Logger::getInstance("smsc.ussman");
    snprintf(_logId, sizeof(_logId)-1, "ConnMgr[%u]", cm_id);
  }
  virtual ~USSConnManager();

  const char * logId(void) const { return &_logId[0]; }

  using smsc::inman::interaction::ConnectManagerAC::mgrId;
  using smsc::inman::interaction::ConnectManagerAC::bind;
  using smsc::inman::interaction::ConnectManagerAC::stop;

  //Starts acquisition and processing of incoming packets.
  void start(void);

protected:
  //Composes and sends USSResultMessage packet (negative)
  //returns zero on success, negative value in case of socket error,
  //positive value in case of serialization error.
  int denyRequest(WorkerID dlg_id, interaction::USSResultMessage::Status_e err_st);

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

} //ussman
} //smsc

#endif /* __SMSC_USSMAN_CONNECT_MANAGER_HPP */
