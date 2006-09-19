static char const ident[] = "$Id$";
#include <assert.h>

#include "service.hpp"
using smsc::inman::inap::TCSessionAC;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::CHARGE_SMS_TAG;
using smsc::inman::interaction::CHARGE_SMS_RESULT_TAG;
using smsc::inman::interaction::DELIVERY_SMS_RESULT_TAG;
using smsc::inman::BILL_MODE;
using smsc::inman::comp::EventReportSMSArg;
using smsc::inman::cache::_sabBillType;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class BillingConnect implementation:
 * ************************************************************************** */
BillingConnect::BillingConnect(BillingCFG * cfg, SSNSession * ssn_sess,
                               Connect* conn, Logger * uselog/* = NULL*/)
    : _cfg(*cfg), ssnSess(ssn_sess), _conn(conn)
{
//    assert(conn && cfg && cfg->tmWatcher);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.BillConn");
    _bcId = _conn->getSocketId();
    if (!_cfg.maxBilling)
        _cfg.maxBilling = 20;
}

BillingConnect::~BillingConnect()
{
    MutexGuard grd(_mutex);

    //delete died billings first
    cleanUpBills();

    //abort all active Billings
    for (BillingMap::iterator it = workers.begin(); it != workers.end(); it++) {
        Billing * bill = (*it).second;
        bill->Abort("BillingConnect destroyed");
        delete bill;
    }
    workers.clear();
}

//mutex shouldbe locked prior to calling this function
void BillingConnect::cleanUpBills(void)
{
    if (corpses.size()) {
        for (BillingList::iterator it = corpses.begin(); it != corpses.end(); it++)
            delete (*it);
        corpses.clear();
    }
    return;
}

//returns true on success, false on closed connect(possibly due to error)
bool BillingConnect::sendCmd(SerializableObject* cmd)
{
    if (_conn)
        return (_conn->sendObj(cmd) > 0) ? true : false;
    return false;
}

void BillingConnect::billingDone(Billing* bill)
{
    MutexGuard grd(_mutex);
    //delete died billings first
    cleanUpBills();

    unsigned int billId = bill->getId();
    BillingMap::iterator it = workers.find(billId);
    if (it == workers.end())
        smsc_log_error(logger, "BillConn[%u]: Attempt to free unregistered Billing[%u]",
                        _bcId, billId);
    else
        workers.erase(it); //billId
    corpses.push_back(bill);
    //smsc_log_debug(logger, "Billing[%u.%u]: released", _bcId, billId);
    return; //grd off
}

SSNSession* BillingConnect::ssnSession(void)
{
    if (!ssnSess) {
        TCAPDispatcher * disp = TCAPDispatcher::getInstance();
        ssnSess = disp->openSSN(_cfg.ss7.own_ssn, _cfg.ss7.maxDlgId);
    }
    return (!ssnSess || (ssnSess->getState() != smsc::inman::inap::ssnBound)) ? NULL: ssnSess;
}

/* -------------------------------------------------------------------------- *
 * ConnectListener interface implementation:
 * -------------------------------------------------------------------------- */
