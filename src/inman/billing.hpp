#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

#include "common/observable.hpp"
#include "comp/comps.hpp"
#include "inap/inap.hpp"
#include "inap/session.hpp"
#include "inap/connect.hpp"
#include "interaction/messages.hpp"

#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"

using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;

using smsc::inman::comp::DeliveryMode_e;

using smsc::inman::inap::Inap;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSF;
using smsc::inman::interaction::InmanHandler;

namespace smsc    {
namespace inman   {

class Billing;

class BillingListener
{
public:
	virtual void onBillingFinished(Billing*) = 0;
};

class Billing : public SSF, public ObservableT< BillingListener >, public InmanHandler
{
public:

	Billing(Dialog*, DeliveryMode_e mode);

	virtual ~Billing();

	Dialog*  getDialog() const
	{
		return dialog;
	}

	virtual void initialDPSMS();
	virtual void eventReportSMS();

    virtual void connectSMS(ConnectSMSArg* arg);
    virtual void continueSMS();
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void releaseSMS(ReleaseSMSArg* arg);
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void resetTimerSMS(ResetTimerSMSArg* arg);
	virtual void endDialog();

	virtual void onChargeSms(ChargeSms*);
	virtual void onDeliverySmsResult(DeliverySmsResult*);

protected:	
	Mutex			mutex;		
	DeliveryMode_e 	mode;
	Dialog*		dialog;
	Inap*	 		inap;
    Logger*	 		logger;   	

};

}
}




#endif
