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

/* cap3SMS CONTRACT:   SCF <-> SSF

->  smsActivationPackage:
        InitialDP -> SSF
[ <-  smsConnectPackage:
        SCF <- connectSMS ]
<-  smsProcessingPackage:
        SCF <- ContinueSMS | ReleaseSMS
[ <-> smsEventHandlingPackage
        SCF <- RquestReportSMSEvent
        eventReportSMS -> SSF       ]
[ <- smsBillingPackage:
        SCF <- FurnishChargingInformationSMS ]
[ <- smsTimerPackage:
        SCF <- ResetTimerSMS ]
*/

class SSF
{
  public: //operations:
    virtual void connectSMS(ConnectSMSArg* arg) = 0;
    virtual void continueSMS() = 0;
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg) = 0;
    virtual void releaseSMS(ReleaseSMSArg* arg) = 0;
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg) = 0;
    virtual void resetTimerSMS(ResetTimerSMSArg* arg) = 0;
    virtual void abortSMS(unsigned char ercode, bool tcapLayer) = 0;
};

class SCF
{
  public: //operations:
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
    //error handling:
    virtual void onOperationError(Invoke *op, TcapEntity * resE) = 0;
    virtual void onOperationLCancel(Invoke *op) = 0;
};

class InapOpResListener;
class Inap : public DialogListener, public SCF
{
public:
    typedef enum {
        ctrIdle = 0, ctrInited = 1, ctrReleased, ctrContinued,
        ctrRequested, ctrReported, ctrFinished, ctrAborted
    } SCFState;
    //timeout for OPERATIONs Invoke
    Inap(Session* pSession, SSF * ssfHandler, USHORT_T timeout = 0, Logger * uselog = NULL);
    virtual ~Inap();

    // SCF interface
    void initialDPSMS(InitialDPSMSArg* arg);     //begins TCAP dialog
    void eventReportSMS(EventReportSMSArg* arg); //continues TCAP dialog
    void onOperationError(Invoke *op, TcapEntity * resE);
    void onOperationLCancel(Invoke *op);

    // DialogListener interface
    virtual void onDialogInvoke(Invoke* op);
    virtual void onDialogPAbort(UCHAR_T abortCause);
    virtual void onDialogREnd(bool compPresent);

    Invoke* initOperation(UCHAR_T opcode);
    void    releaseOperation(Invoke * inv);

protected:
    void    releaseAllOperations(void);
protected:
    typedef std::map<USHORT_T, InapOpResListener*> ResultHandlersMAP;
    Dialog*     dialog;     //TCAP dialog
    Session*    session;    //TCAP dialogs factory
    Logger*     logger;
    SSF*        ssfHdl;
    ResultHandlersMAP resHdls;
    SCFState    _state;     //current state of cap3SMS CONTRACT
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

