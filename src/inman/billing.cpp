#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
#include <assert.h>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/billing.hpp"
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPCSBilling;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::DeliveredSmsData;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::CsBillingHdr_dlg;

using smsc::util::URCRegistry;
using smsc::inman::CAPSmTaskMT;

#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
using smsc::inman::comp::_RCS_MOSM_RPCause;

#include "inman/INManErrors.hpp"
using smsc::inman::iaprvd::_RCS_IAPQStatus;
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;

using smsc::core::synchronization::MutexTryGuard;


namespace smsc  {
namespace inman {

const char * const _BILLmodes[] = {"OFF", "CDR", "IN"};
const char * const _MSGtypes[] = {"unknown", "SMS", "USSD", "XSMS"};
const char * const _CDRmodes[] = {"none", "billMode", "all"};

//According to CDRRecord::ChargingPolicy
static const char *_chgPolicy[] = { "ON_SUBMIT", "ON_DELIVERY", "ON_DATA_COLLECTED" };

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
                spck.Cmd().setValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
                spck.Cmd().setError(
                    _RCS_INManErrors->mkhash(INManErrorId::cfgLimitation),
                    _RCS_INManErrors->explainCode(INManErrorId::cfgLimitation).c_str());
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
                        format(dump, "%u:%u, ", worker->getId(), worker->getState());
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
Billing::~Billing()
{
    MutexGuard grd(bilMutex);
    doCleanUp();
    smsc_log_debug(logger, "%s: Deleted", _logId);
}

//returns true if required (depending on chargeMode) CDR data fullfilled
bool Billing::CDRComplete(void) const
{
    if (cdr._chargePolicy == CDRRecord::ON_SUBMIT)
        return (cdr._finalized >= CDRRecord::dpSubmitted) ? true : false;
    return (cdr._finalized == CDRRecord::dpCollected)  ? true : false;
}

//returns true if all billing stages are completed
bool Billing::BillComplete(void) const
{
    return ((state >= Billing::bilReported) && CDRComplete()) ? true : false;
}

/* -------------------------------------------------------------------------- *
 * WorkerAC interface implementation:
 * -------------------------------------------------------------------------- */
static const char * const _nm_cmd[] = {
    "", "CHARGE_SMS", "CHARGE_SMS_RESULT", "DELIVERY_SMS_RESULT", "DELIVERED_SMS_DATA"
};
//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::handleCommand(INPPacketAC* pck)
{
    MutexGuard      grd(bilMutex);
    unsigned short  cmdId = (pck->pCmd())->Id();
    PGraphState     pgState = Billing::pgCont;

    smsc_log_debug(logger, "%s: --> %s at state %u", _logId, _nm_cmd[cmdId], state);
    switch (state) {
    case Billing::bilIdle: {
        switch (cmdId) {
        case INPCSBilling::CHARGE_SMS_TAG: {
            state = Billing::bilStarted;
            bool badPdu = false;
            ChargeSms* chgReq = static_cast<ChargeSms*>(pck->pCmd());
            //complete the command deserialization
            try {
                chgReq->loadDataBuf();
                chgReq->export2CDR(cdr);
                chgReq->exportCAPInfo(csInfo);
            } catch (SerializerException & exc) {
                smsc_log_error(logger, "%s: %s", _logId, exc.what());
                badPdu = true;
            }
            if (badPdu || (badPdu = !verifyChargeSms())) {
                SPckChargeSmsResult spck;
                spck.Cmd().setValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
                spck.Cmd().setError(
                    _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData),
                    _RCS_INManErrors->explainCode(INManErrorId::protocolInvalidData).c_str());
                spck.Hdr().dlgId = _wId;
                _mgr->sendCmd(&spck);
                pgState = Billing::pgEnd;
            } else { //process charging request
                pgState = onChargeSms();
            }
        } break;

        case INPCSBilling::DELIVERED_SMS_DATA_TAG: { 
            state = Billing::bilStarted;
            bool badPdu = false;
            DeliveredSmsData* chgReq = static_cast<DeliveredSmsData*>(pck->pCmd());
            //complete the command deserialization
            try {
                chgReq->loadDataBuf();
                chgReq->export2CDR(cdr);
            } catch (SerializerException & exc) {
                smsc_log_error(logger, "%s: %s", _logId, exc.what());
                badPdu = true;
            }
            if (badPdu || (badPdu = !verifyChargeSms())) {
                billErr = _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
                pgState = Billing::pgEnd;
            } else { //process charging request
                pgState = onChargeSms();
            }
        } break;

        default: {
            smsc_log_error(logger, "%s: protocol error: cmd %u, state %u",
                           _logId, (unsigned)cmdId, state);
            pgState = Billing::pgEnd;
        }
        } /* eosw */
    } break;

    case Billing::bilContinued: {
        if (cmdId == INPCSBilling::DELIVERY_SMS_RESULT_TAG) {
            StopTimer(state);
            bool badPdu = false;
            DeliverySmsResult* smsRes = static_cast<DeliverySmsResult*>(pck->pCmd());
            //complete the command deserialization
            try { 
                smsRes->loadDataBuf(); 
                smsRes->export2CDR(cdr);
            } catch (SerializerException & exc) {
                smsc_log_error(logger, "%s: %s", _logId, exc.what());
                badPdu = true;
                state = Billing::bilAborted;
            }
            pgState = badPdu ? Billing::pgEnd : onDeliverySmsResult();
            break;
        }
    } //no break, fall into default !!!
    default: //ignore unknown/illegal command
        smsc_log_error(logger, "%s: protocol error: cmd %u, state %u",
                       _logId, (unsigned)cmdId, state);
    } /* eosw */
    if (pgState == Billing::pgEnd)
        doFinalize();
    return; //grd off
}

//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::Abort(const char * reason/* = NULL*/)
{
    MutexGuard grd(bilMutex);
    abortThis(reason,  false);
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock bilMutex,
 *       it's a caller responsibility to lock bilMutex !!!
 * ---------------------------------------------------------------------------------- */
void Billing::doCleanUp(void)
{
    //check for pending query to AbonentProvider
    if (providerQueried) {
        abPolicy->getIAProvider()->cancelQuery(abNumber, this);
        providerQueried = false;
    }
    if (!timers.empty()) { //release active timers
        for (TimersMAP::iterator it = timers.begin(); it != timers.end(); ++it) {
            TimerHdl & timer = it->second;
            timer.Stop();
            smsc_log_debug(logger, "%s: Released timer[%s] at state %u",
                        _logId, timer.IdStr(), state);
        }
        timers.clear();
    }
    return;
}

//NOTE: CDR must be finalized, cdrMode != none, returns number of CDRs created
unsigned Billing::writeCDR(void)
{
    unsigned cnt = 0;
    cdr._contract = static_cast<CDRRecord::ContractType>((unsigned)abCsi.abRec.ab_type);
    //set the location MSC of charged abonent to last known one
    if (cdr._chargeType) {  //MT: from delivery report
        if (cdr._dstMSC.empty() && !abCsi.vlrNum.empty())
            cdr._dstMSC = abCsi.vlrNum.toString();
    } else {                //MO: from chargeReq or abonentProvider
        if (!abCsi.vlrNum.empty())
            cdr._srcMSC = abCsi.vlrNum.toString();
    }
    if (!cdr._inBilled || (_cfg.cdrMode == BillingCFG::cdrALL)) {
        //remove TonNpi for MSCs ids
        TonNpiAddress tna;
        if (tna.fromText(cdr._srcMSC.c_str()))
            cdr._srcMSC = tna.getSignals();
        else if (cdr._chargeType == CDRRecord::MO_Charge)
            smsc_log_error(logger, "%s: empty MSC for %s", _logId, abNumber.toString().c_str());

        if (tna.fromText(cdr._dstMSC.c_str()))
            cdr._dstMSC = tna.getSignals();
        else if (cdr._chargeType == CDRRecord::MT_Charge)
            smsc_log_error(logger, "%s: empty MSC for %s", _logId, abNumber.toString().c_str());

        _cfg.bfs->bill(cdr); cnt++;
        smsc_log_info(logger, "%s: CDR written: msgId: %llu, status: %u, IN billed: %s, charged: %s",
                    _logId, cdr._msgId, cdr._dlvrRes, cdr._inBilled ? "true": "false",
                    abNumber.toString().c_str());
    }
    return cnt;
}

void Billing::doFinalize(bool doReport/* = true*/)
{
    unsigned cdrs = 0;
    if (_cfg.cdrMode && BillComplete() && _cfg.bfs)
        cdrs = writeCDR();

    smsc_log_info(logger, "%s: %scomplete(%s, %s), %s --> %s(cause %u),"
                          " abonent(%s), type %s, CDR(s) written: %u", _logId,
            BillComplete() ? "" : "IN", cdr._chargeType ? "MT" : "MO",
            _chgPolicy[cdr._chargePolicy], cdr.dpType().c_str(),
            cdr._inBilled ? abScf->Ident() : _cfg.billModeStr(billMode), billErr,
            abNumber.getSignals(), abCsi.abRec.type2Str(), cdrs);

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
    if (capTask) {
        capTask->Signal(TaskSchedulerITF::sigAbort);
        capTask->UnrefBy(this);
        capTask = NULL;
    }
    state = Billing::bilAborted;
    doFinalize(doReport);
}


RCHash Billing::startCAPSmTask(void)
{
    if (!_cfg.ss7.userId) {
        smsc_log_error(logger, "%s: SS7 stack is not connected!", _logId);
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit);
    }
    TaskSchedulerITF * capSched = _cfg.schedMgr->getScheduler(
                            (abScf->idpReqMode == INScfCFG::idpReqMT) ? 
                            TaskSchedulerITF::schedMT : TaskSchedulerITF::schedSEQ);
    if (!capSched) {
        smsc_log_error(logger, "%s: TaskScheduler is not srarted", _logId);
        return _RCS_INManErrors->mkhash(INManErrorId::internalError);
    }

    uint32_t skeyMOSM = abCsi.abRec.tdpSCF[TDPCategory::dpMO_SM].serviceKey;
    std::auto_ptr<CAPSmTaskAC> smTask;
    if (abScf->idpReqMode == INScfCFG::idpReqMT) {
        smTask.reset(new CAPSmTaskMT(abNumber, _cfg.ss7, abScf,  skeyMOSM, CAPSmTaskAC::idpMO, logger));
    } else
        smTask.reset(new CAPSmTaskSQ(abNumber, _cfg.ss7, abScf,  skeyMOSM, CAPSmTaskAC::idpMO, logger));

    //determine number of required CAP dialogs and corresponding destination addresses
    if (!xsmsSrv || xsmsSrv->chargeBearer) {
        TonNpiAddress dstAdr;
        if (cdr._chargeType) {  //MT
            //charge target abonent as if it attempts to send a SMS
            if (!dstAdr.fromText(cdr._srcAdr.c_str())) {
                smsc_log_error(logger, "%s: invalid Call.Adr '%s'", _logId, cdr._srcAdr.c_str());
                return _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
            }
        } else {                //MO
            if (!dstAdr.fromText(cdr._dstAdr.c_str())) {
                smsc_log_error(logger, "%s: invalid Dest.Adr '%s'", _logId, cdr._dstAdr.c_str());
                return _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
            }
        }
        smTask->enqueueDA(dstAdr);
    }
    if (xsmsSrv)
        smTask->enqueueDA(xsmsSrv->adr);
    
    RCHash rval = 0;
    try { //compose InitialDPSMS argument
        switch (abScf->idpLiAddr) {
        case INScfCFG::idpLiSSF:
            smTask->Arg().setLocationInformationMSC(_cfg.ss7.ssf_addr); break;
        case INScfCFG::idpLiSMSC:
            smTask->Arg().setLocationInformationMSC(csInfo.smscAddress.c_str()); break;
        default:
            smTask->Arg().setLocationInformationMSC(abCsi.vlrNum);
        }
        smTask->Arg().setIMSI(abCsi.abRec.getImsi());
        smTask->Arg().setSMSCAddress(csInfo.smscAddress.c_str());
        smTask->Arg().setTimeAndTimezone(cdr._submitTime);
        smTask->Arg().setTPShortMessageSpecificInfo(csInfo.tpShortMessageSpecificInfo);
        smTask->Arg().setTPValidityPeriod(csInfo.tpValidityPeriod, smsc::inman::comp::tp_vp_relative);
        smTask->Arg().setTPProtocolIdentifier(csInfo.tpProtocolIdentifier);
        smTask->Arg().setTPDataCodingScheme(csInfo.tpDataCodingScheme);

    } catch (const CustomException & c_exc) {
        smsc_log_error(logger, "%s: %s", _logId, c_exc.what());
        rval = (RCHash)(c_exc.errorCode());
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "%s: %s", _logId, exc.what());
        rval = _RCS_TC_Dialog->mkhash(INManErrorId::protocolInvalidData);
    }
    if (!rval) {
        if (!capSched->StartTask(smTask.get(), this)) {
            smsc_log_error(logger, "%s: Failed to start %s", _logId, smTask->TaskName());
            rval = _RCS_TC_Dialog->mkhash(INManErrorId::logicTimedOut);
        } else {
            state = Billing::bilInited;
            smsc_log_debug(logger, "%s: Initiated %s", _logId, smTask->TaskName());
            capTask = smTask.release();
            StartTimer(_cfg.maxTimeout);
            //execution will continue in onCapSMSResult() by another thread.
            //rval == 0 means Billing::pgCont;
        }
    }
    return rval;
}

