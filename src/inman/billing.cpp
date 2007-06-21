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
using smsc::inman::interaction::DeliveredSmsData;
using smsc::util::URCRegistry;

#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
using smsc::inman::comp::_RCS_MOSM_RPCause;

#include "inman/INManErrors.hpp"
using smsc::inman::iaprvd::_RCS_IAPQStatus;
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;


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
        , providerQueried(false), capDlgActive(false), capSess(NULL)
        , abPolicy(NULL), billErr(0), xsmsSrv(0), msgType(ChargeObj::msgUnknown)
        , billMode(ChargeObj::billOFF)
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
            state = Billing::bilApproved;
            bool badPdu = false;
            DeliverySmsResult* smsRes = static_cast<DeliverySmsResult*>(pck->pCmd());
            //complete the command deserialization
            try { 
                smsRes->loadDataBuf(); 
                smsRes->export2CDR(cdr);
            } catch (SerializerException & exc) {
                smsc_log_error(logger, "%s: %s", _logId, exc.what());
                badPdu = true;
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
        smsc_log_info(logger, "%s: CDR written: msgId = %llu, IN billed: %s, charged: %s",
                    _logId, cdr._msgId, cdr._inBilled ? "true": "false",
                    abNumber.toString().c_str());
    }
    return cnt;
}