void BillingConnect::onCommandReceived(Connect* conn, SerializableObject* recvCmd)
{
    Billing* bill = NULL;
    InmanCommand* cmd = static_cast<InmanCommand*>(recvCmd);
    assert(cmd);
    {
        MutexGuard   grd(_mutex);
        //delete died billings first
        cleanUpBills(); 
        //assign command to Billing
        unsigned int dlgId = cmd->getDialogId();
        smsc_log_debug(logger, "BillConn[%u]: Cmd 0x%X for Billing[%u] received",
                       _bcId, cmd->getObjectId(), dlgId);
        
        BillingMap::iterator it = workers.find(dlgId);
        if (it == workers.end()) {
            if (workers.size() < _cfg.maxBilling) {
                bill = new Billing(this, dlgId, logger);
                workers.insert(BillingMap::value_type(dlgId, bill));
            } else {
                ChargeSmsResult res(errProtocol, InProtocol_ResourceLimitation,
                                    smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
                res.setDialogId(dlgId);
                sendCmd(&res);
                smsc_log_warn(logger, "BillConn[%u]: maxBilling limit reached: %u",
                              _bcId, _cfg.maxBilling);

                if (logger->isDebugEnabled()) {
                    std::string dump;
                    format(dump, "BillConn[%u]: Workers [%u of %u]: ", _bcId,
                           workers.size(), _cfg.maxBilling);
                    for (it = workers.begin(); it != workers.end(); it++) {
                        Billing* worker = (*it).second;
                        format(dump, "[%u].%u, ", worker->getId(), worker->getState());
                    }
                    smsc_log_debug(logger, dump.c_str());
                }   
            }
        } else
            bill = (*it).second;
    } //grd off
    if (bill)
        bill->handleCommand(cmd);
    return;
}

//Stops all Billings due to error condition pending on socket
void BillingConnect::onConnectError(Connect* conn, bool fatal/* = false*/)
{
    MutexGuard grd(mutex);
    CustomException * exc = conn->hasException();
    smsc_log_error(logger, "BillConn[%u]: %s",
                   _bcId, exc ? exc->what() : "connect error");

//    if (fatal) { /*stops all billings*/ }
    return; //grd off
}

/* ************************************************************************** *
 * class Billing implementation:
 * ************************************************************************** */
Billing::Billing(BillingConnect* bconn, unsigned int b_id, 
                Logger * uselog/* = NULL*/)
        : _bconn(bconn), _bId(b_id), state(bilIdle), capDlg(NULL)
        , postpaidBill(false), abType(smsc::inman::cache::btUnknown)
        , providerQueried(false), capDlgActive(false), capSess(NULL)
        , abPolicy(NULL)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.Billing");
    _cfg = bconn->getConfig();
}


Billing::~Billing()
{
    MutexGuard grd(bilMutex);
    doCleanUp();
    if (capDlg) {
        delete capDlg;
        capDlg = NULL;
    }
    smsc_log_debug(logger, "Billing[%u.%u]: Deleted", _bconn->bConnId(), _bId);
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
            smsc_log_debug(logger, "Billing[%u.%u]: Released timer[%u] at state %u",
                        _bconn->bConnId(), _bId, timer->getId(), state);
        }
        timers.clear();
    }
    return;
}

void Billing::doFinalize(bool doReport/* = true*/)
{
    smsc_log_info(logger, "Billing[%u.%u]: %scomplete, CDR is %sprepared, "
                   "cdrMode: %d, billingType: %sPAID(%s)",
                   _bconn->bConnId(), _bId, BillComplete() ? "" : "IN",
                   cdr._finalized ? "" : "NOT ",
                   _cfg.cdrMode, postpaidBill ? "POST": "PRE",
                   cdr._inBilled ? "IN": "CDR");

    if ((_cfg.cdrMode && cdr._finalized && _cfg.bfs)
        && ((_cfg.cdrMode == BillingCFG::CDR_ALL)
            || ((_cfg.cdrMode == BillingCFG::CDR_POSTPAID) && postpaidBill))) {
        _cfg.bfs->bill(cdr);
        smsc_log_info(logger, "Billing[%u.%u]: CDR written: msgId = %llu, billed by IN: %s",
                    _bconn->bConnId(), _bId, cdr._msgId, cdr._inBilled ? "true": "false");
    }
    smsc_log_debug(logger, "Billing[%u.%u]: %s, state: %u",
                   _bconn->bConnId(), _bId, doReport ? "finished" : "cancelled", state);
    doCleanUp();
    if (doReport) {
        state = bilComplete;
        _bconn->billingDone(this);
    }
    return;
}

