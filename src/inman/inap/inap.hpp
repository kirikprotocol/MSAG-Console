#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_INAP__
#define __SMSC_INMAN_INAP_INAP__

#include <map>

#include "inman/inap/dialog.hpp"
#include "inman/comp/comps.hpp"

using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::FurnishChargingInformationSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;
using smsc::inman::comp::InitialDPSMSArg;

namespace smsc {
namespace inman {
namespace inap {

class SSF
{
  public:
    virtual void connectSMS(ConnectSMSArg* arg) = 0;
    virtual void continueSMS() = 0;
    virtual void furnishChargingInformationSMS(FurnishChargingInformationSMSArg* arg) = 0;
    virtual void releaseSMS(ReleaseSMSArg* arg) = 0;
    virtual void requestReportSMSEvent(RequestReportSMSEventArg* arg) = 0;
    virtual void resetTimerSMS(ResetTimerSMSArg* arg) = 0;
    virtual void abortSMS(unsigned char ercode) = 0;
};

class SCF
{
  public:
    virtual void initialDPSMS(InitialDPSMSArg* arg) = 0;
    virtual void eventReportSMS(EventReportSMSArg* arg) = 0;
    virtual void onOperationError(Invoke *op, TcapEntity * resE) = 0;
};

class InapOpResListener;
class Inap : public DialogListener, public SCF
{
  public:
    Inap(Dialog* dialog, SSF * ssfHandler);
    virtual ~Inap();

    // SCF interface
    void initialDPSMS(InitialDPSMSArg* arg);
    void eventReportSMS(EventReportSMSArg* arg);
    void onOperationError(Invoke *op, TcapEntity * resE);
    // DialogListener interface
    virtual void onDialogInvoke( Invoke* op ); 

 protected:
    typedef std::map<USHORT_T, InapOpResListener*> ResultHandlersMAP;
    Dialog*     dialog;
    Logger*     logger;
    SSF*        ssfHdl;
    ResultHandlersMAP resHdls;
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
    void resultNL(TcapEntity* resNL) {};

protected:
    Invoke * orgInv;    //originating Invoke
    Inap   * orgInap;   //parent Inap
};


} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_INAP__ */