//NOTE: bilMutex should be locked upon entry!
bool Billing::StartTimer(TimeoutHDL & tmo_hdl)
{
    OPAQUE_OBJ  timerArg((unsigned)state);
    TimerHdl timer = tmo_hdl.CreateTimer(this, &timerArg);
    if (timer.Start() != TimeWatcherITF::errOk) {
        smsc_log_error(logger, "%s: failed to start timer[%s]:%u",
                    _logId, timer.IdStr(), state);
        return false;
    }
    timers.insert(TimersMAP::value_type((unsigned)state, timer));
    smsc_log_debug(logger, "%s: started timer[%s]:%u",
                _logId, timer.IdStr(), state);
    return true;
}

//NOTE: bilMutex should be locked upon entry!
void Billing::StopTimer(Billing::BillingState bilState)
{
    TimersMAP::iterator it = timers.find((unsigned)bilState);
    if (it != timers.end()) {
        smsc_log_debug(logger, "%s: releasing timer[%s]:%u at state %u",
            _logId, it->second.IdStr(), (unsigned)bilState, (unsigned)state);
        it->second.Stop();
        timers.erase(it);
    } else
        smsc_log_warn(logger, "%s: no active timer for state: %u",
                    _logId, (unsigned)bilState);
    return;
}

//returns false if PDU contains invalid data preventing request processing
//NOTE: bilMutex should be locked upon entry!
bool Billing::verifyChargeSms(void)
{
    //determine which abonent should be charged
    if (!abNumber.fromText(cdr._chargeType ? cdr._dstAdr.c_str() : cdr._srcAdr.c_str())) {
        smsc_log_error(logger, "%s: invalid %s.Adr '%s'", _logId,
                        cdr._chargeType ? "Dest" : "Call",
                        cdr._chargeType ? cdr._dstAdr.c_str() : cdr._srcAdr.c_str());
        return false;
    }
    //check if abonent IMSI is already present
    abCsi.abRec.setImsi(cdr._chargeType ? cdr._dstIMSI.c_str() : cdr._srcIMSI.c_str());
    //check if abonent location MSC number is already present
    abCsi.vlrNum.fromText(cdr._chargeType ? cdr._dstMSC.c_str() : cdr._srcMSC.c_str());

    smsc_log_info(logger, "%s: %s(%s, %s): '%s' -> '%s'", _logId,
                cdr.dpType().c_str(), cdr._chargeType ? "MT" : "MO",
                _chgPolicy[cdr._chargePolicy], cdr._srcAdr.c_str(), cdr._dstAdr.c_str());

    uint32_t smsXMask = cdr._smsXMask & ~SMSX_RESERVED_MASK;
    if (smsXMask) {
        if (cdr._bearer != CDRRecord::dpSMS) {
            smsc_log_error(logger, "%s: invalid bearer for SMS Extra service", _logId);
            return false;
        }
        SmsXServiceMap::iterator it = _cfg.smsXMap.find(smsXMask);
        if (it != _cfg.smsXMap.end()) {
            xsmsSrv = &(*it).second;
            cdr._serviceId = xsmsSrv->cdrCode;
            smsc_log_info(logger, "%s: %s[0x%x]: %u, %s%s", _logId,
                    xsmsSrv->name.empty() ? "SMSExtra service" : xsmsSrv->name.c_str(),
                    smsXMask, xsmsSrv->cdrCode, xsmsSrv->adr.toString().c_str(),
                    xsmsSrv->chargeBearer ? ", chargeBearer" : "");
        } else {
            smsc_log_error(logger, "%s: SMSExtra service[0x%x] misconfigured, ignoring!",
                           _logId, smsXMask);
            cdr._smsXMask &= SMSX_RESERVED_MASK;
        }
    }

    if (xsmsSrv)
        msgType = ChargeObj::msgXSMS;
    else
        msgType = (cdr._bearer == CDRRecord::dpUSSD) ? ChargeObj::msgUSSD : ChargeObj::msgSMS;

    //determine billmode
    billPrio = cdr._chargeType ? _cfg.mt_billMode.modeFor(msgType) : 
                                    _cfg.mo_billMode.modeFor(msgType);
    billMode = billPrio->first;
    return true;
}