void Billing::doFinalize(bool doReport/* = true*/)
{
    unsigned cdrs = 0;
    if (_cfg.cdrMode && BillComplete() && _cfg.bfs)
        cdrs = writeCDR();

    smsc_log_info(logger, "%s: %scomplete(%s, %s), %s --> %s(cause: %u),"
                          " abonent(%s), type %s, CDR(s) written: %u", _logId,
            BillComplete() ? "" : "IN", cdr._chargeType ? "MT" : "MO",
            _chgPolicy[cdr._chargePolicy], cdr.dpType().c_str(),
            cdr._inBilled ? (abScf._ident.empty() ? "SCF" : abScf._ident.c_str()) :
                            _cfg.billModeStr(billMode), billErr,
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
    if (capDlgActive) {
        capDlg->endDPSMS(); //send sms_o_failure to SCF
        capDlgActive = false;
    }
    state = Billing::bilAborted;
    doFinalize(doReport);
}

RCHash Billing::startCAPDialog(INScfCFG * use_scf)
{
    if (!_cfg.ss7.userId) {
        smsc_log_error(logger, "%s: SS7 stack is not connected!", _logId);
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit);
    }

    TCAPDispatcher * disp = TCAPDispatcher::getInstance();
    SSNSession * ssnSess = disp->findSession(_cfg.ss7.own_ssn);
    if (!ssnSess) //attempt to open SSN
        ssnSess = disp->openSSN(_cfg.ss7.own_ssn, _cfg.ss7.maxDlgId);

    if (!ssnSess || (ssnSess->getState() != smsc::inman::inap::ssnBound)) {
        smsc_log_error(logger, "%s: SSN session is not available/bound", _logId);
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit);
    }

    if (!(capSess = ssnSess->newSRsession(_cfg.ss7.ssf_addr,
            ACOID::id_ac_cap3_sms_AC, 146, use_scf->scf.scfAddress))) {
        std::string sid;
        TCSessionAC::mkSignature(sid, _cfg.ss7.own_ssn, _cfg.ss7.ssf_addr,
                                 ACOID::id_ac_cap3_sms_AC, 146, &(use_scf->scf.scfAddress));
        smsc_log_error(logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit);
    }
    smsc_log_debug(logger, "%s: using TCSR[%u]: %s", _logId,
                   capSess->getUID(), capSess->Signature().c_str());
    
    //determine destination address for CAP dialog
    TonNpiAddress dstAdr;
    if (cdr._chargeType) {  //MT
        if (!dstAdr.fromText(cdr._srcAdr.c_str())) {
            smsc_log_error(logger, "%s: invalid Call.Adr '%s'", _logId, cdr._srcAdr.c_str());
            return _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
        }
    } else {                //MO
        if (xsmsSrv) {
            dstAdr = xsmsSrv->adr;
        } else if (!dstAdr.fromText(cdr._dstAdr.c_str())) {
            smsc_log_error(logger, "%s: invalid Dest.Adr '%s'", _logId, cdr._dstAdr.c_str());
            return _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
        }
    }
    RCHash rval = 0;
    try { //Initiate CAP3 dialog: charge target abonent as if it attempts to send a SMS
        capDlg = new CapSMSDlg(capSess, this, _cfg.ss7.capTimeout, logger); //throws
        smsc_log_debug(logger, "%s: Initiating CapSMS[%u] %s -> %s", _logId, capDlg->getId(),
                        abNumber.toString().c_str(), dstAdr.toString().c_str());

        InitialDPSMSArg arg(smsc::inman::comp::DeliveryMode_Originating, use_scf->scf.serviceKey);

        switch (use_scf->idpLiAddr) {
        case INScfCFG::idpLiSSF:
            arg.setLocationInformationMSC(_cfg.ss7.ssf_addr); break;
        case INScfCFG::idpLiSMSC:
            arg.setLocationInformationMSC(csInfo.smscAddress.c_str()); break;
        default:
            arg.setLocationInformationMSC(abCsi.vlrNum);
        }
        arg.setCallingPartyNumber(abNumber);
        arg.setIMSI(abCsi.abRec.getImsi());
        arg.setDestinationSubscriberNumber(dstAdr);

        arg.setSMSCAddress(csInfo.smscAddress.c_str());
        arg.setTimeAndTimezone(cdr._submitTime);
        arg.setTPShortMessageSpecificInfo(csInfo.tpShortMessageSpecificInfo);
        arg.setTPValidityPeriod(csInfo.tpValidityPeriod, smsc::inman::comp::tp_vp_relative);
        arg.setTPProtocolIdentifier(csInfo.tpProtocolIdentifier);
        arg.setTPDataCodingScheme(csInfo.tpDataCodingScheme);
        capDlg->initialDPSMS(&arg); //begins TCAP dialog
        capDlgActive = true;
        state = Billing::bilInited;
        return 0;
    } catch (const CustomException & c_exc) {
        smsc_log_error(logger, "%s: %s", _logId, c_exc.what());
        rval = (RCHash)(c_exc.errorCode());
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "%s: %s", _logId, exc.what());
        rval = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit);
    }
    delete capDlg;
    capDlg = NULL;
    return rval;
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
            smsc_log_info(logger, "%s: %s[0x%x]: %u, %s", _logId,
                    xsmsSrv->name.empty() ? "SMSExtra service" : xsmsSrv->name.c_str(),
                    smsXMask, xsmsSrv->cdrCode, xsmsSrv->adr.toString().c_str());
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
        return chargeResult(ChargeSmsResult::CHARGING_NOT_POSSIBLE,
                    _RCS_INManErrors->mkhash(INManErrorId::cfgMismatch));
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
    if (abCsi.abRec.ab_type == AbonentContractInfo::abtPostpaid) {
        billMode = ChargeObj::bill2CDR;
        //do not interact IN platform, just create CDR
        return chargeResult(ChargeSmsResult::CHARGING_POSSIBLE);
    }

    //Here goes either abtPrepaid or abtUnknown ..
    //check for cache consistency
    if ((abCsi.abRec.ab_type == AbonentContractInfo::abtPrepaid)
         && (!abCsi.abRec.getSCFinfo() || !abCsi.abRec.getImsi()))
        abCsi.abRec.ab_type = AbonentContractInfo::abtUnknown;

    if (!(abPolicy = _cfg.policies->getPolicy(&abNumber)))
        smsc_log_error(logger, "%s: no policy set for %s", _logId, 
                        abNumber.toString().c_str());

    //check if AbonentProvider should be requested for contract type
    bool askProvider = ((abCsi.abRec.ab_type == AbonentContractInfo::abtUnknown)
                        && ((billMode == ChargeObj::bill2IN)
                            || (_cfg.cntrReq == BillingCFG::reqAlways)
                            || (cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED))
                        );

    //check if AbonentProvider should be requested for current abonent location
    if (abCsi.vlrNum.empty() && !askProvider && abPolicy
        && (abPolicy->getIAPAbilities() & smsc::inman::iaprvd::abSCF)) {
        askProvider = true;
    }

    if (askProvider && abPolicy) {
        smsc_log_debug(logger, "%s: using policy %s for %s", _logId, abPolicy->Ident(),
                        abNumber.toString().c_str());
        // configure SCF by quering provider first
        IAProviderITF *prvd = abPolicy->getIAProvider(logger);
        if (prvd) {
            if (prvd->startQuery(abNumber, this)) {
                providerQueried = true;
                StartTimer(_cfg.abtTimeout);
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
        return chargeResult(ChargeSmsResult::CHARGING_POSSIBLE);
    }
    //Here goes either abtPrepaid or abtUnknown ..
    RCHash err = 0;
    if (billMode == ChargeObj::bill2IN) {
        const char * errmsg = NULL;
        const GsmSCFinfo * p_scf = abCsi.abRec.getSCFinfo();
        if (p_scf) { //SCF is set only for prepaid abonent by cache/IAProvider
            abScf.scf = *p_scf;
            if (abPolicy) //lookup policy for extra SCF parms (serviceKey, RPC lists)
                abPolicy->getSCFparms(&abScf);
        } else {    //attempt to determine SCF params from config.xml
            if (!abPolicy) {
                errmsg = "unable to determine IN params, no policy set";
            } else {
                if (abCsi.abRec.ab_type == AbonentContractInfo::abtPrepaid) {
                    //look for single IN serving
                    if (abPolicy->scfMap.size() == 1)
                        abScf = *((*(abPolicy->scfMap.begin())).second);
                    else
                        errmsg = "unable to determine IN params";
                } else { //ab_type == AbonentContractInfo::abtUnknown
                    //look for single IN serving with postpaidRPC defined
                    INScfCFG * pin = NULL;
                    if (abPolicy->scfMap.size() == 1) {
                        pin = (*(abPolicy->scfMap.begin())).second;
                        if (pin->postpaidRPC.size())
                            abScf = *pin;
                        else
                            pin = NULL;
                    }
                    if (!pin)
                        errmsg = "unable to determine IN params (postpaidRPC)";
                }
            }
        }
        if (!errmsg && !abScf.scf.serviceKey)  //check for serviceKey being set
            errmsg = "unable to determine IN serviceKey";
        if (!errmsg && abCsi.vlrNum.empty())   //check for charged abonent location
            errmsg = "failed to determine abonent location MSC";
        if (errmsg) {
            err = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
            billMode = billPrio->second;
            if (billMode == ChargeObj::billOFF) {
                smsc_log_error(logger, "%s: %s", _logId, errmsg);
                return chargeResult(ChargeSmsResult::CHARGING_NOT_POSSIBLE, err);
            }
            smsc_log_error(logger, "%s: %s, switching to CDR mode", _logId, errmsg);
        }
    }

    if (billMode == ChargeObj::bill2IN) {
        smsc_log_debug(logger, "%s: using SCF %s:{%u}", _logId,
                abScf._ident.size() ? abScf.ident() : abScf.scf.scfAddress.getSignals(),
                abScf.scf.serviceKey);
        RCHash capErr = startCAPDialog(&abScf);
        if (!capErr)
            return pgCont; //awaiting response from IN point
        
        err = capErr;
        std::string errStr = URCRegistry::explainHash(capErr);
        billMode = billPrio->second;
        if (billMode == ChargeObj::billOFF) {
            smsc_log_error(logger, "%s: %s", _logId, errStr.c_str());
            return chargeResult(ChargeSmsResult::CHARGING_NOT_POSSIBLE, err);
        }
        smsc_log_error(logger, "%s: %s, switching to CDR mode", _logId, errStr.c_str());
    }
    //billMode == ChargeObj::bill2CDR
    return chargeResult(ChargeSmsResult::CHARGING_POSSIBLE, err);
}

//NOTE: bilMutex should be locked upon entry
Billing::PGraphState Billing::onDeliverySmsResult(void)
{
    bool submitted = cdr._dlvrRes ? false : true;
    smsc_log_info(logger, "%s: --> DELIVERY_%s (code: %u)", _logId,
                    (submitted) ? "SUCCEEDED" : "FAILED", cdr._dlvrRes);

    if (capDlgActive) { //continue CAP dialog if it's still active
        RCHash rval = 0;
        try {
            capDlg->reportSubmission(submitted);
            cdr._inBilled = true;
            state = bilReported;
        } catch (const CustomException & c_exc) {
            smsc_log_error(logger, "%s: %s", _logId, c_exc.what());
            rval = (RCHash)(c_exc.errorCode());
        } catch (const std::exception& exc) {
            smsc_log_error(logger, "%s: %s", _logId, exc.what());
            rval = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgFatal);
        }
        if (rval) {
            billErr = rval;
            capDlgActive = false;
            billMode = ChargeObj::bill2CDR; //message may be already delivered!
        }
    }
    if (!capDlgActive) {
        if ((cdr._chargePolicy == CDRRecord::ON_SUBMIT) && cdr._dlvrRes)
            state = bilAborted;
        else
            state = bilReported;
        return Billing::pgEnd;
    } //else wait onEndCapDlg();
    return Billing::pgCont;
}

