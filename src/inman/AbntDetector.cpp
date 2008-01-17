#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/AbntDetector.hpp"
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPCSAbntContract;
using smsc::inman::interaction::SPckContractResult;
using smsc::inman::interaction::AbntContractRequest;
using smsc::inman::interaction::CSAbntContractHdr_dlg;

#include "inman/INManErrors.hpp"
using smsc::inman::iaprvd::_RCS_IAPQStatus;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class AbntDetectorManager implementation:
 * ************************************************************************** */

/* -------------------------------------------------------------------------- *
 * ConnectListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
void AbntDetectorManager::onPacketReceived(Connect* conn,
                                std::auto_ptr<SerializablePacketAC>& recv_cmd)
                                /*throw(std::exception)*/
{
    //check service header
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

        SPckContractResult spck;
        spck.Hdr().dlgId = srvHdr->dlgId;
        spck.Cmd().setError(
            _RCS_INManErrors->mkhash(INManErrorId::protocolGeneralError),
            _RCS_INManErrors->explainCode(INManErrorId::protocolGeneralError).c_str());

        _conn->sendPck(&spck);
    } else {
        smsc_log_debug(logger, "%s: Cmd[0x%X] for AbntDet[%u] received", _logId,
                       pck->pCmd()->Id(), srvHdr->dlgId);
    }

    AbonentDetector* dtr = NULL;
    {
        MutexGuard   grd(_mutex);
        cleanUpWorkers();
        
        WorkersMap::iterator it = workers.find(srvHdr->dlgId);
        if (it == workers.end()) {
            if (workers.size() < _cfg.maxRequests) {
                dtr = new AbonentDetector(srvHdr->dlgId, this, logger);
                workers.insert(WorkersMap::value_type(srvHdr->dlgId, dtr));
            } else {
                SPckContractResult spck;
                spck.Cmd().setError(
                    _RCS_INManErrors->mkhash(INManErrorId::cfgLimitation),
                    _RCS_INManErrors->explainCode(INManErrorId::cfgLimitation).c_str());
                spck.Hdr().dlgId = srvHdr->dlgId;
                _conn->sendPck(&spck);
                smsc_log_error(logger, "%s: maxRequests limit reached: %u", _logId,
                              _cfg.maxRequests);

                if (logger->isDebugEnabled()) {
                    std::string dump;
                    format(dump, "%s: Workers [%u of %u]: ", _logId,
                           workers.size(), _cfg.maxRequests);
                    for (it = workers.begin(); it != workers.end(); it++) {
                        format(dump, "[%u], ", ((*it).second)->getId());
                    }
                    smsc_log_debug(logger, dump.c_str());
                }   
            }
        } else
            dtr = (AbonentDetector*)((*it).second);
    }
    if (dtr)
        dtr->handleCommand(pck);
    return;
}

/* ************************************************************************** *
 * class AbonentDetector implementation:
 * ************************************************************************** */
AbonentDetector::AbonentDetector(unsigned w_id, AbntDetectorManager * owner, Logger * uselog/* = NULL*/)
        : WorkerAC(w_id, owner, uselog), providerQueried(false), abScf(0)
        , abPolicy(NULL), _wErr(0), _state(adIdle)
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
    static const char *_nm_state[] = { "adIdle",
        "adIAPQuering", "adIAPQueried",
        "adTimedOut", "adCompleted",
        "adReported", "adAborted"
    };
    return _nm_state[st];
}

