#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

//#include "inman/inap/session.hpp"
#include "inman/inap/inap.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"
//#include "inman/storage/cdrutil.hpp"

using smsc::inman::inap::Inap;
//using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSF;
//using smsc::inman::cdr::CDRRecord;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::core::synchronization::Mutex;

namespace smsc    {
namespace inman   {


class Service;
class Billing : public SSF, public InmanHandler
{
public:
    typedef enum { billPrepaid, billPostpaid } BillingType;
    typedef enum { bilIdle, bilStarted, bilInited, bilProcessed, bilApproved, bilComplete, billAborted } BillingState;

    Billing(Service* service, unsigned int id, Session*, Connect*);
    virtual ~Billing();

    unsigned int getId() const { return id; }
    
    void     handleCommand(InmanCommand* cmd);
    //
    BillingType getBillingType(void) const;
    //retuns false if CDR was not complete
    bool     BillComplete(void) const;
    //returns true if succeeded, false if CDR was not complete
    //bool     getCDRasCSV(const std::string & rec);
    
    const CDRRecord & getCDRRecord(void) const;

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
    virtual void abortSMS(unsigned char errcode, bool tcapLayer);

protected:
    void abortBilling(InmanErrorType errType, uint16_t errCode);

    Mutex           bilMutex;   //
    BillingState    state;
    unsigned int    id;         //unique billing dialogue id
    Session*        session;    //TCAP dialogs factory
    Inap*           inap;       //Inap wrapper for dialog
    Logger*         logger;
    Connect*        connect;
    Service*        service;
    CDRRecord       cdr;        //data for CDR record creation
    BillingType     billType;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