void Billing::abortThis(const char * reason/* = NULL*/, bool doReport/* = true*/)
{
    smsc_log_error(logger, "Billing[%u.%u]: Aborting%s%s",
                   _bconn->bConnId(), _bId, reason ? ", reason: " : "", reason ? reason : "");
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
    SSNSession * ssnSess = _bconn->ssnSession();
    if (!ssnSess) {
        smsc_log_error(logger, "Billing[%u.%u]: SSN session is not available/bound",
                       _bconn->bConnId(), _bId);
        return false;
    }
    if (!(capSess = ssnSess->newSRsession(_cfg.ss7.ssf_addr,
            ACOID::id_ac_cap3_sms_AC, 146, use_scf->scf.scfAddress))) {
        std::string sid;
        TCSessionAC::mkSignature(sid, _cfg.ss7.own_ssn, _cfg.ss7.ssf_addr,
                                 ACOID::id_ac_cap3_sms_AC, 146, &(use_scf->scf.scfAddress));
        smsc_log_error(logger, "Billing[%u.%u]: Unable to init TCSR session: %s",
                       _bconn->bConnId(), _bId, sid.c_str());
        return false;
    }
    smsc_log_debug(logger, "Billing[%u.%u]: using TCSR[%u]: %s", _bconn->bConnId(), _bId,
                   capSess->getUID(), capSess->Signature().c_str());

    try { //Initiate CAP3 dialog
        capDlg = new CapSMSDlg(capSess, this, _cfg.ss7.capTimeout, logger); //initialize TCAP dialog
        capDlgActive = true;
        smsc_log_debug(logger, "Billing[%u.%u]: Initiating CapSMS[%u]",
                        _bconn->bConnId(), _bId, capDlg->getId());

        InitialDPSMSArg arg(smsc::inman::comp::DeliveryMode_Originating, use_scf->scf.serviceKey);

        arg.setDestinationSubscriberNumber(cdr._dstAdr.c_str());
        arg.setCallingPartyNumber(cdr._srcAdr.c_str());
        arg.setIMSI(cdr._srcIMSI.c_str());
        arg.setLocationInformationMSC(cdr._srcMSC.c_str());
        arg.setTimeAndTimezone(cdr._submitTime);

        arg.setSMSCAddress(csInfo.smscAddress.c_str());
        arg.setTPShortMessageSpecificInfo(csInfo.tpShortMessageSpecificInfo);
        arg.setTPValidityPeriod(csInfo.tpValidityPeriod, smsc::inman::comp::tp_vp_relative);
        arg.setTPProtocolIdentifier(csInfo.tpProtocolIdentifier);
        arg.setTPDataCodingScheme(csInfo.tpDataCodingScheme);
        capDlg->initialDPSMS(&arg); //begins TCAP dialog
        state = Billing::bilInited;
        return true;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Billing[%u.%u]: %s", _bconn->bConnId(), _bId, exc.what());
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
    smsc_log_debug(logger, "Billing[%u.%u]: Starting timer[%u]:%u",
                _bconn->bConnId(), _bId, timer->getId(), state);
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
        smsc_log_debug(logger, "Billing[%u.%u]: Released timer[%u]:%u at state %u",
                    _bconn->bConnId(), _bId, timer->getId(), bilState, state);
    } else
        smsc_log_warn(logger, "Billing[%u.%u]: no active timer for state: %u",
                    _bconn->bConnId(), _bId, bilState);
    return;
}

