static char const ident[] = "$Id$";
#include <assert.h>

#include "billing.hpp"
#include "inman/inap/inap.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

Billing::Billing(Inap* prot) : inap( prot )
{
	context = new BillingContext( *this );
	assert( inap );
}

Billing::~Billing()
{
	delete context;
}

void Billing::start()
{
	InitialDPSMSArg * pArg = new InitialDPSMSArg();

	pArg->setDestinationSubscriberNumber();
	pArg->setCallingPartyNumber();
	pArg->setMode();
	pArg->setIMSI();
	pArg->setlocationInformationMSC();
	pArg->setSMSCAddress();
	pArg->setTimeAndTimezone();
	pArg->setTPShortMessageSpecificInfo();
	pArg->setTPProtocolIdentifier();
	pArg->setTPDataCodingScheme();
	pArg->setTPValidityPeriod();

	inap->initialDPSMS( pArg );

	context->start();
}

void Billing::connectSMS(ConnectSMSArg* arg)
{
	context->connectSMS();
}

void Billing::continueSMS()
{
	context->continueSMS();
}

void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
	context->furnishChargingInformationSMS();
}

void Billing::releaseSMS(ReleaseSMSArg* arg)
{
	context->releaseSMS();
}

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
	context->requestReportSMSEvent( arg );
}

void Billing::resetTimerSMS(ResetTimerSMSArg* arg)
{
	context->resetTimerSMS();
}


void Billing::notifyFailure()
{
}

void Billing::notifySuccess()
{
}

void Billing::armDetectionPoints(RequestReportSMSEventArg* data)
{
}

void Billing::sendReportSMSEvent()
{
}

}
}
}
