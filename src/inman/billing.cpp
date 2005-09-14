static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "billing.hpp"
#include "inman/inap/inap.hpp"
#include "inman/comp/comps.hpp"

using std::auto_ptr;
using std::runtime_error;
using smsc::inman::comp::EventTypeSMS_e;
using smsc::inman::comp::messageType_e;

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
		arg.setDestinationSubscriberNumber(".0.1.131133"); // missing for MT
	}
	else
	{
		arg.setCalledPartyNumber(".0.1.131133"); // missing for MO
	}

	arg.setCallingPartyNumber(".1.1.79139163393");

	const char imsi[] = { 0x52, 0x00, 0x31, 0x09, 0x31, 0x88, 0x87, 0xF0, 0x00 };
	arg.setIMSI( imsi );

	arg.setSMSCAddress(".1.1.79029869990");

/*	struct tm timeValue;
	timeValue.tm_sec  =   26;
	timeValue.tm_min  =   03;
	timeValue.tm_hour =   15;
	timeValue.tm_mday =   22;
	timeValue.tm_mon  =   04   - 1;
	timeValue.tm_year =   2005 - 1900;
	timeValue.tm_wday = 0;
	timeValue.tm_yday = 0;
	timeValue.tm_isdst= -1;
	time_t tm = mktime( &timeValue );
	assert( tm > 0 );
	*/
	time_t tm = time( &tm );

	arg.setTimeAndTimezone( tm );

	arg.setTPShortMessageSpecificInfo( 0x11 );
	arg.setTPProtocolIdentifier( 0x00 );
	arg.setTPDataCodingScheme( 0x08 );
//	arg.setTPValidityPeriod(time_t vpVal, enum TP_VP_format fmt);
//	arg.setlocationInformationMSC(const Address& addr);

	inap->initialDPSMS( &arg );
	dialog->beginDialog();
}

void Billing::eventReportSMS()
{
	if( !inap ) throw runtime_error("Dialog closed");

	EventTypeSMS_e eventType;
	messageType_e  messageType = smsc::inman::comp::MessageType_notification;

	if( mode == smsc::inman::comp::DeliveryMode_Originating )
	{
		eventType = smsc::inman::comp::EventTypeSMS_o_smsSubmission;
	}
	else // DeliveryMode_Terminating
	{
		eventType = smsc::inman::comp::EventTypeSMS_t_smsDelivery;
	}

	smsc_log_debug( logger, "--> EventReportSMS( EventType: 0x%X, MessageType: 0x%X )", 
								eventType, messageType );

	EventReportSMSArg report( eventType, messageType );

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
	smsc_log_debug( logger, "--> Dialog end" );
	notify1( &BillingListener::onBillingFinished, this );
}

}
}