//NOTE: bilMutex should be locked upon entry
Billing::PGraphState Billing::chargeResult(ChargeSmsResult::ChargeSmsResult_t chg_res,
                                            RCHash inmanErr /* = 0*/)
{
    if (inmanErr)
        billErr = inmanErr;
    std::string reply;

    if (chg_res != ChargeSmsResult::CHARGING_POSSIBLE) {
        format(reply, "NOT_POSSIBLE (cause %u", billErr);
        if (billErr) {
            reply += ": ";
            reply += URCRegistry::explainHash(billErr);
        }
        reply += ")";
        state = Billing::bilReleased;
    } else {
        format(reply, "POSSIBLE (via %s, cause %u)",
            billMode == ChargeObj::bill2CDR ? "CDR" :
            (abScf._ident.empty() ? "SCF" : abScf._ident.c_str()), billErr);
        state = Billing::bilContinued;
    }
    smsc_log_info(logger, "%s: <-- %s %s CHARGING_%s, abonent(%s) type: %s (%u)",
                _logId, cdr.dpType().c_str(), cdr._chargeType ? "MT" : "MO",
                reply.c_str(), abNumber.getSignals(),
                abCsi.abRec.type2Str(), (unsigned)abCsi.abRec.ab_type);

    if (cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED) {
        if (chg_res == ChargeSmsResult::CHARGING_POSSIBLE)
            return onDeliverySmsResult();
        return Billing::pgEnd;
    }
    SPckChargeSmsResult res;
    res.Cmd().setValue(chg_res);
    if (billErr)
        res.Cmd().setError(billErr, URCRegistry::explainHash(billErr).c_str());
    res.Hdr().dlgId = _wId;
    if (_mgr->sendCmd(&res)) {
        if (chg_res == ChargeSmsResult::CHARGING_POSSIBLE) {
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
            if (ConfigureSCFandCharge() == Billing::pgEnd)
                doFinalize();
            return;
        }
        if (state == Billing::bilContinued) {
            //SMSC doesn't respond with DeliveryResult
            abortThis("SMSC DeliverySmsResult is timed out");
            return;
        }
    } //else: operation already finished
    return; //grd off
}

