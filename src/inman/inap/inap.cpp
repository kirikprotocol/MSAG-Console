static char const ident[] = "$Id$";
#include <assert.h>
#include "inap.hpp"
#include "inman/comp/comps.hpp"
#include "invoke.hpp"
#include "dialog.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::InapOpCode;

Inap::Inap(TcapDialog* dlg) : dialog( dlg )
{
	assert( dialog );
	dialog->addListener( this );
}

Inap::~Inap()
{
	dialog->removeListener( this );
}

void Inap::onDialogInvoke( Invoke* op )
{
	assert( op );
	switch(op->getOpcode())
	{
		case InapOpCode::FurnishChargingInformationSMS:
			assert( op->getParam() );
			notify1( &SSF::furnishChargingInformationSMS, 
								static_cast<FurnishChargingInformationSMSArg*>(op->getParam()) );
			break;
		case InapOpCode::ConnectSMS:
			assert( op->getParam() );
			notify1( &SSF::connectSMS,
					static_cast<ConnectSMSArg*>(op->getParam()) );
			break;
		case InapOpCode::RequestReportSMSEvent:
			assert( op->getParam() );
			notify1( &SSF::requestReportSMSEvent, 
					static_cast<RequestReportSMSEventArg*>(op->getParam()) );
			break;
		case InapOpCode::ContinueSMS:
			notify0( &SSF::continueSMS );
			break;
		case InapOpCode::ReleaseSMS:
			assert( op->getParam() );
			notify1( &SSF::releaseSMS,
			static_cast<ReleaseSMSArg*>(op->getParam())  );
			break;
		case InapOpCode::ResetTimerSMS:
			assert( op->getParam() );
			notify1( &SSF::resetTimerSMS,
			static_cast<ResetTimerSMSArg*>(op->getParam())  );
			break;
	}
}

void Inap::initialDPSMS(InitialDPSMSArg* arg)
{
	 Invoke* op = dialog->invoke( InapOpCode::InitialDPSMS );
	 assert( op );
	 op->setParam( arg );
	 op->send( dialog );
}

void Inap::eventReportSMS(EventReportSMSArg* arg)
{
  	Invoke* op = dialog->invoke( InapOpCode::EventReportSMS );
	assert( op );
	op->setParam( arg );
	op->send( dialog );
}

}
}
}
