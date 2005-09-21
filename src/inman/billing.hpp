#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

#include "inman/common/observable.hpp"
#include "inman/comp/comps.hpp"
#include "inap/inap.hpp"
#include "inap/session.hpp"
#include "inap/connect.hpp"

#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"

using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;

using smsc::inman::comp::DeliveryMode_e;

using smsc::inman::inap::Inap;
using smsc::inman::inap::Session;
using smsc::inman::inap::TcapDialog;
using smsc::inman::inap::SSF;

namespace smsc    {
namespace inman   {

class Billing;

class BillingListener
{
public:
	virtual void onBillingFinished(Billing*) = 0;
};

class Billing : public SSF, public ObservableT< BillingListener >
{
public:

	Billing(TcapDialog*, DeliveryMode_e mode);

	virtual ~Billing();

	TcapDialog*  getDialog() const
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

protected:	
	Mutex			mutex;		
	DeliveryMode_e 	mode;
	TcapDialog*		dialog;
	Inap*	 		inap;
    Logger*	 		logger;   	

};

}
}




#endif
