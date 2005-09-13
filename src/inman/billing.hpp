#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_BILLING__
#define __SMSC_INMAN_INTERACTION_BILLING__

#include "inman/comp/comps.hpp"
using smsc::inman::comp::RequestReportSMSEventArg;
#include "billing_sm.h"

#include "inman/inap/inap.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/connect.hpp"
#include "inman/common/observable.hpp"

using smsc::inman::inap::Inap;
using smsc::inman::inap::Session;
using smsc::inman::inap::TcapDialog;
using smsc::inman::inap::SSF;

namespace smsc  	  {
namespace inman 	  {

class Billing : public SSF
{
public:

	Billing(Session* pSession);
	virtual ~Billing();

	virtual void initialDPSMS();
	virtual void eventReportSMS();

    virtual void connectSMS(ConnectSMSArg* arg);
    virtual void continueSMS();
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void releaseSMS(ReleaseSMSArg* arg);
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void resetTimerSMS(ResetTimerSMSArg* arg);
	virtual void endDialog();

protected:	
	Session* 	session;
	TcapDialog*	dialog;
	Inap*	 	inap;
    Logger*	 	logger;   	

};

}
}



#endif