/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                            IAPQStatus::Code qry_status)
{
    MutexGuard grd(bilMutex);

    providerQueried = false;
    if (state > bilStarted) {
        smsc_log_warn(logger, "%s: abonentQueried at state: %u", _logId, state);
        return;
    }
    StopTimer(state);
    state = bilQueried;
    abCsi.abRec = ab_info.abRec; //renew abonent info
    if (!ab_info.vlrNum.empty())
        abCsi.vlrNum = ab_info.vlrNum;

    if (qry_status != IAPQStatus::iqOk)
        billErr = _RCS_IAPQStatus->mkhash(qry_status);

    if (ConfigureSCFandCharge() == Billing::pgEnd)
        doFinalize();
    return;
}

/* -------------------------------------------------------------------------- *
 * CapSMS_SSFhandlerITF interface implementation:
 * NOTE: all callbacks are the processing graph entry points, so lock bilMutex !!!
 * -------------------------------------------------------------------------- */
void Billing::onDPSMSResult(unsigned dlg_id, unsigned char rp_cause/* = 0*/)
{
    MutexGuard grd(bilMutex);
    uint32_t            scfErr = 0;
    ChargeSmsResult::ChargeSmsResult_t  chgRes = ChargeSmsResult::CHARGING_POSSIBLE;

    if (!rp_cause) {    //ContinueSMS
        if (abCsi.abRec.ab_type != AbonentContractInfo::abtPrepaid)  //Update abonents cache
            _cfg.abCache->setAbonentInfo(abNumber, AbonentRecord(abCsi.abRec.ab_type = 
                                AbonentContractInfo::abtPrepaid, 0, &abScf.scf));
    } else {            //ReleaseSMS
        capDlgActive = false;
        scfErr = _RCS_MOSM_RPCause->mkhash(rp_cause);
        chgRes = ChargeSmsResult::CHARGING_NOT_POSSIBLE;
        //check for RejectSMS causes for postpaid abonents:
        for (RPCList::iterator it = abScf.postpaidRPC.begin(); 
                                it != abScf.postpaidRPC.end(); it++) {
            if ((*it) == rp_cause) {
                //Update abonents cache
                if (abCsi.abRec.ab_type != AbonentContractInfo::abtPostpaid)
                    _cfg.abCache->setAbonentInfo(abNumber, AbonentRecord(
                            abCsi.abRec.ab_type = AbonentContractInfo::abtPostpaid));
                billMode = ChargeObj::bill2CDR;
                chgRes = ChargeSmsResult::CHARGING_POSSIBLE;
                chargeResult(chgRes, scfErr);
                return;  //wait for onEndCapDlg()
            }
        }
        //NOTE: in case of technical failure, the message still may be
        //charged(depending on billMode settings), so check for RejectSMS
        //causes indicating that charging can't be done because of low balance.
        if (billPrio->second == ChargeObj::bill2CDR) {
            chgRes = ChargeSmsResult::CHARGING_POSSIBLE;
            for (RPCList::iterator it = abScf.rejectRPC.begin();
                                    it != abScf.rejectRPC.end(); it++) {
                if ((*it) == rp_cause) {
                    chgRes = ChargeSmsResult::CHARGING_NOT_POSSIBLE;
                    break;
                }
            }
        }
        if (chgRes == ChargeSmsResult::CHARGING_POSSIBLE) {
            billMode = ChargeObj::bill2CDR;
            smsc_log_info(logger, "%s: switching to CDR mode", _logId);
        }
    }
    chargeResult(chgRes, scfErr);
    return;  //wait for onEndCapDlg()
}