/* -------------------------------------------------------------------------- *
 * WorkerAC interface implementation:
 * -------------------------------------------------------------------------- */
 //NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::handleCommand(INPPacketAC* pck)
{
    MutexGuard  grd(_mutex);
    unsigned short cmdId = (pck->pCmd())->Id();

    bool wDone = false;
    //complete the command deserialization
    try { (pck->pCmd())->loadDataBuf(); }
    catch (SerializerException & exc) {
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
    _state = adAborted;
    smsc_log_error(logger, "%s: Aborting %s%s", _logId,
                   reason ? ", reason: " : "..", reason ? reason : "");
    doCleanUp();
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

    if (req->cacheMode())
        _cfg.abCache->getAbonentInfo(abNumber, &abRec);
    
    if (abRec.ab_type == AbonentContractInfo::abtPostpaid) {
        _state = adCompleted;
        return true;
    }
        
    if (!(abPolicy = _cfg.policies->getPolicy(&abNumber))) {
        smsc_log_error(logger, "%s: no policy set for %s", _logId, 
                       abNumber.toString().c_str());
    } else {
        smsc_log_debug(logger, "%s: using policy: %s", _logId, abPolicy->Ident());
        if ((abRec.ab_type == AbonentContractInfo::abtUnknown) || !abRec.getSCFinfo(TDPCategory::dpMO_SM)) {
            // configure SCF by quering provider first
            IAProviderITF *prvd = abPolicy->getIAProvider();
            if (prvd) {
                if (StartTimer() && (providerQueried = prvd->startQuery(abNumber, this))) {
                    _state = adIAPQuering;
                    //execution will continue in onIAPQueried()/onTimerEvent() by another thread.
                    return false;
                }
                smsc_log_error(logger, "%s: startQuery(%s) failed!", _logId,
                               abNumber.getSignals());
            }
        }
    }
    //lookup config.xml for extra SCF parms (serviceKey, RPC lists)
    ConfigureSCF();
    _state = adCompleted;
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
    _state = adIAPQueried;
    providerQueried = false;
    StopTimer();
    if (qry_status) {
        _wErr = qry_status;
        abRec.Merge(ab_info.abRec); //merge known abonent info
    } else {
        abRec = ab_info.abRec;      //renew abonent info, overwrite TDPScfMAP
        _cfg.abCache->setAbonentInfo(abNumber, abRec);
    }
    if (abRec.ab_type == AbonentContractInfo::abtPrepaid)
        //lookup config.xml for extra SCF parms (serviceKey, RPC lists)
        ConfigureSCF();
    reportAndExit();
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks Mutex !!!
TimeWatcherITF::SignalResult
    AbonentDetector::onTimerEvent(TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj)
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
        if (abPolicy)    //lookup policy for extra SCF parms (serviceKey, RPC lists)
            abScf = abPolicy->getSCFparms(&(p_scf->scfAddress));
    } else if (abPolicy) {  //attempt to determine SCF and its params from config.xml
        //look for single IN serving
        if (abPolicy->scfMap.size() == 1)
            abScf = abPolicy->scfMap.begin()->second;
        if (!abScf)
            smsc_log_error(logger, "%s: unable to get gsmSCF from config.xml", _logId);
    }
    if (abRec.ab_type == AbonentContractInfo::abtPrepaid) {
        if (!abScf) {
            abRec.ab_type = AbonentContractInfo::abtUnknown;
        } else {
            uint32_t skeyMOSM = abScf->getSKey(&abRec.tdpSCF);
            if (skeyMOSM) //renew MO-SM SCF params
                abRec.tdpSCF[TDPCategory::dpMO_SM] = GsmSCFinfo(abScf->scfAdr, skeyMOSM);
        }
    }
}

void AbonentDetector::doCleanUp(void)
{
    if (providerQueried) {  //check for pending query to AbonentProvider
        abPolicy->getIAProvider()->cancelQuery(abNumber, this);
        providerQueried = false;
    }
    StopTimer();
    return;
}

void AbonentDetector::reportAndExit(void)
{
    doCleanUp();
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
            GsmSCFinfo          smScf;
            const GsmSCFinfo *  p_scf = abRec.getSCFinfo(TDPCategory::dpMO_SM);

            if (!p_scf) { //check if SCF for MO-BC may be used
                p_scf = abRec.getSCFinfo(TDPCategory::dpMO_BC);
                smScf.scfAddress = p_scf->scfAddress;
            } else 
                smScf = *p_scf;
            format(dstr, ", SCF %s", smScf.toString().c_str());
        }

        if (abRec.getImsi()) {
            dstr += ", IMSI: "; dstr += abRec.getImsi();
        }
        smsc_log_info(logger, dstr.c_str());
    }

    SPckContractResult spck;
    spck.Hdr().dlgId = _wId;
    if (_wErr)
        spck.Cmd().setError(_wErr, URCRegistry::explainHash(_wErr).c_str());
    else {
        spck.Cmd().setContractInfo(abRec);
        if (abPolicy)
            spck.Cmd().setPolicy(abPolicy->Ident());
    }
    _mgr->sendCmd(&spck);
    _state = adReported;
    /**/
    _mgr->workerDone(this);
}

} //inman
} //smsc

