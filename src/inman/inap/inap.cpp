static char const ident[] = "$Id$";
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
}

Inap::~Inap()
{
}

void Inap::invoke(Invoke* op)
{
}

void Inap::initialDPSMS(InitialDPSMSArg* arg)
{
	 Invoke* op = dialog->invoke( InapOpCode::InitialDPSMS );
	 op->setParam( arg );
	 op->send( dialog );
}

void Inap::eventReportSMS(EventReportSMSArg* arg)
{
  	Invoke* op = dialog->invoke( InapOpCode::EventReportSMS );
	op->setParam( arg );
	op->send( dialog );
}

}
}
}
