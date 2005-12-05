static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "billing.hpp"
#include "service.hpp"

using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::CHARGE_SMS_TAG;
using smsc::inman::interaction::CHARGE_SMS_RESULT_TAG;
using smsc::inman::interaction::DELIVERY_SMS_RESULT_TAG;

using std::auto_ptr;
using std::runtime_error;

namespace smsc  {
namespace inman {

Billing::Billing(Service* serv, unsigned int bid, Session* pSession,
                 Connect* conn, BILL_MODE bMode, USHORT_T capTimeout/* = 0*/,
                 USHORT_T tcpTimeout/* = 0*/, Logger * uselog/* = NULL*/)
        : service(serv), id(bid), logger(uselog)
        , session(pSession), connect(conn), state(bilIdle)
        , inap(NULL), postpaidBill(false)
        , billMode(bMode), _capTimeout(capTimeout)
{
    assert( connect );
    assert( session );
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Billing");
}


Billing::~Billing()
{
    if (inap)
        delete inap;
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
            onChargeSms(static_cast<ChargeSms*>(cmd));
        } else {
            bilMutex.Unlock();
            smsc_log_error(logger, "Billing[%u]: protocol error: cmd %u, state %u",
                           id, (unsigned)cmdId, state);
            service->billingFinished(this);
        }
    } break;

