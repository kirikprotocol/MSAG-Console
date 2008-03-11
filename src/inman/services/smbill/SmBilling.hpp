#pragma ident "$Id$"
/* ************************************************************************** *
 * Billing: implements SM/USSD billing process logic:
 * 1) determination of abonent contract type by quering abonent provider
 * 2) determination of IN platform and configuring its dialog parameters
 * 3) dialog with IN platform
 * 4) writing CDRs
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INAP_BILLING__
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
using smsc::inman::interaction::SMCAPSpecificInfo;

#include "inman/services/smbill/SmBillManager.hpp"
using smsc::core::timers::TimeWatcherITF;
using smsc::core::timers::TimerListenerITF;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::OPAQUE_OBJ;
using smsc::util::TaskRefereeITF;
using smsc::util::ScheduledTaskAC;
using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::iaprvd::AbonentSubscription;
using smsc::inman::tcpsrv::WorkerAC;

#include "inman/services/smbill/CAPSmTask.hpp"

namespace smsc {
namespace inman {
namespace smbill {

class Billing : public WorkerAC, TaskRefereeITF,
                IAPQueryListenerITF, TimerListenerITF {
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

private:
    typedef std::map<unsigned, TimerHdl> TimersMAP;

    mutable Mutex           _sync;
    const SmBillingCFG &    _cfg;
    volatile BillingState   state;
    //prefix for logging info
    char _logId[sizeof("Billing[%u:%u]") + sizeof(unsigned int)*3 + 1];

    CDRRecord           cdr;        //data for CDR record creation & CAP3 interaction
    SMCAPSpecificInfo   csInfo;     //data for CAP3 interaction
    const BModesPrio *  billPrio;   //billing modes priority 
    ChargeParm::MSG_TYPE  msgType;  //
    ChargeParm::BILL_MODE billMode; //current billing mode

    TimersMAP           timers;     //active timers
    AbonentSubscription abCsi;  //CAMEL subscription info of abonent is to charge
    TonNpiAddress       abNumber;   //ISDN number of abonent is to charge
    volatile bool       providerQueried;
    // ...
    const INScfCFG *    abScf;      //corresponding IN-point configuration
    CAPSmTaskAC *       capTask;
    XSmsService *       xsmsSrv;    //optional SMS Extra service config.
    RCHash              billErr;    //global error code made by URCRegistry

    //Returns false if PDU contains invalid data preventing request processing
    bool verifyChargeSms(void);

    void doCleanUp(void);
    unsigned writeCDR(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    RCHash startCAPSmTask(void);
    bool StartTimer(const TimeoutHDL & tmo_hdl);
    void StopTimer(BillingState bilState);
    PGraphState chargeResult(bool do_charge, RCHash last_err = 0);
    PGraphState ConfigureSCFandCharge(void);

protected:
    //-- TaskRefereeITF interface methods: --//
    void onTaskReport(TaskSchedulerITF * sched, ScheduledTaskAC * task);

    //-- IAPQueryListenerITF interface methods: --//
    void onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                                    RCHash qry_status);
    //-- TimerListenerITF interface methods: --//
    TimeWatcherITF::SignalResult
        onTimerEvent(TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj);

    //-- INPBillingHandlerITF analogous methods:
    PGraphState onChargeSms(void);
    PGraphState onDeliverySmsResult(void);

public:
    Billing(unsigned b_id, SmBillManager * owner, Logger * uselog = NULL)
        : WorkerAC(b_id, owner, uselog), state(bilIdle), abScf(0)
        , providerQueried(false), billErr(0), capTask(0), xsmsSrv(0)
        , msgType(ChargeParm::msgUnknown), billMode(ChargeParm::billOFF)
        , _cfg(owner->getConfig())
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman.Billing");
        snprintf(_logId, sizeof(_logId)-1, "Billing[%u:%u]", _mgr->cmId(), _wId);
    }

    virtual ~Billing();

    BillingState getState(void) const
    {
        MutexGuard grd(_sync);
        return state;
    }

    //-- WorkerAC interface methods
    void     handleCommand(INPPacketAC* cmd);
    void     Abort(const char * reason = NULL); //aborts billing due to fatal error

//    BillingState getState(void) const { return state; }

    //returns true if required CDR data fullfilled
    bool     CDRComplete(void) const;
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;
};

} //smbill
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

