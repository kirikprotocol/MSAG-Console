static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>

#include "billing.hpp"
#include "service.hpp"

using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::CHARGE_SMS_TAG;
using smsc::inman::interaction::CHARGE_SMS_RESULT_TAG;
using smsc::inman::interaction::DELIVERY_SMS_RESULT_TAG;
using smsc::inman::BILL_MODE;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class BillingService implementation:
 * ************************************************************************** */
BillingConnect::BillingConnect(BillingCFG * cfg, Session* ss7_sess, Connect* conn, Service * in_srvc,
               Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(*cfg), _ss7Sess(ss7_sess)
    , _conn(conn), _inSrvc(in_srvc)
{
    assert(conn && in_srvc && cfg);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.BillConn");
    _bcId = _conn->getSocketId();
    if (!_cfg.maxBilling)
        _cfg.maxBilling = 10;
}

BillingConnect::~BillingConnect()
{
    //stop all Billings 
    BillingMap  cpMap = workers;
    for (BillingMap::const_iterator it = cpMap.begin(); it != cpMap.end(); it++) {
        Billing * bill = (*it).second;
        bill->Stop(NULL);
    }
}

//returns true on success, false on closed connect(possibly due to error)
bool BillingConnect::sendCmd(SerializableObject* cmd, bool timerOn)
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
        smsc_log_debug(logger, "BillConn[%u]: Billing[%u] %scomplete, CDR is %scomposed, "
                       "cdrMode: %d, billingType: %sPAID",
                        _bcId, billId, bill->BillComplete() ? "" : "IN",
                       bill->CDRComplete() ? "" : "NOT ",
                       _cfg.cdrMode, bill->isPostpaidBill() ? "POST": "PRE");
        if (_cfg.cdrMode && bill->CDRComplete()) {
            if ((_cfg.cdrMode == BillingCFG::CDR_ALL)
                || ((_cfg.cdrMode == BillingCFG::CDR_POSTPAID)
                    && bill->isPostpaidBill())) {
                _inSrvc->writeCDR(_bcId, billId, bill->getCDRRecord());
            }
        }
    }
    delete bill;
    smsc_log_debug(logger, "BillConn[%u]: Billing[%u] finished", _bcId, billId);
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
            bill = new Billing(this, dlgId, &_cfg, logger);
            workers.insert(BillingMap::value_type(dlgId, bill));
        } else {
            ChargeSmsResult res(InErrINprotocol, InProtocol_ResourceLimitation,
                                smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
            res.setDialogId(dlgId);
            sendCmd(&res, false);
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
            bill->Stop(exc);
        }
    }
}

/* ************************************************************************** *
 * class Billing implementation:
 * ************************************************************************** */
Billing::Billing(BillingConnect* bconn, unsigned int b_id, 
            BillingCFG * cfg, Logger * uselog/* = NULL*/)
        : _bconn(bconn), _bId(b_id), _cfg(*cfg), _ss7Sess(NULL)
        , logger(uselog), state(bilIdle), inap(NULL), postpaidBill(false)
{
    assert( bconn && cfg );
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Billing");
}


Billing::~Billing()
{
    if (inap)
        delete inap;
}

void Billing::Stop(CustomException * exc)
{
    bilMutex.Lock();
    smsc_log_debug(logger, "Billing[%u.%u]: Stopping%s%s",
                   _bconn->bConnId(), _bId,
                   exc ? ", reason " : "", exc ? exc->what() : "");
    state = Billing::bilAborted;

    if (inap) { //todo: send U_ABORT to SCF and close TCAP dialog
        delete inap;
        inap = NULL;
    }
    bilMutex.Unlock();
    _bconn->billingDone(this);
}

