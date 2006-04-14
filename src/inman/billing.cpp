static char const ident[] = "$Id$";
#include <assert.h>

#include "service.hpp"
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
 * class BillingService implementation:
 * ************************************************************************** */
BillingConnect::BillingConnect(BillingCFG * cfg, SSNSession* ss7_sess, Connect* conn, 
            TimeWatcher* tm_watcher, Logger * uselog/* = NULL*/)
    : _cfg(*cfg), _ss7Sess(ss7_sess), _conn(conn), _tmWatcher(tm_watcher)
{
    assert(conn && cfg && tm_watcher);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.BillConn");
    _bcId = _conn->getSocketId();
    if (!_cfg.maxBilling)
        _cfg.maxBilling = 20;
}

BillingConnect::~BillingConnect()
{
    MutexGuard grd(_mutex);
    //stop all Billings 
    for (BillingMap::iterator it = workers.begin(); it != workers.end(); it++) {
        Billing * bill = (*it).second;
        bill->Abort("BillingConnect destroyed", false);
        delete bill;
    }
    workers.clear();
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
    assert(bill);
    unsigned int billId = bill->getId();

    _mutex.Lock();
    BillingMap::iterator it = workers.find(billId);
    if (it == workers.end()) {
        _mutex.Unlock();
        smsc_log_error(logger, "BillConn[%u]: Attempt to free unregistered Billing[%u]",
                        _bcId, billId);
    } else {
        workers.erase(billId);
        _mutex.Unlock();
        smsc_log_info(logger, "Billing[%u.%u]: %scomplete, CDR is %scomposed, "
                       "cdrMode: %d, billingType: %sPAID",
                        _bcId, billId, bill->BillComplete() ? "" : "IN",
                       bill->CDRComplete() ? "" : "NOT ",
                       _cfg.cdrMode, bill->isPostpaidBill() ? "POST": "PRE");
        if (_cfg.cdrMode && bill->CDRComplete()) {
            if ((_cfg.cdrMode == BillingCFG::CDR_ALL)
                || ((_cfg.cdrMode == BillingCFG::CDR_POSTPAID)
                    && bill->isPostpaidBill())) {
                _cfg.bfs->bill(bill->getCDRRecord());
                smsc_log_info(logger, "Billing[%u.%u]: CDR written: msgId = %llu, billed by IN: %s",
                              _bcId, billId, (bill->getCDRRecord())._msgId, 
                              (bill->getCDRRecord())._inBilled ? "true": "false");
            }
        }
    }
    delete bill;
    smsc_log_debug(logger, "Billing[%u:%u]: finished", _bcId, billId);
}

/* -------------------------------------------------------------------------- *
 * ConnectListener interface implementation:
 * -------------------------------------------------------------------------- */
