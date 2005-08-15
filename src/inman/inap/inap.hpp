#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_INAP__
#define __SMSC_INMAN_INAP_INAP__

#include <map>

#include "ss7cp.h"
#include "dialog.hpp"
#include "inap_sm.h"
#include "inman/comp/comps.hpp"
namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::FurnishChargingInformationSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;
using smsc::inman::comp::InitialDPSMSArg;

class SSF
{
  public:
    virtual void connectSMS(ConnectSMSArg* arg) = 0;
    virtual void continueSMS() = 0;
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg) = 0;
    virtual void releaseSMS(ReleaseSMSArg* arg) = 0;
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg) = 0;
    virtual void resetTimerSMS(ResetTimerSMSArg* arg) = 0;
};
class SCF
{
  public:
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
};

class INAP : public DialogListener
{

  public:
    INAP(Dialog* dialog);
    virtual ~INAP();

  public: // DialogListener impl
    virtual void start();
    virtual void invoke(const TcapOperation&);
    virtual void invokeSuccessed(const TcapOperation&);
    virtual void invokeFailed(const TcapOperation&);

  public: // Called from FSM
    virtual void sendInitialDP();
    virtual void sendEventReport();

  protected:
    Dialog*     dialog;
    INAPContext * context;
};

}
}
}

#endif
