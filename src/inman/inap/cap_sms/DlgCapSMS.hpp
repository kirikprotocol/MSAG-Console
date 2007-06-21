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

#include "inman/comp/cap_sms/CapSMSComps.hpp"
using smsc::util::RCHash;
using smsc::inman::comp::InitialDPSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;

#include "inman/inap/cap_sms/FSMCapSMS.hpp"

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


class CapSMS_SSFhandlerITF { //SSF <- CapSMSDlg <- SCF
//NOTE: These callbacks should not delete CapSMSDlg !!!
public:
    virtual void onDPSMSResult(unsigned dlg_id, unsigned char rp_cause = 0) = 0;
    //Dialog finalization/error handling:
    //if ercode != 0, CAP dialog is abnormally ended
    //NOTE: CAP dialog may be deleted only from this callback !!!
    virtual void onEndCapDlg(unsigned dlg_id, RCHash errcode = 0) = 0;
};

class CapSMS_SCFContractorITF { //SSF -> SCF
public:
    // initiates capSMS dialog
    virtual void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException) = 0;
    // reports SMS delivery status (continues) and ends capSMS dialog
    virtual void reportSubmission(bool submitted) throw(CustomException) = 0;
};


#define CAPSMS_END_TIMEOUT  2 //seconds
//NOTE: CapSMS contract allows a prearranged end scenario (if SCF set monitorMode
//      for associated SMS event to notifyAndContinue), so CapSMSDlg sets
//      a small timeout (CAPSMS_END_TIMEOUT) on last EventReportSMS Invoke
//      and releases TC Dialog either on receiving T_END_IND or LCancel for
//      EventReportSMS.
class CapSMSDlg : public CapSMS_SCFContractorITF, DialogListener, InvokeListener {
public:
    //NOTE: timeout is for OPERATIONs Invokes lifetime
    CapSMSDlg(TCSessionSR* pSession, CapSMS_SSFhandlerITF * ssfHandler,
                USHORT_T timeout = 0, Logger * uselog = NULL);
    virtual ~CapSMSDlg(); //Dialog is not deleted, but just released !!!

    inline unsigned getId(void) const { return capId; }

    // SCFcontractor interface
    //  initiates capSMS dialog (over TCAP dialog)
    void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException);
    //  reports delivery status(continues) and ends capSMS dialog
    void reportSubmission(bool submitted) throw(CustomException);

    void endDPSMS(void); //ends TC dialog, releases Dialog(), resets SSFhandler

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
    void endTCap(void); //ends TC dialog, releases Dialog()
    // reports delivery status (continues) and ends capSMS dialog
    void eventReportSMS(bool submitted) throw(CustomException);

    Mutex       _sync;
    unsigned    capId;
                //prefix for logging info
    char        _logId[sizeof("CapSMS[0x%X]") + sizeof(unsigned)*3 + 1];

    Dialog*         dialog;     //TCAP dialog
    TCSessionSR*    session;    //TCAP dialogs factory
    CAPSmsState     _capState;  //current state of cap3SMS CONTRACT
    CapSMS_SSFhandlerITF* ssfHdl;
    std::auto_ptr<RequestReportSMSEventArg> rrse; //keeps detection points
    messageType_e   reportType; //notification or request
    unsigned char   rPCause;    //result or reject cause
    Logger*         logger;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAP3SMS__ */