//NOTE: bilMutex should be locked upon entry!
Billing::PGraphState Billing::onChargeSms(void)
{
    if ((billMode == ChargeObj::bill2IN) && !_cfg.ss7.userId)
        billMode = billPrio->second;

    if (billMode == ChargeObj::billOFF) {
        return chargeResult(false, _RCS_INManErrors->mkhash(INManErrorId::cfgMismatch));
    }
    //Here goes either bill2IN or bill2CDR ..
    if (cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
        billMode = ChargeObj::bill2CDR;
    else if (cdr._smsXMask & SMSX_NOCHARGE_SRV)
        billMode = ChargeObj::bill2CDR;
    else if (cdr._smsXMask & SMSX_INCHARGE_SRV)
        billMode = ChargeObj::bill2IN;

    //check for SMS extra sevice number being set
    if ((xsmsSrv && xsmsSrv->adr.empty())) {
        billMode = ChargeObj::bill2CDR;
        billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgSpecific);
    }

    AbonentRecord cacheRec;
    _cfg.abCache->getAbonentInfo(abNumber, &cacheRec);
    abCsi.abRec.Merge(cacheRec);     //merge available abonent info
    //check for IMSI being defined
    if (!abCsi.abRec.getImsi())
        abCsi.abRec.ab_type = AbonentContractInfo::abtUnknown;

    if (abCsi.abRec.ab_type == AbonentContractInfo::abtPostpaid) {
        billMode = ChargeObj::bill2CDR;
        //do not interact IN platform, just create CDR
        return chargeResult(true);
    }

    //Here goes either abtPrepaid or abtUnknown ..
    if (!(abPolicy = _cfg.policies->getPolicy(&abNumber)))
        smsc_log_error(logger, "%s: no policy set for %s", _logId, 
                        abNumber.toString().c_str());

    //check if AbonentProvider should be requested for contract type
    bool askProvider = ((abCsi.abRec.ab_type == AbonentContractInfo::abtUnknown)
                        && ((billMode == ChargeObj::bill2IN)
                            || (_cfg.cntrReq == BillingCFG::reqAlways)
                            || (cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED))
                        );

    //check for MO_SM SCF params defined
    if ((abCsi.abRec.ab_type == AbonentContractInfo::abtPrepaid)
         && !abCsi.abRec.getSCFinfo(TDPCategory::dpMO_SM))
        askProvider = true;
    //check if AbonentProvider should be requested for current abonent location
    if (abCsi.vlrNum.empty() && abPolicy
        && (abPolicy->getIAPAbilities() & IAProvider::abSCF))
        askProvider = true;

    //verify that abonent number is in ISDN international format
    if (!abNumber.interISDN())
        askProvider = false;

    if (askProvider && abPolicy) {
        smsc_log_debug(logger, "%s: using policy %s for %s", _logId, abPolicy->Ident(),
                        abNumber.toString().c_str());
        // configure SCF by quering provider first
        IAProviderITF *prvd = abPolicy->getIAProvider();
        if (prvd) {
            if (StartTimer(_cfg.abtTimeout)
                && (providerQueried = prvd->startQuery(abNumber, this))) {
                //execution will continue in onIAPQueried() by another thread.
                return Billing::pgCont;
            }
            smsc_log_error(logger, "%s: startIAPQuery(%s) failed!", _logId,
                               abNumber.getSignals());
        } else {
            billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
            smsc_log_warn(logger, "%s: no IAProvider configured", _logId);
        }
    }
    return ConfigureSCFandCharge();
}

