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

#include "inman/inap/DlgCapSMS.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"
#include "inman/common/TimeWatcher.hpp"
#include "inman/incache.hpp"
#include "inman/storage/FileStorages.hpp"

using smsc::inman::inap::CAPErrorSource;
using smsc::inman::inap::CapSMSDlg;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSFhandler;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListener;
using smsc::inman::interaction::InmanCommand;
using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::SMCAPSpecificInfo;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::sync::StopWatch;
using smsc::inman::sync::TimeWatcher;
using smsc::inman::sync::TimerListenerITF;
using smsc::inman::sync::TimersLIST;
using smsc::inman::sync::OPAQUE_OBJ;

using smsc::inman::cache::InAbonentProviderITF;
using smsc::inman::cache::InAbonentQueryListenerITF;
using smsc::inman::cache::AbonentCacheITF;
using smsc::inman::cache::AbonentBillType;
using smsc::inman::cache::AbonentId;

using smsc::inman::filestore::InBillingFileStorage;

namespace smsc    {
namespace inman   {

typedef enum { BILL_NONE = 0, BILL_ALL, BILL_USSD, BILL_SMS } BILL_MODE;
typedef enum { policyIN = 0, policyDB, policyHLR } BillPolicy;

class RPCList : public std::list<unsigned char> {
public:
    int init(const char * str) throw(CustomException)
    {
        if (!str || !str[0])
            RPCList::clear();
        else {
            std::string rplist(str);
            std::string::size_type pos = 0, commaPos;
            do {
                commaPos = rplist.find_first_of(',', pos);
                std::string rp_s = rplist.substr(pos, commaPos);
                int rp_i = atoi(rp_s.c_str());
                if (!rp_i || (rp_i > 0xFF))
                    throw CustomException(format("bad element \'%s\'", rp_s.c_str()).c_str());

                RPCList::push_back((unsigned char)rp_i);
                pos = commaPos + 1;
            } while (commaPos != rplist.npos);
        }
        return RPCList::size();
    }
    int print(std::string & ostr)
    {
        int i = 0;
        RPCList::iterator it = RPCList::begin();
        for (; it != RPCList::end(); it++, i++)
            format(ostr, "%s%u", i ? ", ":"", (*it));
        return i;
    }
};

struct BillingCFG {
    typedef enum { CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2} CDR_MODE;
    AbonentCacheITF * abCache;
    InAbonentProviderITF * abProvider;
    InBillingFileStorage * bfs;
//billing parameters
    BILL_MODE       billMode;
    BillPolicy      policy;
    CDR_MODE        cdrMode;
    const char *    cdrDir;         //location to store CDR files
    long            cdrInterval;    //rolling interval for CDR files
    unsigned short  maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    unsigned short  abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    unsigned short  maxBilling;     //maximum number of Billings per connect
//SS7 interaction:
    unsigned char   userId;         //PortSS7 user id [1..20]
    unsigned short  capTimeout;     //optional timeout for operations with IN platform
    const char*     ssf_addr;       //
    const char*     scf_addr;       //IN platform address
    int             ssn;            //
    unsigned short  maxDlgId;       //maximum number of simultaneous TC dialogs, max: 65530
    unsigned int    serviceKey;     //service id for InitialDP operation
    RPCList         rejectRPC;      //list of RP causes forcing charging denial
    RPCList         postpaidRPC;    //list of RP causes returned for postpaid abonents
};

class Billing;
//Manages SMSC Requests on given Connect in parallel mode
//(for each request initiates new Billing).
class BillingConnect: public ConnectListener {
public: 
    BillingConnect(BillingCFG * cfg, SSNSession* ss7_sess, Connect* conn,
                   TimeWatcher* tm_watcher, Logger * uselog = NULL);
    ~BillingConnect();

    unsigned int bConnId(void) const { return _bcId; }
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
    SSNSession*  _ss7Sess;
    TimeWatcher* _tmWatcher;
};

class Billing : public SSFhandler, public InmanHandler,
                public InAbonentQueryListenerITF, public TimerListenerITF {
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

    Billing(BillingConnect* bconn, unsigned int b_id,
            BillingCFG * cfg, TimeWatcher* tm_watcher, Logger * uselog = NULL);
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

    //SSFHandler interface methods:
    void onConnectSMS(ConnectSMSArg* arg);
    void onContinueSMS(void);
    void onFurnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg);
    void onReleaseSMS(ReleaseSMSArg* arg);
    void onRequestReportSMSEvent(RequestReportSMSEventArg* arg);
    void onResetTimerSMS(ResetTimerSMSArg* arg);
    void onEndSMS(bool approved/* = true*/);
    void onAbortSMS(unsigned char errcode, CAPErrorSource errLayer);

    //InAbonentQueryListenerITF interface methods:
    void onAbonentQueried(const AbonentId & ab_number, AbonentBillType ab_type);
    //TimerListenerITF interface methods:
    void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj);

protected:
    typedef std::map<unsigned, StopWatch*> TimersMAP;

    SSNSession * activateSSN(void);
    void doCleanUp(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    bool startCAPDialog(void);
    void StartTimer(unsigned short timeout);
    void StopTimer(BillingState bilState);
    void ChargeAbonent(AbonentBillType ab_type);
    void doCharge(uint32_t inmanErr = 0);

    Mutex           bilMutex;
    BillingCFG      _cfg;
    Logger*         logger;
    unsigned int    _bId;       //unique billing dialogue id
    BillingConnect* _bconn;     //parent BillingConnect
    BillingState    state;

    SSNSession*     _ss7Sess;   //TCAP dialogs factory
    bool            capDlgActive;
    CapSMSDlg*      capDlg;     //CapSMS wrapper for TC dialog

    CDRRecord       cdr;        //data for CDR record creation & CAP3 interaction
    SMCAPSpecificInfo csInfo;   //data for CAP3 interaction
    bool            postpaidBill;
    TimeWatcher*    tmWatcher;
    TimersMAP       timers;     //active timers
    AbonentBillType abBillType; //calling abonent billing type
    TonNpiAddress   abNumber;   //calling abonent ISDN number
    bool            providerQueried;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

