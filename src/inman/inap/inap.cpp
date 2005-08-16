static char const ident[] = "$Id$";
#include "inap.hpp"

namespace smsc {
namespace inman {
namespace inap {

INAP::INAP(Dialog* dlg) : dialog( dlg )
{
  context = new INAPContext( *this );
  dialog->addDialogListener( this );
}

INAP::~INAP()
{
  dialog->removeDialogListener( this );
  delete context;
}

void INAP::invoke(const TcapOperation&)
{
}

void INAP::invokeSuccessed(const TcapOperation& result)
{
  context->successed( result );
}

void INAP::invokeFailed(const TcapOperation& error)
{
  context->failed( error );
}
void INAP::start()
{
  context->start();
}
void INAP::sendInitialDP()
{
}

void INAP::sendEventReport()
{
}

void INAP::eventReportSMS(EventReportSMSArg* arg)
{
}
void INAP::initialDPSMS(InitialDPSMSArg* arg)
{
  TcapOperation* out = new TcapOperation();// may be allocate via dialogue
  //TcapOperation out = _request ("sayHello");
  out->write(arg);
  //TcapOperation in = _invoke (out);
  //result = in.read ();
}

}
}
}
