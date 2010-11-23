/* ************************************************************************** *
 * Billing: implements SM/USSD billing process logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * 3) dialog with IN platform
 * 4) writing CDRs
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INAP_BILLING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_BILLING__

/* SMBilling interoperation scheme:
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

#include "inman/interaction/msgbill/MsgBilling.hpp"
#include "inman/services/smbill/SmBillManager.hpp"
#include "inman/services/smbill/CAPSmTask.hpp"


namespace smsc {
namespace inman {
namespace smbill {

using smsc::core::synchronization::Condition;
using smsc::core::timers::TimeWatcherITF;
using smsc::core::timers::TimerListenerITF;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::OPAQUE_OBJ;
using smsc::inman::TaskRefereeITF;
using smsc::inman::ScheduledTaskAC;

using smsc::inman::iaprvd::AbonentId;
using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::iaprvd::AbonentSubscription;
using smsc::inman::iapmgr::IAPRule;
using smsc::inman::iapmgr::IAPPrio_e;
using smsc::inman::comp::CSIUid_e;

using smsc::inman::tcpsrv::WorkerAC;
using smsc::inman::interaction::INPPacketAC;
using smsc::inman::interaction::SMCAPSpecificInfo;
using smsc::inman::cdr::CDRRecord;


class Billing : public WorkerAC, TaskRefereeITF,
                IAPQueryListenerITF, TimerListenerITF {
public:
    enum BillingState {
        bilIdle = 0,
        bilStarted,     // SSF <- SMSC : CHARGE_SMS_TAG
                        //   [Timer] SSF -> IAProvider
        bilQueried,     // SSF <- IAProvider: query result
        bilInited,      // [Timer] SSF -> SCF : InitialDPSMS
        bilReleased,    // SSF <- SCF : ReleaseSMS
                        //   SSF -> SMSC : CHARGE_SMS_RESULT_TAG(No)
        bilContinued,   // SSF <- SCF : ContinueSMS
                        //   [Timer] SSF -> SMSC : CHARGE_SMS_RESULT_TAG(Ok)
        bilAborted,     // SSF <- SMSC/Billing : Abort
        bilSubmitted,   // SSF <- SMSC : DELIVERY_SMS_RESULT_TAG
                        //   [Timer] SSF -> SCF : EventReportSMS
        bilReported,    // SSF <- SCF: T_END{OK|NO}
        bilComplete     // 
    };

    enum PGraphState {  //billing processing graph state
        pgAbort = -1,   //processing has aborted (worker aborted itself)
        pgCont = 0,     //processing continues (negotiation with
                        //client or external module is expected)
        pgEnd = 1       //processing has been finished (worker may be released)
    };

    static const char * nmBState(BillingState bil_state);

private:
    typedef std::map<unsigned, TimerHdl> TimersMAP;

    mutable Mutex           _sync;
    const SmBillingCFG &    _cfg;
    volatile BillingState   state;
    //prefix for logging info
    char _logId[sizeof("Billing[%u:%u]") + sizeof(unsigned int)*3 + 1];

    CDRRecord           cdr;        //data for CDR record creation & CAP3 interaction
    SMCAPSpecificInfo   csInfo;     //data for CAP3 interaction
    uint8_t             chrgFlags;  //flags which customize billing settings
    const BModesPrio *  billPrio;   //billing modes priority 
    ChargeParm::MSG_TYPE  msgType;  //
    ChargeParm::BILL_MODE billMode; //current billing mode

    TimersMAP           timers;     //active timers
    AbonentSubscription abCsi;      //CAMEL subscription info of abonent is to charge
    TonNpiAddress       abNumber;   //ISDN number of abonent is to charge
    IAPRule             _iapRule;   //abonent policy rule
    IAPPrio_e           _curIAPrvd;  //UId of last IAProvider asked
    volatile bool       providerQueried;
    // ...
    const INScfCFG *    _cfgScf;    //serving gsmSCF(IN-point) configuration
    const XSmsService * xsmsSrv;    //optional SMS Extra service config.
    RCHash              billErr;    //global error code made by URCRegistry
    // ...
    TaskId              capTask;    //id of started CapSMS task
    TaskSchedulerITF *  capSched;   //CapSMS task scheduler
    std::string         capName;    //CapSMS task name for logging
    Condition           capEvent;

    uint32_t getServiceKey(CSIUid_e csi_id) const;

    //Returns false if PDU contains invalid data preventing request processing
    bool verifyChargeSms(void);
    //Returns true if qyery is started, so execution will continue in another thread.
    bool startIAPQuery(void);
    void cancelIAPQuery(void);
    void doCleanUp(void);
    unsigned writeCDR(void);
    void doFinalize(void);
    void abortThis(const char * reason = NULL);
    RCHash startCAPSmTask(void);
    bool unrefCAPSmTask(bool wait_report = true);
    void abortCAPSmTask(void);
    bool StartTimer(const TimeoutHDL & tmo_hdl);
    void StopTimer(BillingState bilState);
    PGraphState chargeResult(bool do_charge, RCHash last_err = 0);
    const AbonentPolicy * determinePolicy(void);
    void configureMOSM(void);
    INManErrorId::Code_e configureSCF(void);
    PGraphState ConfigureSCFandCharge(void);
    PGraphState onSubmitReport(RCHash scf_err, bool in_billed = false);

protected:
    //-- TaskRefereeITF interface methods: --//
    void onTaskReport(TaskSchedulerITF * sched, const ScheduledTaskAC * task);

    //-- IAPQueryListenerITF interface methods: --//
    void onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                                    RCHash qry_status);
    //-- TimerListenerITF interface methods: --//
    TimeWatcherITF::SignalResult
        onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj);

    //-- INPBillingHandlerITF analogous methods:
    PGraphState onChargeSms(void);
    PGraphState onDeliverySmsResult(void);

public:
    Billing(unsigned b_id, SmBillManager * owner, Logger * uselog = NULL)
        : WorkerAC(b_id, owner, uselog), _cfg(owner->getConfig())
        , state(bilIdle), chrgFlags(0), billPrio(0)
        , msgType(ChargeParm::msgUnknown), billMode(ChargeParm::billOFF)
        , _curIAPrvd(AbonentPolicy::iapNone), providerQueried(false)
        , _cfgScf(0), xsmsSrv(0), billErr(0), capTask(0), capSched(0)
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman.Billing");
        snprintf(_logId, sizeof(_logId)-1, "Billing[%u:%u]", _mgr->cmId(), _wId);
    }

    virtual ~Billing();

    const char * nmBState(void) const
    {
        return nmBState(state);
    }
    BillingState getState(void) const
    {
        MutexGuard grd(_sync);
        return state;
    }
    //returns true if required CDR data fullfilled
    bool     CDRComplete(void) const;
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;

    //-- WorkerAC interface methods
    void     logState(std::string & use_str) const;
    void     handleCommand(INPPacketAC* cmd);
    void     Abort(const char * reason = NULL); //aborts billing due to fatal error
};

} //smbill
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