//NOTE: bilMutex should be locked upon entry!
bool Billing::onChargeSms(ChargeSms* sms)
{
    sms->export2CDR(cdr);
    sms->exportCAPInfo(csInfo);

    if (!abNumber.fromText(cdr._srcAdr.c_str())) {
        smsc_log_error(logger, "Billing[%u.%u]: invalid Call.Adr <%s>",
                       _bconn->bConnId(), _bId, cdr._srcAdr.c_str());
        return false;
    }
    smsc_log_info(logger, "Billing[%u.%u]: %s: Call.Adr <%s>, Dest.Adr <%s>",
                    _bconn->bConnId(), _bId, 
                   (cdr._bearer == CDRRecord::dpUSSD) ? "dpUSSD" : "dpSMS",
                   cdr._srcAdr.c_str(), cdr._dstAdr.c_str());
    //check for source IMSI & MSC
    if (!strcmp("MAP_PROXY", cdr._srcSMEid.c_str())) {
        if (!cdr._srcIMSI.length() || !cdr._srcMSC.length()) {
            std::string rec;
            CDRRecord::csvEncode(cdr, rec);
            smsc_log_error(logger, "Billing[%u.%u]: empty source IMSI or MSC!",
                           _bconn->bConnId(), _bId);
            smsc_log_error(logger, "Billing[%u.%u]: header: <%s>", 
                           _bconn->bConnId(), _bId, smsc::inman::cdr::_CDRRecordHeader_TEXT);
            smsc_log_error(logger, "Billing[%u.%u]: cdr   : <%s>", 
                           _bconn->bConnId(), _bId, rec.c_str());
        }
    }

    postpaidBill = ( (_cfg.billMode == smsc::inman::BILL_ALL)
                    || ((_cfg.billMode == smsc::inman::BILL_USSD)
                        && (cdr._bearer == CDRRecord::dpUSSD))
                    || ((_cfg.billMode == smsc::inman::BILL_SMS)
                        && (cdr._bearer == CDRRecord::dpSMS)) 
                   ) ? false : true;

    AbonentRecord   abRec; //ab_type = btUnknown
    if (postpaidBill
        || ((abType = _cfg.abCache->getAbonentInfo(abNumber, &abRec))
             == smsc::inman::cache::btPostpaid)) {
        postpaidBill = true; //do not interact IN platform, just create CDR
        chargeResult(smsc::inman::interaction::CHARGING_POSSIBLE);
        return true;
    }

    //here goes: btPrepaid or btUnknown
    abPolicy = _cfg.policies->getPolicy(&abNumber);
    smsc_log_debug(logger, "Billing[%u.%u]: using policy: %s",
                           _bconn->bConnId(), _bId, abPolicy->ident);

    if (abRec.getSCFinfo() && (abRec.ab_type == smsc::inman::cache::btPrepaid)) {
        return ConfigureSCFandCharge(abRec.ab_type, &abRec.gsmSCF); 
    }
    // configure SCF by quering provider
    IAProviderITF *prvd = abPolicy->getIAProvider(logger);
    if (prvd) {
        if (prvd->startQuery(abNumber, this)) {
            providerQueried = true;
            StartTimer(_cfg.abtTimeout);
            return true; //execution will continue in onIAPQueried() by another thread.
        }
        smsc_log_error(logger, "Billing[%u.%u]: startIAPQuery(%s) failed!",
                       _bconn->bConnId(), _bId, abNumber.getSignals());
    }
    return ConfigureSCFandCharge(abRec.ab_type, NULL);
}

bool Billing::ConfigureSCFandCharge(AbonentBillType ab_type, const MAPSCFinfo * p_scf/* = NULL*/)
{
    abType = ab_type;

    if (p_scf) { //SCF is set for abonent by cache/IAProvider
        abScf.scf = *p_scf;
        //lookup policy for extra SCF parms (serviceKey, RPC lists)
        abPolicy->getSCFparms(&abScf);
    } else {
        switch (abType) {
        case smsc::inman::cache::btUnknown: {
            INScfCFG * pin = NULL;
            if (abPolicy->scfMap.size() == 1) { //single IN serving, look for postpaidRPC
                pin = (*(abPolicy->scfMap.begin())).second;
                if (pin->postpaidRPC.size())
                    abScf = *pin;
            }
            if (!pin) {
                smsc_log_error(logger, "Billing[%u.%u]: unable to determine"
                                       " abonent type/SCF, switching to CDR mode",
                               _bconn->bConnId(), _bId);
                postpaidBill = true;
            }
        } break;

        case smsc::inman::cache::btPrepaid: {
            if (abPolicy->scfMap.size() == 1) { //single IN serving
                abScf = *((*(abPolicy->scfMap.begin())).second);
            } else {
                smsc_log_error(logger, "Billing[%u.%u]: unable to determine"
                                       " abonent SCF, switching to CDR mode",
                               _bconn->bConnId(), _bId);
                postpaidBill = true;
            }
        } break;
        default:; //case smsc::inman::cache::btPostpaid:
            postpaidBill = true;
        }

    }
    if (!postpaidBill) {
        smsc_log_debug(logger, "Billing[%u.%u]: using SCF %s:{%u}", _bconn->bConnId(), _bId, 
                       abScf._ident.size() ? abScf.ident() : abScf.scf.scfAddress.getSignals(),
                       abScf.scf.serviceKey);
        postpaidBill = !startCAPDialog(&abScf);
    }


    if (postpaidBill) //do not interact IN platform, just create CDR
        chargeResult(smsc::inman::interaction::CHARGING_POSSIBLE);
    return true;
}

