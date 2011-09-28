#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/AbntDetector.hpp"
using smsc::util::format;
using smsc::util::URCRegistry;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::MutexTryGuard;

using smsc::inman::iaprvd::IAPProperty;
using smsc::inman::iaprvd::IAProviderAC;
using smsc::inman::iaprvd::CSIRecord;

using smsc::inman::iapmgr::IAProviderInfo;

using smsc::inman::comp::UnifiedCSI;

using smsc::inman::interaction::SPckContractResult;
using smsc::inman::interaction::AbntContractRequest;

#include "inman/INManErrors.hpp"
using smsc::inman::_RCS_INManErrors;

namespace smsc {
namespace inman {
namespace abdtcr {
/* ************************************************************************** *
 * class AbonentDetector implementation:
 * ************************************************************************** */
AbonentDetector::~AbonentDetector()
{
  MutexGuard grd(_sync);
  doCleanUp();
  if (_logger)
    _logger->log_((_state < adCompleted) ? Logger::LEVEL_WARN : Logger::LEVEL_DEBUG,
                  "%s: Deleted at state %s", _logId, state2Str());
}

const char * AbonentDetector::state2Str(ADState_e st)
{
  static const char *_nm_state[] = {
    "adIdle", "adIAPQuering"
  , "adTimedOut", "adDetermined"
  , "adCompleted", "adAborted"
  };
  return _nm_state[st];
}

void AbonentDetector::configure(const AbonentDetectorCFG & use_cfg, const char * id_pfx)
{
  _cfg = use_cfg;
  snprintf(_logId, sizeof(_logId)-1, "AbntDet[%s:%u]", id_pfx, _wrkId);
}

//Note: input packet is completely deserialized !
void AbonentDetector::wrkHandlePacket(const SPckContractRequest & recv_pck) /*throw()*/
{
  bool wDone = true;
  {
    MutexGuard  grd(_sync);
    if (_state != adIdle) {
      smsc_log_error(_logger, "%s: protocol error: cmd %u is received at state %s",
                     _logId, (unsigned)recv_pck._Cmd.Id(), state2Str());
      return;
    }
      
    if (verifyReq(&recv_pck._Cmd))
      wDone = onContractReq(&recv_pck._Cmd, _wrkId);
    else
      _wErr = _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
  
    if (wDone)
      reportAndClean();
  }
  if (wDone)
    _wrkMgr->workerDone(*this);
  //else: execution continues either in onTimerEvent() or in onIAPQueried()
}

/* -------------------------------------------------------------------------- *
 * WorkerIface interface implementation:
 * -------------------------------------------------------------------------- */
void AbonentDetector::wrkInit(smsc::inman::interaction::WorkerID w_id,
                              smsc::inman::interaction::WorkerManagerIface * use_mgr,
                              Logger * use_log/* = NULL*/)
{
  _wrkId = w_id; _wrkMgr = use_mgr;
  if (!(_logger = use_log))
    _logger = Logger::getInstance("smsc.inman.abdtcr");
}

//Prints some information about worker state/status
void AbonentDetector::wrkLogState(std::string & use_str) const
{
  MutexGuard grd(_sync);
  format(use_str, "%u{%u}", _wrkId, (unsigned)_state);
}

//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::wrkAbort(const char * abrt_reason/* = NULL*/)
{
  bool wDone = false;
  {
    MutexGuard  grd(_sync);
    if (_state < adCompleted) {
      smsc_log_error(_logger, "%s: Aborting at state: %u%s%s", _logId, _state,
                     abrt_reason ? ", reason: " : "..", abrt_reason ? abrt_reason : "");
      _state = adAborted;
      reportAndClean();
      wDone = true;
    } else {
      smsc_log_warn(_logger, "%s: Aborting at state: %u%s%s", _logId, _state,
                     abrt_reason ? ", reason: " : "..", abrt_reason ? abrt_reason : "");
    }
  }
  if (wDone)
    _wrkMgr->workerDone(*this);
}

/* -------------------------------------------------------------------------- *
 * AbntContractReqHandlerITF methods and helpers
 * -------------------------------------------------------------------------- */
bool AbonentDetector::verifyReq(const AbntContractRequest * p_req)
{
  if (!_abNumber.fromText(p_req->subscrAddr().c_str())) {
    smsc_log_error(_logger, "%s: invalid AbntAdr <%s>", _logId, 
                   p_req->subscrAddr().c_str());
    return false;
  }
  return true;
}

//Returns false if execution will continue in another thread. 
bool AbonentDetector::onContractReq(const AbntContractRequest * p_req, uint32_t req_id)
{
  smsc_log_info(_logger, "%s: <-- REQUEST, abonent(%s), cacheMode(%s)", _logId,
                p_req->subscrAddr().c_str(), p_req->cacheMode() ? "true" : "false");

  if (p_req->cacheMode() && _cfg.abCache) {
    _cfg.abCache->getAbonentInfo(_abNumber, &_abCsi, _cfg.cacheTmo);
    if (!_abCsi.isUnknown()) {
      _state = adDetermined;
      return true;
    }
  }
  // ----------------------------
  //Here goes only abtUnknown
  // ----------------------------

  //determine policy rule
  const IAPRule * pRule = _cfg.iapMgr->getPolicy(_abNumber);
  if (pRule) {
    _iapRule = *pRule;
  } else {
    _iapRule._nmPool = "<default>";
    _iapRule._iaPolicy = _cfg.iapMgr->getPolicy(_cfg.policyNm);
  }

  if (!_iapRule._iaPolicy) {
    smsc_log_error(_logger, "%s: no IAPolicy configured for %s", _logId, _abNumber.getSignals());
  } else {
    smsc_log_debug(_logger, "%s: using policy: %s", _logId, _iapRule.toString().c_str());
    if (_iapRule._iaPolicy->_prvdPrio.first.empty()) {
      _wErr = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
      smsc_log_error(_logger, "%s: no IAProvider configured for %s", _logId, _abNumber.getSignals());
    } else if (startIAPQuery()) {
      _state = adIAPQuering;
      //execution will continue in onIAPQueried()/onTimerEvent() by another thread.
      return false;
    }
  }
  //lookup config.xml for extra SCF parms (serviceKey, ...)
  if (_abCsi.isPrepaid())
    configureSCF();
  _state = adDetermined;
  return true;
}

//Returns true if qyery is started, so execution will continue in another thread.
bool AbonentDetector::startIAPQuery(void)
{
  unsigned i = _curIAPrvd;
  bool prvdQueried = false;

  while (i < AbonentPolicy::iapSecondary) {
    const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(static_cast<IAPPrio_e>(++i));
    if (pPrvd) {
      _curIAPrvd = static_cast<IAPPrio_e>(i);

      if ((prvdQueried = pPrvd->_iface->startQuery(_abNumber, *this))) {
        if ((prvdQueried = startTimer()))
          break;
        //problems with timer facility
        while (pPrvd && !pPrvd->_iface->cancelQuery(_abNumber, *this)) {
          addRef(refIdItself);
          _sync.wait();
          unRef(refIdItself);
        }
        _wErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
        break;
        /* */
      } else {
        _wErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
        smsc_log_error(_logger, "%s: %s.startQuery(%s) failed!", _logId, pPrvd->_ident.c_str(),
                       _abNumber.getSignals());
      }
    }
  }
  if (prvdQueried)
    addRef(refIdIAProvider);
  return prvdQueried;
}


/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//Returns false if listener unable to handle query report right now, so
//requests query to be rereported.
//NOTE: it's the processing graph entry point, so locks Mutex !!!
bool AbonentDetector::onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                    RCHash qry_status)
{
  {
    MutexGuard grd(_sync);
    unRef(refIdIAProvider);
  
    if (_state != adIAPQuering) {
      smsc_log_warn(_logger, "%s: onIAPQueried() at state: %s", _logId, state2Str());
      return true;
    }
    stopTimer();
  
    if (qry_status) {
      _wErr = qry_status;
      if (startIAPQuery()) //check if next IAProvider may ne requested
        return true;       //keep adIAPQuering state
    } else {
      _abCsi.Merge(ab_info); //merge known abonent info
      if (_cfg.abCache) {
        try {
          _cfg.abCache->setAbonentInfo(_abNumber, _abCsi); //throws
        } catch (const std::exception & exc) {
          smsc_log_error(_logger, "%s: abCache exception: %s", _logId, exc.what());
        }
      }
    }
    _state = adDetermined;
    if (_abCsi.isPrepaid())
      configureSCF(); //lookup config.xml for extra SCF parms (serviceKey, ...)
    reportAndClean();
  }
  _wrkMgr->workerDone(*this);
  return true;
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks Mutex !!!
TimeWatcherITF::SignalResult
    AbonentDetector::onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj)
{
  {
    MutexTryGuard grd(_sync);
    if (!grd.tgtLocked()) //detector is busy, request resignalling
      return TimeWatcherITF::evtResignal;
  
    smsc_log_debug(_logger, "%s: timer[%s] signaled at state %s", _logId,
                   tm_hdl.IdStr(), state2Str());
    unRef(refIdTMWatcher);
    _iapTimer.clear();

    if (_state > adIAPQuering)
      return TimeWatcherITF::evtOk;
  
    if (startIAPQuery())              //check if next IAProvider may ne requested
      return TimeWatcherITF::evtOk;   //keep adIAPQuering state
  
    _state = adTimedOut;
    //_abCsi.clear(); //abtUnknown
    _wErr = _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut);
    reportAndClean();
  }
  _wrkMgr->workerDone(*this);
  return TimeWatcherITF::evtOk;
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock Mutex,
 *       it's a caller responsibility to lock it !!!
 * ---------------------------------------------------------------------------------- */
//Checks abonent subscription for MO_SM parameters, in case of absence
//attempts to determine it from IAPManager configuration
void AbonentDetector::configureSCF(void)
{
  configureMOSM();
  //verify IMSI
  if (!_abCsi.getImsi() && _cfgScf && !_cfgScf->_prm->_dfltImsi.empty())
    _abCsi.setImsi(_cfgScf->_prm->_dfltImsi.c_str());
  if (!_abCsi.getImsi()) {
    smsc_log_warn(_logger, "%s: unable to determine IMSI for abonent(%s)", _logId,
                   _abNumber.toString().c_str());
  }
}

//Adjusts the MO-SM gsmSCF parameters combining cache/IAProvider CSIs
//and gsmSCF parameters from config.xml
void AbonentDetector::configureMOSM(void)
{
  uint32_t          keyMOSM = 0;
  const CSIRecord * pCsi = _abCsi.csiSCF.find(UnifiedCSI::csi_MO_SM);

  if (pCsi)
    keyMOSM = pCsi->scfInfo.serviceKey;
  else //check if SCF for MO-BC may be used
    pCsi = _abCsi.csiSCF.find(UnifiedCSI::csi_O_BC);

  //check if MO_SM parameters are configured for serving gsmSCF
  //serviceKey from config.xml has a higher priority
  if (!_cfgScf && pCsi && _iapRule._iaPolicy)
    _cfgScf = _iapRule._iaPolicy->getSCFparms(pCsi->scfInfo.scfAddress);

  if (_cfgScf) {
    uint32_t  keyCfg = _cfgScf->getSKey(UnifiedCSI::csi_MO_SM, &_abCsi.csiSCF);

    if (keyCfg && (keyCfg != keyMOSM)) { //update/create MO_SM record
      CSIRecord & csiRec = _abCsi.csiSCF[UnifiedCSI::csi_MO_SM];
      csiRec.iapId = IAPProperty::iapUnknown;
      csiRec.scfInfo.serviceKey = keyCfg;
      if (pCsi->csiId != UnifiedCSI::csi_MO_SM)
        csiRec.scfInfo.scfAddress = pCsi->scfInfo.scfAddress;
    }
  }
}

//
bool AbonentDetector::startTimer(void)
{
  TimeWatcherITF::Error tErr = TimeWatcherITF::errBadTimer;

  _iapTimer.init(_cfg.abtTimeout.CreateTimer(this));
  if (_iapTimer->Id() && ((tErr = _iapTimer->Start()) == TimeWatcherITF::errOk)) {
    smsc_log_debug(_logger, "%s: started timer[%s]", _logId, _iapTimer->IdStr());
    return true;
  }
  smsc_log_error(_logger, "%s: failed to start timer[%s], code: %u",
                 _logId, _iapTimer->IdStr(), tErr);
  return false;
}
//
void AbonentDetector::stopTimer(void)
{
  if (_iapTimer.get()) {
    smsc_log_debug(_logger, "%s: releasing timer[%s]", _logId, _iapTimer->IdStr());
    _iapTimer->Stop();
    _iapTimer.clear();
  }
}
//
bool AbonentDetector::sendResult(void) const
{
  GsmSCFinfo  smScf;
  std::string dstr;
  format(dstr, "%s: --> RESULT, abonent(%s) type %s", _logId,
         _abNumber.getSignals(), _abCsi.type2Str());

  if (_abCsi.isUnknown()) {
    format(dstr, ", errCode %u", _wErr);
    if (_wErr) {
      dstr += ": "; dstr += URCRegistry::explainHash(_wErr);
    }
  } else {
    if (_abCsi.isPrepaid()) {
      const GsmSCFinfo * pScf = _abCsi.getSCFinfo(UnifiedCSI::csi_MO_SM);

      if (!pScf) { //check if SCF for MO-BC may be used
        pScf = _abCsi.getSCFinfo(UnifiedCSI::csi_O_BC);
        smScf.scfAddress = pScf->scfAddress;
      } else 
        smScf = *pScf;
      format(dstr, ", SCF %s", smScf.toString().c_str());
    }
    if (_abCsi.getImsi()) {
      dstr += ", IMSI: "; dstr += _abCsi.getImsi();
    }
  }
  smsc_log_info(_logger, dstr.c_str());
  SPckContractResult spck;
  spck._Hdr.dlgId = _wrkId;
  
  if (_abCsi.isUnknown() && _wErr)
    spck._Cmd.setError(_wErr, URCRegistry::explainHash(_wErr).c_str());
  else {
    spck._Cmd.setContractInfo(_abCsi.abType, _abCsi.getImsi());
    if (!smScf.empty())
      spck._Cmd.setGsmSCF(smScf);
    if (_iapRule._iaPolicy)
      spck._Cmd.setPolicy(_iapRule._iaPolicy->getIdent());
  }

  smsc::inman::interaction::PacketBuffer_T<512>  pckBuf;
  try {
    spck.serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    smsc_log_fatal(_logger, "%s: --> RESULT serialization exception - %s", _logId, exc.what());
    return false;
  }
  return (_wrkMgr->sendPck(pckBuf) != 0);
}
//
void AbonentDetector::doCleanUp(void)
{
  if (hasRef(refIdIAProvider)) {  //check for pending query to AbonentProvider
    const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(_curIAPrvd);
    while (pPrvd && !pPrvd->_iface->cancelQuery(_abNumber, *this)) {
      addRef(refIdItself);
      _sync.wait();
      unRef(refIdItself);
    }
    unRef(refIdIAProvider);
  }
  stopTimer();
}
//
void AbonentDetector::reportAndClean(void)
{
  doCleanUp();
  if (!sendResult()) {
    smsc::util::CustomException sndExc;
    if (_wrkMgr->hasExceptionOnSend(&sndExc)) {
      smsc_log_error(_logger, "%s: result sending failure: %s", _logId, sndExc.what());
    } else {
      smsc_log_error(_logger, "%s: result sending failure", _logId);
    }
  }
  if (_state < adCompleted)
    _state = adCompleted;
}

} //abdtcr
} //inman
} //smsc

