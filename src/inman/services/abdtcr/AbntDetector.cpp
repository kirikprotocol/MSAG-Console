#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/AbntDetector.hpp"

using smsc::inman::iaprvd::IAPProperty;
using smsc::inman::iaprvd::IAProviderAC;
using smsc::inman::iaprvd::CSIRecord;

using smsc::inman::iapmgr::IAProviderInfo;

using smsc::inman::comp::UnifiedCSI;

using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPCSAbntContract;
using smsc::inman::interaction::SPckContractResult;
using smsc::inman::interaction::AbntContractRequest;
using smsc::inman::interaction::CSAbntContractHdr_dlg;

#include "inman/INManErrors.hpp"
using smsc::inman::_RCS_INManErrors;

namespace smsc {
namespace inman {
namespace abdtcr {

/* ************************************************************************** *
 * class AbntDetectorManager implementation:
 * ************************************************************************** */
//NOTE: _mutex SHOULD be locked upon entry!
int AbntDetectorManager::denyRequest(unsigned dlg_id, INManErrorId::Code_e use_error)
{
    if (!_conn)
        return -1;

    SPckContractResult spck;
    spck.Hdr().dlgId = dlg_id;
    spck.Cmd().setError(_RCS_INManErrors->mkhash(use_error),
                        _RCS_INManErrors->explainCode(use_error).c_str());

    smsc_log_debug(logger, "%s: <-- AbntDet[%u], errCode %u: %s",
                   _logId, dlg_id, spck.Cmd().errorCode(), spck.Cmd().errorMsg());
    return _conn->sendPck(&spck);
}

/* -------------------------------------------------------------------------- *
 * ConnectListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
void AbntDetectorManager::onPacketReceived(Connect* conn,
                                std::auto_ptr<SerializablePacketAC>& recv_cmd)
                                /*throw(std::exception)*/
{
    INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
    //check for header
    if (!pck->pHdr() || ((pck->pHdr())->Id() != INPCSAbntContract::HDR_DIALOG)) {
        smsc_log_error(logger, "%s: missed/unsupported cmd header", _logId);
        return;
    }
    CSAbntContractHdr_dlg * srvHdr = static_cast<CSAbntContractHdr_dlg*>(pck->pHdr());

    if (pck->pCmd()->Id() != INPCSAbntContract::ABNT_CONTRACT_REQUEST_TAG) {
        smsc_log_error(logger, "%s: illegal Cmd[0x%X] received", _logId,
                       pck->pCmd()->Id());
        denyRequest(srvHdr->dlgId, INManErrorId::protocolGeneralError);
        //ignore sending failure here !!!
        return;
    }
    AbonentDetector * dtr = NULL;
    {
        MutexGuard   grd(_mutex);
        smsc_log_debug(logger, "%s: Cmd[0x%X] for AbntDet[%u] received", _logId,
                       pck->pCmd()->Id(), srvHdr->dlgId);
        if (!isRunning()) {
            denyRequest(srvHdr->dlgId, INManErrorId::srvInoperative); //ignore sending result here
            return;
        }

        dtr = (AbonentDetector *)getWorker(srvHdr->dlgId);
        if (!dtr) {
            if (numWorkers() < _cfg.maxRequests) {
                insWorker(dtr = new AbonentDetector(srvHdr->dlgId, this, logger));
            } else {
                smsc_log_error(logger, "%s: maxRequests limit reached: %u", _logId,
                            _cfg.maxRequests);
                denyRequest(srvHdr->dlgId, INManErrorId::cfgLimitation); //ignore sending result here

                if (logger->isDebugEnabled()) {
                    std::string dump;
                    format(dump, "%s: Workers [%u of %u]: ", _logId,
                           numWorkers(), _cfg.maxRequests);
                    dumpWorkers(dump);
                    smsc_log_debug(logger, dump.c_str());
                }   
            }
        }
    } //grd off
    if (dtr)
        dtr->handleCommand(pck);
    return;
}

/* ************************************************************************** *
 * class AbonentDetector implementation:
 * ************************************************************************** */
AbonentDetector::AbonentDetector(unsigned w_id, AbntDetectorManager * owner, Logger * uselog/* = NULL*/)
  : WorkerAC(w_id, owner, uselog), _state(adIdle), _lastIAPrvd(IAPProperty::iapUnknown)
  , providerQueried(false), _cfgScf(NULL), _wErr(0)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman");
    _cfg = owner->getConfig();
    snprintf(_logId, sizeof(_logId)-1, "AbntDet[%u:%u]", _mgr->cmId(), _wId);
}


AbonentDetector::~AbonentDetector()
{
    MutexGuard grd(_mutex);
    doCleanUp();
    smsc_log_debug(logger, "%s: Deleted", _logId);
}

const char * AbonentDetector::State2Str(ADState st)
{
    static const char *_nm_state[] = {
        "adIdle", "adIAPQuering"
      , "adTimedOut", "adDetermined"
      , "adCompleted", "adAborted"
    };
    return _nm_state[st];
}

/* -------------------------------------------------------------------------- *
 * WorkerAC interface implementation:
 * -------------------------------------------------------------------------- */
//Prints some information about worker state/status
void AbonentDetector::logState(std::string & use_str) const
{
    MutexGuard grd(_mutex);
    format(use_str, "%u{%u}", _wId, _state);
}

//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::handleCommand(INPPacketAC* pck)
{
    MutexGuard  grd(_mutex);
    //unsigned short cmdId = (pck->pCmd())->Id();

    bool wDone = false;
    //complete the command deserialization
    try { (pck->pCmd())->loadDataBuf(); }
    catch (const SerializerException & exc) {
        smsc_log_error(logger, "%s: %s", _logId, exc.what());
        wDone = true;
    }
    if (wDone || (wDone = !verifyReq(static_cast<AbntContractRequest*>(pck->pCmd()))))
        _wErr = _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
    else
        wDone = onContractReq(static_cast<AbntContractRequest*>(pck->pCmd()), _wId);

    if (wDone)
        reportAndExit();
    //else: execution continues either in onTimerEvent() or in onIAPQueried()
    return;
}

//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::Abort(const char * reason/* = NULL*/)
{
    MutexGuard  grd(_mutex);
    if (_state < adCompleted) {
      smsc_log_error(logger, "%s: Aborting at state: %u%s%s", _logId, _state,
                     reason ? ", reason: " : "..", reason ? reason : "");
      _state = adAborted;
      reportAndExit();
    } else {
      smsc_log_warn(logger, "%s: Aborting at state: %u%s%s", _logId, _state,
                     reason ? ", reason: " : "..", reason ? reason : "");
    }
}

/* -------------------------------------------------------------------------- *
 * AbntContractReqHandlerITF methods and helpers
 * -------------------------------------------------------------------------- */
bool AbonentDetector::verifyReq(AbntContractRequest* req)
{
    if (!abNumber.fromText(req->subscrAddr().c_str())) {
        smsc_log_error(logger, "%s: invalid AbntAdr <%s>", _logId, 
                       req->subscrAddr().c_str());
        return false;
    }
    return true;
}
//Returns false if execution will continue in another thread. 
bool AbonentDetector::onContractReq(AbntContractRequest* req, uint32_t req_id)
{
    smsc_log_info(logger, "%s: --> REQUEST, abonent(%s), cacheMode(%s)", _logId,
                  req->subscrAddr().c_str(), req->cacheMode() ? "true" : "false");

    if (req->cacheMode() && _cfg.abCache) {
      _cfg.abCache->getAbonentInfo(abNumber, &abRec, _cfg.cacheTmo);
      if (!abRec.isUnknown()) {
        _state = adDetermined;
        return true;
      }
    }
    // ----------------------------
    //Here goes only abtUnknown
    // ----------------------------

    //determine policy rule
    const IAPRule * pRule = _cfg.iapMgr->getPolicy(abNumber);
    if (pRule) {
      _iapRule = *pRule;
    } else {
      _iapRule._nmPool = "<default>";
      _iapRule._iaPolicy = _cfg.iapMgr->getPolicy(_cfg.policyNm);
    }
            
    if (_iapRule._iaPolicy) {
        smsc_log_debug(logger, "%s: using policy: %s", _logId, _iapRule.toString().c_str());

        const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(_lastIAPrvd);
        if (pPrvd) {
          _lastIAPrvd = pPrvd->_iface->getType();
          if (StartTimer() && (providerQueried = pPrvd->_iface->startQuery(abNumber, this))) {
              _state = adIAPQuering;
              //execution will continue in onIAPQueried()/onTimerEvent() by another thread.
              return false;
          }
          _wErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
          smsc_log_error(logger, "%s: startQuery(%s) failed!", _logId,
                         abNumber.getSignals());

        }
    }
    //lookup config.xml for extra SCF parms (serviceKey, RPC lists)
    if (abRec.isPrepaid())
      ConfigureSCF();
    _state = adDetermined;
    return true;
}

/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                    RCHash qry_status)
{
    MutexGuard grd(_mutex);
    if (_state != adIAPQuering) {
        smsc_log_warn(logger, "%s: IAPQueried() at state: %s", State2Str());
        return;
    }
    providerQueried = false;
    StopTimer();

    abRec.Merge(ab_info); //merge known abonent info
    if (qry_status)
      _wErr = qry_status;
    else if (_cfg.abCache)
      _cfg.abCache->setAbonentInfo(abNumber, abRec);

    if (abRec.isPrepaid())
      //lookup config.xml for extra SCF parms (serviceKey, RPC lists)
      ConfigureSCF();
    _state = adDetermined;
    reportAndExit();
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks Mutex !!!
TimeWatcherITF::SignalResult
    AbonentDetector::onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj)
{
    MutexTryGuard grd(_mutex);
    if (!grd.tgtLocked()) //detector is busy, request resignalling
        return TimeWatcherITF::evtResignal;

    smsc_log_debug(logger, "%s: timer[%s] signaled at state %s", _logId,
                   tm_hdl.IdStr(), State2Str());
    if (_state > adIAPQuering)
        return TimeWatcherITF::evtOk;

    iapTimer.reset();
    _state = adTimedOut;
    abRec.clear(); //abtUnknown
    _wErr = _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut);
    reportAndExit();
    return TimeWatcherITF::evtOk;
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock Mutex,
 *       it's a caller responsibility to lock it !!!
 * ---------------------------------------------------------------------------------- */
//Checks abonent subscription for MO_SM parameters, in case of absence
//attempts to determine it from IAPManager configuration
void AbonentDetector::ConfigureSCF(void)
{
    const GsmSCFinfo * p_scf = abRec.getSCFinfo(UnifiedCSI::csi_MO_SM);
    uint32_t           recMOSM = 0;

    if (p_scf)
      recMOSM = p_scf->serviceKey;
    else //check if SCF for MO-BC may be used
      p_scf = abRec.getSCFinfo(UnifiedCSI::csi_O_BC);

    //check if extra parameters are configuured for serving gsmSCF
    if (p_scf && _iapRule._iaPolicy)
      _cfgScf = _iapRule._iaPolicy->getSCFparms(p_scf->scfAddress);

    //verify serviceKey
    if (!recMOSM && _cfgScf) {
      CSIRecord csiRec(IAPProperty::iapUnknown, UnifiedCSI::csi_MO_SM, *p_scf);
      csiRec.scfInfo.serviceKey = _cfgScf->getSKey(UnifiedCSI::csi_MO_SM,
                                      abRec.csiSCF.empty() ? NULL : &abRec.csiSCF);
      //serviceKey from config.xml has a higher priority
      if (csiRec.scfInfo.serviceKey && (csiRec.scfInfo.serviceKey != recMOSM)) {
        abRec.csiSCF.insertRecord(csiRec);
        recMOSM = csiRec.scfInfo.serviceKey;
      }
    }

    //verify IMSI
    if (!abRec.getImsi() && _cfgScf && !_cfgScf->_dfltImsi.empty())
      abRec.setImsi(_cfgScf->_dfltImsi.c_str());
    if (!abRec.getImsi()) {
      smsc_log_warn(logger, "%s: unable to determine IMSI for abonent(%s)", _logId,
                     abNumber.toString().c_str());
    }
}

bool AbonentDetector::StartTimer(void)
{
    iapTimer.reset(new TimerHdl(_cfg.abtTimeout.CreateTimer(this)));
    TimeWatcherITF::Error tErr = TimeWatcherITF::errBadTimer;
    if (iapTimer->Id() && ((tErr = iapTimer->Start()) == TimeWatcherITF::errOk)) {
        smsc_log_debug(logger, "%s: started timer[%s]", _logId, iapTimer->IdStr());
        return true;
    }
    smsc_log_error(logger, "%s: failed to start timer[%s], code: %u",
                   _logId, iapTimer->IdStr(), tErr);
    return false;
}
void AbonentDetector::StopTimer(void)
{
    if (iapTimer.get()) {
        smsc_log_debug(logger, "%s: releasing timer[%s]", _logId, iapTimer->IdStr());
        iapTimer->Stop();
        iapTimer.reset();
    }
}

bool AbonentDetector::sendResult(void)
{
  GsmSCFinfo  smScf;
  std::string dstr;
  format(dstr, "%s: <-- RESULT, abonent(%s) type %s", _logId,
         abNumber.getSignals(), abRec.type2Str());

  if (abRec.isUnknown()) {
      format(dstr, ", errCode %u", _wErr);
      if (_wErr) {
          dstr += ": "; dstr += URCRegistry::explainHash(_wErr);
      }
  } else {
      if (abRec.isPrepaid()) {
          const GsmSCFinfo * pScf = abRec.getSCFinfo(UnifiedCSI::csi_MO_SM);

          if (!pScf) { //check if SCF for MO-BC may be used
              pScf = abRec.getSCFinfo(UnifiedCSI::csi_O_BC);
              smScf.scfAddress = pScf->scfAddress;
          } else 
              smScf = *pScf;
          format(dstr, ", SCF %s", smScf.toString().c_str());
      }
      if (abRec.getImsi()) {
          dstr += ", IMSI: "; dstr += abRec.getImsi();
      }
  }
  smsc_log_info(logger, dstr.c_str());
  SPckContractResult spck;
  spck.Hdr().dlgId = _wId;
  if (_wErr)
      spck.Cmd().setError(_wErr, URCRegistry::explainHash(_wErr).c_str());
  else {
      spck.Cmd().setContractInfo(abRec.abType, abRec.getImsi());
      if (!smScf.empty())
        spck.Cmd().setGsmSCF(smScf);
      if (_iapRule._iaPolicy)
          spck.Cmd().setPolicy(_iapRule._iaPolicy->getIdent());
  }
  return _mgr->sendCmd(&spck);
}

void AbonentDetector::doCleanUp(void)
{
    if (providerQueried) {  //check for pending query to AbonentProvider
      const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(_lastIAPrvd);
      if (pPrvd)
        pPrvd->_iface->cancelQuery(abNumber, this);
      providerQueried = false;
    }
    StopTimer();
    return;
}


void AbonentDetector::reportAndExit(void)
{
    doCleanUp();
    if (!sendResult()) {
      const char * reason = _mgr->connectError()->what();
      smsc_log_error(logger, "%s: result sending failure%s%s", _logId,
                    reason ? ", reason: " : " ..", reason ? reason : "");
    }
    if (_state < adCompleted)
      _state = adCompleted;
    /**/
    _mgr->workerDone(this);
}

} //abdtcr
} //inman
} //smsc

