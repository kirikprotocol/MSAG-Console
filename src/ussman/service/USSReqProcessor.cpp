#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "ussman/comp/map_uss/MapUSSFactory.hpp"
using smsc::ussman::comp::_ac_map_networkUnstructuredSs_v2;

#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNBinding;

#include "ussman/service/USSReqProcessor.hpp"
using smsc::util::URCRegistry;

using smsc::core::synchronization::MutexGuard;

namespace smsc {
namespace ussman {

USSReqProcessor::~USSReqProcessor()
{
  MutexGuard  grd(_sync);
  doCleanUp();
}

//
void USSReqProcessor::configure(const USSRequestCfg & use_cfg, const char * id_pfx)
{
  _cfg = &use_cfg;
  snprintf(_logId, sizeof(_logId)-1, "USSreq[%s:%u]", id_pfx, _wrkId);
}

//Note: input packet is completely deserialized !
void USSReqProcessor::wrkHandlePacket(const SPckUSSRequest & recv_pck) /*throw()*/
{
  bool wDone = false;
  {
    MutexGuard  grd(_sync);
    {
      char tBuf[interaction::USSRequestMessage::_ussReq_strSZ];
      recv_pck._Cmd.log2str(tBuf, (unsigned)sizeof(tBuf));
      smsc_log_info(_logger, "%s: <-- REQUEST %s", _logId, tBuf);
    }
    _reqData = recv_pck._Cmd.getOpData();
    _resPck.setDlgId(recv_pck.getDlgId());
    _resPck._Cmd._msIsdn = recv_pck._Cmd._msIsdn;
    
    if (!initMapDialog(recv_pck._Cmd._inSSN, recv_pck._Cmd._inAddr)) {
      _resPck._Cmd._status = interaction::USSResultMessage::reqFAILED;
      sendResult();
      doCleanUp();
      wDone = true;
    } else {
      try {
        const char *
          dlgId = _mapDlg->requestSS(recv_pck._Cmd._ussData, &_resPck._Cmd._msIsdn, recv_pck._Cmd.getIMSI());
        addRef(refIdMapDialog);
        smsc_log_debug(_logger, "%s: initiated %s", _logId, dlgId);
      } catch (const std::exception & ex) {
        smsc_log_error(_logger, "%s: MapUSSDlg exception %s", _logId, ex.what());
        _resPck._Cmd._status = interaction::USSResultMessage::reqFAILED;
        sendResult();
        doCleanUp();
        wDone = true;
      }
    }
  }
  if (wDone)
    _wrkMgr->workerDone(*this); //worker may be destroyed at this point
}

/* -------------------------------------------------------------------------- *
 * WorkerIface interface implementation:
 * -------------------------------------------------------------------------- */
void USSReqProcessor::wrkInit(smsc::inman::interaction::WorkerID w_id,
                              smsc::inman::interaction::WorkerManagerIface * use_mgr,
                              Logger * use_log/* = NULL*/)
{
  _wrkId = w_id; _wrkMgr = use_mgr;
  if (!(_logger = use_log))
    _logger = Logger::getInstance("smsc.ussman");
}

//Prints some information about worker state/status
void USSReqProcessor::wrkLogState(std::string & use_str) const
{
  MutexGuard grd(_sync);
  smsc::util::format(use_str, "%u{%u}", _wrkId, (unsigned)_state);
}

//NOTE: it's the processing graph entry point, so locks Mutex !!!
void USSReqProcessor::wrkAbort(const char * abrt_reason/* = NULL*/)
{
  bool wDone = false;
  {
    MutexGuard  grd(_sync);
    if (_state < procDone) {
      smsc_log_error(_logger, "%s: Aborting at state: %u%s%s", _logId, _state,
                     abrt_reason ? ", reason: " : "..", abrt_reason ? abrt_reason : "");
      _state = procAborted;
      _resPck._Cmd._status = interaction::USSResultMessage::reqFAILED;
      sendResult();
      doCleanUp();
      wDone = true;
    } else {
      smsc_log_warn(_logger, "%s: Aborting at state: %u%s%s", _logId, _state,
                     abrt_reason ? ", reason: " : "..", abrt_reason ? abrt_reason : "");
    }
  }
  if (wDone)
    _wrkMgr->workerDone(*this); //worker may be destroyed at this point
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
void USSReqProcessor::sendResult(void) const /*throw()*/
{
  smsc::inman::interaction::PacketBuffer_T<512>  pckBuf;
  try {
    _resPck.serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    smsc_log_fatal(_logger, "%s: --> RESULT serialization exception - %s", _logId, exc.what());
    return;
  }
  {
    char tBuf[interaction::USSResultMessage::_ussRes_strSZ];
    _resPck._Cmd.log2str(tBuf, (unsigned)sizeof(tBuf));
    smsc_log_info(_logger, "%s: --> RESULT %s", _logId, tBuf);
  }
  if (!_wrkMgr->sendPck(pckBuf)) {
    smsc_log_fatal(_logger, "%s: --> RESULT sending failed!", _logId);
  }
}


bool USSReqProcessor::initMapDialog(uint8_t rmt_ssn, const TonNpiAddress & rnpi)
  /*throw()*/
{
  if (!_cfg->_ssnSess || (_cfg->_ssnSess->bindStatus() < SSNBinding::ssnPartiallyBound)) {
    smsc_log_error(_logger, "%s: SSN[%u] is not available/bound!", _logId,
                   (unsigned)_cfg->_tcUsr.ownSsn);
    return false;
  }
  smsc::inman::inap::TCSessionSR * mapSess =
    _cfg->_ssnSess->newSRsession(_cfg->_tcUsr.ownAddr, _ac_map_networkUnstructuredSs_v2,
                                rmt_ssn, rnpi, _cfg->_tcUsr.fakeSsn);
  if (!mapSess) {
    std::string sid =
      _cfg->_ssnSess->mkSignature(_cfg->_tcUsr.ownAddr, _ac_map_networkUnstructuredSs_v2,
                                rmt_ssn, &rnpi);
    smsc_log_error(_logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
  } else {
    smsc_log_debug(_logger, "%s: using TCSR[%u]: %s", _logId, mapSess->getUID(),
                   mapSess->Signature().c_str());

    _mapDlg.init().init(*mapSess, *this, _logger);
  }
  return (mapSess != 0);
}

void USSReqProcessor::rlseMapDialog(void) /*throw()*/
{
  while (_mapDlg.get() && !_mapDlg->unbindUser()) { //MAPDlg refers this object
    addRef(refIdItself);
    _sync.wait();
    unRef(refIdItself);
  }
  if (_mapDlg.get())
    _mapDlg->endDialog();
  /* */
  unRef(refIdMapDialog);
}

void USSReqProcessor::doCleanUp(void) /*throw()*/
{
  if (hasRef(refIdMapDialog))
    rlseMapDialog();
  _mapDlg.clear();
}

// ------------------------------------------------------------------------
// -- USSDhandlerITF interface methods:
// ------------------------------------------------------------------------
void USSReqProcessor::onMapResult(const USSDataString & res_arg)
{
  MutexGuard  grd(_sync);
  if (_state > procInited) //aborted
    return;
  //If possible, convert uss data to Latin1/UCS2 text
  if (_resPck._Cmd._ussData.setText(res_arg) || _resPck._Cmd._ussData.setUCS2Text(res_arg))
    return;
  _resPck._Cmd._ussData = res_arg;
}

ObjAllcStatus_e
  USSReqProcessor::onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code/* = 0*/)
{
  ObjAllcStatus_e rval = ObjFinalizerIface::objActive;
  {
    MutexGuard  grd(_sync);
    rlseMapDialog(); //first unbind MAP User
  
    rval = use_finalizer.finalizeObj();
    if (rval == ObjFinalizerIface::objFinalized) {
      _mapDlg.clear();
      rval = ObjFinalizerIface::objDestroyed;
    }
  
    if (err_code) {
      smsc_log_error(_logger, "%s: MAPDlg error(%u) %s", _logId, err_code,
                     URCRegistry::explainHash(err_code).c_str());
      _resPck._Cmd._status = interaction::USSResultMessage::reqFAILED;
    }
    sendResult();
    _sync.notify();
  }
  _wrkMgr->workerDone(*this); //worker may be destroyed at this point
  return rval;
}

} //ussman
} //smsc

