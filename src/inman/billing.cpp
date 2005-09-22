static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "billing.hpp"
#include "inman/inap/inap.hpp"
#include "inman/comp/comps.hpp"

using std::auto_ptr;
using std::runtime_error;

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
	smsc_log_debug(logger, "ChargeSms command received");

	assert( inap );

	smsc_log_debug( logger, "--> InitialDPSMS" );

	InitialDPSMSArg arg( smsc::inman::comp::DeliveryMode_Originating );
	
	arg.setDestinationSubscriberNumber( ".1.1.79139859489" ); // missing for MT
	arg.setCallingPartyNumber( ".1.1.79139343290" );
	arg.setIMSI( "250013900405871" );

	Address vlr( ".1.1.79139860001" );

	arg.setlocationInformationMSC( vlr );
	arg.setSMSCAddress(".1.1.79029869990");

	time_t tm;
	time( &tm );

	arg.setTimeAndTimezone( tm );
	arg.setTPShortMessageSpecificInfo( 0x11 );
	arg.setTPValidityPeriod( 60*5 , smsc::inman::comp::tp_vp_relative );
	arg.setTPProtocolIdentifier( 0x00 );
	arg.setTPDataCodingScheme( 0x08 );

	inap->initialDPSMS( &arg );
	dialog->beginDialog();
}

void Billing::onDeliverySmsResult(DeliverySmsResult* smsRes)
{
	assert( smsRes );
	smsc_log_debug(logger, "DeliverySmsResult command received");

	messageType_e  messageType = MessageType_notification;
	EventTypeSMS_e eventType   = EventTypeSMS_o_smsSubmission;

	// DeliveryMode_Terminating: eventType = EventTypeSMS_t_smsDelivery;

	smsc_log_debug( logger, "--> EventReportSMS( EventType: 0x%X, MessageType: 0x%X )", 
								eventType, messageType );

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
	smsc_log_debug( logger, "<-- ContinueSMS" );
}

void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
	smsc_log_debug( logger, "<-- FurnishChargingInformationSMS" );
}

void Billing::releaseSMS(ReleaseSMSArg* arg)
{
	smsc_log_debug( logger, "<-- ReleaseSMS" );
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
