#ident "$Id$"

#ifndef __SMSC_INMAN_PAYMENT__
#define __SMSC_INMAN_PAYMENT__

#include "billing_sm.h"
#include "inman/inap/inap.hpp"
#include "inman/common/observable.hpp"

using smsc::inman::inap::Inap;
using smsc::inman::inap::SSF;

namespace smsc  	  {
namespace inman 	  {
namespace interaction {

class SMS
{
public:

	SMS()
	{
	}

	virtual ~SMS()
	{
	}
};

class BillingListener
{
public:
	virtual void successed() = 0;
	virtual void failed() 	 = 0;
};

class Billing : public SSF, public ObservableT< BillingListener >
{
  public:
	Billing(Inap*);
	virtual ~Billing();

	virtual void start();

	void sendIntialDPSMS();

  public: // SSF interface
    virtual void connectSMS(ConnectSMSArg* arg);
    virtual void continueSMS();
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void releaseSMS(ReleaseSMSArg* arg);
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void resetTimerSMS(ResetTimerSMSArg* arg);

  protected:
  	BillingContext* context;
  	Inap* 			inap;
};

}
}
}


#endif
