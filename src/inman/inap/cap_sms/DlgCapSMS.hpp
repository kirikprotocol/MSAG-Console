#ident "$Id$"
/* ************************************************************************* *
 * cap3SMS CONTRACT implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_CAP3SMS__
#define __SMSC_INMAN_INAP_CAP3SMS__

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::TCSessionSR;

#include "inman/inap/cap_sms/FSMCapSMS.hpp"
using smsc::util::RCHash;
using smsc::inman::comp::CapSMSOp;
using smsc::inman::comp::InitialDPSMSArg;
using smsc::inman::comp::ConnectSMSArg;


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

//3GPP TS 23.078 version 6.x.x Release 6, clause 7.5.2.1)
class CapSMS_SSFhandlerITF { //SSF <- CapSMSDlg <- SCF
public:
    //Stands for following signals to MSC/SGSN:
    //  Int_ReleaseSMS, Int_ConnectSMS, Int_ContinueSMS
    virtual void onDPSMSResult(unsigned dlg_id, unsigned char rp_cause,
                                    std::auto_ptr<ConnectSMSArg> & sms_params) = 0;

    //if ercode != 0, CAP dialog is abnormally ended
    //NOTE: CAP dialog may be deleted only from this callback !!!
    //Stands for following signals to MSC/SGSN:  Int_Continue, Int_Error
    virtual void onEndCapDlg(unsigned dlg_id, RCHash errcode = 0) = 0;
};

class CapSMS_SCFContractorITF { //SSF -> CapSMSDlg --> SCF
public:
    // initiates capSMS dialog
    virtual void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException) = 0;
    // reports SMS delivery status (continues) and ends capSMS dialog
    virtual void reportSubmission(bool submitted) throw(CustomException) = 0;
    // aborts capSMS dialog (
    virtual void abortSMS(void) throw(CustomException) = 0;
};


#define CAPSMS_END_TIMEOUT  2 //seconds
//NOTE: CapSMS contract allows a prearranged end scenario (if SCF set monitorMode
//      for associated SMS event to notifyAndContinue), so CapSMSDlg sets
//      a small timeout (CAPSMS_END_TIMEOUT) on last EventReportSMS Invoke
//      and releases TC Dialog either on receiving T_END_IND or LCancel for
//      EventReportSMS.
class CapSMSDlg : SMS_SSF_Fsm, DialogListener, InvokeListener {
public:
    //NOTE: timeout is for OPERATIONs Invokes lifetime
    CapSMSDlg(TCSessionSR* pSession, CapSMS_SSFhandlerITF * ssfHandler,
                USHORT_T timeout = 0, const char * scf_ident = NULL,
                Logger * uselog = NULL);
    virtual ~CapSMSDlg(); //Dialog is not deleted, but just released !!!

    inline unsigned getId(void) const { return capId; }

    // SCFcontractor interface
    //  initiates capSMS dialog (over TCAP dialog)
    void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException);
    //  reports delivery status(continues) and ends capSMS dialog
    void reportSubmission(bool submitted) throw(CustomException);

    void abortSMS(void); //ends TC dialog, releases Dialog(), resets SSFhandler

protected:
    friend class Dialog;
    // DialogListener interface
    void onDialogInvoke(Invoke* op, bool lastComp);
    void onDialogContinue(bool compPresent);
    void onDialogPAbort(UCHAR_T abortCause);
    void onDialogREnd(bool compPresent);
    void onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                        USHORT_T userInfo_len, UCHAR_T *pUserInfo);
    void onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                        UCHAR_T invId = 0, UCHAR_T opCode = 0);

    // InvokeListener interface (no OPs with returnResult defined)
    void onInvokeResult(Invoke* inv, TcapEntity* res) { }
    void onInvokeError(Invoke* inv, TcapEntity* resE);
    void onInvokeResultNL(Invoke* inv, TcapEntity* res) { }
    void onInvokeLCancel(Invoke* inv);

private:
    void endTCap(bool u_abort = false); //ends TC dialog, releases Dialog()
    // reports delivery status (continues capSMS dialog)
    bool eventReportSMS(bool submitted) throw(CustomException);
    inline void setTimer(Invoke * new_op)
    {
        if (_timer)
            dialog->releaseInvoke(_timer->getId());
        _timer = new_op;
    }
    inline void stopTimer(void) { setTimer(NULL); }
    inline void resetTimer(void)
    {
        if (_timer)
            dialog->resetInvokeTimer(_timer->getId());
    }
    inline void logBadInvoke(UCHAR_T op_code)
    {
        smsc_log_error(logger, "%s: inconsistent %s, state %s(%s), {%s}", _logId,
                        CapSMSOp::code2Name(op_code), nmFSMState(), nmRelations(),
                        _capState.s.Print().c_str());
    }

    Mutex       _sync;
    unsigned    capId;
    //prefix for logging info
    char        _logId[sizeof("CapSMS[0x%X]") + sizeof(unsigned)*3 + 1];

    Dialog*         dialog;     //TCAP dialog
    TCSessionSR*    session;    //TCAP dialogs factory
    CapSMS_SSFhandlerITF* ssfHdl;
    unsigned char   rPCause;    //result or reject cause
    Invoke *        _timer;     //last initiated operation
    const char *    nmScf;
    Logger*         logger;
    messageType_e   reportType; //notification or request
    std::auto_ptr<ConnectSMSArg> smsParams;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAP3SMS__ */

