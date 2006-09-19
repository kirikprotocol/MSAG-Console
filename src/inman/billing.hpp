#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_BILLING__
#define __SMSC_INMAN_INAP_BILLING__

/* Inman interoperation scheme:
SMSC   < - >           Billing(SSF)     < - >  CapSMSDlg  < - >    In-platform(SCF)
ChargeSms           -> [ bilStarted,
                         bilInited:      -->   InitialDP ->  SCF]
                                                [ CapSMSDlg <- ReleaseSMS(SCF)
                                                  | CapSMSDlg <- ContinueSMS(SCF) ]
                       [ bilReleased     <--   onDPSMSResult()
ChargeSmsResult     <-   | bilProcessed ]

[ CHARGING_POSSIBLE:
  DeliverySmsResult -> [  bilApproved:   -->   eventReportSMS  --> SCF
                          bilComplete :  <--   onEndCapDlg() ]
]
*/

//#include "inman/common/RPCList.hpp"
//using smsc::inman::common::RPCList;
#include "inman/inman.hpp"
using smsc::inman::AbonentPolicy;
using smsc::inman::AbonentPolicies;
using smsc::inman::INScfCFG;
//using smsc::inman::INScfsMAP;

#include "inman/common/TimeWatcher.hpp"
using smsc::inman::sync::StopWatch;
using smsc::inman::sync::TimeWatcher;
using smsc::inman::sync::TimerListenerITF;
using smsc::inman::sync::TimersLIST;
using smsc::inman::sync::OPAQUE_OBJ;

#include "inman/abprov/IAProvider.hpp"
using smsc::inman::iaprvd::IAProviderITF;
using smsc::inman::iaprvd::IAPQueryListenerITF;

#include "inman/storage/CDRStorage.hpp"
using smsc::inman::filestore::InBillingFileStorage;

#include "inman/inap/cap_sms/DlgCapSMS.hpp"
using smsc::inman::inap::CapSMSDlg;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::CapSMS_SSFhandlerITF;

#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListener;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::SMCAPSpecificInfo;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::ChargeSmsResult_t;


namespace smsc    {
namespace inman   {

typedef enum { BILL_NONE = 0, BILL_ALL, BILL_USSD, BILL_SMS } BILL_MODE;

struct BillingCFG {
    typedef enum { CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2} CDR_MODE;
    AbonentCacheITF *   abCache;
    InBillingFileStorage * bfs;
    TimeWatcher *       tmWatcher;
//billing parameters
    AbonentPolicies * policies;
    BILL_MODE       billMode;
    CDR_MODE        cdrMode;
    const char *    cdrDir;         //location to store CDR files
    long            cdrInterval;    //rolling interval for CDR files
    unsigned short  maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    unsigned short  abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    unsigned short  maxBilling;     //maximum number of Billings per connect

    SS7_CFG         ss7;            //SS7 interaction:

    BillingCFG()
    {
        abCache = NULL; bfs = NULL; tmWatcher = NULL;
        cdrDir = NULL;
        cdrInterval = 0;
        billMode = smsc::inman::BILL_ALL;
        cdrMode =  CDR_ALL;
        maxTimeout = abtTimeout = maxBilling = ss7.maxDlgId = ss7.capTimeout = 0;
        ss7.own_ssn = ss7.userId = 0;
    }

};

class Billing;
//Manages SMSC Requests on given Connect in parallel/asynchronous mode
//(for each request initiates new Billing).
class BillingConnect: public ConnectListener {
public: 
    BillingConnect(BillingCFG * cfg, SSNSession * ssn_sess,
                   Connect* conn, Logger * uselog = NULL);
    ~BillingConnect();

    unsigned int    bConnId(void) const { return _bcId; }
    SSNSession*     ssnSession(void);
    const BillingCFG & getConfig(void) const { return _cfg;}

    //sends command, returns true on success
    bool sendCmd(SerializableObject* cmd);
    //releases completed Billing, writting CDR if required
    void billingDone(Billing* bill);
    //
    CustomException * getConnectError(void) const { return _conn->hasException(); }

