static char const ident[] = "$Id$";
#include <assert.h>
#include <memory>

#include "billing.hpp"
#include "inman/inap/inap.hpp"

using std::auto_ptr;

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
		inap->removeListener( this );
		delete inap;
		// dialog must be closed at this time 
	}

	void Billing::initialDPSMS()
	{
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

void Billing::connectSMS(ConnectSMSArg* arg)
    {
		smsc_log_debug( logger, "ConnectSMS" );
    }

void Billing::continueSMS()
    {
		smsc_log_debug( logger, "ContinueSMS" );
    }

void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
    {
		smsc_log_debug( logger, "FurnishChargingInformationSMS" );
    }

void Billing::releaseSMS(ReleaseSMSArg* arg)
    {
		smsc_log_debug( logger, "ReleaseSMS" );
    }

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
	smsc_log_debug( logger, "RequestReportSMSEvent" );
	assert( arg );
	const RequestReportSMSEventArg::SMSEventVector& dps = arg->getSMSEvents();
	RequestReportSMSEventArg::SMSEventVector::const_iterator it;
	for( it = dps.begin(); it != dps.end(); it++ )
	{
		RequestReportSMSEventArg::SMSEvent dp = *it;
		smsc_log_debug( logger, "Event type: 0x%X, Monitor type: 0x%X", dp.event, dp.monitorType );
	}
	
}

void Billing::resetTimerSMS(ResetTimerSMSArg* arg)
    {
		smsc_log_debug( logger, "ResetTimerSMS" );
    }


}
}
