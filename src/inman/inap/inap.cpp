static char const ident[] = "$Id$";
#include "inap.hpp"
#include "inman/comp/comps.hpp"
#include "operations.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::InapOpCode;

InapProtocol::InapProtocol(TcapDialog* dialog) : Protocol( dialog )
{
}

InapProtocol::~InapProtocol()
{
}

void InapProtocol::initialDPSMS(InitialDPSMSArg* arg)
{
  TcapOperation* op = dialog->createOperation( InapOpCode::InitialDPSMS );
  op->setParam( arg );
  op->invoke();
}

void InapProtocol::eventReportSMS(EventReportSMSArg* arg)
{
  TcapOperation* op = dialog->createOperation( InapOpCode::EventReportSMS );
  op->setParam( arg );
  op->invoke();
}

}
}
}
