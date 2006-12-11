#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "AbntDetector.hpp"
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPCSAbntContract;
using smsc::inman::interaction::SPckContractResult;
using smsc::inman::interaction::AbntContractRequest;
using smsc::inman::interaction::CSAbntContractHdr_dlg;

#include "inerrcodes.hpp"
using smsc::inman::InmanErrorCode;

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
        spck.Cmd().setError(InmanErrorCode::combineError(
                        errInman, InProtocol_GeneralError));
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
                spck.Cmd().setError(InmanErrorCode::combineError(
                                errInman, InLogic_ResourceLimitation));
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
        : WorkerAC(w_id, owner, uselog), providerQueried(false)
        , abPolicy(NULL), iapTimer(NULL)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman");
    _cfg = owner->getConfig();
    snprintf(_logId, sizeof(_logId)-1, "AbntDet[%u:%u]", _mgr->cmId(), _wId);
}


AbonentDetector::~AbonentDetector()
{
    MutexGuard grd(_mutex);
    doCleanUp();
//    smsc_log_debug(logger, "%s: Deleted", _logId);
}

/* -------------------------------------------------------------------------- *
 * WorkerAC interface implementation:
 * -------------------------------------------------------------------------- */
 //NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::handleCommand(INPPacketAC* pck)
{
    MutexGuard  grd(_mutex);
    unsigned short cmdId = (pck->pCmd())->Id();

    bool goon = true;
    //complete the command deserialization
    try { (pck->pCmd())->loadDataBuf(); }
    catch (SerializerException & exc) {
        smsc_log_error(logger, "%s: %s", _logId, exc.what());
        goon = false;
    }
    if (!goon || !onContractReq(static_cast<AbntContractRequest*>(pck->pCmd()), _wId)) {
        sendResult(InmanErrorCode::combineError(
                errInman, InProtocol_InvalidData));
        doCleanUp();
        _mgr->workerDone(this);
    }
    //execution continues either in onTimerEvent() or in onIAPQueried()
    return;
}

//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::Abort(const char * reason/* = NULL*/)
{
    MutexGuard grd(_mutex);
    smsc_log_error(logger, "%s: Aborting %s%s", _logId,
                   reason ? ", reason: " : "..", reason ? reason : "");
    doCleanUp();
}


