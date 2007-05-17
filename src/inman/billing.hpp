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
        bilQueried,     // SSF <- IAProvider: query result
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

    //-- WorkerAC interface methods
    void     handleCommand(INPPacketAC* cmd);
    void     Abort(const char * reason = NULL); //aborts billing due to fatal error

    BillingState getState(void) const { return state; }
    //retuns false if CDR was not complete
    bool     CDRComplete(void) const { return cdr._finalized; }
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;

    //-- CapSMS_SSFhandlerITF interface methods:
    void onDPSMSResult(unsigned char rp_cause = 0);
    //dialog finalization/error handling:
    //if ercode != 0, no result has been got from CAP service,
    void onEndCapDlg(RCHash errcode = 0);

    //-- IAPQueryListenerITF interface methods:
    void onIAPQueried(const AbonentId & ab_number, const AbonentRecord & ab_rec,
                                                    IAPQStatus::Code qry_status);
    //-- TimerListenerITF interface methods:
    void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj);

protected:
    //-- INPBillingHandlerITF interface methods:
    //Returns true if worker finished request processing and may be released
    bool onChargeSms(ChargeSms* sms, CsBillingHdr_dlg *hdr);
    void onDeliverySmsResult(DeliverySmsResult* dlvr_res, CsBillingHdr_dlg *hdr);

private:
    typedef std::map<unsigned, StopWatch*> TimersMAP;

    //Returns false if PDU contains invalid data preventing request processing
    bool Billing::verifyChargeSms(ChargeSms* sms);

    void doCleanUp(void);
    unsigned writeCDR(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    bool startCAPDialog(INScfCFG * use_scf);
    void StartTimer(unsigned short timeout);
    void StopTimer(BillingState bilState);
    //Returns false if result sending has been failed.
    bool chargeResult(ChargeSmsResult::ChargeSmsResult_t chg_res, RCHash inmanErr = 0);
    //Returns true if ChargeSmsResult::CHARGING_POSSIBLE was sent
    bool ConfigureSCFandCharge(AbonentContractInfo::ContractType ab_type,
                               const GsmSCFinfo * p_scf = NULL);

    Mutex           bilMutex;
    BillingCFG      _cfg;
                    //prefix for logging info
    char            _logId[sizeof("Billing[%u:%u]") + sizeof(unsigned int)*3 + 1];
    BillingState    state;

    TCSessionSR*    capSess;   //TCAP dialogs factory
    bool            capDlgActive;
    CapSMSDlg*      capDlg;     //Cap3SMS dialog controlling class

    CDRRecord       cdr;        //data for CDR record creation & CAP3 interaction
    SMCAPSpecificInfo csInfo;   //data for CAP3 interaction
    ChargeObj::MSG_TYPE msgType; // 
    bool            bill2CDR;
    TimersMAP       timers;     //active timers
    AbonentContractInfo::ContractType abType;     //calling abonent contract type
    TonNpiAddress   abNumber;   //calling abonent ISDN number
    bool            providerQueried;
    // ...
    AbonentPolicy * abPolicy;
    INScfCFG        abScf;
    XSmsService   * xsmsSrv;    //optional SMS Extra service config.
    RCHash          billErr;    //global error code made by URCRegistry
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