void BillingConnect::onCommandReceived(Connect* conn, SerializableObject* recvCmd)
{
    InmanCommand* cmd = static_cast<InmanCommand*>(recvCmd);
    assert( cmd );

    unsigned int dlgId = cmd->getDialogId();
    smsc_log_debug(logger, "BillConn[%u]: Cmd 0x%X for Billing[%u] received",
                   _bcId, cmd->getObjectId(), dlgId);

    Billing* bill = NULL;
    _mutex.Lock();
    BillingMap::iterator it = workers.find(dlgId);
    if (it == workers.end()) {
        if (workers.size() < _cfg.maxBilling) {
            bill = new Billing(this, dlgId, &_cfg, _tmWatcher, logger);
            workers.insert(BillingMap::value_type(dlgId, bill));
        } else {
            ChargeSmsResult res(InErrINprotocol, InProtocol_ResourceLimitation,
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
    _mutex.Unlock();
    if (bill)
        bill->handleCommand(cmd);
}

//Stops all Billings due to error condition pending on socket
void BillingConnect::onConnectError(Connect* conn, bool fatal/* = false*/)
{
    CustomException * exc = conn->hasException();
    smsc_log_error(logger, "BillConn[%u]: %s",
                   _bcId, exc ? exc->what() : "connect error");

    if (fatal) {
        _mutex.Lock();
        BillingMap  cpMap = workers;
        _mutex.Unlock();

        for (BillingMap::const_iterator it = cpMap.begin(); it != cpMap.end(); it++) {
            Billing * bill = (*it).second;
            bill->Abort(exc->what(), true);
        }
    }
}

/* ************************************************************************** *
 * class Billing implementation:
 * ************************************************************************** */
Billing::Billing(BillingConnect* bconn, unsigned int b_id, 
            BillingCFG * cfg, TimeWatcher* tm_watcher, Logger * uselog/* = NULL*/)
        : _bconn(bconn), _bId(b_id), _cfg(*cfg), _ss7Sess(NULL)
        , state(bilIdle), inap(NULL), postpaidBill(false), tmWatcher(tm_watcher)
        , abBillType(smsc::inman::cache::btUnknown), providerQueried(false)
{
    assert(bconn && cfg && tm_watcher);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.Billing");
}


Billing::~Billing()
{
    MutexGuard grd(bilMutex);
    //check for pending query to AbonentProvider
    if (providerQueried) {
        _cfg.abProvider->cancelQuery(abNumber, this);
        providerQueried = false;
    }
    if (timers.size()) {
        for (TimersMAP::iterator it = timers.begin(); it != timers.end(); it++) {
            StopWatch * timer = (*it).second;
            timer->release();
            smsc_log_debug(logger, "Billing[%u.%u]: Released timer[%u] at state %u",
                        _bconn->bConnId(), _bId, timer->getId(), state);
        }
    }
    if (inap)
        delete inap;
}

void Billing::Abort(const char * reason/* = NULL*/, bool doReport/* = true*/)
{
    bilMutex.Lock();
    smsc_log_error(logger, "Billing[%u.%u]: Aborting%s%s",
                   _bconn->bConnId(), _bId, reason ? ", reason " : "", reason ? reason : "");
    if (inap) {
        if ((state >= bilContinued) && (state < bilComplete)) { //send sms_o_failure to SCF
            smsc_log_debug(logger, "Billing[%u.%u]: SSF --> SCF EventReportSMS(o_smsFailure (0x%X))",
                           _bconn->bConnId(), _bId, EventTypeSMS_o_smsFailure);
            try {
                EventReportSMSArg    report(EventTypeSMS_o_smsFailure, MessageType_notification);
                inap->eventReportSMS(&report);
            } catch (std::exception & exc) { }
        }
        delete inap;
        inap = NULL;
    }
    state = Billing::bilAborted;
    bilMutex.Unlock();
    if (doReport)
        _bconn->billingDone(this);
}

void Billing::handleCommand(InmanCommand* cmd)
{
    unsigned short  cmdId = cmd->getObjectId();

    smsc_log_debug(logger, "Billing[%u.%u]: got Cmd 0x%X at state %u",
                                _bconn->bConnId(), _bId, cmdId, state);
    bilMutex.Lock();
    switch (state) {
    case Billing::bilIdle: {
        if (cmdId == CHARGE_SMS_TAG) {
            state = Billing::bilStarted;
            bilMutex.Unlock();
            bool goon = true;
            //complete the command deserialization
            try { cmd->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_error(logger, "Billing[%u.%u]: %s",
                                _bconn->bConnId(), _bId, exc.what());
                goon = false;
            }
            if (!goon || !onChargeSms(static_cast<ChargeSms*>(cmd))) {
                ChargeSmsResult res(InErrINprotocol, InProtocol_InvalidData,
                                    smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
                res.setDialogId(_bId);
                _bconn->sendCmd(&res);
                _bconn->billingDone(this);
            }
        } else {
            bilMutex.Unlock();
            smsc_log_error(logger, "Billing[%u.%u]: protocol error: cmd %u, state %u",
                           _bconn->bConnId(), _bId, (unsigned)cmdId, state);
            _bconn->billingDone(this);
        }
    } break;

    case Billing::bilContinued: {
        if (cmdId == DELIVERY_SMS_RESULT_TAG) {
            StopTimer(state, true);
            state = Billing::bilApproved;
            bilMutex.Unlock();
            //complete the command deserialization
            try { cmd->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_error(logger, "Billing[%u.%u]: %s",
                                _bconn->bConnId(), _bId, exc.what());
                Abort(exc.what(), true);
                return;
            }
            onDeliverySmsResult(static_cast<DeliverySmsResult*>(cmd));
            break;
        }
    } //no break, fall into default !!!

//    case Billing::bilStarted:
//    case Billing::bilInited:
//    case Billing::bilReleased:
//    case Billing::bilApproved:
//    case Billing::bilComplete:
//    case Billing::bilAborted:
    default: //ignore unknown/illegal command
        bilMutex.Unlock();
        smsc_log_error(logger, "Billing[%u.%u]: protocol error: cmd %u, state %u",
                       _bconn->bConnId(), _bId, (unsigned)cmdId, state);
    } /* eosw */
}

//returns true if all billing stages are completed
bool Billing::BillComplete(void) const
{
    return ((state == Billing::bilComplete) && cdr._finalized) ? true : false;
}

const CDRRecord & Billing::getCDRRecord(void) const
{
    return cdr;
}

SSNSession * Billing::activateSSN(void)
{
    smsc_log_debug(logger, "Billing[%u.%u]: Searching for TCAP session [SSN=%u] ..",
                    _bconn->bConnId(), _bId, _cfg.ssn);

    TCAPDispatcher *disp = TCAPDispatcher::getInstance();

    if (disp->getState() != TCAPDispatcher::ss7CONNECTED)
        return (_ss7Sess = NULL);
    if (!(_ss7Sess = disp->findSession(_cfg.ssn)))
        _ss7Sess = disp->openSession(_cfg.ssn, _cfg.ssf_addr,
                                     _cfg.scf_addr, ACOID::id_ac_cap3_sms_AC);
    return _ss7Sess;
}

bool Billing::startCAPDialog(void)
{
    if (!activateSSN()
        || (_ss7Sess->getState() != SSNSession::ssnBound)) {
        smsc_log_error(logger, "Billing[%u.%u]: SSNSession is not available/bounded",
                       _bconn->bConnId(), _bId);
        return false;
    }
    try { //Initiate CAP3 dialog
        inap = new Inap(_ss7Sess, this, _cfg.capTimeout, logger); //initialize TCAP dialog
        assert(inap);

        InitialDPSMSArg arg(smsc::inman::comp::DeliveryMode_Originating, _cfg.serviceKey);

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

        smsc_log_debug(logger, "Billing[%u.%u]: SSF --> SCF InitialDPSMS",
                        _bconn->bConnId(), _bId);
        state = Billing::bilInited;
        inap->initialDPSMS(&arg); //begins TCAP dialog
        return true;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Billing[%u.%u]: %s",
                       _bconn->bConnId(), _bId, exc.what());
        return false;
    }
}

void Billing::StartTimer(unsigned short timeout, bool locked/* = false*/)
{
    OPAQUE_OBJ  timerArg;

    timerArg.setUInt((unsigned)state);
    StopWatch * timer = tmWatcher->createTimer(this, &timerArg);
    smsc_log_debug(logger, "Billing[%u.%u]: Starting timer[%u]:%u",
                _bconn->bConnId(), _bId, timer->getId(), state);
    if (!locked)
        bilMutex.Lock();
    timers.insert(TimersMAP::value_type((unsigned)state, timer));
    if (!locked)
        bilMutex.Unlock();
    timer->start((long)timeout, false);
    return;
}

//NOTE: requires the Mutex being unlocked before call !
void Billing::StopTimer(Billing::BillingState bilState, bool locked/* = false */)
{
    if (!locked)
        bilMutex.Lock();

    TimersMAP::iterator it = timers.find((unsigned)bilState);
    if (it != timers.end()) {
        StopWatch * timer = (*it).second;
        timer->release();
        timers.erase(it);
        smsc_log_debug(logger, "Billing[%u.%u]: Stopped timer[%u]:%u at state %u",
                    _bconn->bConnId(), _bId, timer->getId(), bilState, state);
    } else
        smsc_log_warn(logger, "Billing[%u.%u]: no active timer for state: %u",
                    _bconn->bConnId(), _bId, bilState);
    if (!locked)
        bilMutex.Unlock();
    return;
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj)
{
    assert(opaque_obj);
    bilMutex.Lock();
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
            //abonent type query is expired, process in postpaid mode.
            if (!postpaidBill)
                smsc_log_debug(logger, "Billing[%u.%u]: switched to billing via CDR",
                        _bconn->bConnId(), _bId);
            postpaidBill = true;
            _cfg.abProvider->cancelQuery(abNumber, this);
            providerQueried = false;
            bilMutex.Unlock();
            ChargeAbonent(smsc::inman::cache::btUnknown);
            return;
        }
        if (state == Billing::bilContinued) {
            //SMSC doesn't respond with DeliveryResult
            bilMutex.Unlock();
            Abort("TCP dialog is timed out", true);
            return;
        }
    }
    bilMutex.Unlock();
    return; //operation already finished
}

void Billing::ChargeAbonent(AbonentBillType ab_type)
{
    smsc_log_debug(logger, "Billing[%u.%u]: charging, abonent type: %s (%u)",
                    _bconn->bConnId(), _bId, _sabBillType[ab_type], (unsigned)ab_type);

    abBillType = ab_type;
    if ((ab_type == smsc::inman::cache::btPostpaid)
        || ((ab_type == smsc::inman::cache::btUnknown)
            && !_cfg.postpaidRPC.size()))
        postpaidBill = true;

    if (!postpaidBill)
        postpaidBill = !startCAPDialog();

    if (postpaidBill) { //do not interact IN platform, just create CDR
        smsc_log_debug(logger, "Billing[%u.%u]: initiated billing via CDR",
                        _bconn->bConnId(), _bId);
        onContinueSMS();
    }

}
/* -------------------------------------------------------------------------- *
 * InAbonentQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onAbonentQueried(const AbonentId & ab_number, AbonentBillType ab_type)
{
    {
        MutexGuard grd(bilMutex);

        providerQueried = false;
        if (state > bilStarted) {
            smsc_log_warn(logger, "Billing[%u.%u]: abonentQueried at state: %u",
                          _bconn->bConnId(), _bId, state);
            return;
        }
        StopTimer(state, true);
        state = bilQueried;
    }
    ChargeAbonent(ab_type);
}

/* -------------------------------------------------------------------------- *
 * InmanHandler interface implementation:
 * -------------------------------------------------------------------------- */
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

    postpaidBill = ( (_cfg.billMode == smsc::inman::BILL_ALL)
                    || ((_cfg.billMode == smsc::inman::BILL_USSD)
                        && (cdr._bearer == CDRRecord::dpUSSD))
                    || ((_cfg.billMode == smsc::inman::BILL_SMS)
                        && (cdr._bearer == CDRRecord::dpSMS)) 
                   ) ? false : true;

    //ask AbonentsCache for abonent type
    AbonentBillType ab_type = _cfg.abCache->getAbonentInfo(abNumber);
    if (!postpaidBill && (ab_type == smsc::inman::cache::btUnknown)
        && !_cfg.postpaidRPC.size() && _cfg.abProvider) {
        //IN point unable to tell abonent billing type, request cache
        //to retrieve it from AbonentProvider
        if (_cfg.abProvider->startQuery(abNumber, this)) {
            providerQueried = true;
            StartTimer(_cfg.abtTimeout);
            return true; //execution will continue in onAbonentQueried() by another thread.
        }
        smsc_log_error(logger, "Billing[%u.%u]: startQuery(%s) failed!",
                       _bconn->bConnId(), _bId, abNumber.getSignals());
        //continue with btUnknown
    }
    ChargeAbonent(ab_type);
    return true;
}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
    bilMutex.Lock();
    EventTypeSMS_e eventType = smsRes->GetValue() ? EventTypeSMS_o_smsFailure :
                                                    EventTypeSMS_o_smsSubmission;

    smsc_log_info(logger, "Billing[%u.%u]: --> DELIVERY_%s (code: %u)",
                   _bconn->bConnId(), _bId,
                   (eventType == EventTypeSMS_o_smsFailure) ? "FAILED" : "SUCCEEDED",
                   smsRes->GetValue());

    smsRes->export2CDR(cdr);
    if (inap) { //continue TCAP dialog if it's still active
        smsc_log_debug(logger, "Billing[%u.%u]: SSF --> SCF EventReportSMS "
                       "( EventType: o_sms%s (0x%X), MessageType: notification(0x%X) )",
                       _bconn->bConnId(), _bId,
                       (eventType == EventTypeSMS_o_smsFailure) ? "Failure" : "Submission",
                       eventType, MessageType_notification);
        try {
            EventReportSMSArg    report(eventType, MessageType_notification);
            inap->eventReportSMS(&report);
            cdr._inBilled = true;
        } catch (std::exception & exc) {
            postpaidBill = true;
            smsc_log_error(logger, "Billing[%u.%u]: %s", exc.what());
            delete inap;
            inap = NULL;
        }
    }
    state = Billing::bilComplete;
    bilMutex.Unlock();
    _bconn->billingDone(this);
}