/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::onIAPQueried(const AbonentId & ab_number, AbonentBillType ab_type,
                           const MAPSCFinfo * scf/* = NULL*/)
{
    MutexGuard grd(_mutex);
    providerQueried = false;
    StopTimer();
    ConfigureSCFandReport(ab_type, scf);
    _mgr->workerDone(this);
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks Mutex !!!
void AbonentDetector::onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj)
{
    MutexGuard grd(_mutex);
    smsc_log_debug(logger, "%s: timer[%u] signaled", _logId, iapTimer->getId());
    iapTimer = NULL;
    doCleanUp();
    abRec.reset(); //abtUnknown
    sendResult(InmanErrorCode::combineError(errInman, InLogic_TimedOut));
    _mgr->workerDone(this);
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock Mutex,
 *       it's a caller responsibility to lock it !!!
 * ---------------------------------------------------------------------------------- */
void AbonentDetector::doCleanUp(void)
{
    if (providerQueried) {  //check for pending query to AbonentProvider
        abPolicy->getIAProvider(logger)->cancelQuery(abNumber, this);
        providerQueried = false;
    }
    if (iapTimer) //release active timer
        StopTimer();
    return;
}

/* -------------------------------------------------------------------------- *
 * AbntContractReqHandlerITF
 * -------------------------------------------------------------------------- */
bool AbonentDetector::onContractReq(AbntContractRequest* req, uint32_t req_id)
{
    if (!abNumber.fromText(req->subscrAddr().c_str())) {
        smsc_log_error(logger, "%s: invalid AbntAdr <%s>", _logId, 
                       req->subscrAddr().c_str());
        return false;
    }
    smsc_log_info(logger, "%s: AbntAdr <%s>", _logId, req->subscrAddr().c_str());

    if (req->cacheMode())
        _cfg.abCache->getAbonentInfo(abNumber, &abRec);
    
    if (abRec.ab_type != AbonentContractInfo::abtPostpaid)
        sendResult();

    if (!(abPolicy = _cfg.policies->getPolicy(&abNumber))) {
        smsc_log_error(logger, "%s: no policy set for %s", _logId, 
                       abNumber.toString().c_str());
    } else {
        smsc_log_debug(logger, "%s: using policy: %s", _logId, abPolicy->Ident());
        if ((abRec.ab_type == AbonentContractInfo::abtUnknown)
            || ((abRec.ab_type == AbonentContractInfo::abtPrepaid) && !abRec.getSCFinfo())) {
            // configure SCF by quering provider first
            IAProviderITF *prvd = abPolicy->getIAProvider(logger);
            if (prvd) {
                if (prvd->startQuery(abNumber, this)) {
                    providerQueried = true;
                    StartTimer(_cfg.abtTimeout);
                    return true; //execution will continue in onIAPQueried() by another thread.
                }
                smsc_log_error(logger, "%s: startIAPQuery(%s) failed!", _logId,
                               abNumber.getSignals());
            }
        }
    }
    ConfigureSCFandReport(abRec.ab_type, abRec.getSCFinfo());
    return true;
}

//here goes: abtPrepaid or abtUnknown or abtPostpaid(SMSX)
void AbonentDetector::ConfigureSCFandReport(AbonentBillType ab_type, const MAPSCFinfo * p_scf/* = NULL*/)
{
    bool scfKnown = true;
    abRec.ab_type = ab_type;
    INScfCFG        abScf;

    if (p_scf) { //SCF is set for abonent by cache/IAProvider
        abScf.scf = *p_scf;
        //lookup policy for extra SCF parms (serviceKey, RPC lists)
        if (abPolicy)
            abPolicy->getSCFparms(&abScf);
    } else if (abPolicy) {  //attempt to determine SCF and its params from config.xml
        if (ab_type == AbonentContractInfo::abtUnknown) {
            INScfCFG * pin = NULL;
            if (abPolicy->scfMap.size() == 1) { //single IN serving, look for postpaidRPC
                pin = (*(abPolicy->scfMap.begin())).second;
                if (pin->postpaidRPC.size())
                    abScf = *pin;
                else
                    pin = NULL;
            }
            if (!pin) {
                scfKnown = false;
                smsc_log_error(logger, "%s: unable to get gsmSCF from config.xml", _logId);
            }
        } else if (ab_type == AbonentContractInfo::abtPrepaid) {
            if (abPolicy->scfMap.size() == 1) { //single IN serving
                abScf = *((*(abPolicy->scfMap.begin())).second);
            } else {
                scfKnown = false;
                smsc_log_error(logger, "%s: unable to get gsmSCF from config.xml", _logId);
            }
        } else //btPostpaid
            scfKnown = false;
    } else
        scfKnown = false;

    if (!scfKnown && (ab_type == AbonentContractInfo::abtPrepaid))
        abRec.ab_type = AbonentContractInfo::abtUnknown;
    abRec.gsmSCF = abScf.scf;
    sendResult();
}

//NOTE: _mutex should be locked upon entry
void AbonentDetector::sendResult(uint32_t inmanErr /* = 0*/)
{
    smsc_log_info(logger, "%s: <-- RESULT, abonent(%s) type: %s (%u), errCode: %u", _logId,
                    abNumber.getSignals(), AbonentContractInfo::type2Str(abRec.ab_type),
                    abRec.ab_type, inmanErr);

    SPckContractResult spck;
    spck.Hdr().dlgId = _wId;
    if (inmanErr)
        spck.Cmd().setError(inmanErr);
    else {
        spck.Cmd().setContractInfo(abRec);
        if (abPolicy)
            spck.Cmd().setPolicy(abPolicy->Ident());
    }
    _mgr->sendCmd(&spck);
}

} //inman
} //smsc

