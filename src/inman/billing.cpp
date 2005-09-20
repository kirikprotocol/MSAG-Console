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

Billing::Billing(TcapDialog* pDialog, DeliveryMode_e md)
		: logger(Logger::getInstance("smsc.inman.inap.Billing"))
		, dialog( pDialog )
		, mode( md )
{
			
	assert( dialog );
	inap   = new Inap( dialog );
	assert( inap );
	inap->addListener( this );
}

Billing::~Billing()
{
	inap->removeListener( this );
	delete inap;
}

void Billing::initialDPSMS()
{
	if( !inap ) throw runtime_error("Dialog closed");

	smsc_log_debug( logger, "--> InitialDPSMS" );

	InitialDPSMSArg arg( mode );

	if( mode == smsc::inman::comp::DeliveryMode_Originating )
	{
		arg.setDestinationSubscriberNumber( ".1.1.79139859489" ); // missing for MT
	}
	else
	{
		arg.setCalledPartyNumber(".1.1.79139859489"); // missing for MO
	}

	//arg.setCallingPartyNumber(".1.1.79139163393");
	//arg.setIMSI( "250013901388780" );

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

void Billing::eventReportSMS()
{
	if( !inap ) throw runtime_error("Dialog closed");

	EventTypeSMS_e eventType;
	messageType_e  messageType = MessageType_notification;

	if( mode == smsc::inman::comp::DeliveryMode_Originating )
	{
		eventType = EventTypeSMS_o_smsSubmission;
	}
	else // DeliveryMode_Terminating
	{
		eventType = EventTypeSMS_t_smsDelivery;
	}

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
	eventReportSMS();
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

void Billing::endDialog()
{
	smsc_log_debug( logger, "<-- Dialog end" );
	notify1( &BillingListener::onBillingFinished, this );
}

}
}
