#ident "$Id$"
// cap3SMS CONTRACT implementation (over TCAP dialog)

#ifndef __SMSC_INMAN_INAP_INAP__
#define __SMSC_INMAN_INAP_INAP__

#include <map>

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/comps.hpp"

using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::FurnishChargingInformationSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;
using smsc::inman::comp::InitialDPSMSArg;
using smsc::inman::inap::Session;

namespace smsc {
namespace inman {
namespace inap {

/* cap3SMS CONTRACT:    Inap(SSF) <-> In-platform(SCF)

->  smsActivationPackage:
        InitialDP -> SCF
[ <-  smsConnectPackage:
        SSF <- connectSMS ]
<-  smsProcessingPackage:
        SSF <- ContinueSMS | ReleaseSMS
[ <-> smsEventHandlingPackage
        SSF <- RequestReportSMSEvent
        eventReportSMS -> SCF       ]
[ <- smsBillingPackage:
        SSF <- FurnishChargingInformationSMS ]
[ <- smsTimerPackage:
        SSF <- ResetTimerSMS ]
*/

class SSFhandler
{
  public: //operations:
    virtual void onConnectSMS(ConnectSMSArg* arg) = 0;
    virtual void onContinueSMS() = 0;
    virtual void onFurnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg) = 0;
    virtual void onReleaseSMS(ReleaseSMSArg* arg) = 0;
    virtual void onRequestReportSMSEvent(RequestReportSMSEventArg* arg) = 0;
    virtual void onResetTimerSMS(ResetTimerSMSArg* arg) = 0;
    virtual void onAbortSMS(unsigned char ercode, bool tcapLayer) = 0;
};

class SCFcontractor
{
  public: //operations:
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
    //error handling:
    virtual void onOperationError(Invoke *op, TcapEntity * resE) = 0;
    virtual void onOperationLCancel(Invoke *op) = 0;
};

typedef union {
    unsigned short value;
    struct {
        unsigned int ctrInited : 2;
        unsigned int ctrReleased : 1;
        unsigned int ctrContinued : 1;
        unsigned int ctrRequested : 1;
        unsigned int ctrReported : 1;
        unsigned int ctrFinished : 1;
        unsigned int ctrAborted : 1;
    } s;
} CAP3State;

typedef union {
    unsigned short value;
    struct {
        unsigned int dlgInited : 1;
        unsigned int dlgRContinued : 2; //1: T_CONT_IND, 2: + INVOKE_IND
        unsigned int dlgLContinued : 1; //1: T_CONT_REQ
        unsigned int dlgREnded : 2;     //1: T_END_IND, 2: + INVOKE_IND
        unsigned int dlgLEnded : 1;     //1: T_END_REQ
        unsigned int dlgAborted : 1;
    } s;
} TCAPState;

class InapOpResListener;
//NOTE: Inap doesn't maintain own timer for operations, it uses instead the 
//innate timer of the SS7 stack for Invoke lifetime.
class Inap : public DialogListener, public SCFcontractor
{
public:
    //NOTE: timeout is for OPERATIONs Invokes lifetime
    Inap(Session* pSession, SSFhandler * ssfHandler,
         USHORT_T timeout = 0, Logger * uselog = NULL);
    virtual ~Inap();

    // SCFcontractor interface
    void initialDPSMS(InitialDPSMSArg* arg);     //begins TCAP dialog
    void eventReportSMS(EventReportSMSArg* arg); //continues TCAP dialog
    void onOperationError(Invoke *op, TcapEntity * resE);
    void onOperationLCancel(Invoke *op);

    // DialogListener interface
    virtual void onDialogInvoke(Invoke* op, bool lastComp);
    virtual void onDialogContinue(bool compPresent);
    virtual void onDialogPAbort(UCHAR_T abortCause);
    virtual void onDialogREnd(bool compPresent);

    Invoke* initOperation(UCHAR_T opcode);
    void    releaseOperation(Invoke * inv);

protected:
    void    releaseAllOperations(void);

    typedef std::map<USHORT_T, InapOpResListener*> ResultHandlersMAP;
    Dialog*     dialog;     //TCAP dialog
    Session*    session;    //TCAP dialogs factory
    Logger*     logger;
    SSFhandler* ssfHdl;
    ResultHandlersMAP resHdls;
    CAP3State   _capState;  //current state of cap3SMS CONTRACT
    TCAPState   _dlgState;  //current state of TCAP Dialog
};

class InapOpResListener: public InvokeListener
{
public:
    InapOpResListener(Invoke * op, Inap * pInap);
    ~InapOpResListener() {}

    Invoke * getOrgInv() const;
    //Gets result from TCAP 
    void error(TcapEntity* err);
    void result(TcapEntity* resL) {}
    void resultNL(TcapEntity* resNL) {}
    void lcancel(void);

protected:
    Invoke * orgInv;    //originating Invoke
    Inap   * orgInap;   //parent Inap
};


} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_INAP__ */