    case Billing::bilProcessed: {
        if (cmdId == DELIVERY_SMS_RESULT_TAG) {
            state = Billing::bilApproved;
            bilMutex.Unlock();
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
        smsc_log_error(logger, "Billing[%u]: protocol error: cmd %u, state %u",
                       id, (unsigned)cmdId, state);
    } /* eosw */
}

//retuns false if CDR was not complete
bool Billing::BillComplete(void) const
{
    return ((state == Billing::bilComplete) && cdr._finalized) ? true : false;
}

const CDRRecord & Billing::getCDRRecord(void) const
{
    return cdr;
}

/* -------------------------------------------------------------------------- *
 * SSF interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onChargeSms(ChargeSms* sms)
{
    sms->export2CDR(cdr);

    if ( (billMode == smsc::inman::BILL_ALL)
        || ((billMode == smsc::inman::BILL_USSD)
            && (cdr._bearer == CDRRecord::dpUSSD))
        || ((billMode == smsc::inman::BILL_SMS)
            && (cdr._bearer == CDRRecord::dpSMS)) ) {
        smsc_log_debug( logger, "SSF initiated billing via SCF");

        inap = new Inap(session, this, _capTimeout, logger); //initialize TCAP dialog
        assert(inap);

        InitialDPSMSArg arg(smsc::inman::comp::DeliveryMode_Originating);

        arg.setDestinationSubscriberNumber(sms->getDestinationSubscriberNumber().c_str()); // missing for MT
        arg.setCallingPartyNumber(sms->getCallingPartyNumber().c_str());
        arg.setIMSI(sms->getCallingIMSI().c_str());
        arg.setLocationInformationMSC( sms->getLocationInformationMSC().c_str());
        arg.setSMSCAddress(sms->getSMSCAddress().c_str());
        arg.setTimeAndTimezone(sms->getSubmitTimeTZ());
        arg.setTPShortMessageSpecificInfo(sms->getTPShortMessageSpecificInfo());
        arg.setTPValidityPeriod(sms->getTPValidityPeriod(), smsc::inman::comp::tp_vp_relative);
        arg.setTPProtocolIdentifier(sms->getTPProtocolIdentifier());
        arg.setTPDataCodingScheme(sms->getTPDataCodingScheme());

        smsc_log_debug( logger, "SSF --> SCF InitialDPSMS" );
        state = Billing::bilInited;
        inap->initialDPSMS(&arg); //begins TCAP dialog
    } else {
        //do not ask IN platform, just create CDR
        smsc_log_debug(logger, "SSF initiated billing via CDR");
        continueSMS();
    }
}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
    messageType_e  messageType = MessageType_notification;
    EventTypeSMS_e eventType = (smsRes->GetValue() == smsc::inman::interaction::DELIVERY_SUCCESSED) ? 
                                EventTypeSMS_o_smsSubmission : EventTypeSMS_o_smsFailure;

    smsc_log_debug(logger, "SSF --> SCF EventReportSMS( EventType: DELIVERY_%s (0x%X), MessageType: 0x%X )",
        (eventType == EventTypeSMS_o_smsFailure) ? "FAILED" : "SUCCEEDED", eventType, messageType);

    smsRes->export2CDR(cdr);
    if (inap) { //continue TCAP dialog if it's still active
        smsc::inman::comp::EventReportSMSArg    report(eventType, messageType);
        inap->eventReportSMS(&report);
    }
    state = Billing::bilComplete;
    service->billingFinished(this);
}

/* -------------------------------------------------------------------------- *
 * InmanHandler(SCF) interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::connectSMS(ConnectSMSArg* arg)
{
    smsc_log_debug(logger, "SSF <-- SCF ConnectSMS");
}

void Billing::continueSMS()
{
    smsc_log_debug(logger, "SSF <-- SCF ContinueSMS");
    ChargeSmsResult res;
    res.setDialogId(id);
    connect->send(&res);
    state = Billing::bilProcessed;
}

#define POSTPAID_RPCause 41     //RP Cause: 'Temporary Failure'
void Billing::releaseSMS(ReleaseSMSArg* arg)
{
    //NOTE: For postpaid abonent IN-platform returns RP Cause: 'Temporary Failure'
    postpaidBill = (arg->rPCause == POSTPAID_RPCause) ? true : false;

    smsc_log_debug(logger, "SSF <-- SCF ReleaseSMS, RP cause: %u", (unsigned)arg->rPCause);
    
    ChargeSmsResult res(InErrRPCause, (uint16_t)arg->rPCause, postpaidBill ?
                        smsc::inman::interaction::CHARGING_POSSIBLE : 
                        smsc::inman::interaction::CHARGING_NOT_POSSIBLE);
    res.setDialogId(id);
    connect->send(&res);
    if (postpaidBill) {
        state = Billing::bilProcessed;
        smsc_log_debug(logger, "SSF switched to billing via CDR");
    } else {
        state = Billing::bilReleased;
        smsc_log_debug(logger, "SSF cancels billing");
        service->billingFinished(this);
    }
}

//Called by Inap if CAP dialog with IN-platform is aborted.
//may be called if state is [bilInited .. bilApproved]
void Billing::abortSMS(unsigned char errCode, bool tcapLayer)
{
    bilMutex.Lock();
    smsc_log_error(logger, "SSF <-- SCF System Error, code: %u, layer %s",
                   (unsigned)errCode, tcapLayer ? "TCAP" : "CAP3");

    switch (state) {
    case Billing::bilInited: {
        //continue dialog with MSC despite of CAP error, switch to CDR mode
        ChargeSmsResult res(tcapLayer ? InErrTCAP : InErrCAP3, (uint16_t)errCode,
                            smsc::inman::interaction::CHARGING_POSSIBLE);
        res.setDialogId(id);
        connect->send(&res);
        state = Billing::bilProcessed;
        postpaidBill = true;
        smsc_log_warn(logger, "SSF switched to billing via CDR because of CAP error");
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
        smsc_log_warn(logger, "SSF switched to billing via CDR because of CAP error");
    }
    delete inap;
    inap = NULL;
    bilMutex.Unlock();
}


void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
    smsc_log_debug( logger, "SSF <-- SCF FurnishChargingInformationSMS" );
}

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
    smsc_log_debug(logger, "SSF <-- SCF RequestReportSMSEvent");

    const RequestReportSMSEventArg::SMSEventVector& dps = arg->getSMSEvents();

    RequestReportSMSEventArg::SMSEventVector::const_iterator it;

    for( it = dps.begin(); it != dps.end(); it++ ) {
        RequestReportSMSEventArg::SMSEvent dp = *it;
        smsc_log_debug(logger, "SSF Detection point (Event type: 0x%X, Monitor mode: 0x%X)",
                        dp.event, dp.monitorType);
    }
}

void Billing::resetTimerSMS(ResetTimerSMSArg* arg)
{
    smsc_log_debug( logger, "SSF <-- SCF ResetTimerSMS" );
}

} //inman
} //smsc

