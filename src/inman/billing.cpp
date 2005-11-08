static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "billing.hpp"
#include "inman/inap/inap.hpp"
#include "inman/comp/comps.hpp"
#include "service.hpp"
#include "inman/comp/acdefs.hpp"

using smsc::inman::interaction::ChargeSmsResult;

using std::auto_ptr;
using std::runtime_error;

namespace smsc  {
namespace inman {

Billing::Billing(Service* serv, int bid, Session* pSession, Connect* conn)
		: service( serv )
		, logger(Logger::getInstance("smsc.inman.inap.Billing"))
		, id( bid )
		, session( pSession )
		, connect( conn )
{
	assert( session );
	dialog = session->openDialog(id_ac_cap3_sms_AC);
	assert( dialog );
	inap = new Inap( dialog );
	assert( inap );
	inap->addListener( this );
}

Billing::~Billing()
{
	inap->removeListener( this );
	delete inap;
	assert( session );
	session->closeDialog( dialog );
	delete dialog;
}

void Billing::onChargeSms(ChargeSms* sms)
{
	assert( sms );
	assert( inap );
	assert(id == sms->getDialogId());

	smsc_log_debug( logger, "SSF --> SCF InitialDPSMS" );

	InitialDPSMSArg arg( smsc::inman::comp::DeliveryMode_Originating );

	arg.setDestinationSubscriberNumber( sms->getDestinationSubscriberNumber().c_str() ); // missing for MT
	arg.setCallingPartyNumber( sms->getCallingPartyNumber().c_str() );
	arg.setIMSI( sms->getIMSI().c_str() );
	arg.setLocationInformationMSC( sms->getLocationInformationMSC().c_str() );
	arg.setSMSCAddress( sms->getSMSCAddress().c_str() );
	arg.setTimeAndTimezone( sms->getTimeAndTimezone() );
	arg.setTPShortMessageSpecificInfo( sms->getTPShortMessageSpecificInfo() );
	arg.setTPValidityPeriod( sms->getTPValidityPeriod() , smsc::inman::comp::tp_vp_relative );
	arg.setTPProtocolIdentifier( sms->getTPProtocolIdentifier() );
	arg.setTPDataCodingScheme( sms->getTPDataCodingScheme() );

	inap->initialDPSMS( &arg );
	dialog->beginDialog();
}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
	assert( smsRes );
	assert(id == smsRes->getDialogId());

	messageType_e  messageType = MessageType_notification;

	EventTypeSMS_e eventType;

	const char* szType;

	if( smsRes->GetValue() == smsc::inman::interaction::DELIVERY_SUCCESSED)
	{
		eventType = EventTypeSMS_o_smsSubmission;
		szType = "DELIVERY_SUCCESSED";
	}
	else
	{
		eventType = EventTypeSMS_o_smsFailure;
		szType = "DELIVERY_FAILED";
	}

	// DeliveryMode_Terminating: eventType = EventTypeSMS_t_smsDelivery;

	smsc_log_debug( logger, "SSF --> SCF EventReportSMS( EventType: %s (0x%X), MessageType: 0x%X )", 
								szType, eventType, messageType );

	smsc::inman::comp::EventReportSMSArg report( eventType, messageType );

	inap->eventReportSMS( &report );
	dialog->continueDialog();

	service->billingFinished( this );
}

void Billing::connectSMS(ConnectSMSArg* arg)
{
	smsc_log_debug( logger, "SSF <-- SCF ConnectSMS" );
}

void Billing::continueSMS()
{
	assert( connect );
	smsc_log_debug( logger, "SSF <-- SCF ContinueSMS" );
	ChargeSmsResult res( smsc::inman::interaction::CHARGING_POSSIBLE );
	res.setDialogId(id);
	connect->send( &res );
}

void Billing::releaseSMS(ReleaseSMSArg* arg)
{
	assert( connect );
	smsc_log_debug( logger, "SSF <-- SCF ReleaseSMS" );
	ChargeSmsResult res( smsc::inman::interaction::CHARGING_NOT_POSSIBLE );
	res.setDialogId(id);
	connect->send( &res );
	service->billingFinished( this );
}

void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
	smsc_log_debug( logger, "SSF <-- SCF FurnishChargingInformationSMS" );
}

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
	assert( arg );

	smsc_log_debug( logger, "SSF <-- SCF RequestReportSMSEvent" );

	const RequestReportSMSEventArg::SMSEventVector& dps = arg->getSMSEvents();

	RequestReportSMSEventArg::SMSEventVector::const_iterator it;

	for( it = dps.begin(); it != dps.end(); it++ )
	{
		RequestReportSMSEventArg::SMSEvent dp = *it;
		smsc_log_debug( logger, "Detection point (Event type: 0x%X, Monitor mode: 0x%X)", dp.event, dp.monitorType );
	}

}

void Billing::resetTimerSMS(ResetTimerSMSArg* arg)
{
	smsc_log_debug( logger, "SSF <-- SCF ResetTimerSMS" );
}

}
}
