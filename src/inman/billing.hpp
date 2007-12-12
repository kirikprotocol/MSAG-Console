#ident "$Id$"
/* ************************************************************************** *
 * Billing: implements Billing process logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * 3) dialog with IN platform
 * 4) writing CDRs
 * ************************************************************************** */
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

#include "inman/BillingManager.hpp"
using smsc::inman::AbonentPolicy;
using smsc::inman::sync::StopWatch;
using smsc::inman::sync::TimerListenerITF;
using smsc::inman::sync::OPAQUE_OBJ;
using smsc::inman::iaprvd::IAPQStatus;
using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::iaprvd::AbonentSubscription;
using smsc::util::TaskRefereeITF;
using smsc::util::ScheduledTaskAC;

#include "inman/CAPSmTask.hpp"
using smsc::inman::CAPSmTaskAC;

#include "inman/interaction/MsgBilling.hpp"
//using smsc::inman::interaction::INPBillingHandlerITF;
using smsc::inman::interaction::SMCAPSpecificInfo;

namespace smsc    {
namespace inman   {

class Billing : public WorkerAC, public TaskRefereeITF,
                public IAPQueryListenerITF, public TimerListenerITF
                /*,INPBillingHandlerITF*/ {
public:
    typedef enum {
        bilIdle,
        bilStarted,     // SSF <- SMSC : CHARGE_SMS_TAG
        bilQueried,     // SSF <- IAProvider: query result
        bilInited,      // SSF -> SCF : InitialDPSMS
        bilReleased,    // SSF <- SCF : ReleaseSMS
                        // SSF -> SMSC : CHARGE_SMS_RESULT_TAG(No)
        bilContinued,   // SSF <- SCF : ContinueSMS
                        // SSF -> SMSC : CHARGE_SMS_RESULT_TAG(Ok)
        bilAborted,     // SSF <- SMSC : Abort
        bilReported,    // SSF <- SMSC : DELIVERY_SMS_RESULT_TAG
                        // SSF -> SCF : EventReportSMS
        bilComplete     // 
    } BillingState;

    enum PGraphState {  //billing processing graph state
        pgAbort = -1,   //processing has aborted (worker aborted itself)
        pgCont = 0,     //processing continues (negotiation with
                        //client or external module is expected)
        pgEnd = 1       //processing has been finished (worker may be released)
    };

    Billing(unsigned b_id, BillingManager * owner, Logger * uselog = NULL)
        : WorkerAC(b_id, owner, uselog), state(bilIdle), abScf(0)
        , providerQueried(false), abPolicy(NULL), billErr(0), capTask(0)
        , xsmsSrv(0), msgType(ChargeObj::msgUnknown), billMode(ChargeObj::billOFF)
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman.Billing");
        _cfg = owner->getConfig();
        snprintf(_logId, sizeof(_logId)-1, "Billing[%u:%u]", _mgr->cmId(), _wId);
    }

    virtual ~Billing();

    //-- WorkerAC interface methods
    void     handleCommand(INPPacketAC* cmd);
    void     Abort(const char * reason = NULL); //aborts billing due to fatal error

    BillingState getState(void) const { return state; }
    //returns true if required CDR data fullfilled
    bool     CDRComplete(void) const;
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;

    //-- TaskRefereeITF interface methods: --//
    void onTaskReport(TaskSchedulerITF * sched, ScheduledTaskAC * task);

    //-- IAPQueryListenerITF interface methods: --//
    void onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                                    RCHash qry_status);
    //-- TimerListenerITF interface methods: --//
    void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj);

protected:
    //-- INPBillingHandlerITF analogous methods:
    PGraphState onChargeSms(void);
    PGraphState onDeliverySmsResult(void);

private:
    typedef std::map<unsigned, StopWatch*> TimersMAP;

    //Returns false if PDU contains invalid data preventing request processing
    bool verifyChargeSms(void);

    void doCleanUp(void);
    unsigned writeCDR(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    RCHash startCAPSmTask(void);
    void StartTimer(unsigned short timeout);
    void StopTimer(BillingState bilState);
    PGraphState chargeResult(bool do_charge, RCHash last_err = 0);
    PGraphState ConfigureSCFandCharge(void);

    Mutex           bilMutex;
    BillingCFG      _cfg;
                    //prefix for logging info
    char            _logId[sizeof("Billing[%u:%u]") + sizeof(unsigned int)*3 + 1];
    BillingState    state;

    CDRRecord       cdr;        //data for CDR record creation & CAP3 interaction
    SMCAPSpecificInfo csInfo;   //data for CAP3 interaction
    ChargeObj::MSG_TYPE msgType; // 
    TimersMAP       timers;     //active timers
    AbonentSubscription     abCsi;  //CAMEL subscription info of abonent is to charge
    TonNpiAddress   abNumber;   //ISDN number of abonent is to charge
    bool            providerQueried;
    // ...
    AbonentPolicy * abPolicy;
    const INScfCFG* abScf;      //corresponding IN-point configuration
    XSmsService   * xsmsSrv;    //optional SMS Extra service config.
    RCHash          billErr;    //global error code made by URCRegistry
    const BModesPrio * billPrio;   //billing modes priority 
    ChargeObj::BILL_MODE billMode;//current billing mode
//    TaskId          smTaskId;   //id of CAPSmTask if started
//    TaskSchedulerITF * capSched;   //scheduler for CAPSmTask
    CAPSmTaskAC     *capTask;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

