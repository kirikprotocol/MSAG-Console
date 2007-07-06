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

class CAPSmsStatus {
public:
    unsigned        dlg_id;
    CapSMSDlg *     pDlg;
    TonNpiAddress   dstAdr;
    bool            active;
    bool            answered;
    bool            ended;
    RCHash          scfErr;
    ChargeSmsResult::ChargeSmsResult_t  chgRes;

    CAPSmsStatus()
        : dlg_id(0), pDlg(0), active(false), answered(false), ended(false)
        , scfErr(0), chgRes(ChargeSmsResult::CHARGING_NOT_POSSIBLE)
    { }
    CAPSmsStatus(const TonNpiAddress & dst_adr, CapSMSDlg *cap_dlg = NULL)
        : dlg_id(cap_dlg ? cap_dlg->getId() : 0), pDlg(cap_dlg), dstAdr(dst_adr)
        , active(cap_dlg ? true : false), answered(false), ended(false)
        , scfErr(0), chgRes(ChargeSmsResult::CHARGING_NOT_POSSIBLE)
    { }

    void abortDlg(void)
    {
        if (active) {
            pDlg->endDPSMS(); //send sms_o_failure to SCF if necessary
            active = false; ended = true;
        }
    }

    ~CAPSmsStatus()
    {
        if (pDlg) {
            abortDlg();
            delete pDlg;
            pDlg = NULL;
        }
    }
};

class Billing : public WorkerAC, public CapSMS_SSFhandlerITF, 
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
        pgEnd = 1       //processing has been finished (worker may be relaesed)
    };

    Billing(unsigned b_id, BillingManager * owner, Logger * uselog = NULL);
    virtual ~Billing();

    //-- WorkerAC interface methods
    void     handleCommand(INPPacketAC* cmd);
    void     Abort(const char * reason = NULL); //aborts billing due to fatal error

    BillingState getState(void) const { return state; }
    //returns true if required CDR data fullfilled
    bool     CDRComplete(void) const;
    //returns true if all billing stages are completed
    bool     BillComplete(void) const;

    //-- CapSMS_SSFhandlerITF interface methods:
    void onDPSMSResult(unsigned dlg_id, unsigned char rp_cause = 0);
    //dialog finalization/error handling:
    //if ercode != 0, no result has been got from CAP service,
    void onEndCapDlg(unsigned dlg_id, RCHash errcode = 0);

    //-- IAPQueryListenerITF interface methods:
    void onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                                                    IAPQStatus::Code qry_status);
    //-- TimerListenerITF interface methods:
    void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj);

protected:
    //-- INPBillingHandlerITF analogous methods:
    PGraphState onChargeSms(void);
    PGraphState onDeliverySmsResult(void);

private:
    typedef std::map<unsigned, StopWatch*> TimersMAP;
    typedef std::list<CAPSmsStatus> IDPStatus;

    class IDPResult {
    public:
        uint8_t     dlgCnt;
        IDPStatus   dlgRes;

        inline void prepareDlg(const TonNpiAddress & dst_adr, CapSMSDlg *cap_dlg = NULL)
        {
            dlgRes.push_back(CAPSmsStatus(dst_adr, cap_dlg));
            ++dlgCnt;
        }
        CAPSmsStatus * getIDPDlg(unsigned dlg_id)
        {
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it) {
                if (it->dlg_id == dlg_id)
                    return it.operator->();
            }
            return NULL;
        }

        bool allEnded(void)
        {
            bool res = true;
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it)
                res &= (it->ended ? true : false);
            return res;
        }

        bool allAnswered(void)
        {
            if (dlgRes.empty())
                return false;

            bool res = true;
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it)
                res &= (it->answered ? true : false);
            return res;
        }

        bool allActive(void)
        {
            if (dlgRes.empty())
                return false;

            bool res = true;
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it)
                res &= (it->active ? true : false);
            return res;
        }

        unsigned numActive(void)
        {
            unsigned res = 0;
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it)
                if (it->active) ++res;
            return res;
        }

        ChargeSmsResult::ChargeSmsResult_t Result(void)
        {
            ChargeSmsResult::ChargeSmsResult_t res = ChargeSmsResult::CHARGING_POSSIBLE;
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it) {
                if (res < it->chgRes)
                    res = it->chgRes;
            }
            return res;
        }

        //Returns first SCF error encountered
        RCHash SCFError(void)
        {
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it) {
                if (it->scfErr)
                    return it->scfErr;
            }
            return 0;
        }

        void cleanUp(void)
        {
            for (IDPStatus::iterator it = dlgRes.begin(); it != dlgRes.end(); ++it)
                delete it.operator->();
            dlgRes.clear();
            dlgCnt = 0;
        }

        IDPResult() : dlgCnt(0)
        { }
        ~IDPResult() { cleanUp(); }
    };
    

    //Returns false if PDU contains invalid data preventing request processing
    bool verifyChargeSms(void);

    void doCleanUp(void);
    unsigned writeCDR(void);
    void doFinalize(bool doReport = true);
    void abortThis(const char * reason = NULL, bool doReport = true);
    RCHash startCAPDialog(void);
    void StartTimer(unsigned short timeout);
    void StopTimer(BillingState bilState);
    PGraphState verifyIDPresult(CAPSmsStatus * res, const char * res_reason = NULL);
    PGraphState chargeResult(ChargeSmsResult::ChargeSmsResult_t chg_res, RCHash inmanErr = 0);
    PGraphState ConfigureSCFandCharge(void);

    Mutex           bilMutex;
    BillingCFG      _cfg;
                    //prefix for logging info
    char            _logId[sizeof("Billing[%u:%u]") + sizeof(unsigned int)*3 + 1];
    BillingState    state;

    TCSessionSR*    capSess;    //TCAP dialogs factory
    IDPResult       idpRes;     //Cap3SMS dialog(s) controlling class

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
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_BILLING__ */

