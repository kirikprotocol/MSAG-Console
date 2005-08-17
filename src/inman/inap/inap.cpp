static char const ident[] = "$Id$";
#include "inap.hpp"

namespace smsc {
namespace inman {
namespace inap {

InapProtocol::InapProtocol(TcapDialog* dialog) : Protocol( dialog )
{
}

InapProtocol::~InapProtocol()
{
}

void InapProtocol::initialDPSMS(InitialDPSMSArg* arg)
{
  TcapOperation* op = dialog->createOperation();
  op->setOperation( arg );
  op->invoke();
}

void InapProtocol::eventReportSMS(EventReportSMSArg* arg)
{
  TcapOperation* op = dialog->createOperation();
  op->setOperation( arg );
  op->invoke();
}

}
}
}