//Here goes either bill2IN or bill2CDR ..
Billing::PGraphState Billing::ConfigureSCFandCharge(void)
{
    if (abCsi.abRec.ab_type == AbonentContractInfo::abtPostpaid) {
        billMode = ChargeObj::bill2CDR;
        return chargeResult(true);
    }
    //Here goes either abtPrepaid or abtUnknown ..
    RCHash err = 0;
    if (billMode == ChargeObj::bill2IN) {
        const char * errmsg = NULL;
        const GsmSCFinfo * p_scf = abCsi.abRec.getSCFinfo(TDPCategory::dpMO_SM);
        if (!p_scf) //check if SCF for MO-BC may be used
            p_scf = abCsi.abRec.getSCFinfo(TDPCategory::dpMO_BC);
        
        if (!abCsi.abRec.getImsi())
            errmsg = "unable to determine abonent IMSI";
        else if (p_scf) { //SCF is set only for prepaid abonent by cache/IAProvider
            if (abPolicy) //lookup policy for extra SCF parms (serviceKey, RPC lists)
                abScf = abPolicy->getSCFparms(&(p_scf->scfAddress));
        } else {    //attempt to determine SCF params from config.xml
            if (!abPolicy) {
                errmsg = "unable to determine IN params (no policy set)";
            } else {
                //look for single IN serving
                if (abPolicy->scfMap.size() == 1)
                    abScf = abPolicy->scfMap.begin()->second;
                else
                    errmsg = "unable to determine IN params (too many INs)";
            }
        }
        //check for MO_SM serviceKey being defined
        if (!errmsg) {
            if (!abScf)
                errmsg = "unable to determine IN params";
            else {
            // renew MO-SM SCF params in abonent record tdpSCF map and in abonent cache
                p_scf = abCsi.abRec.getSCFinfo(TDPCategory::dpMO_SM);
                uint32_t recMOSM = p_scf ? p_scf->serviceKey : 0;
                uint32_t cfgMOSM = abScf->getSKey(&abCsi.abRec.tdpSCF, TDPCategory::dpMO_SM);
                //serviceKey from config.xml has a higher priority
                if ((cfgMOSM != recMOSM) && cfgMOSM) {
                    recMOSM = cfgMOSM;
                    abCsi.abRec.tdpSCF[TDPCategory::dpMO_SM] = GsmSCFinfo(abScf->scfAdr, recMOSM);
                }
                if (!recMOSM)
                    errmsg = "unable to determine IN MO-SM serviceKey";
            }
        }
        if (!errmsg && abCsi.vlrNum.empty())   //check for charged abonent location
            errmsg = "failed to determine abonent location MSC";
        if (errmsg) {
            err = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
            if ((billMode = billPrio->second) == ChargeObj::billOFF) {
                smsc_log_error(logger, "%s: %s", _logId, errmsg);
                return chargeResult(false, err);
            }
            smsc_log_error(logger, "%s: %s, switching to CDR mode", _logId, errmsg);
        }
    }

    if (billMode == ChargeObj::bill2IN) {
        RCHash capErr = startCAPSmTask();
        if (!capErr)
            return pgCont; //awaiting response from IN point
        
        err = capErr;
        std::string errStr = URCRegistry::explainHash(capErr);
        if ((billMode = billPrio->second) == ChargeObj::billOFF) {
            smsc_log_error(logger, "%s: %s", _logId, errStr.c_str());
            return chargeResult(false, err);
        }
        smsc_log_error(logger, "%s: %s, switching to CDR mode", _logId, errStr.c_str());
    }
    //billMode == ChargeObj::bill2CDR
    return chargeResult(true, err);
}

