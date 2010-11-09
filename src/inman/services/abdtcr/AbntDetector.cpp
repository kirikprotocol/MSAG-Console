#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/AbntDetector.hpp"
using smsc::inman::iaprvd::IAProviderITF;
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
int AbntDetectorManager::denyRequest(unsigned dlg_id, INManErrorId::Codes use_error)
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
        : WorkerAC(w_id, owner, uselog), providerQueried(false), abScf(0)
        , _wErr(0), _state(adIdle)
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
    unsigned short cmdId = (pck->pCmd())->Id();

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

    if (req->cacheMode() && _cfg.abCache)
        _cfg.abCache->getAbonentInfo(abNumber, &abRec, _cfg.cacheTmo);
    
    if (abRec.ab_type == AbonentContractInfo::abtPostpaid) {
        _state = adDetermined;
        return true;
    }
        
    if (_cfg.iaPol) {
        smsc_log_debug(logger, "%s: using policy: %s", _logId, _cfg.iaPol->Ident());
        if ((abRec.ab_type == AbonentContractInfo::abtUnknown)
             || !abRec.getSCFinfo(TDPCategory::dpMO_SM)) {
            // configure SCF by quering provider first
            IAProviderITF *prvd = _cfg.iaPol->getIAProvider();
            if (prvd) {
                if (StartTimer() && (providerQueried = prvd->startQuery(abNumber, this))) {
                    _state = adIAPQuering;
                    //execution will continue in onIAPQueried()/onTimerEvent() by another thread.
                    return false;
                }
                _wErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
                smsc_log_error(logger, "%s: startQuery(%s) failed!", _logId,
                               abNumber.getSignals());
            }
        }
    }
    //lookup config.xml for extra SCF parms (serviceKey, RPC lists)
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
//    _state = adIAPQueried;
    providerQueried = false;
    StopTimer();
    if (qry_status) {
        _wErr = qry_status;
        abRec.Merge(ab_info.abRec); //merge known abonent info
    } else {
        abRec = ab_info.abRec;      //renew abonent info, overwrite TDPScfMAP
        if (_cfg.abCache)
            _cfg.abCache->setAbonentInfo(abNumber, abRec);
    }
    if (abRec.ab_type == AbonentContractInfo::abtPrepaid)
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
    abRec.reset(); //abtUnknown
    _wErr = _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut);
    reportAndExit();
    return TimeWatcherITF::evtOk;
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock Mutex,
 *       it's a caller responsibility to lock it !!!
 * ---------------------------------------------------------------------------------- */
void AbonentDetector::ConfigureSCF(void)
{
    const GsmSCFinfo * p_scf = abRec.getSCFinfo(TDPCategory::dpMO_SM);
    if (!p_scf) //check if SCF for MO-BC may be used
        p_scf = abRec.getSCFinfo(TDPCategory::dpMO_BC);

    if (p_scf) { //SCF is set only for prepaid abonent by cache/IAProvider
        if (_cfg.iaPol)  //lookup policy for extra SCF parms (serviceKey, RPC lists)
            abScf = _cfg.iaPol->getSCFparms(&(p_scf->scfAddress));
    } else if (_cfg.iaPol) {  //attempt to determine SCF and its params from config.xml
        //look for single IN serving
        if (_cfg.iaPol->ScfMap().size() == 1)
            abScf = _cfg.iaPol->ScfMap().begin()->second;
        if (!abScf) {
            if (!_wErr)
                _wErr = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
            smsc_log_error(logger, "%s: unable to get gsmSCF from config.xml", _logId);
        }
    }
    if (abRec.ab_type == AbonentContractInfo::abtPrepaid) {
        if (!abRec.getSCFinfo(TDPCategory::dpMO_SM) && abScf) {
            uint32_t skeyMOSM = abScf->getSKey(&abRec.tdpSCF);
            if (skeyMOSM) //renew MO-SM SCF params
                abRec.tdpSCF[TDPCategory::dpMO_SM] = GsmSCFinfo(abScf->scfAdr, skeyMOSM);
        }
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
         abNumber.getSignals(), AbonentContractInfo::type2Str(abRec.ab_type));

  if (abRec.ab_type == AbonentContractInfo::abtUnknown) {
      format(dstr, ", errCode %u", _wErr);
      if (_wErr) {
          dstr += ": "; dstr += URCRegistry::explainHash(_wErr);
      }
  } else {
      if (abRec.ab_type == AbonentContractInfo::abtPrepaid) {
          const GsmSCFinfo * pScf = abRec.getSCFinfo(TDPCategory::dpMO_SM);

          if (!pScf) { //check if SCF for MO-BC may be used
              pScf = abRec.getSCFinfo(TDPCategory::dpMO_BC);
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
      spck.Cmd().setContractInfo(abRec.ab_type, abRec.getImsi());
      if (!smScf.empty())
        spck.Cmd().setGsmSCF(smScf);
      if (_cfg.iaPol)
          spck.Cmd().setPolicy(_cfg.iaPol->Ident());
  }
  return _mgr->sendCmd(&spck);
}

void AbonentDetector::doCleanUp(void)
{
    if (providerQueried) {  //check for pending query to AbonentProvider
        _cfg.iaPol->getIAProvider()->cancelQuery(abNumber, this);
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

