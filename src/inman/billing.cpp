static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "inman/comp/acdefs.hpp"
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
{
    assert( connect );
    assert( session );
    dialog = session->openDialog(id_ac_cap3_sms_AC);
    assert( dialog );
    inap = new Inap( dialog, this );
    assert( inap );
}


Billing::~Billing()
{
    delete inap;
    session->closeDialog( dialog );
    delete dialog;
}

void Billing::handleCommand(InmanCommand* cmd)
{
    unsigned short  cmdId;
    bool            accepted = false;

    switch (cmdId = cmd->getObjectId()) {
    case CHARGE_SMS_TAG: {
        if (state == Billing::bilIdle) {
            onChargeSms(static_cast<ChargeSms*>(cmd));
            accepted = true;
        }
    } break;

    case DELIVERY_SMS_RESULT_TAG: {
        if (state == Billing::bilProcessed) {
            onDeliverySmsResult(static_cast<DeliverySmsResult*>(cmd));
            accepted = true;
        }
    } break;

    default:;
    } /* eosw */

    if (accepted == false) {
        smsc_log_error(logger, "SSF: protocol error: cmd %u, state %u",
                       (unsigned)cmdId, state);
        abortBilling(INMAN_PROTOCOL_ERROR);
    }
}

void Billing::abortBilling(unsigned int errCode)
{
    ChargeSmsResult res(errCode);
    res.setDialogId(id);
    connect->send(&res);
    state = Billing::bilClosed;
    service->billingFinished( this );
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

    inap->initialDPSMS(&arg);
    dialog->beginDialog();
    state = Billing::bilInited;

}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
    messageType_e  messageType = MessageType_notification;
    EventTypeSMS_e eventType = (smsRes->GetValue() == smsc::inman::interaction::DELIVERY_SUCCESSED) ? 
                                EventTypeSMS_o_smsSubmission : EventTypeSMS_o_smsFailure;

    smsc_log_debug(logger, "SSF --> SCF EventReportSMS( EventType: DELIVERY_%s (0x%X), MessageType: 0x%X )",
        (eventType = EventTypeSMS_o_smsFailure) ? "FAILED" : "SUCCEEDED", eventType, messageType);

    smsRes->export2CDR(cdr);
    smsc::inman::comp::EventReportSMSArg    report(eventType, messageType);

    inap->eventReportSMS(&report);
    dialog->continueDialog();
    state = Billing::bilClosed;

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

void Billing::releaseSMS(ReleaseSMSArg* arg)
{
    smsc_log_debug(logger, "SSF <-- SCF ReleaseSMS, RP cause: %u", (unsigned)arg->rPCause);
    ChargeSmsResult res((uint32_t)arg->rPCause);
    res.setDialogId(id);
    connect->send(&res);
    state = Billing::bilProcessed;
}

void Billing::abortSMS(unsigned char errCode)
{
    smsc_log_error( logger, "SSF <-- SCF System Error, code: %u", (unsigned)errCode);
    abortBilling(INMAN_SCF_ERROR_BASE + errCode);
}


void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
    smsc_log_debug( logger, "SSF <-- SCF FurnishChargingInformationSMS" );
}

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
    assert( arg );
    smsc_log_debug(logger, "SSF <-- SCF RequestReportSMSEvent");

    const RequestReportSMSEventArg::SMSEventVector& dps = arg->getSMSEvents();

    RequestReportSMSEventArg::SMSEventVector::const_iterator it;

    for( it = dps.begin(); it != dps.end(); it++ ) {
        RequestReportSMSEventArg::SMSEvent dp = *it;
        smsc_log_debug(logger, "Detection point (Event type: 0x%X, Monitor mode: 0x%X)",
                        dp.event, dp.monitorType);
    }
}

void Billing::resetTimerSMS(ResetTimerSMSArg* arg)
{
    smsc_log_debug( logger, "SSF <-- SCF ResetTimerSMS" );
}

} //inman
} //smsc