//NOTE: bilMutex should be locked upon entry
Billing::PGraphState Billing::onDeliverySmsResult(void)
{
    bool submitted = cdr._dlvrRes ? false : true;
    smsc_log_info(logger, "%s: --> DELIVERY_%s (code: %u)", _logId,
                    (submitted) ? "SUCCEEDED" : "FAILED", cdr._dlvrRes);

    if (capTask) { //report message submission to SCF
        RCHash rval = capTask->reportSMSubmission(submitted);
        if (rval) {
            billErr = rval;
            //if message has been already delivered, then just create CDR
            smsc_log_error(logger, "%s: %ssubmission report to %s failed: %s ", _logId,
                submitted ? "switching to CDR mode, " : "",
                abScf->Ident(), URCRegistry::explainHash(rval).c_str());
            if (submitted)
                billMode = ChargeObj::bill2CDR;
        } else
            cdr._inBilled = true;
        capTask->UnrefBy(this);
        capTask = NULL;
    }
    if ((cdr._chargePolicy == CDRRecord::ON_SUBMIT) && cdr._dlvrRes)
        state = bilAborted; //SMSC aborts charge request
    else
        state = bilReported;
    return Billing::pgEnd;
}

//NOTE: bilMutex should be locked upon entry
Billing::PGraphState Billing::chargeResult(bool do_charge, RCHash last_err /* = 0*/)
{
    if (last_err)
        billErr = last_err;
    std::string reply;

    if (!do_charge) {
        format(reply, "NOT_POSSIBLE (cause %u", billErr);
        state = Billing::bilReleased;
    } else {
        format(reply, "POSSIBLE (via %s, cause %u",
            (billMode == ChargeObj::bill2CDR) ? "CDR" : abScf->Ident(), billErr);
        state = Billing::bilContinued;
    }
    if (billErr) {
        reply += ": ";
        reply += URCRegistry::explainHash(billErr);
    }
    reply += ")";
    smsc_log_info(logger, "%s: <-- %s %s CHARGING_%s, abonent(%s) type: %s (%u)",
                _logId, cdr.dpType().c_str(), cdr._chargeType ? "MT" : "MO",
                reply.c_str(), abNumber.getSignals(),
                abCsi.abRec.type2Str(), (unsigned)abCsi.abRec.ab_type);

    if (cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
        return do_charge ? onDeliverySmsResult() : Billing::pgEnd;

    SPckChargeSmsResult res;
    res.Cmd().setValue(do_charge ? ChargeSmsResult::CHARGING_POSSIBLE : 
                                ChargeSmsResult::CHARGING_NOT_POSSIBLE);
    cdr._contract = static_cast<CDRRecord::ContractType>((unsigned)abCsi.abRec.ab_type);
    res.Cmd().setContract(cdr._contract);
    if (billErr)
        res.Cmd().setError(billErr, URCRegistry::explainHash(billErr).c_str());
    res.Hdr().dlgId = _wId;
    if (_mgr->sendCmd(&res)) {
        if (do_charge) {
            StartTimer(_cfg.maxTimeout); //expecting DeliverySmsResult
            return Billing::pgCont;
        }
        return Billing::pgEnd;
    }
    //TCP connect fatal failure
    abortThis(_mgr->connectError()->what());
    return Billing::pgAbort;
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks bilMutex !!!
TimeWatcherITF::SignalResult
    Billing::onTimerEvent(TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj)
{
    assert(opaque_obj);
    MutexTryGuard grd(bilMutex);
    if (!grd.tgtLocked()) //billing is busy, request resignalling
        return TimeWatcherITF::evtResignal;

    smsc_log_debug(logger, "%s: timer[%s] signaled, states: %u -> %u",
        _logId, tm_hdl.IdStr(), opaque_obj->val.ui, (unsigned)state);

    TimersMAP::iterator it = timers.find(opaque_obj->val.ui);
    if (it != timers.end())
        timers.erase(it); //deletes handle (unrefs timer)

    if (opaque_obj->val.ui == (unsigned)state) {
        //target operation doesn't complete yet.
        if (state == Billing::bilStarted) {
            //abonent provider query is expired
            abPolicy->getIAProvider()->cancelQuery(abNumber, this);
            providerQueried = false;
            if (ConfigureSCFandCharge() == Billing::pgEnd)
                doFinalize();
            return TimeWatcherITF::evtOk;
        }
        if (state == Billing::bilInited) { //CapSMTask lasts too long
            RCHash err = _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut);
            std::string errStr(capTask->TaskName());
            errStr += " lasts too long";
            
            capTask->Signal(TaskSchedulerITF::sigAbort);
            capTask->UnrefBy(this);
            capTask = NULL;
            
            bool doCharge = false;
            if ((billMode = billPrio->second) == ChargeObj::billOFF) {
                smsc_log_error(logger, "%s: %s", _logId, errStr.c_str());
            } else {
                smsc_log_error(logger, "%s: %s, switching to CDR mode", _logId, errStr.c_str());
                doCharge = true;
            }
            if (chargeResult(doCharge, err) == Billing::pgEnd)
                doFinalize();
            return TimeWatcherITF::evtOk;
        }
        if (state == Billing::bilContinued) {
            //SMSC doesn't respond with DeliveryResult
            abortThis("SMSC DeliverySmsResult is timed out");
            return TimeWatcherITF::evtOk;
        }
    } //else: operation already finished
    return TimeWatcherITF::evtOk; //grd off
}

/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                            RCHash qry_status)
{
    MutexGuard grd(bilMutex);

    providerQueried = false;
    if (state > bilStarted) {
        smsc_log_warn(logger, "%s: abonentQueried at state: %u", _logId, state);
        return;
    }
    StopTimer(state);
    state = bilQueried;
    if (qry_status) {
        billErr = qry_status;
        abCsi.abRec.Merge(ab_info.abRec); //merge known abonent info
    } else {
        abCsi.abRec = ab_info.abRec; //renew abonent info, overwrite TDPScfMAP
        _cfg.abCache->setAbonentInfo(abNumber, abCsi.abRec);
    }
    if (!ab_info.vlrNum.empty())
        abCsi.vlrNum = ab_info.vlrNum;

    if (ConfigureSCFandCharge() == Billing::pgEnd)
        doFinalize();
    return;
}

