static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

//#include "inman/comp/acdefs.hpp"
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

Billing::Billing(Service* serv, unsigned int bid, Session* pSession, Connect* conn)
        : service( serv )
        , logger(Logger::getInstance("smsc.inman.inap.Billing"))
        , id( bid )
        , session( pSession )
        , connect( conn )
        , state (bilIdle)
        , inap(NULL)
        , billType(Billing::billPrepaid)
{
    assert( connect );
    assert( session );
}


Billing::~Billing()
{
    if (inap)
        delete inap;
}

void Billing::handleCommand(InmanCommand* cmd)
{
    unsigned short  cmdId;
    bool            accepted = false;

    bilMutex.Lock();
    switch (cmdId = cmd->getObjectId()) {
    case CHARGE_SMS_TAG: {  //TCP dialog init
        if (state == Billing::bilIdle) {
            state = Billing::bilStarted;
            bilMutex.Unlock();
            inap = new Inap(session, this); //initialize TCAP dialog 
            assert(inap);
            onChargeSms(static_cast<ChargeSms*>(cmd));
            accepted = true;
        }
    } break;

    case DELIVERY_SMS_RESULT_TAG: {
        if (state == Billing::bilProcessed) {
            state = Billing::bilApproved;
            bilMutex.Unlock();
            onDeliverySmsResult(static_cast<DeliverySmsResult*>(cmd));
            accepted = true;
        }
    } break;

    default:;
    } /* eosw */

    if (accepted == false) {
        smsc_log_error(logger, "SSF: protocol error: cmd %u, state %u",
                       (unsigned)cmdId, state);
        state = Billing::billAborted;
        bilMutex.Unlock();
        abortBilling(InErrINprotocol, InProtocol_GeneralError);
    }
}

//void Billing::abortBilling(unsigned int errCode)
void Billing::abortBilling(InmanErrorType errType, uint16_t errCode)
{
    ChargeSmsResult res(errType, errCode);
    res.setDialogId(id);
    connect->send(&res);
    service->billingFinished( this );
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

Billing::BillingType Billing::getBillingType(void) const
{
    return billType;
}
/* -------------------------------------------------------------------------- *
 * SSF interface implementation:
 * -------------------------------------------------------------------------- */
void Billing::onChargeSms(ChargeSms* sms)
{
    smsc_log_debug( logger, "SSF --> SCF InitialDPSMS" );

    sms->export2CDR(cdr);
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

    inap->initialDPSMS(&arg); //begins TCAP dialog
    state = Billing::bilInited;
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

#define POSTPAID_RPCause 41
void Billing::releaseSMS(ReleaseSMSArg* arg)
{
    smsc_log_debug(logger, "SSF <-- SCF ReleaseSMS, RP cause: %u", (unsigned)arg->rPCause);
    ChargeSmsResult res(InErrRPCause, (uint16_t)arg->rPCause);
    res.setDialogId(id);
    connect->send(&res);
    state = Billing::bilProcessed;
    //NOTE: For postpaid abonent IN-platform returns RP Cause: 'Temporary Failure'
    if (arg->rPCause == POSTPAID_RPCause)
        billType = Billing::billPostpaid;
}

void Billing::abortSMS(unsigned char errCode, bool tcapLayer)
{
    smsc_log_error(logger, "SSF <-- SCF System Error, code: %u, layer %s",
                   (unsigned)errCode, tcapLayer ? "TCAP" : "CAP3");
    //continue dialog with MSC despite of SS7 error, the CDR should be created.
    ChargeSmsResult res(tcapLayer ? InErrTCAP : InErrCAP3, (uint16_t)errCode,
                        smsc::inman::interaction::CHARGING_POSSIBLE);
    res.setDialogId(id);
    connect->send(&res);
    state = Billing::bilProcessed;
    //release TCAP dialog
    delete inap;
    inap = NULL;
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

