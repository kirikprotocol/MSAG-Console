#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_INAP__
#define __SMSC_INMAN_INAP_INAP__

#include <map>

#include "ss7cp.h"
#include "dialog.hpp"

#include "inman/comp/comps.hpp"

using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::FurnishChargingInformationSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;
using smsc::inman::comp::InitialDPSMSArg;

namespace smsc {
namespace inman {
namespace inap {

class SSF
{
  public:
    virtual void connectSMS(ConnectSMSArg* arg) = 0;
    virtual void continueSMS() = 0;
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg) = 0;
    virtual void releaseSMS(ReleaseSMSArg* arg) = 0;
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg) = 0;
    virtual void resetTimerSMS(ResetTimerSMSArg* arg) = 0;
    virtual void endDialog() = 0;
};

class SCF
{
  public:
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
};

class Inap : public TcapDialogListener, public SCF, public ObservableT< SSF >
{
  public:
    Inap(TcapDialog* dialog);
    virtual ~Inap();
  public:

    void initialDPSMS(InitialDPSMSArg* arg);
    void eventReportSMS(EventReportSMSArg* arg);

   	virtual void onDialogInvoke( Invoke* op ); // TcapDialogListener
   	virtual void onDialogEnd(); // TcapDialogListener

   protected:

  	TcapDialog* dialog;
  	Logger*		logger;
};

}
}
}

#endif