/* -------------------------------------------------------------------------- *
 * SSFhandler interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onConnectSMS(ConnectSMSArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF ConnectSMS", 
                   _bconn->bConnId(), _bId);
}

void Billing::onContinueSMS(uint32_t inmanErr /* = 0*/)
{
    smsc_log_info(logger,
            "Billing[%u.%u]: <-- CHARGING_POSSIBLE (via %s), abonent(%s) type: %s (%u)",
            _bconn->bConnId(), _bId, postpaidBill ? "CDR" : "SCF",
            abNumber.getSignals(), _sabBillType[abBillType], (unsigned)abBillType);

    BillAction  action = Billing::doCont;
    {
        MutexGuard grd(bilMutex);
        ChargeSmsResult res(inmanErr, smsc::inman::interaction::CHARGING_POSSIBLE);
        res.setDialogId(_bId);
        if (_bconn->sendCmd(&res)) {
            state = Billing::bilContinued;
            StartTimer(_cfg.maxTimeout, true);
        } else     //TCP connect fatal failure
            action = Billing::doAbort;
    }
    if (action == Billing::doAbort)
        Abort(_bconn->getConnectError()->what(), true);
    return;
}

//Huawei: #define POSTPAID_RPCause 41     //RP Cause: 'Temporary Failure'
void Billing::onReleaseSMS(ReleaseSMSArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF ReleaseSMS, RP cause: %u",
                   _bconn->bConnId(), _bId, (unsigned)arg->rPCause);
    BillAction  action = Billing::doEnd;
    {
        MutexGuard grd(bilMutex);
        //check for RejectSMS causes for postpaid abonents:
        for (RPCList::iterator it = _cfg.postpaidRPC.begin();
                                it != _cfg.postpaidRPC.end(); it++) {
            if ((*it) == arg->rPCause) {
                postpaidBill = true;
                //Update abonents cache
                _cfg.abCache->setAbonentInfo(abNumber, smsc::inman::cache::btPostpaid);
                break;
            }
        }
        if (!postpaidBill) { //check for RejectSMS causes indicating that abonent
                             //can't be charged (not just the technical failure)
            postpaidBill = true;
            for (RPCList::iterator it = _cfg.rejectRPC.begin();
                                    it != _cfg.rejectRPC.end(); it++) {
                if ((*it) == arg->rPCause) {
                    postpaidBill = false;
                    break;
                }
            }
        }

        uint32_t scfErr = InmanErrorCode::GetCombinedError(InErrRPCause, (uint16_t)arg->rPCause);
        if (postpaidBill) {
            smsc_log_info(logger,
                "Billing[%u.%u]: <-- CHARGING_POSSIBLE (via CDR), abonent(%s) type: %s (%u)",
                _bconn->bConnId(), _bId, abNumber.getSignals(),
                _sabBillType[abBillType], (unsigned)abBillType);
            state = Billing::bilContinued;
            delete inap;
            inap = NULL;
            action = Billing::doCont;
        } else {
            smsc_log_info(logger,
                "Billing[%u.%u]: <-- CHARGING_NOT_POSSIBLE (code %u), abonent(%s) type: %s (%u)",
                _bconn->bConnId(), _bId, scfErr, abNumber.getSignals(),
                _sabBillType[abBillType], (unsigned)abBillType);
            state = Billing::bilReleased;
        }

        ChargeSmsResult res(scfErr, postpaidBill ?
                            smsc::inman::interaction::CHARGING_POSSIBLE : 
                            smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
        res.setDialogId(_bId);
        if (_bconn->sendCmd(&res)) //TCP connect fatal failure
            StartTimer(_cfg.maxTimeout, true);
        else
            action = Billing::doAbort;
    }
    if (action == Billing::doEnd)
        _bconn->billingDone(this);
    else if (action == Billing::doAbort)
        Abort(_bconn->getConnectError()->what(), true);
    return;
}

