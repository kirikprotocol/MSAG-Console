#ident "$Id$"
// cap3SMS CONTRACT implementation (over TCAP dialog)

#ifndef __SMSC_INMAN_INAP_CAPSMS__
#define __SMSC_INMAN_INAP_CAPSMS__

#include "core/synchronization/Mutex.hpp"
#include "inman/inerrcodes.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/cap_sms/CapSMSComps.hpp"

using smsc::core::synchronization::Mutex;

using smsc::inman::InmanErrorType;
using smsc::inman::_InmanErrorSource;

using smsc::inman::comp::InitialDPSMSArg;

using smsc::inman::inap::TCSessionSR;

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
        SSF <- FurnishChargingInformationSMS ]  -- unused by known IN-points !!!
[ <- smsTimerPackage:
        SSF <- ResetTimerSMS ]
*/


class CapSMS_SSFhandlerITF { //SSF <- CapSMSDlg <- SCF
//NOTE: These callbacks should not delete CapSMSDlg !!!
public:
    virtual void onDPSMSResult(unsigned char rp_cause = 0) = 0;
    //dialog finalization/error handling:
    virtual void onEndCapDlg(unsigned char ercode = 0, InmanErrorType errLayer = smsc::inman::errOk) = 0;
};


class CapSMS_SCFContractorITF { //SSF -> SCF
public: //operations:
    virtual void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException) = 0;
    virtual void eventReportSMS(bool submitted) throw(CustomException) = 0;
};

#define CAP_OPER_INITED 0x2 //'10'B  on BIG-ENDIAN
#define CAP_OPER_FAIL   0x3 //'11'B  on BIG-ENDIAN 
typedef union {
    unsigned short value;
    struct {
        unsigned int ctrInited : 2;     //InitialDPSMS -> SCF
        unsigned int ctrReleased : 1;   //ReleaseSMS <- SCF
        unsigned int ctrContinued : 1;  //ContinueSMS <- SCF
        // ...
        unsigned int ctrRequested : 1;  //RequestReportSMSEvent <- SCF
        unsigned int ctrReported : 2;   //EventReportSMS -> SCF
        unsigned int ctrFinished : 1;
        // ...
        unsigned int ctrAborted : 1;
        unsigned int reserved : 7;
    } s;
#define PRE_CONNECT_SMS_MASK    0x8000

#define PRE_RELEASE_SMS_MASK1   0x8000
#define PRE_RELEASE_SMS_MASK2   0x8100

#define PRE_CONTINUE_SMS_MASK1  0x8000
#define PRE_CONTINUE_SMS_MASK2  0x8800

#define PRE_REQUEST_EVENT_MASK1 0x8000
#define PRE_REQUEST_EVENT_MASK2 0x9000
} CAP3State;


#define CAPSMS_END_TIMEOUT  2 //seconds
//NOTE: CapSMS contract allows a prearranged end scenario, so CapSMSDlg sets
//      a small timeout (CAPSMS_END_TIMEOUT) on last EventReportSMS Invoke
//      and releases TC Dialog either on receiving T_END_IND or LCancel for
//      EventReportSMS.
class CapSMSDlg : public CapSMS_SCFContractorITF, DialogListener, InvokeListener {
public:
    //NOTE: timeout is for OPERATIONs Invokes lifetime
    CapSMSDlg(TCSessionSR* pSession, CapSMS_SSFhandlerITF * ssfHandler,
                USHORT_T timeout = 0, Logger * uselog = NULL);
    virtual ~CapSMSDlg(); //Dialog is not deleted, but just released !!!
    enum {
        smsContractViolation = 0
    };

    unsigned getId(void) const { return capId; }

    // SCFcontractor interface
    //  initiates capSMS dialog (over TCAP dialog)
    void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException);
    //  reports delivery status(continues) and ends capSMS dialog
    void eventReportSMS(bool submitted) throw(CustomException);

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

    Mutex       _sync;
    unsigned    capId;
    Dialog*     dialog;     //TCAP dialog
    TCSessionSR* session;   //TCAP dialogs factory
    Logger*     logger;
    CAP3State   _capState;  //current state of cap3SMS CONTRACT
    CapSMS_SSFhandlerITF* ssfHdl;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAPSMS__ */

