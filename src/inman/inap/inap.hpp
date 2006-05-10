#ident "$Id$"
// cap3SMS CONTRACT implementation (over TCAP dialog)

#ifndef __SMSC_INMAN_INAP_CAPSMS__
#define __SMSC_INMAN_INAP_CAPSMS__

#include "core/synchronization/Mutex.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/cap_sms/CapSMSComps.hpp"

using smsc::core::synchronization::Mutex;

using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::FurnishChargingInformationSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;
using smsc::inman::comp::InitialDPSMSArg;
using smsc::inman::inap::SSNSession;

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
        SSF <- FurnishChargingInformationSMS ]
[ <- smsTimerPackage:
        SSF <- ResetTimerSMS ]
*/

typedef enum {
    layerTCAP = 3, layerCAP3, layerTCuser, layerCAPuser
} CAPErrorSource; //cooordinated with InmanErrorType !!!
extern const char * _CAPSmsLayer[];

class SSFhandler { //SSF <- SCF
//NOTE: These callbacks should not delete CapSMSDlg !!!
public:
    //CapSMS operations:
    virtual void onConnectSMS(ConnectSMSArg* arg) = 0;
    virtual void onContinueSMS(void) = 0;
    virtual void onFurnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg) = 0;
    virtual void onReleaseSMS(ReleaseSMSArg* arg) = 0;
    virtual void onRequestReportSMSEvent(RequestReportSMSEventArg* arg) = 0;
    virtual void onResetTimerSMS(ResetTimerSMSArg* arg) = 0;
    //dialog finalization/error handling:
    virtual void onEndSMS(bool approved = true) = 0;
    virtual void onAbortSMS(unsigned char ercode, CAPErrorSource errLayer) = 0;
};

class SCFcontractor { //SSF -> SCF
public: //operations:
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
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
//NOTE: CapSMSDlg doesn't maintain own timer for operations, it uses instead the 
//      innate timer of the SS7 stack for Invoke lifetime.
//      CapSMSDlg forcedly releases Invokes only on T_END_IND or P_ABORT_IND,
//      otherwise Invokes left as pending ones until LCancel_IND
class CapSMSDlg : public SCFcontractor, DialogListener, InvokeListener {
public:
    //NOTE: timeout is for OPERATIONs Invokes lifetime
    CapSMSDlg(SSNSession* pSession, SSFhandler * ssfHandler,
         USHORT_T timeout = 0, Logger * uselog = NULL);
    //NOTE: ~CapSMSDlg() doesn't delete Dialog, it just releases it !!!
    virtual ~CapSMSDlg();
    enum {
        smsContractViolation = 0
    };

    // SCFcontractor interface
    //  inittiate capSMS starting TCAP dialog
    void initialDPSMS(InitialDPSMSArg* arg) throw(CustomException);
    //  continues and ends TCAP dialog
    void eventReportSMS(EventReportSMSArg* arg) throw(CustomException);

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
    SSNSession* session;    //TCAP dialogs factory
    Logger*     logger;
    SSFhandler* ssfHdl;
    CAP3State   _capState;  //current state of cap3SMS CONTRACT
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAPSMS__ */

