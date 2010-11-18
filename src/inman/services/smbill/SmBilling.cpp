#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/SmBilling.hpp"
using smsc::inman::iaprvd::IAProviderAC;
using smsc::inman::iaprvd::CSIRecord;
using smsc::inman::iapmgr::IAProviderInfo;
using smsc::inman::iapmgr::INParmsCapSms;

using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPCSBilling;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::DeliveredSmsData;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::CsBillingHdr_dlg;
using smsc::core::synchronization::MutexTryGuard;
//using smsc::core::synchronization::TimeSlice;
using smsc::util::URCRegistry;

using smsc::inman::smbill::_SMSubmitOK;
using smsc::inman::smbill::_SMSubmitNO;

using smsc::inman::comp::UnifiedCSI;

#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
using smsc::inman::comp::_RCS_MOSM_RPCause;

//#include "inman/INManErrors.hpp"
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;

#ifdef SMSEXTRA
#define SMSX_WEB_GT "GT SMSCX"
#endif /* SMSEXTRA */

namespace smsc {
namespace inman {
namespace smbill {

const char * const _BILLmodes[] = {"OFF", "CDR", "IN"};
const char * const _MSGtypes[] = {"unknown", "SMS", "USSD", "XSMS"};
const char * const _CDRmodes[] = {"none", "billMode", "all"};

/* ************************************************************************** *
 * class SmBillManager implementation:
 * ************************************************************************** */
//NOTE: _mutex SHOULD be locked upon entry!
int SmBillManager::denyCharging(unsigned dlg_id, INManErrorId::Code_e use_error)
{
    if (!_conn)
        return -1;

    SPckChargeSmsResult spck;
    spck.Hdr().dlgId = dlg_id;
    spck.Cmd().setValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
    spck.Cmd().setError(_RCS_INManErrors->mkhash(use_error),
                        _RCS_INManErrors->explainCode(use_error).c_str());

    smsc_log_debug(logger, "%s: <-- CHARGING_NOT_POSSIBLE (cause %u: %s)",
                _logId, spck.Cmd().getError(), spck.Cmd().getMsg());
    return _conn->sendPck(&spck);
}
/* -------------------------------------------------------------------------- *
 * ConnectListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
void SmBillManager::onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
                        /*throw(std::exception)*/
{
    //check service header
    INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
    //check for header
    if (!pck->pHdr() || ((pck->pHdr())->Id() != INPCSBilling::HDR_DIALOG)) {
        smsc_log_error(logger, "%s: missed/unsupported cmd header", _logId);
        return;
    }
    Billing * bill = NULL;
    unsigned short  cmdId = (pck->pCmd())->Id();
    CsBillingHdr_dlg * srvHdr = static_cast<CsBillingHdr_dlg*>(pck->pHdr());
    {
        MutexGuard   grd(_mutex);
        //assign command to Billing
        smsc_log_debug(logger, "%s: Cmd[0x%X] for Billing[%u] received", _logId,
                       cmdId, srvHdr->dlgId);

        if (!isRunning() && (cmdId == INPCSBilling::CHARGE_SMS_TAG)) {
            denyCharging(srvHdr->dlgId, INManErrorId::srvInoperative); //ignore sending result here
            return;
        }

        bill = (Billing *)getWorker(srvHdr->dlgId);
        if (!bill) {
            if (numWorkers() < _cfg.prm->maxBilling) {
                insWorker(bill = new Billing(srvHdr->dlgId, this, logger));
                _denyCnt = 0;
            } else {
                ++_denyCnt;
                denyCharging(srvHdr->dlgId, INManErrorId::cfgLimitation); //ignore sending result here
                smsc_log_warn(logger, "%s: maxBilling limit reached: %u", _logId,
                            _cfg.prm->maxBilling);
                if (logger->isDebugEnabled()
                    && (_denyCnt >= (_cfg.prm->maxBilling/3))) {
                    std::string dump;
                    format(dump, "%s: Workers [%u of %u]: ", _logId,
                           numWorkers(), _cfg.prm->maxBilling);
                    dumpWorkers(dump);
                    smsc_log_debug(logger, dump.c_str());
                    _denyCnt = 0;
                }   
            }
        }
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
    MutexGuard grd(_sync);
    doCleanUp();
    smsc_log_debug(logger, "%s: Deleted", _logId);
}

const char * Billing::nmBState(BillingState bil_state)
{
    switch (bil_state) {
    case bilStarted:        return "bilStarted";
    case bilQueried:        return "bilQueried";
    case bilInited:         return "bilInited";
    case bilReleased:       return "bilReleased";
    case bilContinued:      return "bilContinued";
    case bilAborted:        return "bilAborted";
    case bilSubmitted:      return "bilSubmitted";
    case bilReported:       return "bilReported";
    case bilComplete:       return "bilComplete";
    case bilIdle:           return "bilIdle";
    }
    return "billUnknown";
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
//Prints some information about worker state/status
//NOTE: it's the processing graph entry point, so locks _sync !!!
void Billing::logState(std::string & use_str) const
{
    MutexGuard grd(_sync);
    format(use_str, "%u{%u}", _wId, state);
}

static const char * const _nm_cmd[] = {
    "", "CHARGE_SMS", "CHARGE_SMS_RESULT", "DELIVERY_SMS_RESULT", "DELIVERED_SMS_DATA"
};
//NOTE: it's the processing graph entry point, so locks _sync !!!
void Billing::handleCommand(INPPacketAC* pck)
{
    MutexGuard      grd(_sync);
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
                chrgFlags = chgReq->getChargingFlags();
            } catch (const SerializerException & exc) {
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
                chrgFlags = chgReq->getChargingFlags();
            } catch (const SerializerException & exc) {
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
            } catch (const SerializerException & exc) {
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

//NOTE: it's the processing graph entry point, so locks _sync !!!
void Billing::Abort(const char * reason/* = NULL*/)
{
    MutexGuard grd(_sync);
    if ((state != bilAborted) && (state != bilComplete)) {
      abortThis(reason);
    } else {
      smsc_log_warn(logger, "%s: Abort requested at state %u%s%s",
                     _logId, state, reason ? ", reason: " : "", reason ? reason : "");
    }
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock _sync,
 *       it's a caller responsibility to lock _sync !!!
 * ---------------------------------------------------------------------------------- */

void Billing::cancelIAPQuery(void)
{
  const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(_lastIAPrvd);
  if (pPrvd)
    pPrvd->_iface->cancelQuery(abNumber, this);
  providerQueried = false;
}

void Billing::doCleanUp(void)
{
    //check for pending query to AbonentProvider
    if (providerQueried)
      cancelIAPQuery();

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
    cdr._contract = static_cast<CDRRecord::ContractType>((unsigned)abCsi.abType);
    //set the location MSC of charged abonent to last known one
    if (cdr._chargeType) {  //MT: from delivery report
        if (cdr._dstMSC.empty() && !abCsi.vlrNum.empty())
            cdr._dstMSC = abCsi.vlrNum.toString();
    } else {                //MO: from chargeReq or abonentProvider
        if (!abCsi.vlrNum.empty())
            cdr._srcMSC = abCsi.vlrNum.toString();
    }
    if (!cdr._inBilled || (_cfg.prm->cdrMode == ChargeParm::cdrALL)) {
        //remove TonNpi for MSCs ids
        TonNpiAddress tna;
        if (tna.fromText(cdr._srcMSC.c_str()))
            cdr._srcMSC = tna.getSignals();
        else if ((cdr._chargeType == CDRRecord::MO_Charge) && abNumber.interISDN()) {
            smsc_log_warn(logger, "%s: empty MSC for %s", _logId, abNumber.toString().c_str());
        }

        if (tna.fromText(cdr._dstMSC.c_str()))
            cdr._dstMSC = tna.getSignals();
        else if ((cdr._chargeType == CDRRecord::MT_Charge) && abNumber.interISDN()) {
            smsc_log_warn(logger, "%s: empty MSC for %s", _logId, abNumber.toString().c_str());
        }
        _cfg.bfs->bill(cdr); cnt++;
        smsc_log_info(logger, "%s: TDR written: msgId: %llu, status: %u, IN billed: %s, charged: %s",
                    _logId, cdr._msgId, cdr._dlvrRes, cdr._inBilled ? "true": "false",
                    abNumber.toString().c_str());
    }
    return cnt;
}

void Billing::doFinalize(void)
{
    unsigned cdrs = 0;
    if (_cfg.prm->cdrMode && BillComplete() && _cfg.bfs.get())
        cdrs = writeCDR();

    smsc_log_info(logger, "%s: %scomplete(%s, %s), %s --> %s(cause %u),"
                          " abonent(%s), type %s, TDR(s) written: %u", _logId,
            BillComplete() ? "" : "IN", cdr._chargeType ? "MT" : "MO",
            cdr.nmPolicy(), cdr.dpType().c_str(),
            cdr._inBilled ? _cfgScf->Ident() : _cfg.prm->billModeStr(billMode), billErr,
            abNumber.getSignals(), abCsi.type2Str(), cdrs);

    doCleanUp();
    if (state != bilAborted)
      state = bilComplete;
    _mgr->workerDone(this);
}

//FSM switching:
//  entry:  [ any state ]
//  return: [ bilAborted, bilReported ]
void Billing::abortThis(const char * reason/* = NULL*/)
{
    smsc_log_error(logger, "%s: Aborting at state %u%s%s",
                   _logId, state, reason ? ", reason: " : "", reason ? reason : "");
    if (capTask) {
        unrefCAPSmTask();
        abortCAPSmTask();
    }
    if (state < bilReported)
      state = Billing::bilAborted;
    doFinalize();
}


RCHash Billing::startCAPSmTask(void)
{
    capSched = _cfg.schedMgr->getScheduler((_cfgScf->_capSms.idpReqMode == INParmsCapSms::idpReqMT) ? 
                        TaskSchedulerITF::schedMT : TaskSchedulerITF::schedSEQ);
    if (!capSched) {
        smsc_log_error(logger, "%s: TaskScheduler is not srarted", _logId);
        return _RCS_INManErrors->mkhash(INManErrorId::internalError);
    }
    CAPSmTaskCFG    cfgSS7(*(_cfg.prm->capSms.get()), _cfg.tcDisp, _cfgScf);
    const GsmSCFinfo * smScf = abCsi.getSCFinfo(UnifiedCSI::csi_MO_SM);
    std::auto_ptr<CAPSmTaskAC> smTask;

    if (_cfgScf->_capSms.idpReqMode == INParmsCapSms::idpReqMT) {
        smTask.reset(new CAPSmTaskMT(abNumber, cfgSS7, smScf->serviceKey, CAPSmTaskAC::idpMO, logger));
    } else
        smTask.reset(new CAPSmTaskSQ(abNumber, cfgSS7, smScf->serviceKey, CAPSmTaskAC::idpMO, logger));

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
        switch (_cfgScf->_capSms.idpLiAddr) {
        case INParmsCapSms::idpLiSSF:
            smTask->Arg().setLocationInformationMSC(cfgSS7.ownAddr); break;
        case INParmsCapSms::idpLiSMSC:
            smTask->Arg().setLocationInformationMSC(csInfo.smscAddress.c_str()); break;
        default:
            smTask->Arg().setLocationInformationMSC(abCsi.vlrNum);
        }
        smTask->Arg().setIMSI(abCsi.abImsi);
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
        if (!(capTask = capSched->StartTask(smTask.get(), this))) {
            smsc_log_error(logger, "%s: Failed to start %s", _logId, smTask->TaskName());
            rval = _RCS_TC_Dialog->mkhash(INManErrorId::logicTimedOut);
        } else {
            state = Billing::bilInited;
            capName = smTask->TaskName();
            smsc_log_debug(logger, "%s: Initiated %s", _logId, capName.c_str());
            smTask.release();
            //execution will continue in onTaskReport() or in onTimerEvent() by another thread.
            StartTimer(_cfg.maxTimeout);
        }
    }
    return rval; //!rval means Billing::pgCont;
}

//NOTE: _sync should be locked upon entry!
bool Billing::StartTimer(const TimeoutHDL & tmo_hdl)
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

//NOTE: _sync should be locked upon entry!
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
//NOTE: _sync should be locked upon entry!
bool Billing::verifyChargeSms(void)
{
    //determine which abonent should be charged
    if (!abNumber.fromText(cdr._chargeType ? cdr._dstAdr.c_str() : cdr._srcAdr.c_str())) {
        smsc_log_error(logger, "%s: invalid %s.Adr '%s'", _logId,
                        cdr._chargeType ? "Dest" : "Call",
                        cdr._chargeType ? cdr._dstAdr.c_str() : cdr._srcAdr.c_str());
        return false;
    }
    //remember IMSI of abonent is to charge (if present)
    abCsi.abImsi = cdr._chargeType ? cdr._dstIMSI : cdr._srcIMSI;
    //remember location MSC of abonent is to charge (if present)
    abCsi.vlrNum.fromText(cdr._chargeType ? cdr._dstMSC.c_str() : cdr._srcMSC.c_str());

    smsc_log_info(logger, "%s: %s(%s, %s): '%s' -> '%s'", _logId,
                cdr.dpType().c_str(), cdr._chargeType ? "MT" : "MO",
                cdr.nmPolicy(), cdr._srcAdr.c_str(), cdr._dstAdr.c_str());

    uint32_t smsXMask = cdr._smsXMask & ~SMSX_RESERVED_MASK;
    if (smsXMask) {
        if (cdr._bearer != CDRRecord::dpSMS) {
            smsc_log_error(logger, "%s: invalid bearer for SMS Extra service", _logId);
            return false;
        }
        if (!_cfg.prm->smsXMap.get()) {
            smsc_log_error(logger, "%s: SMS Extra services are not configured!", _logId);
            return false;
        }
        SmsXServiceMap::iterator it = _cfg.prm->smsXMap->find(smsXMask);
        if (it != _cfg.prm->smsXMap->end()) {
            xsmsSrv = &(it->second);
            cdr._serviceId = xsmsSrv->svcCode;
            smsc_log_info(logger, "%s: %s[0x%x]: %u, %s%s", _logId,
                    xsmsSrv->name.empty() ? "SMSExtra service" : xsmsSrv->name.c_str(),
                    smsXMask, xsmsSrv->svcCode, xsmsSrv->adr.toString().c_str(),
                    xsmsSrv->chargeBearer ? ", chargeBearer" : "");
        } else {
            smsc_log_error(logger, "%s: SMSExtra service[0x%x] misconfigured, ignoring!",
                           _logId, smsXMask);
            cdr._smsXMask &= SMSX_RESERVED_MASK;
        }
    }

    if (xsmsSrv)
        msgType = ChargeParm::msgXSMS;
    else
        msgType = (cdr._bearer == CDRRecord::dpUSSD) ? ChargeParm::msgUSSD : ChargeParm::msgSMS;

    //determine billmode
    billPrio = cdr._chargeType ? _cfg.prm->mt_billMode.modeFor(msgType) : 
                                    _cfg.prm->mo_billMode.modeFor(msgType);
    billMode = billPrio->first;

    //according to #B2501:
    if (msgType == ChargeParm::msgSMS) {
      //only bill2CDR & billOFF allowed for ordinary SMS
      cdr._smsXMask &= ~SMSX_INCHARGE_SRV;
      if (billMode == ChargeParm::bill2IN) {
        smsc_log_error(logger, "%s: incompatible billingMode and messageType", _logId);
        return false; 
      }
    }
    return true;
}

//NOTE: _sync should be locked upon entry!
Billing::PGraphState Billing::onChargeSms(void)
{
    if ((billMode == ChargeParm::bill2IN) && !_cfg.prm->capSms.get())
        billMode = billPrio->second;

    if (billMode == ChargeParm::billOFF) {
        return chargeResult(false, _RCS_INManErrors->mkhash(INManErrorId::cfgMismatch));
    }
    //Here goes either bill2IN or bill2CDR ..
    if ((cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
        || (cdr._chargePolicy == CDRRecord::ON_SUBMIT_COLLECTED))
        billMode = ChargeParm::bill2CDR;
    else if (cdr._smsXMask & SMSX_NOCHARGE_SRV)
        billMode = ChargeParm::bill2CDR;
    else if (cdr._smsXMask & SMSX_INCHARGE_SRV)
        billMode = ChargeParm::bill2IN;
    else if (chrgFlags & ChargeSms::chrgCDR)
        billMode = ChargeParm::bill2CDR;

#ifdef SMSEXTRA
    //TMP_PATCH: Use bill2CDR mode in case of special MSC address reserved for SMSX WEB gateway
    if (!strcmp(abCsi.vlrNum.getSignals(), SMSX_WEB_GT))
      billMode = ChargeParm::bill2CDR;
#endif /* SMSEXTRA */

    //check for SMS extra sevice number being set
    if ((xsmsSrv && xsmsSrv->adr.empty())) {
        billMode = ChargeParm::bill2CDR;
        billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgSpecific);
    }

    if (_cfg.abCache) {
        //NOTE: vlrNum doesn't stored in cache
        AbonentSubscription cacheRec;
        _cfg.abCache->getAbonentInfo(abNumber, &cacheRec);
        abCsi.Merge(cacheRec);     //merge available abonent info
    }
    //check for IMSI being defined
    if (!abCsi.getImsi())
        abCsi.abType = AbonentContractInfo::abtUnknown;

    if (abCsi.isPostpaid()) {
        billMode = ChargeParm::bill2CDR;
        //do not interact IN platform, just create CDR
        return chargeResult(true);
    }

    //Here goes either abtPrepaid or abtUnknown ..
    if (abCsi.isPrepaid())
      configureMOSM(); //check if cache data enough to determine gsmSCF params

    //check if AbonentProvider should be requested for contract type/gsmSCF params

    /* **************************************************** */
    /* conditions which switch ON provider request         */
    /* **************************************************** */
    bool askProvider = ((_cfg.prm->cntrReq == ChargeParm::reqAlways)
                        || ( (billMode == ChargeParm::bill2IN) 
                             && (abCsi.isUnknown() || !getServiceKey(UnifiedCSI::csi_MO_SM))
                           )
                        );

    //check if AbonentProvider should be requested for current abonent location
    if (abCsi.vlrNum.empty() && _cfg.iapMgr)
        askProvider = true;

    /* **************************************************** */
    /* conditions which switch OFF IAProvider request       */
    /* **************************************************** */

    //verify that abonent number is in ISDN international format
    if (!abNumber.interISDN() || (abNumber.length < 10)) //HOT-PATCH for short SME ISDN numbers
        askProvider = false;

    if (chrgFlags & ChargeSms::chrgCDR)
        askProvider = false;

    if (askProvider && _cfg.iapMgr) {
      if (determinePolicy()) {
        const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(_lastIAPrvd);
        if (pPrvd) {
          _lastIAPrvd = pPrvd->_iface->getType();
          if (StartTimer(_cfg.abtTimeout)
              && (providerQueried = pPrvd->_iface->startQuery(abNumber, this))) {
              state = bilStarted;
              //execution will continue in onIAPQueried()/onTimerEvent() by another thread.
              return Billing::pgCont;
          }
          billErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
          smsc_log_error(logger, "%s: startIAPQuery(%s) failed!", _logId, abNumber.getSignals());
        } else {
          billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
          smsc_log_warn(logger, "%s: no IAProvider configured for %s", _logId, abNumber.getSignals());
        }
      } else {
        billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
      }
    }
    return ConfigureSCFandCharge();
}

//Here goes either bill2IN or bill2CDR ..
Billing::PGraphState Billing::ConfigureSCFandCharge(void)
{
    if (abCsi.isPostpaid()) {
      billMode = ChargeParm::bill2CDR;
      return chargeResult(true);
    }
    //Here goes either abtPrepaid or abtUnknown ..
    RCHash err = 0;
    if (billMode == ChargeParm::bill2IN) {
      INManErrorId::Code_e rc = configureSCF();
      if (rc) {
        err = _RCS_INManErrors->mkhash(rc);
        if ((billMode = billPrio->second) == ChargeParm::billOFF)
          return chargeResult(false, err);
        //else //billCDR
        smsc_log_info(logger, "%s: switching to CDR mode for abonent(%s)",
                      _logId, abNumber.toString().c_str());
      }
    }
    if (billMode == ChargeParm::bill2IN) {
      err = startCAPSmTask();
      if (!err)
          return pgCont; //awaiting response from IN point

      smsc_log_error(logger, "%s: %s", _logId, URCRegistry::explainHash(err).c_str());
      if ((billMode = billPrio->second) == ChargeParm::billOFF)
        return chargeResult(false, err);
      //else //billCDR
      smsc_log_info(logger, "%s: switching to CDR mode for abonent(%s)",
                    _logId, abNumber.toString().c_str());
    }
    //billMode == ChargeParm::bill2CDR
    return chargeResult(true, err);
}

INManErrorId::Code_e Billing::configureSCF(void)
{
  configureMOSM();
  const GsmSCFinfo * smScf = abCsi.getSCFinfo(UnifiedCSI::csi_MO_SM);

  //verify gsmSCF & serviceKey
  if (smScf && !smScf->serviceKey) {
    smsc_log_error(logger, "%s: misconfigured serviceKey(%s) for gsmSCF(%s)", _logId,
                   UnifiedCSI::nmTDP(UnifiedCSI::csi_MO_SM),
                   smScf->scfAddress.toString().c_str());
    return INManErrorId::cfgInconsistency;
  }
  if (!smScf) {
    smsc_log_error(logger, "%s: unable to determine gsmSCF for abonent(%s)", _logId,
                   abNumber.toString().c_str());
    return INManErrorId::cfgInconsistency;
  }

  //verify IMSI
  if (!abCsi.getImsi() && _cfgScf && !_cfgScf->_dfltImsi.empty())
    abCsi.setImsi(_cfgScf->_dfltImsi.c_str());
  if (!abCsi.getImsi()) {
    smsc_log_error(logger, "%s: unable to determine IMSI for abonent(%s)", _logId,
                   abNumber.toString().c_str());
    return INManErrorId::cfgInconsistency;
  }

  //verify abonent location MSC
  if (abCsi.vlrNum.empty()) { 
    smsc_log_error(logger, "%s: unable to determine location MSC for abonent(%s)",
                   _logId, abNumber.toString().c_str());
    return INManErrorId::cfgInconsistency;
  }
  return INManErrorId::noErr;
}

//Adjusts the MO-SM gsmSCF parameters combining cache/IAProvider CSIs
//and gsmSCF parameters from config.xml
void Billing::configureMOSM(void)
{
  uint32_t          keyMOSM = 0;
  const CSIRecord * pCsi = abCsi.csiSCF.getCSIRecord(UnifiedCSI::csi_MO_SM);

  if (pCsi)
    keyMOSM = pCsi->scfInfo.serviceKey;
  else //check if SCF for MO-BC may be used
    pCsi = abCsi.csiSCF.getCSIRecord(UnifiedCSI::csi_O_BC);

  //check if MO_SM parameters are configured for serving gsmSCF
  //serviceKey from config.xml has a higher priority
  if (!_cfgScf && pCsi && determinePolicy())
    _cfgScf = _iapRule._iaPolicy->getSCFparms(pCsi->scfInfo.scfAddress);

  if (_cfgScf) {
    uint32_t  keyCfg = _cfgScf->getSKey(UnifiedCSI::csi_MO_SM, &abCsi.csiSCF);

    if (keyCfg && (keyCfg != keyMOSM)) { //update/create MO_SM record
      CSIRecord & csiRec = abCsi.csiSCF[UnifiedCSI::csi_MO_SM];
      csiRec.iapId = IAPProperty::iapUnknown;
      csiRec.scfInfo.serviceKey = keyCfg;
      if (pCsi->csiId != UnifiedCSI::csi_MO_SM)
        csiRec.scfInfo.scfAddress = pCsi->scfInfo.scfAddress;
    }
  }
}

uint32_t Billing::getServiceKey(CSIUid_e csi_id) const
{
  if (_cfgScf) //serviceKey from config.xml has a higher priority
    return _cfgScf->getSKey(csi_id, &abCsi.csiSCF);

  const CSIRecord * pCsi = abCsi.csiSCF.getCSIRecord(csi_id);
  return pCsi ? pCsi->scfInfo.serviceKey : 0;
}

const AbonentPolicy * Billing::determinePolicy(void)
{
  if (!_iapRule._iaPolicy && _cfg.iapMgr) {
    //determine policy rule
    const IAPRule * pRule = _cfg.iapMgr->getPolicy(abNumber);
    if (pRule) {
      _iapRule = *pRule;
    } else {
      _iapRule._nmPool = "<default>";
      _iapRule._iaPolicy = _cfg.iapMgr->getPolicy(_cfg.policyNm);
    }
    if (_iapRule._iaPolicy) {
      smsc_log_debug(logger, "%s: using policy %s for %s", _logId, _iapRule.toString().c_str(),
                      abNumber.toString().c_str());
    } else {
      smsc_log_error(logger, "%s: no IAPolicy configured for %s", _logId, abNumber.getSignals());
    }
  }
  return _iapRule._iaPolicy;
}

//NOTE: _sync should be locked upon entry
//FSM switching:
//  entry:  billContinued
//  return: [ bilAborted, bilSubmitted, bilReported ]
Billing::PGraphState Billing::onDeliverySmsResult(void)
{
    RCHash rval = 0;
    smsc_log_info(logger, "%s: --> DELIVERY_%s (code: %u)", _logId,
                    (!cdr._dlvrRes) ? "SUCCEEDED" : "FAILED", cdr._dlvrRes);
    
    if (capTask) { //report message submission to SCF
        TaskSchedulerITF::SchedulerRC
          schedRc = capSched->SignalTask(capTask, TaskSchedulerITF::sigProc,
                                         !cdr._dlvrRes ? &_SMSubmitOK : &_SMSubmitNO);
        if (TaskSchedulerITF::rcOk == schedRc) {
            state = bilSubmitted;
            //execution will continue either in onTaskReport() or in onTimerEvent()
            StartTimer(_cfg.maxTimeout);
            return Billing::pgCont;
        } //else task cann't be interacted!
        smsc_log_error(logger, "%s: sending signal to %s is failed: %s(%u) ", _logId,
                       capName.c_str(), capSched->nmRCode(schedRc), (unsigned)schedRc);
        unrefCAPSmTask(false);
        rval = _RCS_INManErrors->mkhash(INManErrorId::internalError);
    }
    return onSubmitReport(rval);
}

//NOTE: _sync should be locked upon entry
//FSM switching:
//  entry:  [ bilStarted, bilQueried, bilInited ]
//  return: [ bilReleased, bilContinued ]
Billing::PGraphState Billing::chargeResult(bool do_charge, RCHash last_err /* = 0*/)
{
    if (last_err)
        billErr = last_err;
    state = !do_charge ? Billing::bilReleased : Billing::bilContinued;

    if (logger->isInfoEnabled()) {
      std::string reply = !do_charge ? 
                    format("NOT_POSSIBLE (cause %u", billErr) :
                    format("POSSIBLE (via %s, cause %u", (billMode == ChargeParm::bill2CDR) ?
                           "CDR" : _cfgScf->Ident(), billErr);
      if (billErr) {
          reply += ": ";
          reply += URCRegistry::explainHash(billErr);
      }
      reply += ")";
      logger->log_(smsc::logger::Logger::LEVEL_INFO, 
                   "%s: <-- %s %s CHARGING_%s, abonent(%s) type: %s (%u)",
                   _logId, cdr.dpType().c_str(), cdr._chargeType ? "MT" : "MO",
                   reply.c_str(), abNumber.getSignals(),
                   abCsi.type2Str(), (unsigned)abCsi.abType);
    }

    if ((cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
        || (cdr._chargePolicy == CDRRecord::ON_SUBMIT_COLLECTED))
        return do_charge ? onDeliverySmsResult() : Billing::pgEnd;

    SPckChargeSmsResult res;
    res.Cmd().setValue(do_charge ? ChargeSmsResult::CHARGING_POSSIBLE : 
                                ChargeSmsResult::CHARGING_NOT_POSSIBLE);
    cdr._contract = static_cast<CDRRecord::ContractType>((unsigned)abCsi.abType);
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

//NOTE: _sync should be locked upon entry
//FSM switching:
//  entry:  billSubmitted
//  return: [ bilAborted, bilReported ]
Billing::PGraphState Billing::onSubmitReport(RCHash scf_err, bool in_billed/* = false*/)
{
    bool submitted = cdr._dlvrRes ? false : true;
    bool smscAborted = ((cdr._chargePolicy == CDRRecord::ON_SUBMIT) && cdr._dlvrRes);

    if (scf_err) {
        billErr = scf_err;
        //if message has been already delivered, then create CDR
        //even if secondary billing mode is OFF
        if (submitted) 
            billMode = ChargeParm::bill2CDR;
        smsc_log_error(logger, "%s: %s%s interaction failure: %s",
                _logId, submitted ? "switching to CDR mode: " : "", _cfgScf->Ident(),
                URCRegistry::explainHash(scf_err).c_str());
    } else
        cdr._inBilled = in_billed;

    state = smscAborted ? bilAborted : bilReported;
    return Billing::pgEnd;
}


bool Billing::unrefCAPSmTask(bool wait_report/* = true */)
{
    bool rval = capSched->UnrefTask(capTask, this);
    if (!rval && wait_report) {
        //task currently awaits for Billing::_sync to call onTaskReport()
        int res = capEvent.WaitOn(_sync, _cfg.maxTimeout);
        if (!res)
            rval = true;
        else
            smsc_log_error(logger, "%s: %s unbind failed, code %u",
                           _logId, capName.c_str(), res);
    }
    return rval;
}

void Billing::abortCAPSmTask(void)
{
  if (capTask) {
    capSched->AbortTask(capTask);
    capTask = 0;
  }
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks _sync !!!
TimeWatcherITF::SignalResult
    Billing::onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj)
{
    MutexTryGuard grd(_sync);
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
            cancelIAPQuery();
            if (Billing::pgEnd == ConfigureSCFandCharge())
                doFinalize();
            return TimeWatcherITF::evtOk;
        }
        if (state == Billing::bilInited) { //CapSMTask lasts too long
            //CapSMTask suspends while awaiting Continue/Release from SCF
            bool doCharge = ((billMode = billPrio->second) 
                                    == ChargeParm::billOFF) ? false : true;
            smsc_log_error(logger, "%s: %s%s is timed out (RRSM)", _logId, 
                           doCharge ? "switching to CDR mode: " : "", capName.c_str());

            unrefCAPSmTask();
            abortCAPSmTask();
            if (Billing::pgEnd == chargeResult(doCharge,
                        _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut)))
                doFinalize();
            return TimeWatcherITF::evtOk;
        }
        if (state == Billing::bilContinued) {
            //SMSC doesn't respond with DeliveryResult
            abortThis("SMSC DeliverySmsResult is timed out");
            return TimeWatcherITF::evtOk;
        }
        if ((state == Billing::bilSubmitted) && capTask) {
            //CapSMTask suspends while reporting submission to SCF
            smsc_log_error(logger, "%s: %s is timed out (ERSM)", _logId, capName.c_str());
            unrefCAPSmTask();
            abortCAPSmTask();
            if (Billing::pgEnd == onSubmitReport(
                        _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut)))
                doFinalize();
            return TimeWatcherITF::evtOk;
        }
    } //else: operation already finished
    return TimeWatcherITF::evtOk; //grd off
}

/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks _sync !!!
void Billing::onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                            RCHash qry_status)
{
    MutexGuard grd(_sync);

    providerQueried = false;
    if (state > bilStarted) {
      smsc_log_warn(logger, "%s: abonentQueried at state: %u", _logId, state);
      return;
    }
    StopTimer(state);
    state = bilQueried;

#ifdef SMSEXTRA
    //TMP_PATCH: Keep MSC address in case of special one reserved for SMSX WEB gateway
    if (!strcmp(abCsi.vlrNum.getSignals(), SMSX_WEB_GT)) {
      TonNpiAddress orgVLR = abCsi.vlrNum;
      abCsi.Merge(ab_info); //merge known abonent info
      abCsi.vlrNum = orgVLR;
    } else
#endif /* SMSEXTRA */
      abCsi.Merge(ab_info); //merge known abonent info

    if (qry_status)
      billErr = qry_status;
    else if (_cfg.abCache)
      _cfg.abCache->setAbonentInfo(abNumber, abCsi);

    if (ConfigureSCFandCharge() == Billing::pgEnd)
      doFinalize();
    return;
}

/* -------------------------------------------------------------------------- *
 * TaskRefereeITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: whithin this method capSched->UnrefTask() always returns false
//      because of Billing is already targeted by task for reporting.
void Billing::onTaskReport(TaskSchedulerITF * sched, const ScheduledTaskAC * task)
{
    MutexGuard grd(_sync);
    const CAPSmTaskAC * sm_res = static_cast<const CAPSmTaskAC *>(task);
    RCHash capErr = sm_res->scfErr;

    if (state == bilInited) {
        StopTimer(state);
        bool doCharge = sm_res->doCharge;
        bool scfCharge = sm_res->doCharge;

        if (sm_res->curPMode() != CAPSmTaskAC::pmInstructing) {
            smsc_log_error(logger, "%s: %s dissynchonization: %s <- %s", _logId,
                           capName.c_str(), nmBState(), sm_res->nmPMode());
            scfCharge = false;
            capErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
            if (billPrio->second == ChargeParm::bill2CDR) {
                smsc_log_error(logger, "%s: switching to CDR mode: %s interaction failure",
                               _logId, _cfgScf->Ident());
                billMode = ChargeParm::bill2CDR;
                doCharge = true;
            } else
                doCharge = false;
        } else if (!sm_res->doCharge && !sm_res->rejectRPC
                && (billPrio->second == ChargeParm::bill2CDR)) {
            smsc_log_error(logger, "%s: switching to CDR mode: %s interaction failure",
                        _logId, _cfgScf->Ident());
            billMode = ChargeParm::bill2CDR;
            doCharge = true;
        }
        if (!scfCharge) { //CapSMS task will be released by itself
            unrefCAPSmTask(false);
            capTask = 0;
        }
        if (Billing::pgEnd == chargeResult(doCharge, capErr))
            doFinalize();
    } else if (state == bilSubmitted) {
        //submission status was reported to SCF
        StopTimer(state);
        if (sm_res->curPMode() != CAPSmTaskAC::pmAcknowledging) {
            smsc_log_error(logger, "%s: %s dissynchonization: %s <- %s", _logId,
                           capName.c_str(), nmBState(), sm_res->nmPMode());
            capErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
        }
        if (capErr) {
            //Note1: it's unknown whether the SCF finalized transaction or 
            //      not, so unconditionally create CDR and rise in_billed flag
            smsc_log_error(logger, "%s: switching to CDR mode: %s interaction failure",
                           _logId, _cfgScf->Ident());
            billMode = ChargeParm::bill2CDR;
        }
        unrefCAPSmTask(false);
        //Note2: considering Note1 rise in_billed flag despite of capErr value
        if (Billing::pgEnd == onSubmitReport(capErr, true))
            doFinalize();
    } else if (state == bilContinued) {
        //abnormal CAPSmTask termination, charging was allowed so create CDR
        //despite of secondary billmode setting
        unrefCAPSmTask(false);
        smsc_log_error(logger, "%s: switching to CDR mode: %s interaction failure: %s",
                    _logId, _cfgScf->Ident(), URCRegistry::explainHash(capErr).c_str());
        billMode = ChargeParm::bill2CDR;
        billErr = sm_res->scfErr;
    } else {
        smsc_log_warn(logger, "%s: %s reported: %s <- %s", _logId,
                      capName.c_str(), nmBState(), sm_res->nmPMode());
        if ((sm_res->curPMode() == CAPSmTaskAC::pmInstructing) && sm_res->doCharge) {
            //dissynchonization -> abort CAPSmTask
            unrefCAPSmTask(false);
            abortCAPSmTask();
        }
    }
    return;
}

} //smbill
} //inman
} //smsc

