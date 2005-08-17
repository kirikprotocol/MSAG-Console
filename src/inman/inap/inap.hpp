#ident "$Id$"
// ������ (�࠭�����) IN

#ifndef __SMSC_INMAN_INAP_INAP__
#define __SMSC_INMAN_INAP_INAP__

#include <map>

#include "ss7cp.h"
#include "dialog.hpp"
#include "protocol.hpp"

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
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
};

class InapProtocol : public Protocol, SCF
{

  public:
    InapProtocol(TcapDialog* dialog);
    virtual ~InapProtocol();

    void initialDPSMS(InitialDPSMSArg* arg);
    void eventReportSMS(EventReportSMSArg* arg);
};

class InapListener : public ProtocolListener, SSF
{
    virtual void connectSMS(ConnectSMSArg* arg);
    virtual void continueSMS();
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void releaseSMS(ReleaseSMSArg* arg);
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void resetTimerSMS(ResetTimerSMSArg* arg);
};

class InapFactory : public ProtocolFactory
{
	public:
		InapProtocol* createProtocol(TcapDialog*);
};

}
}
}

#endif