    //ConnectListener interface
    void onCommandReceived(Connect* conn, SerializableObject* cmd);
    //Stops all Billings due to fatal socket error
    void onConnectError(Connect* conn, bool fatal/* = false*/);

protected:
    typedef std::map<unsigned int, Billing*> BillingMap;
    typedef std::list<Billing*> BillingList;

    void cleanUpBills(void);

    Mutex       _mutex;
    Logger*     logger;
    unsigned int _bcId;
    BillingCFG  _cfg;
    BillingMap  workers;
    BillingList corpses;
    Connect*    _conn;
    SSNSession *    ssnSess;
    TimeWatcher* _tmWatcher;
};

class Billing : public CapSMS_SSFhandlerITF, public InmanHandler,
                public IAPQueryListenerITF, public TimerListenerITF {
public:
    typedef enum {
        bilIdle, bilAborted,
        bilStarted,     // SSF <- SMSC : CHARGE_SMS_TAG
        bilQueried,     // SSF <- AbProvider: query result
        bilInited,      // SSF -> SCF : InitialDPSMS
        bilReleased,    // SSF <- SCF : ReleaseSMS
                        // SSF -> SMSC : CHARGE_SMS_RESULT_TAG
        bilContinued,   // SSF <- SCF : ContinueSMS
                        // SSF -> SMSC : CHARGE_SMS_RESULT_TAG
        bilApproved,    // SSF <- SMSC : DELIVERY_SMS_RESULT_TAG
        bilReported,    // SSF -> SCF : EventReportSMS
        bilComplete     // 
    } BillingState;

    typedef enum {
        doCont = 0, doEnd, doAbort
    } BillAction;

    Billing(BillingConnect* bconn, unsigned int b_id, Logger * uselog = NULL);
    virtual ~Billing();

    unsigned int getId(void) const { return _bId; }
    BillingState getState(void) const { return state; }

    
    void     handleCommand(InmanCommand* cmd);
    //aborts billing due to fatal error
    void     Abort(const char * reason = NULL);
    //
    bool     isPostpaidBill(void) const { return postpaidBill; }
    //retuns false if CDR was not complete
    bool     CDRComplete(void) const { return cdr._finalized; }
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;
    //    
    const CDRRecord & getCDRRecord(void) const { return cdr; }

    //InmanHandler interface methods:
    bool onChargeSms(ChargeSms*);
    void onDeliverySmsResult(DeliverySmsResult*);

    //CapSMS_SSFhandlerITF interface methods:
    void onDPSMSResult(unsigned char rp_cause = 0);
    //dialog finalization/error handling:
    void onEndCapDlg(unsigned char ercode = 0, InmanErrorType errLayer = smsc::inman::errOk);

    //IAPQueryListenerITF interface methods:
    void onIAPQueried(const AbonentId & ab_number, AbonentBillType ab_type,
                                const MAPSCFinfo * scf = NULL);
    //TimerListenerITF interface methods:
    void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj);

protected:
    typedef std::map<unsigned, StopWatch*> TimersMAP;

    void doCleanUp(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    bool startCAPDialog(INScfCFG * use_scf);
    void StartTimer(unsigned short timeout);
    void StopTimer(BillingState bilState);
    void chargeResult(ChargeSmsResult_t chg_res, uint32_t inmanErr = 0);
    bool ConfigureSCFandCharge(AbonentBillType ab_type, const MAPSCFinfo * p_scf = NULL);

    Mutex           bilMutex;
    BillingCFG      _cfg;
    Logger*         logger;
    unsigned int    _bId;       //unique billing dialogue id
    BillingConnect* _bconn;     //parent BillingConnect
    BillingState    state;

    TCSessionSR*    capSess;   //TCAP dialogs factory
    bool            capDlgActive;
    CapSMSDlg*      capDlg;     //CapSMS wrapper for TC dialog

    CDRRecord       cdr;        //data for CDR record creation & CAP3 interaction
    SMCAPSpecificInfo csInfo;   //data for CAP3 interaction
    bool            postpaidBill;
    TimersMAP       timers;     //active timers
    AbonentBillType abType;     //calling abonent billing type
    TonNpiAddress   abNumber;   //calling abonent ISDN number
    bool            providerQueried;
    // ...
    AbonentPolicy * abPolicy;
    INScfCFG        abScf;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