//Called by Inap if CAP dialog with IN-platform is aborted.
//may be called if state is [bilInited .. bilApproved, bilComplete]
void Billing::onAbortSMS(unsigned char errCode, bool tcapLayer)
{
    smsc_log_error(logger, "Billing[%u.%u]: SSF <-- SCF Error, code: %u, layer %s",
                   _bconn->bConnId(), _bId, (unsigned)errCode, tcapLayer ? "TCAP" : "CAP3");
    bool  doCharge = false;
    {
        MutexGuard grd(bilMutex);
        switch (state) {
        case Billing::bilReleased: {
            //dialog with MSC already cancelled, just release CAP dialog
        } break;

        case Billing::bilInited: {
            //IN dialog initialization failed, release CAP dialog, switch to CDR mode 
            doCharge = true;
        } // no break specially !
        case Billing::bilContinued:
            //dialog with MSC is in process, release CAP dialog, switch to CDR mode
        case Billing::bilApproved:
        case Billing::bilComplete:
            //dialog with MSC finished, release CAP dialog, switch to CDR mode
            cdr._inBilled = false;
        default:
            postpaidBill = true;
            smsc_log_warn(logger, "Billing[%u.%u]: switched to billing via CDR (reason: SCF error).",
                          _bconn->bConnId(), _bId);
        }
        delete inap;
        inap = NULL;
    }
    if (doCharge)
        onContinueSMS(InmanErrorCode::GetCombinedError(
            tcapLayer ? InErrTCAP : InErrCAP3, (uint16_t)errCode));
}


void Billing::onFurnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF FurnishChargingInformationSMS",
                    _bconn->bConnId(), _bId);
}

void Billing::onRequestReportSMSEvent(RequestReportSMSEventArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF RequestReportSMSEvent",
                   _bconn->bConnId(), _bId);

    const RequestReportSMSEventArg::SMSEventVector& dps = arg->getSMSEvents();

    RequestReportSMSEventArg::SMSEventVector::const_iterator it;

    for( it = dps.begin(); it != dps.end(); it++ ) {
        RequestReportSMSEventArg::SMSEvent dp = *it;
        smsc_log_debug(logger, "Billing[%u.%u]: SCF requested Event: type: 0x%X, Monitor mode: 0x%X",
                       _bconn->bConnId(), _bId, dp.event, dp.monitorType);
    }
}

void Billing::onResetTimerSMS(ResetTimerSMSArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF ResetTimerSMS",
                   _bconn->bConnId(), _bId);
}

} //inman
} //smsc