void Billing::onEndCapDlg(unsigned dlg_id, RCHash errcode/* = 0*/)
{
    MutexGuard grd(bilMutex);
    capDlgActive = false;
    if (!errcode) { //succesfull end
        //either T_End{ReleaseSMS} or T-End in response to EventReportSms
        if ((state == bilReleased) || (state == bilReported))
            doFinalize();
        else
            smsc_log_error(logger, "%s: onEndCapDlg(0) at state: %u",
                           _logId, (unsigned)state);
    } else {        //abnormal end
        billErr = errcode;
        bool  contCharge = false;
        ChargeSmsResult::ChargeSmsResult_t chg_res = ChargeSmsResult::CHARGING_POSSIBLE;
        switch (state) {
        case Billing::bilComplete:
        case Billing::bilAborted:
        case Billing::bilReleased: {
            //dialog with SMSC already cancelled/finished, just release CAP dialog
        } break;

        case Billing::bilInited: {
            //IN dialog initialization failed, release CAP dialog,
            //if billMode setting allows, switch to CDR mode 
            if (billPrio->second != ChargeObj::bill2CDR)
                chg_res = ChargeSmsResult::CHARGING_NOT_POSSIBLE;
            contCharge = true;
        } // no break specially !
        case Billing::bilContinued:
            //dialog with SMSC is in process, charging was allowed,
            //release CAP dialog, switch to CDR mode
        case Billing::bilApproved:
        case Billing::bilReported:
            //dialog with SMSC finished, message might be delivered,
            //release CAP dialog, switch to CDR mode
            cdr._inBilled = false;
        default:
            billMode = ChargeObj::bill2CDR;
        }
        smsc_log_error(logger, "%s: %sCapSMSDlg error: %u, %s", _logId,
            (billMode != ChargeObj::bill2CDR) ? "" : "switching to CDR mode, reason ",
            errcode, URCRegistry::explainHash(errcode).c_str());

        if (contCharge && (chargeResult(chg_res, billErr) == Billing::pgEnd))
            doFinalize();
    }
    return;
}

} //inman
} //smsc

