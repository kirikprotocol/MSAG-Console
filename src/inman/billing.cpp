#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
#include <assert.h>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::SSNSession;
using smsc::inman::inap::TCSessionAC;

#include "inman/billing.hpp"
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPCSBilling;
using smsc::inman::interaction::SPckChargeSmsResult;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class BillingManager implementation:
 * ************************************************************************** */

/* -------------------------------------------------------------------------- *
 * ConnectListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
void BillingManager::onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
                        /*throw(std::exception)*/
{
    //check service header
    INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
    //check for header
    if (!pck->pHdr() || ((pck->pHdr())->Id() != INPCSBilling::HDR_DIALOG)) {
        smsc_log_error(logger, "%s: missed/unsupported cmd header", _logId);
        return;
    }
    Billing* bill = NULL;
    CsBillingHdr_dlg * srvHdr = static_cast<CsBillingHdr_dlg*>(pck->pHdr());
    {
        MutexGuard   grd(_mutex);
        //delete died billings first
        cleanUpWorkers();
        //assign command to Billing
        smsc_log_debug(logger, "%s: Cmd[0x%X] for Billing[%u] received", _logId,
                       pck->pCmd()->Id(), srvHdr->dlgId);
        
        WorkersMap::iterator it = workers.find(srvHdr->dlgId);
        if (it == workers.end()) {
            if (workers.size() < _cfg.maxBilling) {
                bill = new Billing(srvHdr->dlgId, this, logger);
                workers.insert(WorkersMap::value_type(srvHdr->dlgId, bill));
            } else {
                SPckChargeSmsResult spck;
                spck.Cmd().SetValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
                spck.Cmd().setError(errInman, InLogic_ResourceLimitation);
                spck.Hdr().dlgId = srvHdr->dlgId;
                _conn->sendPck(&spck);
                smsc_log_warn(logger, "%s: maxBilling limit reached: %u", _logId,
                              _cfg.maxBilling);

                if (logger->isDebugEnabled()) {
                    std::string dump;
                    format(dump, "%s: Workers [%u of %u]: ", _logId,
                           workers.size(), _cfg.maxBilling);
                    for (it = workers.begin(); it != workers.end(); it++) {
                         Billing* worker = (Billing*)((*it).second);
                        format(dump, "[%u].%u, ", worker->getId(), worker->getState());
                    }
                    smsc_log_debug(logger, dump.c_str());
                }   
            }
        } else
            bill = (Billing*)((*it).second);
    } //grd off
    if (bill)
        bill->handleCommand(pck);
    return;
}

/* ************************************************************************** *
 * class Billing implementation:
 * ************************************************************************** */
Billing::Billing(unsigned b_id, BillingManager * owner, Logger * uselog/* = NULL*/)
        : WorkerAC(b_id, owner, uselog), state(bilIdle), capDlg(NULL)
        , postpaidBill(false), abType(AbonentContractInfo::abtUnknown)
        , providerQueried(false), capDlgActive(false), capSess(NULL)
        , abPolicy(NULL), billErr(0)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.Billing");
    _cfg = owner->getConfig();
    snprintf(_logId, sizeof(_logId)-1, "Billing[%u:%u]", _mgr->cmId(), _wId);
}


Billing::~Billing()
{
    MutexGuard grd(bilMutex);
    doCleanUp();
    if (capDlg) {
        delete capDlg;
        capDlg = NULL;
    }
    smsc_log_debug(logger, "%s: Deleted", _logId);
}

//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::Abort(const char * reason/* = NULL*/)
{
    MutexGuard grd(bilMutex);
    abortThis(reason,  false);
}

