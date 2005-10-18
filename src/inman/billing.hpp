#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

#include "common/observable.hpp"
#include "comp/comps.hpp"
#include "inap/inap.hpp"
#include "inap/session.hpp"
#include "interaction/connect.hpp"
#include "interaction/messages.hpp"


using smsc::inman::comp::DeliveryMode_e;

using smsc::inman::inap::Inap;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::Connect;
using smsc::inman::inap::SSF;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;

namespace smsc    {
namespace inman   {

class Service;
class Billing : public SSF, public InmanHandler
{
public:

	Billing(Service* service, int id, Session*, Connect*);

	virtual ~Billing();

	Dialog*  getDialog() const
	{
		return dialog;
	}

	virtual int  getId() const { return id; }
	virtual void onChargeSms(ChargeSms*);
	virtual void onDeliverySmsResult(DeliverySmsResult*);

    virtual void connectSMS(ConnectSMSArg* arg);
    virtual void continueSMS();
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void releaseSMS(ReleaseSMSArg* arg);
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void resetTimerSMS(ResetTimerSMSArg* arg);

protected:	
	void fireBillingFinished();
		
	int				id;
	Session*		session;
	Dialog*			dialog;
	Inap*	 		inap;
    Logger*	 		logger;
    Connect*		connect;
    Service*		service;
};

}
}




#endif
