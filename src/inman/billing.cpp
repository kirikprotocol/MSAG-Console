static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>
#include <stdexcept>

#include "billing.hpp"
#include "inman/inap/inap.hpp"
#include "inman/comp/comps.hpp"

using std::auto_ptr;
using std::runtime_error;
using smsc::inman::comp;

namespace smsc  {
namespace inman {

Billing::Billing(Session* pSession)
		: logger(Logger::getInstance("smsc.inman.inap.Billing"))
		, session( pSession )
{
	assert( session );
	dialog = session->openDialog( 0 ); // 0 = new dialog id
	assert( dialog );
	inap   = new Inap( dialog );
	assert( inap );
	inap->addListener( this );
}

Billing::~Billing()
{
	endDialog();
}

void Billing::initialDPSMS()
{
	if( !inap ) throw runtime_error("Dialog closed");

	smsc_log_debug( logger, "--> InitialDPSMS" );
	auto_ptr<InitialDPSMSArg> arg( new InitialDPSMSArg() );
	arg->setDestinationSubscriberNumber();
	arg->setCallingPartyNumber();
	arg->setMode();
	arg->setIMSI();
	arg->setlocationInformationMSC();
	arg->setSMSCAddress();
	arg->setTimeAndTimezone();
	arg->setTPShortMessageSpecificInfo();
	arg->setTPProtocolIdentifier();
	arg->setTPDataCodingScheme();
	arg->setTPValidityPeriod();

	inap->initialDPSMS( arg.get() );
	dialog->beginDialog();
}

void Billing::eventReportSMS()
{
	if( !inap ) throw runtime_error("Dialog closed");

	smsc_log_debug( logger, "--> EventReportSMS( EventType: 0x%X, MessageType: 0x%X )", 
			smsc::inman::comp::EventTypeSMS_o_smsSubmission,
			smsc::inman::comp::MessageType_notification	 );

	EventReportSMSArg report( smsc::inman::comp::EventTypeSMS_o_smsSubmission,
							  smsc::inman::comp::MessageType_notification	);
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
	smsc_log_debug( logger, "!!! End dialog" );

	if( inap )
	{
		inap->removeListener( this );
		delete inap;
		inap = 0;
	}

	dialog = 0;
}

}
}