void Billing::handleCommand(InmanCommand* cmd)
{
    unsigned short  cmdId = cmd->getObjectId();

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
                smsc_log_fatal(logger, "Billing[%u.%u]: %s",
                                _bconn->bConnId(), _bId, exc.what());
                goon = false;
            }
            if (!goon) {
                ChargeSmsResult res(InErrINprotocol, InProtocol_GeneralError,
                                    smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
                res.setDialogId(_bId);
                _bconn->sendCmd(&res, false);
                _bconn->billingDone(this);
            } else
                onChargeSms(static_cast<ChargeSms*>(cmd));
        } else {
            bilMutex.Unlock();
            smsc_log_error(logger, "Billing[%u.%u]: protocol error: cmd %u, state %u",
                           _bconn->bConnId(), _bId, (unsigned)cmdId, state);
            _bconn->billingDone(this);
        }
    } break;

    case Billing::bilProcessed: {
        if (cmdId == DELIVERY_SMS_RESULT_TAG) {
            state = Billing::bilApproved;
            bilMutex.Unlock();
            bool goon = true;
            //complete the command deserialization
            try { cmd->loadDataBuf(); }
            catch (SerializerException & exc) {
                smsc_log_fatal(logger, "Billing[%u.%u]: %s",
                                _bconn->bConnId(), _bId, exc.what());
                goon = false;
            }
            if (goon)
                onDeliverySmsResult(static_cast<DeliverySmsResult*>(cmd));
            else
                Stop(NULL);
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

Session * Billing::activateSSN(void)
{
    smsc_log_debug(logger, "Billing[%u.%u]: Searching for TCAP session [SSN=%u] ..",
                    _bconn->bConnId(), _bId, _cfg.ssn);

    TCAPDispatcher *disp = TCAPDispatcher::getInstance();

    if ((_ss7Sess = disp->findSession(_cfg.ssn)) != NULL) {
        if (disp->getState() != TCAPDispatcher::ss7LISTEN) {
            if (!disp->reconnect(TCAPDispatcher::ss7LISTEN))
                _ss7Sess = NULL;
        }
        return _ss7Sess;
    }
    if (disp->getState() != TCAPDispatcher::ss7LISTEN) {
        if (!disp->reconnect(TCAPDispatcher::ss7LISTEN))
            return (_ss7Sess = NULL);
        _ss7Sess = disp->openSession(_cfg.ssn, _cfg.ssf_addr, _cfg.scf_addr);
    }

    return _ss7Sess;
}
/* -------------------------------------------------------------------------- *
 * SSF interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onChargeSms(ChargeSms* sms)
{
    smsc_log_debug(logger, "Billing[%u.%u]: Call.Adr <%s>, Dest.Adr <%s>",
                    _bconn->bConnId(), _bId,
                    sms->getCallingPartyNumber().c_str(),
                    sms->getDestinationSubscriberNumber().c_str());

    sms->export2CDR(cdr);

    if ( (_cfg.billMode == smsc::inman::BILL_ALL)
        || ((_cfg.billMode == smsc::inman::BILL_USSD)
            && (cdr._bearer == CDRRecord::dpUSSD))
        || ((_cfg.billMode == smsc::inman::BILL_SMS)
            && (cdr._bearer == CDRRecord::dpSMS)) ) {
        smsc_log_debug(logger, "Billing[%u.%u]: initiating billing via SCF",
                       _bconn->bConnId(), _bId);

        if (!activateSSN()) {
            smsc_log_error(logger, "Billing[%u.%u]: TCAP Session is not available",
                           _bconn->bConnId(), _bId);
            postpaidBill = true;
        }
    } else
        postpaidBill = true;

    if (!postpaidBill) {
        try { //Initiate CAP3 dialog
            inap = new Inap(_ss7Sess, this, _cfg.capTimeout, logger); //initialize TCAP dialog
            assert(inap);

            InitialDPSMSArg arg(smsc::inman::comp::DeliveryMode_Originating, _cfg.serviceKey);

            arg.setDestinationSubscriberNumber(sms->getDestinationSubscriberNumber().c_str());
            arg.setCallingPartyNumber(sms->getCallingPartyNumber().c_str());
            arg.setIMSI(sms->getCallingIMSI().c_str());
            arg.setLocationInformationMSC( sms->getLocationInformationMSC().c_str());
            arg.setSMSCAddress(sms->getSMSCAddress().c_str());
            arg.setTimeAndTimezone(sms->getSubmitTimeTZ());
            arg.setTPShortMessageSpecificInfo(sms->getTPShortMessageSpecificInfo());
            arg.setTPValidityPeriod(sms->getTPValidityPeriod(), smsc::inman::comp::tp_vp_relative);
            arg.setTPProtocolIdentifier(sms->getTPProtocolIdentifier());
            arg.setTPDataCodingScheme(sms->getTPDataCodingScheme());

            smsc_log_debug(logger, "Billing[%u.%u]: SSF --> SCF InitialDPSMS",
                            _bconn->bConnId(), _bId);
            state = Billing::bilInited;
            inap->initialDPSMS(&arg); //begins TCAP dialog
        } catch (std::exception& exc) {
            smsc_log_error(logger, "Billing[%u.%u]: %s",
                           _bconn->bConnId(), _bId, exc.what());
            postpaidBill = true;
        }
    }
    if (postpaidBill) { //do not ask IN platform, just create CDR
        smsc_log_debug(logger, "Billing[%u.%u]: initiated billing via CDR",
                        _bconn->bConnId(), _bId);
        onContinueSMS();
    }
}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
    EventTypeSMS_e eventType = smsRes->GetValue() ? EventTypeSMS_o_smsFailure :
                                                    EventTypeSMS_o_smsSubmission;

    smsc_log_debug(logger, "Billing[%u.%u]: DELIVERY_%s (code: %u)",
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
            smsc::inman::comp::EventReportSMSArg    report(eventType, MessageType_notification);
            inap->eventReportSMS(&report);
        } catch (std::exception & exc) {
            postpaidBill = true;
            smsc_log_error(logger, "Billing[%u.%u]: %s", exc.what());
            delete inap;
            inap = NULL;
        }
    }
    state = Billing::bilComplete;
    _bconn->billingDone(this);
}

/* -------------------------------------------------------------------------- *
 * InmanHandler(SSF) interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onConnectSMS(ConnectSMSArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF ConnectSMS", 
                   _bconn->bConnId(), _bId);
}

void Billing::onContinueSMS(void)
{
    smsc_log_debug(logger, "Billing[%u.%u]: %s", _bconn->bConnId(), _bId,
                   postpaidBill ? "<-- CHARGING_POSSIBLE" : "SSF <-- SCF ContinueSMS");
    ChargeSmsResult res;
    res.setDialogId(_bId);

    if (_bconn->sendCmd(&res, true))
        state = Billing::bilProcessed;
    else      //TCP connect fatal failure
        Stop(_bconn->getConnectError());
}

#define POSTPAID_RPCause 41     //RP Cause: 'Temporary Failure'
void Billing::onReleaseSMS(ReleaseSMSArg* arg)
{
    smsc_log_debug(logger, "Billing[%u.%u]: SSF <-- SCF ReleaseSMS, RP cause: %u",
                   _bconn->bConnId(), _bId, (unsigned)arg->rPCause);

    postpaidBill = true;
    //check for RejectSMS causes:
    for (RPCList::iterator it = _cfg.rejectRPC.begin();
                            it != _cfg.rejectRPC.end(); it++) {
        if ((*it) == arg->rPCause) {
            postpaidBill = false;
            break;
        }
    }
    ChargeSmsResult res(InErrRPCause, (uint16_t)arg->rPCause, postpaidBill ?
                        smsc::inman::interaction::CHARGING_POSSIBLE : 
                        smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
    res.setDialogId(_bId);

    if (_bconn->sendCmd(&res, false)) {
    //NOTE: it's safe to free 'arg' here, TCAP dialog may be closed
        if (postpaidBill) {
            state = Billing::bilProcessed;
            smsc_log_debug(logger, "Billing[%u.%u]: switched to billing via CDR", 
                           _bconn->bConnId(), _bId);
            bilMutex.Lock();
            delete inap;
            inap = NULL;
            bilMutex.Unlock();
        } else {
            state = Billing::bilReleased;
            smsc_log_debug(logger, "Billing[%u.%u]: cancelling billing.",
                           _bconn->bConnId(), _bId);
            _bconn->billingDone(this);
        }
    } else      //TCP connect fatal failure
        Stop(_bconn->getConnectError());
}

//Called by Inap if CAP dialog with IN-platform is aborted.
//may be called if state is [bilInited .. bilApproved]
void Billing::onAbortSMS(unsigned char errCode, bool tcapLayer)
{
    bilMutex.Lock();
    smsc_log_error(logger, "Billing[%u.%u]: SSF <-- SCF Error, code: %u, layer %s",
                   _bconn->bConnId(), _bId,
                   (unsigned)errCode, tcapLayer ? "TCAP" : "CAP3");

    switch (state) {
    case Billing::bilInited: {
        //continue dialog with MSC despite of CAP error, switch to CDR mode
        ChargeSmsResult res(tcapLayer ? InErrTCAP : InErrCAP3, (uint16_t)errCode,
                            smsc::inman::interaction::CHARGING_POSSIBLE);
        res.setDialogId(_bId);

        if (_bconn->sendCmd(&res, true) < 0) {
            bilMutex.Unlock();
            Stop(_bconn->getConnectError());
        } else {
            state = Billing::bilProcessed;
            postpaidBill = true;
            smsc_log_warn(logger, "Billing[%u.%u]: switched to billing via CDR (because of SCF error).",
                          _bconn->bConnId(), _bId);
        }
    } break;

    case Billing::bilReleased: {
        //dialog with MSC already cancelled, just release CAP dialog
    } break;
    case Billing::bilProcessed:
        //dialog with MSC is in process, release CAP dialog, switch to CDR mode
    case Billing::bilApproved:
        //dialog with MSC finished, release CAP dialog, switch to CDR mode
    default:
        postpaidBill = true;
        smsc_log_warn(logger, "Billing[%u.%u]: switched to billing via CDR (because of SCF error).",
                      _bconn->bConnId(), _bId);
    }
    delete inap;
    inap = NULL;
    bilMutex.Unlock();
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

