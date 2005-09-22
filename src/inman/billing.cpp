static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "billing.hpp"
#include "inman/inap/inap.hpp"
#include "inman/comp/comps.hpp"

using smsc::inman::interaction::ChargeSmsResult;

using std::auto_ptr;
using std::runtime_error;

//#define TEST_DATA

namespace smsc  {
namespace inman {

Billing::Billing(Session* pSession, Connect* conn)
		: logger(Logger::getInstance("smsc.inman.inap.Billing"))
		, session( pSession )
		, connect( conn )
{
	assert( session );
	dialog = session->openDialog( 0 );
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

	smsc_log_debug( logger, "--> InitialDPSMS" );

	InitialDPSMSArg arg( smsc::inman::comp::DeliveryMode_Originating );

#ifdef TEST_DATA
	arg.setDestinationSubscriberNumber( ".1.1.79139859489" ); // missing for MT
	arg.setCallingPartyNumber( ".1.1.79139343290" );
	arg.setIMSI( "250013900405871" );

	//Address vlr( ".1.1.79139860001" );

	arg.setLocationInformationMSC( ".1.1.79139860001" );
	arg.setSMSCAddress(".1.1.79029869990");

	time_t tm;
	time( &tm );

	arg.setTimeAndTimezone( tm );
	arg.setTPShortMessageSpecificInfo( 0x11 );
	arg.setTPValidityPeriod( 60*5 , smsc::inman::comp::tp_vp_relative );
	arg.setTPProtocolIdentifier( 0x00 );
	arg.setTPDataCodingScheme( 0x08 );
#else

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
#endif
	inap->initialDPSMS( &arg );
	dialog->beginDialog();
}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
	assert( smsRes );

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

	smsc_log_debug( logger, "--> EventReportSMS( EventType: %s (0x%X), MessageType: 0x%X )", 
								szType, eventType, messageType );

	smsc::inman::comp::EventReportSMSArg report( eventType, messageType );

	inap->eventReportSMS( &report );
	dialog->continueDialog();
}

void Billing::connectSMS(ConnectSMSArg* arg)
{
	smsc_log_debug( logger, "<-- ConnectSMS" );
}

void Billing::continueSMS()
{
	assert( connect );
	smsc_log_debug( logger, "<-- ContinueSMS" );
	ChargeSmsResult res( smsc::inman::interaction::CHARGING_POSSIBLE );
	connect->send( &res );
}

void Billing::releaseSMS(ReleaseSMSArg* arg)
{
	assert( connect );
	smsc_log_debug( logger, "<-- ReleaseSMS" );
	ChargeSmsResult res( smsc::inman::interaction::CHARGING_NOT_POSSIBLE );
	connect->send( &res );
}

void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
	smsc_log_debug( logger, "<-- FurnishChargingInformationSMS" );
}

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
	assert( arg );

	smsc_log_debug( logger, "<-- RequestReportSMSEvent" );

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
	smsc_log_debug( logger, "<-- ResetTimerSMS" );
}

}
}