/* -------------------------------------------------------------------------- *
 * TaskRefereeITF interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onTaskReport(TaskSchedulerITF * sched, ScheduledTaskAC * task)
{
    MutexGuard grd(bilMutex);
    CAPSmTaskAC * sm_res = static_cast<CAPSmTaskAC *>(task);
    if (state == bilInited) {
        StopTimer(state);
        if (!sm_res->doCharge) {
            task->UnrefBy(this);
            capTask = NULL;
        }
        bool doCharge = sm_res->doCharge;
        if (!sm_res->doCharge && !sm_res->rejectRPC 
            && (billPrio->second == ChargeObj::bill2CDR)) {
            smsc_log_error(logger, "%s: %s interaction failure, switching to CDR mode",
                        _logId, abScf->Ident());
            billMode = ChargeObj::bill2CDR;
            doCharge = true;
        }
        if (chargeResult(doCharge, sm_res->scfErr) == Billing::pgEnd)
            doFinalize();
    } else if (state == bilContinued) {
        //abnormal CapSMTask termination, charging was allowed so create CDR
        //despite of secondary billmode setting
        task->UnrefBy(this);
        capTask = NULL;
        smsc_log_error(logger, "%s: %s interaction failure, switching to CDR mode",
                    _logId, abScf->Ident());
        billMode = ChargeObj::bill2CDR;
    } else
        smsc_log_warn(logger, "%s: %s reported at state: %u", _logId,
                        task->TaskName(), state);
    return;
}

} //inman
} //smsc

