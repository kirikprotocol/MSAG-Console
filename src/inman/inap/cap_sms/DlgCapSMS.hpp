/* ************************************************************************* *
 * cap3SMS CONTRACT implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_CAP3SMS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_CAP3SMS__

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/inap/HDSSnSession.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::TCSessionSR;

#include "inman/inap/cap_sms/FSMCapSMS.hpp"
using smsc::util::RCHash;
using smsc::inman::comp::CapSMSOp;
using smsc::inman::comp::SMSInitialDPArg;
using smsc::inman::comp::SMSConnectArg;

#ifdef _THROWS_NONE
#undef _THROWS_NONE
#endif
#define _THROWS_NONE /* throw() */

namespace smsc {
namespace inman {
namespace inap {

/* cap3SMS CONTRACT:    CapSMSDlg(SSF) <-> In-platform(SCF)

->  smsActivationPackage:
        InitialDP -> SCF
[ <-  smsConnectPackage:
        SSF <- connectSMS ]
<-  smsProcessingPackage [ |smsEventHandlingPackage]:
        SSF <- ContinueSMS | ReleaseSMS
[ <-> smsEventHandlingPackage [ |smsProcessingPackage]
        SSF <- RequestReportSMSEvent
        eventReportSMS -> SCF       ]
[ <- smsBillingPackage:
        SSF <- FurnishChargingInformationSMS ]  -- unused !!!
[ <- smsTimerPackage:
        SSF <- ResetTimerSMS ]
*/

//3GPP TS 23.078 version 6.x.x Release 6, clause 7.5.5)
class CapSMS_SSFhandlerITF { //SSF <- CapSMSDlg <- SCF
protected:
    virtual ~CapSMS_SSFhandlerITF() //forbid interface destruction
    { }

public:
    //Stands for following signals to MSC/SGSN:
    //  Int_ReleaseSMS, Int_ConnectSMS, Int_ContinueSMS
    virtual void onDPSMSResult(TCDialogID dlg_id, unsigned char rp_cause,
                                    std::auto_ptr<SMSConnectArg> & sms_params) = 0;

    //if ercode != 0, CAP dialog is abnormally ended
    //NOTE: CAP dialog may be deleted only from this callback !!!
    //Stands for following signals to MSC/SGSN:  Int_Continue, Int_Error
    virtual void onEndCapDlg(TCDialogID dlg_id, RCHash errcode = 0) = 0;
};

class CapSMS_SCFContractorITF { //SSF -> CapSMSDlg --> SCF
protected:
    virtual ~CapSMS_SCFContractorITF() //forbid interface destruction
    { }

public:
    // initiates capSMS dialog
    virtual void initialDPSMS(SMSInitialDPArg* arg) throw(smsc::util::CustomException) = 0;
    // reports SMS delivery status (continues) and ends capSMS dialog
    virtual void reportSubmission(bool submitted) throw(smsc::util::CustomException) = 0;
    // aborts capSMS dialog (
    virtual void abortSMS(void) throw(smsc::util::CustomException) = 0;
};


#define CAPSMS_END_TIMEOUT  2 //seconds
//NOTE: CapSMS contract allows a prearranged end scenario (if SCF set monitorMode
//      for associated SMS event to notifyAndContinue), so CapSMSDlg sets
//      a small timeout (CAPSMS_END_TIMEOUT) on last EventReportSMS Invoke
//      and releases TC Dialog either on receiving T_END_IND or LCancel for
//      EventReportSMS.
class CapSMSDlg : SMS_SSF_Fsm, TCDialogUserITF {
public:
    //NOTE: timeout is for OPERATIONs Invokes lifetime
    CapSMSDlg(TCSessionSR* pSession, CapSMS_SSFhandlerITF * ssfHandler,
                uint16_t inv_timeout = 0, const char * scf_ident = NULL,
                Logger * uselog = NULL) _THROWS_NONE;
    virtual ~CapSMSDlg(); //Dialog is not deleted, but just released !!!

    //Allocates and initializes TCAP dialog
    RCHash Init(void) _THROWS_NONE;

    const TCDialogID & getId(void) const { return capId; }
    const char * Ident(void) const { return _logId; }
    CAPSmsStateT CAPState(void) { MutexGuard tmp(_sync); return _capState; }


    // SCFcontractor interface
    //  initiates capSMS dialog (over TCAP dialog)
    RCHash initialDPSMS(const SMSInitialDPArg * arg) _THROWS_NONE;
    //  reports delivery status(continues) and ends capSMS dialog
    RCHash reportSubmission(bool submitted) _THROWS_NONE;
    //Forcedly ends CapSMS dialog
    void abortSMS(void);

protected:
    friend class smsc::inman::inap::Dialog;
    // TCDialogUserITF interface
    void onDialogInvoke(Invoke* op, bool lastComp);
    void onDialogContinue(bool compPresent);
    void onDialogPAbort(uint8_t abortCause);
    void onDialogREnd(bool compPresent);
    void onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                        uint16_t userInfo_len, uint8_t *pUserInfo);
    void onDialogNotice(uint8_t reportCause,
                        TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                        uint8_t invId = 0, uint8_t opCode = 0);

    // InvokeListener interface (no OPs with returnResult defined)
    void onInvokeResult(InvokeRFP pInv, TcapEntity* res) { }
    void onInvokeError(InvokeRFP pInv, TcapEntity* resE);
    void onInvokeResultNL(InvokeRFP pInv, TcapEntity* res) { }
    void onInvokeLCancel(InvokeRFP pInv);
    //
    void Awake(void) { _sync.notify(); }

private:
    //Forcedly ends CapSMS dialog: sends to SCF 
    //either submission failure report or U_ABORT 
    void endCapSMS(void);
    //Ends TC dialog depending on CapSMS state, releases Dialog()
    void endTCap(bool u_abort = false);
    // reports delivery status (continues capSMS dialog)
    RCHash eventReportSMS(bool submitted) _THROWS_NONE;
    void setTimer(uint8_t new_op)
    {
        if (_timer)
            dialog->releaseInvoke(_timer);
        _timer = new_op;
    }
    void stopTimer(void) { setTimer(0); }
    void resetTimer(void)
    {
        if (_timer)
            dialog->resetInvokeTimer(_timer);
    }
    void logBadInvoke(uint8_t op_code)
    {
        smsc_log_error(logger, "%s: inconsistent %s, state %s(%s), {%s}", _logId,
                        CapSMSOp::code2Name(op_code), nmFSMState(), nmRelations(),
                        _capState.s.Print().c_str());
    }

    EventMonitor    _sync;
    TCDialogID      capId;
    //prefix for logging info
    const char *    _logPfx; //"CapSMS"
    char            _logId[sizeof("CapSMS[%u:%Xh]") + 2*sizeof(unsigned)*3 + 1];

    Dialog*         dialog;     //TCAP dialog
    TCSessionSR*    session;    //TCAP dialogs factory
    CapSMS_SSFhandlerITF* ssfHdl;
    unsigned char   rPCause;    //result or reject cause
    uint8_t         _timer;     //last initiated operation (invoke) id
    const char *    nmScf;
    Logger*         logger;
    messageType_e   reportType; //notification or request
    std::auto_ptr<SMSConnectArg> smsParams;
    uint16_t        invTimeout;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAP3SMS__ */

