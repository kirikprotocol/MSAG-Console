#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

#include "inman/inap/inap.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"

using smsc::inman::inap::Inap;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSFhandler;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::core::synchronization::Mutex;

namespace smsc    {
namespace inman   {

typedef enum { BILL_ALL = 0, BILL_USSD, BILL_SMS, BILL_NONE } BILL_MODE;

/* Inman interoperation scheme:
SMSC   < - >           Billing      < - >    Inap (SSF)  < - >    In-platform(SCF)
ChargeSms           -> [ bilStarted,
                         bilInited:          InitialDP ->  SCF]

                       [ bilReleased:        SSF <- ReleaseSMS
ChargeSmsResult     <-   | bilProcessed:     SSF <- ContinueSMS ]

[ CHARGING_POSSIBLE:
  DeliverySmsResult -> [  bilApproved:       eventReportSMS -> SCF
                          bilComplete :                     <-    ]
]
*/

class Service;
class Billing : public SSFhandler, public InmanHandler
{
public:
    typedef enum {
        bilIdle, bilStarted, bilInited, bilReleased, bilProcessed, 
        bilApproved, bilComplete, bilAborted
    } BillingState;

    Billing(Service* service, unsigned int id, Session*, Connect*, BILL_MODE bMode,
            USHORT_T capTimeout = 0, USHORT_T tcpTimeout = 0, Logger * uselog = NULL);
    virtual ~Billing();

    unsigned int getId() const { return id; }
    
    void     handleCommand(InmanCommand* cmd);
    //
    bool    isPostpaidBill(void) const { return postpaidBill; }
    //retuns false if CDR was not complete
    bool     BillComplete(void) const;
    //returns true if succeeded, false if CDR was not complete
    //bool     getCDRasCSV(const std::string & rec);
    
    const CDRRecord & getCDRRecord(void) const;

    //SSF interface
    virtual void onChargeSms(ChargeSms*);
    virtual void onDeliverySmsResult(DeliverySmsResult*);
    //InmanHandler interface
    virtual void onConnectSMS(ConnectSMSArg* arg);
    virtual void onContinueSMS();
    virtual void onFurnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    virtual void onReleaseSMS(ReleaseSMSArg* arg);
    virtual void onRequestReportSMSEvent(RequestReportSMSEventArg* arg);
    virtual void onResetTimerSMS(ResetTimerSMSArg* arg);
    virtual void onAbortSMS(unsigned char errcode, bool tcapLayer);

protected:
//    void finishBilling(void);
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
    bool            postpaidBill;                                
    BILL_MODE       billMode;
    unsigned short  _capTimeout; //timeout for interconnection with IN-platform
    unsigned short  _tcpTimeout; //timeout for interconnection with SMSC
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