//returns true if all billing stages are completed
bool Billing::BillComplete(void) const
{
    return ((state >= Billing::bilReported) && cdr._finalized) ? true : false;
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock bilMutex,
 *       it's a caller responsibility to lock bilMutex !!!
 * ---------------------------------------------------------------------------------- */
bool Billing::matchBillMode(void) const 
{
    CDRRecord::CDRBearerType bearer = cdr._smsXSrvs ? CDRRecord::dpUSSD : cdr._bearer;

    return ((_cfg.billMode == smsc::inman::BILL_ALL)
            || ((_cfg.billMode == smsc::inman::BILL_USSD)
                && (bearer == CDRRecord::dpUSSD))
            || ((_cfg.billMode == smsc::inman::BILL_SMS)
                && (bearer == CDRRecord::dpSMS))
            ) ? true : false;
}

void Billing::doCleanUp(void)
{
    //check for pending query to AbonentProvider
    if (providerQueried) {
        abPolicy->getIAProvider(logger)->cancelQuery(abNumber, this);
        providerQueried = false;
    }
    if (timers.size()) { //release active timers
        for (TimersMAP::iterator it = timers.begin(); it != timers.end(); it++) {
            StopWatch * timer = (*it).second;
            timer->release();
            smsc_log_debug(logger, "%s: Released timer[%u] at state %u",
                        _logId, timer->getId(), state);
        }
        timers.clear();
    }
    return;
}

//NOTE: CDR must be finalized, cdrMode != none, returns number of CDRs created
unsigned Billing::writeCDR(void)
{
    unsigned cnt = 0;
    if (smsxNumber.length) { //write SMSExtra part first (if necessary)
        if (!cdr._inBilled || (_cfg.cdrMode == BillingCFG::CDR_ALL)) {
            CDRRecord xcdr = cdr;
            xcdr._dstAdr = smsxNumber.toString();
            _cfg.bfs->bill(xcdr); cnt++;
            smsc_log_info(logger, "%s: CDR(Extra) written: "
                        "msgId = %llu, IN billed: %s, dstAdr: %s", _logId,
                        xcdr._msgId, xcdr._inBilled ? "true": "false", xcdr._dstAdr.c_str());
        }
        //write bearer part (not billed by IN)
        if ((abType == AbonentContractInfo::abtPostpaid)
            || (_cfg.cdrMode == BillingCFG::CDR_ALL) || !matchBillMode()) {
            CDRRecord xcdr = cdr;
            xcdr._inBilled = false;
            _cfg.bfs->bill(xcdr); cnt++;
            smsc_log_info(logger, "%s: CDR written: "
                        "msgId = %llu, IN billed: false, dstAdr: %s",
                        _logId, xcdr._msgId, xcdr._dstAdr.c_str());
        }
    } else if (!cdr._inBilled || (_cfg.cdrMode == BillingCFG::CDR_ALL)) {
        _cfg.bfs->bill(cdr); cnt++;
        smsc_log_info(logger, "%s: CDR written: msgId = %llu, IN billed: %s, dstAdr: %s",
                    _logId, cdr._msgId, cdr._inBilled ? "true": "false",
                    cdr._dstAdr.c_str());
    }
    return cnt;
}

void Billing::doFinalize(bool doReport/* = true*/)
{
    unsigned cdrs = 0;
    if (_cfg.cdrMode && cdr._finalized && _cfg.bfs)
        cdrs = writeCDR();

    smsc_log_info(logger, "%s: %scomplete, %s --> %s(cause: %u),"
                          " abonent(%s), type %s, CDR(s) written: %u",
                    _logId, BillComplete() ? "" : "IN", cdr.dpType().c_str(),
                    cdr._inBilled ? "SCF": "CDR", billErr, abNumber.getSignals(),
                    AbonentContractInfo::type2Str(abType), cdrs);

    doCleanUp();
    if (doReport) {
        state = bilComplete;
        _mgr->workerDone(this);
    }
    return;
}

void Billing::abortThis(const char * reason/* = NULL*/, bool doReport/* = true*/)
{
    smsc_log_error(logger, "%s: Aborting at state %u%s%s",
                   _logId, state, reason ? ", reason: " : "", reason ? reason : "");
    if (capDlgActive) {
        if ((state >= bilContinued) && (state < bilReported)) { //send sms_o_failure to SCF
            try { capDlg->eventReportSMS(false);
            } catch (std::exception & exc) { }
        }
        capDlg->endDPSMS();
        capDlgActive = false;
    }
    state = Billing::bilAborted;
    doFinalize(doReport);
}

bool Billing::startCAPDialog(INScfCFG * use_scf)
{
    TCAPDispatcher * disp = TCAPDispatcher::getInstance();
    SSNSession * ssnSess = disp->findSession(_cfg.ss7.own_ssn);
    if (!ssnSess) //attempt to open SSN
        ssnSess = disp->openSSN(_cfg.ss7.own_ssn, _cfg.ss7.maxDlgId);

    if (!ssnSess || (ssnSess->getState() != smsc::inman::inap::ssnBound)) {
        smsc_log_error(logger, "%s: SSN session is not available/bound", _logId);
        return false;
    }

    if (!(capSess = ssnSess->newSRsession(_cfg.ss7.ssf_addr,
            ACOID::id_ac_cap3_sms_AC, 146, use_scf->scf.scfAddress))) {
        std::string sid;
        TCSessionAC::mkSignature(sid, _cfg.ss7.own_ssn, _cfg.ss7.ssf_addr,
                                 ACOID::id_ac_cap3_sms_AC, 146, &(use_scf->scf.scfAddress));
        smsc_log_error(logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
        return false;
    }
    smsc_log_debug(logger, "%s: using TCSR[%u]: %s", _logId,
                   capSess->getUID(), capSess->Signature().c_str());

    try { //Initiate CAP3 dialog
        capDlg = new CapSMSDlg(capSess, this, _cfg.ss7.capTimeout, logger); //initialize TCAP dialog
        capDlgActive = true;
        smsc_log_debug(logger, "%s: Initiating CapSMS[%u] -> %s", _logId, capDlg->getId(),
                       smsxNumber.length ? smsxNumber.toString().c_str() : cdr._dstAdr.c_str());

        InitialDPSMSArg arg(smsc::inman::comp::DeliveryMode_Originating, use_scf->scf.serviceKey);

        switch (use_scf->idpLiAddr) {
        case INScfCFG::idpLiSSF:
            arg.setLocationInformationMSC(_cfg.ss7.ssf_addr); break;
        case INScfCFG::idpLiSMSC:
            arg.setLocationInformationMSC(csInfo.smscAddress.c_str()); break;
        default:
            arg.setLocationInformationMSC(cdr._srcMSC.c_str());
        }
        if (smsxNumber.length)
            arg.setDestinationSubscriberNumber(smsxNumber);
        else
            arg.setDestinationSubscriberNumber(cdr._dstAdr.c_str());
        arg.setSMSCAddress(csInfo.smscAddress.c_str());
        arg.setCallingPartyNumber(cdr._srcAdr.c_str());
        arg.setIMSI(cdr._srcIMSI.c_str());
        arg.setTimeAndTimezone(cdr._submitTime);
        arg.setTPShortMessageSpecificInfo(csInfo.tpShortMessageSpecificInfo);
        arg.setTPValidityPeriod(csInfo.tpValidityPeriod, smsc::inman::comp::tp_vp_relative);
        arg.setTPProtocolIdentifier(csInfo.tpProtocolIdentifier);
        arg.setTPDataCodingScheme(csInfo.tpDataCodingScheme);
        capDlg->initialDPSMS(&arg); //begins TCAP dialog
        state = Billing::bilInited;
        return true;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "%s: %s", _logId, exc.what());
        capDlgActive = false;
        delete capDlg;
        capDlg = NULL;
        return false;
    }
}
//NOTE: bilMutex should be locked upon entry!
//NOTE: Billing uses only those timers, which autorelease on signalling
void Billing::StartTimer(unsigned short timeout)
{
    OPAQUE_OBJ  timerArg;
    timerArg.setUInt((unsigned)state);
    StopWatch * timer = _cfg.tmWatcher->createTimer(this, &timerArg, false);
    smsc_log_debug(logger, "%s: Starting timer[%u]:%u",
                _logId, timer->getId(), state);
    timers.insert(TimersMAP::value_type((unsigned)state, timer));
    timer->start((long)timeout, false);
    return;
}

//NOTE: bilMutex should be locked upon entry!
void Billing::StopTimer(Billing::BillingState bilState)
{
    TimersMAP::iterator it = timers.find((unsigned)bilState);
    if (it != timers.end()) {
        StopWatch * timer = (*it).second;
        timer->release();
        timers.erase(it);
        smsc_log_debug(logger, "%s: Released timer[%u]:%u at state %u",
                    _logId, timer->getId(), bilState, state);
    } else
        smsc_log_warn(logger, "%s: no active timer for state: %u",
                    _logId, bilState);
    return;
}

//NOTE: bilMutex should be locked upon entry!
bool Billing::onChargeSms(ChargeSms* sms, CsBillingHdr_dlg *hdr)
{
    sms->export2CDR(cdr);
    sms->exportCAPInfo(csInfo);

    if (!abNumber.fromText(cdr._srcAdr.c_str())) {
        smsc_log_error(logger, "%s: invalid Call.Adr <%s>", _logId, cdr._srcAdr.c_str());
        return false;
    }
    smsc_log_info(logger, "%s: %s: Call.Adr <%s>, Dest.Adr <%s>", _logId,
                    cdr.dpType().c_str(), cdr._srcAdr.c_str(), cdr._dstAdr.c_str());

    uint32_t smsXSrvs = cdr._smsXSrvs & ~SMSX_RESERVED_MASK;
    if (smsXSrvs) {
        if (cdr._bearer != CDRRecord::dpSMS)
            smsc_log_error(logger, "%s: invalid bearer for SMS Extra service", _logId);

        SmsXServiceMap::iterator it = _cfg.smsXMap.find(smsXSrvs);
        if (it != _cfg.smsXMap.end()) {
            smsxNumber = (*it).second;
            smsc_log_info(logger, "%s: SMSExtra 0x%x configured to %s", _logId,
                          smsXSrvs, smsxNumber.toString().c_str());
        } else {
            smsc_log_error(logger, "%s: SMSExtra 0x%x misconfigured, ignoring!",
                           _logId, smsXSrvs);
            cdr._smsXSrvs &= SMSX_RESERVED_MASK;
        }
    }

    //check for source IMSI & MSC
    if (!strcmp("MAP_PROXY", cdr._srcSMEid.c_str())) {
        if (!cdr._srcIMSI.length() || !cdr._srcMSC.length()) {
            std::string rec;
            CDRRecord::csvEncode(cdr, rec);
            smsc_log_error(logger, "%s: empty source IMSI or MSC!", _logId);
            smsc_log_error(logger, "%s: header: <%s>", 
                           _logId, smsc::inman::cdr::_CDRRecordHeader_TEXT);
            smsc_log_error(logger, "%s: cdr   : <%s>", _logId, rec.c_str());
        }
    }

    postpaidBill = !matchBillMode();
    AbonentRecord   abRec; //ab_type = abtUnknown
    if (postpaidBill
        || (((abType = _cfg.abCache->getAbonentInfo(abNumber, &abRec))
             == AbonentContractInfo::abtPostpaid) && !smsXSrvs)) {
        postpaidBill = true; //do not interact IN platform, just create CDR
        chargeResult(ChargeSmsResult::CHARGING_POSSIBLE);
        return true;
    }

    if (!(abPolicy = _cfg.policies->getPolicy(&abNumber))) {
        smsc_log_error(logger, "%s: no policy set for %s", _logId, 
                       abNumber.toString().c_str());
    } else {
        smsc_log_debug(logger, "%s: using policy: %s", _logId, abPolicy->Ident());
        if ((abType == AbonentContractInfo::abtUnknown)
            || ((abType == AbonentContractInfo::abtPrepaid) && !abRec.getSCFinfo())) {
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
    return ConfigureSCFandCharge(abRec.ab_type, abRec.getSCFinfo());
}

//here goes: abtPrepaid or abtUnknown or abtPostpaid(SMSX)
bool Billing::ConfigureSCFandCharge(AbonentBillType ab_type, const MAPSCFinfo * p_scf/* = NULL*/)
{
    abType = ab_type;

    if (p_scf) { //SCF is set for abonent by cache/IAProvider
        abScf.scf = *p_scf;
        //lookup policy for extra SCF parms (serviceKey, RPC lists)
        if (abPolicy)
            abPolicy->getSCFparms(&abScf);
    } else if (abPolicy) {  //attempt to determine SCF params from config.xml
        if (abType == AbonentContractInfo::abtUnknown) {
            INScfCFG * pin = NULL;
            if (abPolicy->scfMap.size() == 1) { //single IN serving, look for postpaidRPC
                pin = (*(abPolicy->scfMap.begin())).second;
                if (pin->postpaidRPC.size())
                    abScf = *pin;
                else
                    pin = NULL;
            }
            if (!pin) {
                smsc_log_error(logger, "%s: unable to determine"
                                " abonent type/SCF, switching to CDR mode", _logId);
                postpaidBill = true;
            }
        } else if (abType == AbonentContractInfo::abtPrepaid) {
            if (abPolicy->scfMap.size() == 1) { //single IN serving
                abScf = *((*(abPolicy->scfMap.begin())).second);
            } else {
                smsc_log_error(logger, "%s: unable to determine"
                                " abonent SCF, switching to CDR mode", _logId);
                postpaidBill = true;
            }
        } else //btPostpaid
            postpaidBill = true;
    } else
        postpaidBill = true;

    if (!postpaidBill) {
        smsc_log_debug(logger, "%s: using SCF %s:{%u}", _logId, 
                       abScf._ident.size() ? abScf.ident() : abScf.scf.scfAddress.getSignals(),
                       abScf.scf.serviceKey);
        postpaidBill = !startCAPDialog(&abScf);
    }

    if (postpaidBill) //do not interact IN platform, just create CDR
        chargeResult(ChargeSmsResult::CHARGING_POSSIBLE);
    return true;
}

//NOTE: bilMutex should be locked upon entry
void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes, CsBillingHdr_dlg *hdr)
{
    bool submitted = smsRes->GetValue() ? false : true;
    smsc_log_info(logger, "%s: --> DELIVERY_%s (code: %u)",
                   _logId, (submitted) ? "SUCCEEDED" : "FAILED",
                   smsRes->GetValue());

    smsRes->export2CDR(cdr);
    if (capDlgActive) { //continue TCAP dialog if it's still active
        try {
            capDlg->eventReportSMS(submitted);
            cdr._inBilled = true;
            state = bilReported;
        } catch (std::exception & exc) {
            postpaidBill = true;
            smsc_log_error(logger, "%s: %s", exc.what());
            capDlgActive = false;
        }
    }
    if (!capDlgActive) {
        state = bilReported;
        doFinalize();
    } //else wait onEndSMS();
    return;
}

//NOTE: bilMutex should be locked upon entry
void Billing::chargeResult(ChargeSmsResult::ChargeSmsResult_t chg_res, uint32_t inmanErr /* = 0*/)
{
    if (inmanErr)
        billErr = inmanErr;
    std::string reply;

    if (chg_res != ChargeSmsResult::CHARGING_POSSIBLE) {
        format(reply, "NOT_POSSIBLE (cause %u)", inmanErr);
        state = Billing::bilReleased;
    } else {
        format(reply, "POSSIBLE (via %s, cause %u)", postpaidBill ? "CDR" : "SCF", inmanErr);
        state = Billing::bilContinued;
    }
    smsc_log_info(logger, "%s: <-- %s CHARGING_%s, abonent(%s) type: %s (%u)", _logId,
                cdr.dpType().c_str(), reply.c_str(), abNumber.getSignals(),
                AbonentContractInfo::type2Str(abType), (unsigned)abType);

    SPckChargeSmsResult res;
    res.Cmd().SetValue(chg_res);
    res.Cmd().setError(inmanErr);
    res.Hdr().dlgId = _wId;
    if (_mgr->sendCmd(&res))
        StartTimer(_cfg.maxTimeout);
    else     //TCP connect fatal failure
        abortThis(_mgr->connectError()->what());
    return;
}

/* -------------------------------------------------------------------------- *
 * InmanHandler interface implementation:
 * -------------------------------------------------------------------------- */
static const char * const _nm_cmd[] = {
    "", "CHARGE_SMS", "CHARGE_SMS_RESULT", "DELIVERY_SMS_RESULT"
};
 //NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::handleCommand(INPPacketAC* pck)
{
    MutexGuard  grd(bilMutex);
    unsigned short cmdId = (pck->pCmd())->Id();

    smsc_log_debug(logger, "%s: --> %s at state %u", _logId, _nm_cmd[cmdId], state);
    switch (state) {
    case Billing::bilIdle: {
        if (cmdId == INPCSBilling::CHARGE_SMS_TAG) {
            state = Billing::bilStarted;
            bool goon = true;
            //complete the command deserialization
            try { (pck->pCmd())->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_error(logger, "%s: %s", _logId, exc.what());
                goon = false;
            }
            if (!goon || !onChargeSms(static_cast<ChargeSms*>(pck->pCmd()), NULL)) {
                SPckChargeSmsResult spck;
                spck.Cmd().SetValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
                spck.Cmd().setError(errInman, InProtocol_InvalidData);
                spck.Hdr().dlgId = _wId;
                _mgr->sendCmd(&spck);
                doFinalize();
            }
        } else {
            smsc_log_error(logger, "%s: protocol error: cmd %u, state %u",
                           _logId, (unsigned)cmdId, state);
            doFinalize();
        }
    } break;

    case Billing::bilContinued: {
        if (cmdId == INPCSBilling::DELIVERY_SMS_RESULT_TAG) {
            StopTimer(state);
            state = Billing::bilApproved;
            //complete the command deserialization
            try { (pck->pCmd())->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_error(logger, "%s: %s", _logId, exc.what());
                abortThis(exc.what());
                return;
            }
            onDeliverySmsResult(static_cast<DeliverySmsResult*>(pck->pCmd()), NULL);
            break;
        }
    } //no break, fall into default !!!
    default: //ignore unknown/illegal command
        smsc_log_error(logger, "%s: protocol error: cmd %u, state %u",
                       _logId, (unsigned)cmdId, state);
    } /* eosw */
    return; //grd off
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj)
{
    assert(opaque_obj);
    MutexGuard grd(bilMutex);
    
    smsc_log_debug(logger, "%s: timer[%u] signaled, states: %u -> %u",
        _logId, timer->getId(), opaque_obj->val.ui, (unsigned)state);

    TimersMAP::iterator it = timers.find(opaque_obj->val.ui);
    if (it == timers.end())
        smsc_log_warn(logger, "%s: timer[%u] is not registered!", _logId);
    else
        timers.erase(it);

    if (opaque_obj->val.ui == (unsigned)state) {
        //target operation doesn't complete yet.
        if (state == Billing::bilStarted) {
            //abonent provider query is expired
            abPolicy->getIAProvider(logger)->cancelQuery(abNumber, this);
            providerQueried = false;
            ConfigureSCFandCharge(AbonentContractInfo::abtUnknown, NULL);
            return;
        }
        if (state == Billing::bilContinued) {
            //SMSC doesn't respond with DeliveryResult
            abortThis("SMSC response is timed out (DeliverySmsResult)");
            return;
        }
    } //else: operation already finished
    return; //grd off
}

/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::onIAPQueried(const AbonentId & ab_number, const AbonentRecord & ab_rec)
//                           AbonentBillType ab_type, const MAPSCFinfo * scf/* = NULL*/)
{
    MutexGuard grd(bilMutex);

    providerQueried = false;
    if (state > bilStarted) {
        smsc_log_warn(logger, "%s: abonentQueried at state: %u", _logId, state);
        return;
    }
    StopTimer(state);
    state = bilQueried;
    ConfigureSCFandCharge(ab_rec.ab_type, ab_rec.getSCFinfo());
    return;
}

/* -------------------------------------------------------------------------- *
 * CapSMS_SSFhandlerITF interface implementation:
 * NOTE: all callbacks are the processing graph entry points, so lock bilMutex !!!
 * -------------------------------------------------------------------------- */
void Billing::onDPSMSResult(unsigned char rp_cause/* = 0*/)
{
    MutexGuard grd(bilMutex);
    uint32_t            scfErr = 0;
    ChargeSmsResult::ChargeSmsResult_t  chgRes = ChargeSmsResult::CHARGING_POSSIBLE;

    if (!rp_cause) {    //ContinueSMS
        if (abType != AbonentContractInfo::abtPrepaid)  //Update abonents cache
            _cfg.abCache->setAbonentInfo(abNumber, AbonentRecord(abType = 
                                AbonentContractInfo::abtPrepaid, 0, &abScf.scf));
    } else {            //ReleaseSMS
        capDlgActive = false;
        //check for RejectSMS causes for postpaid abonents:
        for (RPCList::iterator it = abScf.postpaidRPC.begin(); 
                                it != abScf.postpaidRPC.end(); it++) {
            if ((*it) == rp_cause) {
                postpaidBill = true;
                //Update abonents cache
                if (abType != AbonentContractInfo::abtPostpaid)
                    _cfg.abCache->setAbonentInfo(abNumber, AbonentRecord(abType = 
                                                 AbonentContractInfo::abtPostpaid));
                break;
            }
        }
        if (!postpaidBill) { //check for RejectSMS causes indicating that abonent
                             //can't be charged (not just the technical failure)
            postpaidBill = true;
            for (RPCList::iterator it = abScf.rejectRPC.begin();
                                    it != abScf.rejectRPC.end(); it++) {
                if ((*it) == rp_cause) {
                    postpaidBill = false;
                    break;
                }
            }
        }
        scfErr = InmanErrorCode::combineError(errRPCause, (uint16_t)rp_cause);
        if (!postpaidBill)
            chgRes = ChargeSmsResult::CHARGING_NOT_POSSIBLE;
    }
    chargeResult(chgRes, scfErr);
    return;
}

void Billing::onEndCapDlg(unsigned char ercode/* = 0*/,
                          InmanErrorType errLayer/* = smsc::inman::errOk*/)
{
    MutexGuard grd(bilMutex);
    capDlgActive = false;
    if (errLayer == smsc::inman::errOk) {   //EndSMS
        if ((state == bilReleased) || (state == bilReported))
            doFinalize(true);
        else if (state != bilContinued)
            smsc_log_error(logger, "%s: onEndCapDlg() at state: %u",
                           _logId, (unsigned)state);
    } else {                                //AbortSMS
        billErr = InmanErrorCode::combineError(errLayer, (uint16_t)ercode); 
        smsc_log_error(logger, "%s: CapSMSDlg Error, code: %u, layer %s",
                       _logId, (unsigned)ercode, _InmanErrorSource[errLayer]);
        bool  contCharge = false;
        switch (state) {
        case Billing::bilComplete:
        case Billing::bilAborted:
        case Billing::bilReleased: {
            //dialog with MSC already cancelled/finished, just release CAP dialog
        } break;

        case Billing::bilInited: {
            //IN dialog initialization failed, release CAP dialog, switch to CDR mode 
            contCharge = true;
        } // no break specially !
        case Billing::bilContinued:
            //dialog with MSC is in process, release CAP dialog, switch to CDR mode
        case Billing::bilApproved:
        case Billing::bilReported:
            //dialog with MSC finished, release CAP dialog, switch to CDR mode
            cdr._inBilled = false;
        default:
            postpaidBill = true;
            smsc_log_warn(logger, "%s: switched to billing via CDR"
                            " (reason: CapSMSDlg error).", _logId);
        }
        if (contCharge)
            chargeResult(ChargeSmsResult::CHARGING_POSSIBLE, billErr);
    }
    return;
}

} //inman
} //smsc

