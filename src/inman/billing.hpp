#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

#include "inman/inap/session.hpp"
//#include "inman/inap/dialog.hpp"
//#include "inman/comp/comps.hpp"
#include "inman/inap/inap.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"

using smsc::inman::inap::Inap;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSF;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;

namespace smsc    {
namespace inman   {

class Service;
class Billing : public SSF, public InmanHandler
{
public:
    typedef enum { bilIdle, bilInited, bilProcessed, bilClosed } BillingState;

    Billing(Service* service, unsigned int id, Session*, Connect*);
    virtual ~Billing();

    unsigned int getId() const { return id; }
    Dialog*  getDialog() const { return dialog; }
    
    void     handleCommand(InmanCommand* cmd);
    
    //SSF interface
    virtual void onChargeSms(ChargeSms*);
    virtual void onDeliverySmsResult(DeliverySmsResult*);
    //InmanHandler interface
    virtual void connectSMS(ConnectSMSArg* arg);
    virtual void continueSMS();
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void releaseSMS(ReleaseSMSArg* arg);
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void resetTimerSMS(ResetTimerSMSArg* arg);
    virtual void abortSMS(unsigned char errcode);

protected:
    void abortBilling(unsigned int errCode);

    BillingState    state;
    unsigned int    id;         //unique billing dialogue id
    Session*        session;    //TCAP dialog factory
    Dialog*         dialog;     //TCAP dialog
    Inap*           inap;       //Inap wrapper for dialog
    Logger*         logger;
    Connect*        connect;
    Service*        service;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

