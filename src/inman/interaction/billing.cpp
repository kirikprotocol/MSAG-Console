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
	context->start();
}

void Billing::sendIntialDPSMS()
{
	InitialDPSMSArg * pArg = new InitialDPSMSArg();
	//TODO: Fill agrument
	inap->initialDPSMS( pArg );
}

void Billing::connectSMS(ConnectSMSArg* arg)
{
}

void Billing::continueSMS()
{
}

void Billing::furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg)
{
}

void Billing::releaseSMS(ReleaseSMSArg* arg)
{
}

void Billing::requestReportSMSEvent(RequestReportSMSEventArg* arg)
{
}

void Billing::resetTimerSMS(ResetTimerSMSArg* arg)
{
}

}
}
}
