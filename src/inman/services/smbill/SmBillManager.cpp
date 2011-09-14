#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/SmBillManager.hpp"
using smsc::inman::interaction::SerializablePacketIface;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::ChargeSmsResult;

namespace smsc {
namespace inman {
namespace smbill {

/* ************************************************************************** *
 * class SmBillManager implementation:
 * ************************************************************************** */
SmBillManager::~SmBillManager()
{
  stop(true);
  WorkerID wrkNum = _wrkPool.usage();
  if (wrkNum) {
    smsc_log_error(_logger, "%s: %u workers are still alive", _logId, wrkNum);
  }
}

//Starts acquisition and processing of incoming packets.
void SmBillManager::start(void)
{
  {
    MutexGuard  grd(_mgrSync);
    if (_connGrd.get())
      _connGrd->setMaxThreads(_cfg.prm->maxThreads); //switch Connect to asynchronous mode
  }
  smsc::inman::interaction::ConnectManagerAC::start();
}

//NOTE: _mutex SHOULD be locked upon entry!
int SmBillManager::denyRequest(unsigned dlg_id, INManErrorId::Code_e use_error)
{
  if (!_connGrd.get())
    return -1;

  SPckChargeSmsResult spck;
  spck._Hdr.dlgId = dlg_id;
  spck._Cmd.setValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
  spck._Cmd.setError(_RCS_INManErrors->mkhash(use_error),
                      _RCS_INManErrors->explainCode(use_error).c_str());

  smsc_log_warn(_logger, "%s: <-- CHARGING_NOT_POSSIBLE[%s:%u], cause %u: %s",
                 _logId, mgrId(), dlg_id, spck._Cmd.getError(), spck._Cmd.getMsg());

  smsc::inman::interaction::PacketBuffer_T<512>  pckBuf;
  try {
    spck.serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    smsc_log_fatal(_logger, "%s: <-- CHARGING_NOT_POSSIBLE[%s:%u] serialization exception - %s",
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
bool SmBillManager::onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
  /*throw(std::exception)*/
{
  INPBilling::PduId pduId = _protoDef.isKnownPacket(recv_pck);
  if (!pduId) {
    smsc_log_error(_logger, "%s: unsupported Cmd received", _logId);
    return true;
  }
  uint16_t cmdId = _protoDef.getCmdId(pduId);
  if (cmdId == INPBilling::CHARGE_SMS_RESULT_TAG) {
    smsc_log_error(_logger, "%s: illegal Cmd[0x%X] received", _logId, (unsigned)cmdId);
    return true;
  }
  SmBillRequestMsg iPck(static_cast<INPBilling::CommandTag_e>(cmdId));

  SerializablePacketIface::DsrlzMode_e srlzRes = SerializablePacketIface::dsmPartial;
  
  try { srlzRes = iPck.deserialize(recv_pck, srlzRes);
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s: corrupted Cmd[0x%X] received - %s", _logId,
                   (unsigned)iPck._cmdId, exc.what());
    return true;
  }
  uint32_t    dlgId = iPck.getDlgId();
  WorkerGuard wrkGrd;
  {
    MutexGuard   grd(_mgrSync);
    if (!isRunning() && (cmdId == INPBilling::CHARGE_SMS_TAG)) {
      denyRequest(dlgId, INManErrorId::srvInoperative); //ignore sending failure here
      return true;
    }
    wrkGrd = findWorker(dlgId);
    if (!wrkGrd.get()) {
      if (cmdId == INPBilling::DELIVERY_SMS_RESULT_TAG) {
        smsc_log_error(_logger, "%s: protocol error: Cmd[0x%X] for non-existent Worker[%u] received",
                       _logId, (unsigned)cmdId, dlgId);
        return true;
      }
      
      if (numWorkers() < _cfg.prm->maxBilling) {
        wrkGrd = _wrkPool.allcObj();
        wrkGrd->wrkInit(dlgId, this, _logger);
        ((Billing*)wrkGrd.get())->configure(_cfg, mgrId());
        monitorWorker(wrkGrd);
        smsc_log_debug(_logger, "%s: allocated Worker[%u], poolIdx = %u",
                       _logId, wrkGrd->wrkId(), wrkGrd.getPoolIdx());
        _denyCnt = 0;
      } else {
        ++_denyCnt;
        smsc_log_warn(_logger, "%s: maxBilling limit reached: %u", _logId, _cfg.prm->maxBilling);
        denyRequest(dlgId, INManErrorId::cfgLimitation); //ignore sending failure here

        if (_logger->isDebugEnabled() && (_denyCnt >= (_cfg.prm->maxBilling/3))) {
          _wrkDump.clear();
          smsc::util::format(_wrkDump, "%s: Workers [%u of %u]: ",
                             _logId, numWorkers(), _cfg.prm->maxBilling);
          dumpWorkers(_wrkDump);
          smsc_log_debug(_logger, _wrkDump.c_str());
          _denyCnt = 0;
        }
        return true;
      }
    }
  }
  ((Billing*)wrkGrd.get())->wrkHandlePacket(iPck, (srlzRes == SerializablePacketIface::dsmPartial) ? &recv_pck : NULL);
//#ifdef INTRUSIVE_POOL_DEBUG
//  WorkerID numRefs = wrkGrd.getRefs();
//  smsc_log_debug(_logger, "%s: Worker[%u] handled packet, having %u refs",
//                 _logId, wrkGrd->wrkId(), (unsigned)numRefs);
//#endif /* INTRUSIVE_POOL_DEBUG */
  return true;
}

} //smbill
} //inman
} //smsc

