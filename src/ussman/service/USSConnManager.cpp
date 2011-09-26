#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "ussman/service/USSConnManager.hpp"

using smsc::inman::interaction::IProtocolAC;

namespace smsc {
namespace ussman {
/* ************************************************************************** *
 * class USSConnManager implementation:
 * ************************************************************************** */
USSConnManager::~USSConnManager()
{
  stop(true);
  WorkerID wrkNum = _wrkPool.usage();
  if (wrkNum) {
    smsc_log_error(_logger, "%s: %u workers are still alive", _logId, wrkNum);
  }
}

//Starts acquisition and processing of incoming packets.
void USSConnManager::start(void)
{
  {
    MutexGuard  grd(_mgrSync);
    if (_connGrd.get())
      _connGrd->setMaxThreads(_cfg._maxThreads); //switch Connect to asynchronous mode
  }
  smsc::inman::interaction::ConnectManagerAC::start();
}


//Composes and sends USSResultMessage packet (negative)
//returns zero on success, negative value in case of socket error,
//positive value in case of serialization error.
int USSConnManager::denyRequest(WorkerID dlg_id,
                                interaction::USSResultMessage::Status_e err_st)
{
  if (!_connGrd.get())
    return -1;

  interaction::SPckUSSResult sPck;

  sPck.setDlgId(dlg_id);
  sPck._Cmd._status = err_st;

  smsc::inman::interaction::PacketBuffer_T<512>  pckBuf;
  try {
    sPck.serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    smsc_log_fatal(_logger, "%s: --> RESULT[%s:%u] serialization exception - %s",
                   _logId, mgrId(), dlg_id, exc.what());
    return 1;
  }

  char tBuf[interaction::USSResultMessage::_ussRes_strSZ];
  sPck._Cmd.log2str(tBuf, (unsigned)sizeof(tBuf));
  smsc_log_info(_logger, "%s: --> RESULT[%s:%u] %s", _logId, mgrId(), dlg_id, tBuf);

  return _connGrd->sendPck(pckBuf);
}

// ----------------------------------------------------
// -- AsynWorkerManagerAC virtual methods:
// ----------------------------------------------------
//Performs actions preceeding final worker releasing.
void USSConnManager::onWorkerRelease(WorkerGuard & wrk_grd)
{
  {
    MutexGuard   grd(_mgrSync);
    const USSOperationData & reqData = ((USSReqProcessor*)wrk_grd.get())->getReqData();
    _reqReg.Delete(reqData);
  }
  WorkerID numRefs = wrk_grd.getRefs();
  //NOTE: numRefs == 2 if worker reported completion from initial wrkHandlePacket() call
  _logger->log_((numRefs > 2) ? Logger::LEVEL_WARN : Logger::LEVEL_DEBUG,
                "%s: Worker[%u] reported completion, having %u refs",
                _logId, wrk_grd->wrkId(), (unsigned)numRefs);
  wrk_grd.release();
}


// -------------------------------------------
// -- PacketListenerIface interface methods
// -------------------------------------------
//Returns true if listener has utilized packet so no more listeners
//should be notified, false - otherwise (in that case packet will be
//reported to other listeners).
bool USSConnManager::onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
  /*throw(std::exception)*/
{
  IProtocolAC::PduId pduId = _protoDef.isKnownPacket(recv_pck);
  if (!pduId) {
    smsc_log_error(_logger, "%s: unsupported Pdu received", _logId);
    return true;
  }
  if (pduId != interaction::USSRequestMessage::_cmdTAG) {
    smsc_log_error(_logger, "%s: illegal Cmd[%u] received", _logId, (unsigned)pduId);
    return true;
  }

  interaction::SPckUSSRequest iPck;
  try {
    iPck.deserialize(recv_pck, smsc::inman::interaction::SerializablePacketIface::dsmComplete);
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s: corrupted Cmd[%u] received - %s", _logId,
                   (unsigned)interaction::USSRequestMessage::_cmdTAG, exc.what());
    return true;
  }

  WorkerGuard wrkGrd;
  {
    MutexGuard   grd(_mgrSync);

    //check request uniqueness
    WorkerID * pwId = _reqReg.GetPtr(iPck._Cmd.getOpData());
    if (pwId) {
      _logger->log_(_cfg._denyDupRequest ? Logger::LEVEL_WARN : Logger::LEVEL_ERROR,
                    "%s: recieved USSreq[%s:%u] is a duplicate of USSreq[%s:%u], %s ..", _logId,
                    mgrId(), iPck.getDlgId(), mgrId(), *pwId, _cfg._denyDupRequest ? "denying" : "ignoring");

      if (_cfg._denyDupRequest)
        denyRequest(iPck.getDlgId(), interaction::USSResultMessage::reqDUPLICATE);  //ignore sending failure here
      return true;
    }
    if (!isRunning()) {
      denyRequest(iPck.getDlgId(), interaction::USSResultMessage::reqDENIED); //ignore sending failure here
      return true;
    }

    //check dialog uniqueness
    wrkGrd = findWorker(iPck.getDlgId());
    if (wrkGrd.get()) {
      smsc_log_error(_logger, "%s: USSreq with duplicate dlgId(%u) received", _logId, iPck.getDlgId());
      return true;
    }

    if (numWorkers() < _cfg._maxRequests) {
      wrkGrd = _wrkPool.allcObj();
      wrkGrd->wrkInit(iPck.getDlgId(), this, _logger);

      ((USSReqProcessor*)wrkGrd.get())->configure(_cfg._wrkCfg, mgrId());
      monitorWorker(wrkGrd);
      _reqReg.Insert(iPck._Cmd.getOpData(), iPck.getDlgId());

      smsc_log_debug(_logger, "%s: allocated Worker[%u], poolIdx = %u", _logId, wrkGrd->wrkId(), wrkGrd.getPoolIdx());
      _denyCnt = 0;
      /* */
    } else {
      ++_denyCnt;
      smsc_log_warn(_logger, "%s: maxRequests limit reached: %u", _logId, _cfg._maxRequests);
      denyRequest(iPck.getDlgId(), interaction::USSResultMessage::reqDENIED); //ignore sending failure here

      if (_logger->isDebugEnabled() && (_denyCnt >= (_cfg._maxRequests/3))) {
        _wrkDump.clear();
        smsc::util::format(_wrkDump, "%s: Workers [%u of %u]: ", _logId, numWorkers(), _cfg._maxRequests);
        dumpWorkers(_wrkDump);
        smsc_log_debug(_logger, _wrkDump.c_str());
        _denyCnt = 0;
      }
      return true;
    }
  }
  ((USSReqProcessor*)wrkGrd.get())->wrkHandlePacket(iPck);
  return true;
}

} //ussman
} //smsc
