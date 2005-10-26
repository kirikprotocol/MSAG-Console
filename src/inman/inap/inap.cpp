static char const ident[] = "$Id$";
#include <assert.h>
#include "inman/inap/inap.hpp"
#include "inman/comp/comps.hpp"
#include "inman/inap/invoke.hpp"
#include "inman/inap/dialog.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::InapOpCode;

Inap::Inap(Dialog* dlg) : dialog( dlg ), logger(Logger::getInstance("smsc.inman.inap.Inap"))
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

    // ListenerList is defined as std::list<SSF*>, that keeps (Billing*) casted to (SSF*)
    switch(op->getOpcode()) {
	case InapOpCode::FurnishChargingInformationSMS: {
	    assert(op->getParam());
	    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
		SSF * ptr = *it;
		ptr->furnishChargingInformationSMS(static_cast<FurnishChargingInformationSMSArg*>
						   (op->getParam()));
	    }
	}   break;
	case InapOpCode::ConnectSMS: {
	    assert(op->getParam());
	    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
		SSF * ptr = *it;
		ptr->connectSMS(static_cast<ConnectSMSArg*>(op->getParam()) );
	    }
	}   break;
	case InapOpCode::RequestReportSMSEvent: {
	    assert(op->getParam());
	    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
		SSF * ptr = *it;
		ptr->requestReportSMSEvent(static_cast<RequestReportSMSEventArg*>(op->getParam()));
	    }
	}   break;
	case InapOpCode::ContinueSMS: {
	    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
		SSF * ptr = *it;
		ptr->continueSMS();
	    }
	}   break;
	case InapOpCode::ReleaseSMS: {
	    assert(op->getParam());
	    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
		SSF * ptr = *it;
		ptr->releaseSMS(static_cast<ReleaseSMSArg*>(op->getParam()));
	    }
	}   break;
	case InapOpCode::ResetTimerSMS: {
	    assert(op->getParam());
	    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
		SSF * ptr = *it;
		ptr->resetTimerSMS(static_cast<ResetTimerSMSArg*>(op->getParam()));
	    }
	}   break;
	default:;
    }
}

void Inap::initialDPSMS(InitialDPSMSArg* arg)
{
	 Invoke* op = dialog->invoke( InapOpCode::InitialDPSMS );
	 assert( op );
	 assert( arg );
	 op->setParam( arg );
	 op->send( dialog );
}

void Inap::eventReportSMS(EventReportSMSArg* arg)
{
  	Invoke* op = dialog->invoke( InapOpCode::EventReportSMS );
	assert( op );
	assert( arg );
	op->setParam( arg );
	op->send( dialog );
}

}
}
}
