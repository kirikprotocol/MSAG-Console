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
using smsc::inman::iaprvd::IAPQueryListenerITF;
using smsc::inman::cache::AbonentBillType;

#include "inman/inap/cap_sms/DlgCapSMS.hpp"
using smsc::inman::inap::CapSMSDlg;
using smsc::inman::inap::CapSMS_SSFhandlerITF;

#include "inman/interaction/MsgBilling.hpp"
using smsc::inman::interaction::INPBillingHandlerITF;
using smsc::inman::interaction::CsBillingHdr_dlg;
using smsc::inman::interaction::SMCAPSpecificInfo;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;

namespace smsc    {
namespace inman   {

class Billing : public WorkerAC, public CapSMS_SSFhandlerITF, 
                public IAPQueryListenerITF, public TimerListenerITF,
                INPBillingHandlerITF {
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

    Billing(unsigned b_id, BillingManager * owner, Logger * uselog = NULL);
    virtual ~Billing();

    //-- WorkerAC interface
    void     handleCommand(INPPacketAC* cmd);
    void     Abort(const char * reason = NULL); //aborts billing due to fatal error

    BillingState getState(void) const { return state; }
    //
    bool     isPostpaidBill(void) const { return postpaidBill; }
    //retuns false if CDR was not complete
    bool     CDRComplete(void) const { return cdr._finalized; }
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;

    //-- CapSMS_SSFhandlerITF interface methods:
    void onDPSMSResult(unsigned char rp_cause = 0);
    //dialog finalization/error handling:
    void onEndCapDlg(unsigned char ercode = 0, InmanErrorType errLayer = smsc::inman::errOk);

    //-- IAPQueryListenerITF interface methods:
    void onIAPQueried(const AbonentId & ab_number, const AbonentRecord & ab_rec);
//                       AbonentBillType ab_type, const MAPSCFinfo * scf = NULL);
    //-- TimerListenerITF interface methods:
    void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj);

protected:
    //-- INPBillingHandlerITF interface methods:
    bool onChargeSms(ChargeSms* sms, CsBillingHdr_dlg *hdr);
    void onDeliverySmsResult(DeliverySmsResult* dlvr_res, CsBillingHdr_dlg *hdr);

    typedef std::map<unsigned, StopWatch*> TimersMAP;

    void doCleanUp(void);
    unsigned writeCDR(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    bool startCAPDialog(INScfCFG * use_scf);
    void StartTimer(unsigned short timeout);
    void StopTimer(BillingState bilState);
    void chargeResult(ChargeSmsResult::ChargeSmsResult_t chg_res, uint32_t inmanErr = 0);
    bool ConfigureSCFandCharge(AbonentBillType ab_type, const MAPSCFinfo * p_scf = NULL);
    bool matchBillMode(void) const;

    Mutex           bilMutex;
    BillingCFG      _cfg;
                    //prefix for logging info
    char            _logId[sizeof("Billing[%u:%u]") + sizeof(unsigned int)*3 + 1];
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
    TonNpiAddress   smsxNumber;   //short number for SMS Extra service
    uint32_t        billErr;      //combined billing error code
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