//NOTE: bilMutex should be locked upon entry
void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
    bool submitted = smsRes->GetValue() ? false : true;
    smsc_log_info(logger, "Billing[%u.%u]: --> DELIVERY_%s (code: %u)",
                   _bconn->bConnId(), _bId, (submitted) ? "SUCCEEDED" : "FAILED",
                   smsRes->GetValue());

    smsRes->export2CDR(cdr);
    if (capDlgActive) { //continue TCAP dialog if it's still active
        try {
            capDlg->eventReportSMS(submitted);
            cdr._inBilled = true;
            state = bilReported;
        } catch (std::exception & exc) {
            postpaidBill = true;
            smsc_log_error(logger, "Billing[%u.%u]: %s", exc.what());
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
void Billing::chargeResult(ChargeSmsResult_t chg_res, uint32_t inmanErr /* = 0*/)
{
    std::string reply;

    if (chg_res != smsc::inman::interaction::CHARGING_POSSIBLE) {
        format(reply, "NOT_POSSIBLE (code %u)", inmanErr);
        state = Billing::bilReleased;
    } else {
        format(reply, "POSSIBLE (via %s)", postpaidBill ? "CDR" : "SCF");
        state = Billing::bilContinued;
    }
    smsc_log_info(logger, "Billing[%u.%u]: <-- CHARGING_%s, abonent(%s) type: %s (%u)",
            _bconn->bConnId(), _bId, reply.c_str(), abNumber.getSignals(),
            _sabBillType[abType], (unsigned)abType);

    ChargeSmsResult res(inmanErr, chg_res);
    res.setDialogId(_bId);
    if (_bconn->sendCmd(&res))
        StartTimer(_cfg.maxTimeout);
    else     //TCP connect fatal failure
        abortThis(_bconn->getConnectError()->what());
    return;
}

/* -------------------------------------------------------------------------- *
 * InmanHandler interface implementation:
 * -------------------------------------------------------------------------- */
 //NOTE: it's the processing graph entry point, so locks bilMutex !!!
void Billing::handleCommand(InmanCommand* cmd)
{
    MutexGuard  grd(bilMutex);
    unsigned short  cmdId = cmd->getObjectId();

    smsc_log_debug(logger, "Billing[%u.%u]: got Cmd 0x%X at state %u",
                                _bconn->bConnId(), _bId, cmdId, state);
    switch (state) {
    case Billing::bilIdle: {
        if (cmdId == CHARGE_SMS_TAG) {
            state = Billing::bilStarted;
            bool goon = true;
            //complete the command deserialization
            try { cmd->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_error(logger, "Billing[%u.%u]: %s",
                                _bconn->bConnId(), _bId, exc.what());
                goon = false;
            }
            if (!goon || !onChargeSms(static_cast<ChargeSms*>(cmd))) {
                ChargeSmsResult res(errProtocol, InProtocol_InvalidData,
                                    smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
                res.setDialogId(_bId);
                _bconn->sendCmd(&res);
                doFinalize();
            }
        } else {
            smsc_log_error(logger, "Billing[%u.%u]: protocol error: cmd %u, state %u",
                           _bconn->bConnId(), _bId, (unsigned)cmdId, state);
            doFinalize();
        }
    } break;

    case Billing::bilContinued: {
        if (cmdId == DELIVERY_SMS_RESULT_TAG) {
            StopTimer(state);
            state = Billing::bilApproved;
            //complete the command deserialization
            try { cmd->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_error(logger, "Billing[%u.%u]: %s",
                                _bconn->bConnId(), _bId, exc.what());
                abortThis(exc.what());
                return;
            }
            onDeliverySmsResult(static_cast<DeliverySmsResult*>(cmd));
            break;
        }
    } //no break, fall into default !!!
    default: //ignore unknown/illegal command
        smsc_log_error(logger, "Billing[%u.%u]: protocol error: cmd %u, state %u",
                       _bconn->bConnId(), _bId, (unsigned)cmdId, state);
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
    
    smsc_log_debug(logger, "Billing[%u.%u]: timer[%u] signaled, states: %u -> %u",
        _bconn->bConnId(), _bId, timer->getId(), opaque_obj->val.ui, (unsigned)state);

    TimersMAP::iterator it = timers.find(opaque_obj->val.ui);
    if (it == timers.end())
        smsc_log_warn(logger, "Billing[%u.%u]: timer[%u] is not registered!",
                    _bconn->bConnId(), _bId);
    else
        timers.erase(it);

    if (opaque_obj->val.ui == (unsigned)state) {
        //target operation doesn't complete yet.
        smsc_log_debug(logger, "Billing[%u.%u]: operation is timed out at state: %u",
                    _bconn->bConnId(), _bId, (unsigned)state);
        if (state == Billing::bilStarted) {
            //abonent provider query is expired
            abPolicy->getIAProvider(logger)->cancelQuery(abNumber, this);
            providerQueried = false;
            ConfigureSCFandCharge(smsc::inman::cache::btUnknown, NULL);
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
void Billing::onIAPQueried(const AbonentId & ab_number, AbonentBillType ab_type,
                           const MAPSCFinfo * scf/* = NULL*/)
{
    MutexGuard grd(bilMutex);

    providerQueried = false;
    if (state > bilStarted) {
        smsc_log_warn(logger, "Billing[%u.%u]: abonentQueried at state: %u",
                      _bconn->bConnId(), _bId, state);
        return;
    }
    StopTimer(state);
    state = bilQueried;
    ConfigureSCFandCharge(ab_type, scf);
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
    ChargeSmsResult_t   chgRes = smsc::inman::interaction::CHARGING_POSSIBLE;

    if (!rp_cause) {    //ContinueSMS
        if (abType != smsc::inman::cache::btPrepaid)  //Update abonents cache
            _cfg.abCache->setAbonentInfo(abNumber, abType = smsc::inman::cache::btPrepaid,
                                         0, &abScf.scf);
    } else {            //ReleaseSMS
        capDlgActive = false;
        //check for RejectSMS causes for postpaid abonents:
        for (RPCList::iterator it = abScf.postpaidRPC.begin(); 
                                it != abScf.postpaidRPC.end(); it++) {
            if ((*it) == rp_cause) {
                postpaidBill = true;
                //Update abonents cache
                if (abType != smsc::inman::cache::btPostpaid)
                    _cfg.abCache->setAbonentInfo(abNumber, abType = smsc::inman::cache::btPostpaid);
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
            chgRes = smsc::inman::interaction::CHARGING_NOT_POSSIBLE;
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
            smsc_log_error(logger, "Billing[%u.%u]: onEndCapDlg() at state: %u",
                           _bconn->bConnId(), _bId, (unsigned)state);
    } else {                                //AbortSMS
        smsc_log_error(logger, "Billing[%u.%u]: CapSMSDlg Error, code: %u, layer %s",
                       _bconn->bConnId(), _bId, (unsigned)ercode, _InmanErrorSource[errLayer]);
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
            smsc_log_warn(logger,
                "Billing[%u.%u]: switched to billing via CDR (reason: CapSMSDlg error).",
                _bconn->bConnId(), _bId);
        }
        if (contCharge)
            chargeResult(smsc::inman::interaction::CHARGING_POSSIBLE,
                        InmanErrorCode::combineError(errLayer, (uint16_t)ercode));
    }
    return;
}

} //inman
} //smsc

