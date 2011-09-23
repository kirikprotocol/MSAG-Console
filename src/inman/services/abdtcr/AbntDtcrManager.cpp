#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/AbntDtcrManager.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::inman::interaction::IProtocolAC;
using smsc::inman::interaction::SPckContractRequest;
using smsc::inman::interaction::SPckContractResult;
using smsc::inman::_RCS_INManErrors;

namespace smsc {
namespace inman {
namespace abdtcr {

/* ************************************************************************** *
 * class AbntDetectorManager implementation:
 * ************************************************************************** */
AbntDetectorManager::~AbntDetectorManager()
{
  stop(true);
  WorkerID wrkNum = _wrkPool.usage();
  if (wrkNum) {
    smsc_log_error(_logger, "%s: %u workers are still alive", _logId, wrkNum);
  }
}

//Starts acquisition and processing of incoming packets.
void AbntDetectorManager::start(void)
{
  {
    MutexGuard  grd(_mgrSync);
    if (_connGrd.get())
      _connGrd->setMaxThreads(_cfg.maxThreads); //switch Connect to asynchronous mode
  }
  smsc::inman::interaction::ConnectManagerAC::start();
}

//Composes and sends ContractResult packet (negative)
//returns zero on success, negative value in case of socket error,
//positive value in case of serialization error.
//NOTE: _mutex SHOULD be locked upon entry!
int AbntDetectorManager::denyRequest(unsigned dlg_id, INManErrorId::Code_e use_error)
{
  if (!_connGrd.get())
    return -1;

  SPckContractResult spck;
  spck._Hdr.dlgId = dlg_id;
  spck._Cmd.setError(_RCS_INManErrors->mkhash(use_error),
                      _RCS_INManErrors->explainCode(use_error).c_str());

  smsc_log_warn(_logger, "%s: <-- AbntDet[%s:%u], errCode %u: %s",
                 _logId, mgrId(), dlg_id, spck._Cmd.errorCode(), spck._Cmd.errorMsg());

  smsc::inman::interaction::PacketBuffer_T<512>  pckBuf;
  try {
    spck.serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    smsc_log_fatal(_logger, "%s: <-- RESULT[%s:%u] serialization exception - %s",
                   _logId, mgrId(), dlg_id, exc.what());
    return 1;
  }
  return _connGrd->sendPck(pckBuf);
}

/* -------------------------------------------------------------------------- *
 * PacketListenerIface interface implementation:
 * -------------------------------------------------------------------------- */
//Returns true if listener has utilized packet so no more listeners
//should be notified, false - otherwise (in that case packet will be
//reported to other listeners).
bool AbntDetectorManager::onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
  /*throw(std::exception)*/
{
  IProtocolAC::PduId pduId = _protoDef.isKnownPacket(recv_pck);
  if (!pduId) {
    smsc_log_error(_logger, "%s: unsupported Pdu received", _logId);
    return true;
  }
  if (pduId != SPckContractRequest::getPduId()) {
    smsc_log_error(_logger, "%s: illegal Cmd[%u] received", _logId,
                   (unsigned)_protoDef.getCmdId(pduId));
    return true;
  }

  SPckContractRequest iPck;
  try {
    iPck.deserialize(recv_pck,
                     smsc::inman::interaction::SerializablePacketIface::dsmComplete);
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s: corrupted Cmd[%u] received - %s", _logId,
                   (unsigned)AbntContractRequest::_cmdTAG, exc.what());
    return true;
  }
  
  WorkerGuard wrkGrd;
  {
    MutexGuard   grd(_mgrSync);
    smsc_log_debug(_logger, "%s: Cmd[0x%X] for AbntDet[%s:%u] received", _logId,
                   (unsigned)iPck._Cmd.Id(), mgrId(), iPck._Hdr.dlgId);
    if (!isRunning()) {
      denyRequest(iPck._Hdr.dlgId, INManErrorId::srvInoperative); //ignore sending failure here
      return true;
    }

    wrkGrd = findWorker(iPck._Hdr.dlgId);
    if (!wrkGrd.get()) {
      if (numWorkers() < _cfg.maxRequests) {
        wrkGrd = _wrkPool.allcObj();
        wrkGrd->wrkInit(iPck._Hdr.dlgId, this, _logger);
        (static_cast <AbonentDetector*>(wrkGrd.get()))->configure(_cfg, mgrId());
        monitorWorker(wrkGrd);
        smsc_log_debug(_logger, "%s: allocated Worker[%u], poolIdx = %u, monitored workers: %u",
                       _logId, wrkGrd->wrkId(), wrkGrd.getPoolIdx(), numWorkers());
        _denyCnt = 0;
        /* */
      } else {
        ++_denyCnt;
        smsc_log_warn(_logger, "%s: maxRequests limit reached: %u", _logId, _cfg.maxRequests);
        denyRequest(iPck._Hdr.dlgId, INManErrorId::cfgLimitation); //ignore sending failure here

        if (_logger->isDebugEnabled() && (_denyCnt >= (_cfg.maxRequests/3))) {
          _wrkDump.clear();
          smsc::util::format(_wrkDump, "%s: Workers [%u of %u]: ", _logId, numWorkers(), _cfg.maxRequests);
          dumpWorkers(_wrkDump);
          smsc_log_debug(_logger, _wrkDump.c_str());
          _denyCnt = 0;
        }
        return true;
      }
    }
  }
  (static_cast <AbonentDetector*>(wrkGrd.get()))->wrkHandlePacket(iPck);
  return true;
}

} //abdtcr
} //inman
} //smsc

