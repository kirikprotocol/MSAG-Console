#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

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

#include "inman/inap/inap.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"

using smsc::inman::inap::Inap;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSFhandler;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListener;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::core::synchronization::Mutex;

namespace smsc    {
namespace inman   {

typedef enum { BILL_NONE = 0, BILL_ALL, BILL_USSD, BILL_SMS } BILL_MODE;

typedef std::list<unsigned char> RPCList;

struct BillingCFG {
    typedef enum { CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2} CDR_MODE;
//billing parameters
    BILL_MODE       billMode;
    CDR_MODE        cdrMode;
    const char *    billingDir;      //location to store CDR files
    long            billingInterval; //rolling interval for CDR files
//TCP interaction:
    unsigned short  tcpTimeout;      //optional timeout for TCP interaction with SMSC
    unsigned short  maxBilling;      //maximum number of Billings per connect
//SS7 interaction:
    unsigned char   userId;          //PortSS7 user id [1..20]
    unsigned short  capTimeout;      //optional timeout for operations with IN platform
    const char*     ssf_addr;        //
    const char*     scf_addr;        //IN platform address
    int             ssn;             //
    unsigned int    serviceKey;      //service id for InitialDP operation
    RPCList         rejectRPC;       //list of RP causes forcing charging denial
};

class Billing;
class Service;
//Manages SMSC Requests on given Connect in parallel mode
//(for each request initiates new Billing).
class BillingConnect: public ConnectListener
{
    typedef std::map<unsigned int, Billing*> BillingMap;

public: 
    BillingConnect(BillingCFG * cfg, Session* ss7_sess, Connect* conn,
                   Service * in_srvc, Logger * uselog = NULL);
    ~BillingConnect();

    unsigned int bConnId(void) const { return _bcId; }
    //sends command, and optionally turn on timer for response waiting
    //returns true on success
    bool sendCmd(SerializableObject* cmd, bool timerOn = false);
    //releases completed Billing, writting CDR if required
    void billingDone(Billing* bill);
    //
    CustomException * getConnectError(void) const { return _conn->hasException(); }

    //ConnectListener interface
    void onCommandReceived(Connect* conn, SerializableObject* cmd);
    //Stops all Billings due to fatal socket error
    void onConnectError(Connect* conn, bool fatal/* = false*/);

protected:
    Mutex       _mutex;
    Logger*     logger;
    unsigned int _bcId;
    BillingCFG  _cfg;
    BillingMap  workers;
    Connect*    _conn;
    Service *   _inSrvc;
    Session*    _ss7Sess;
};

class Billing : public SSFhandler, public InmanHandler
{
public:
    typedef enum {
        bilIdle, bilStarted, bilInited, bilReleased, bilProcessed, 
        bilApproved, bilComplete, bilAborted
    } BillingState;

    Billing(BillingConnect* bconn, unsigned int b_id, 
            BillingCFG * cfg, Logger * uselog = NULL);
    virtual ~Billing();

    unsigned int getId() const { return _bId; }
    
    void     handleCommand(InmanCommand* cmd);
    //stops billing due to external error
    void     Stop(CustomException * exc);
    //
    bool     isPostpaidBill(void) const { return postpaidBill; }
    //retuns false if CDR was not complete
    bool     CDRComplete(void) const { return cdr._finalized; }
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;
    //    
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
    void abortBilling(InmanErrorType errType, uint16_t errCode);
    Session * Billing::activateSSN(void);

    Mutex           bilMutex;   //
    BillingCFG      _cfg;
    Logger*         logger;
    unsigned int    _bId;       //unique billing dialogue id
    BillingConnect* _bconn;     //parent BillingConnect
    BillingState    state;

    Session*        _ss7Sess;   //TCAP dialogs factory
    Inap*           inap;       //Inap wrapper for dialog

    CDRRecord       cdr;        //data for CDR record creation
    bool            postpaidBill;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

