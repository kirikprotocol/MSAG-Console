#ident "$Id$"
/* ************************************************************************* *
 * CAPSmsDialog states definition and helper classes for its analysis
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_CAP3SMS_FSM__
#define __SMSC_INMAN_INAP_CAP3SMS_FSM__

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

struct CAPSmsStateS {
    enum OperationState { operInited = 0x02, operFailed = 0x03 };

    unsigned int ctrInited : 2;     //InitialDPSMS -> SCF
    unsigned int ctrReleased : 1;   //ReleaseSMS <- SCF
    unsigned int ctrContinued : 1;  //ContinueSMS <- SCF
    // ...
    unsigned int ctrRequested : 1;  //RequestReportSMSEvent <- SCF
    unsigned int ctrReported : 2;   //EventReportSMS -> SCF
    unsigned int ctrFinished : 1;   //T_END_[REQ|IND]
    // ...
    unsigned int ctrAborted : 1;    //T_U_ABORT_IND | T_P_ABORT_IND
    unsigned int reserved : 7;
};

typedef union {
    unsigned short   value;
    CAPSmsStateS     s;
} CAPSmsStateT;


//Singleton that computes masks for CAPSmsStateS in order to avoid care on endianness.
class CAPSmsStateMask {
protected:
    unsigned short pre_connect;
    unsigned short pre_release1;
    unsigned short pre_release2;
    unsigned short pre_continue1;
    unsigned short pre_continue2;
    unsigned short pre_request1;
    unsigned short pre_request2;
    unsigned short pre_report;

    void* operator new(size_t);
    CAPSmsStateMask()    
    {
        CAPSmsStateT     usv;
        //pre_connect: T_BEGIN_REQ {InitialDPSms}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        pre_connect = usv.value;
        //pre_release1: T_BEGIN_REQ {InitialDPSms}, T_CONT_IND {}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        pre_release1 = usv.value;
        //pre_release2: T_BEGIN_REQ {InitialDPSms}, T_END_IND {}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        usv.s.ctrFinished = 1;
        pre_release2 = usv.value;
        //pre_continue1: T_BEGIN_REQ {InitialDPSms}, T_CONT_IND {}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        pre_continue1 = usv.value;
        //pre_continue2: T_BEGIN_REQ {InitialDPSms}, T_CONT_IND {RequestReport}, T_CONT_IND {}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        usv.s.ctrRequested = 1;
        pre_continue2 = usv.value;
        //pre_request1: T_BEGIN_REQ {InitialDPSms}, T_CONT_IND
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        pre_request1 = usv.value;
        //pre_request2: T_BEGIN_REQ {InitialDPSms}, T_CONT_IND {ContinueSms}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        usv.s.ctrContinued = 1;
        pre_request2 = usv.value;
        //pre_report: T_BEGIN_REQ {InitialDPSms}, T_CONT_IND {RequestReport, ContinueSms}
        usv.value = 0;
        usv.s.ctrInited = CAPSmsStateS::operInited;
        usv.s.ctrContinued = usv.s.ctrRequested = 1;
        pre_report = usv.value;
    }
    ~CAPSmsStateMask() { }

    static CAPSmsStateMask * getInstance(void)
    {
        static CAPSmsStateMask inst;
        return &inst;
    }

public:
    static bool preConnect(const CAPSmsStateT & usv)
    {
        CAPSmsStateMask * mask = CAPSmsStateMask::getInstance();
        return (usv.value ^ mask->pre_connect) ? false : true;
    }
    static bool preRelease(const CAPSmsStateT & usv)
    {
        CAPSmsStateMask * mask = CAPSmsStateMask::getInstance();
        return ((usv.value ^ mask->pre_release1)
                && (usv.value ^ mask->pre_release2)) ? false : true;
    }
    static bool preContinue(const CAPSmsStateT & usv)
    {
        CAPSmsStateMask * mask = CAPSmsStateMask::getInstance();
        return ((usv.value ^ mask->pre_continue1)
                && (usv.value ^ mask->pre_continue2)) ? false : true;
    }
    static bool preRequest(const CAPSmsStateT & usv)
    {
        CAPSmsStateMask * mask = CAPSmsStateMask::getInstance();
        return ((usv.value ^ mask->pre_request1)
                && (usv.value ^ mask->pre_request2)) ? false : true;
    }
    static bool preReport(const CAPSmsStateT & usv)
    {
        CAPSmsStateMask * mask = CAPSmsStateMask::getInstance();
        return (usv.value ^ mask->pre_report) ? false : true;
    }
};

class CAPSmsState {
protected:
    char    buf[sizeof(CAPSmsStateS)*8*2 + 1];

public:
    CAPSmsStateT    u;

    CAPSmsState(void) { u.value = 0; }
    ~CAPSmsState(void) { }

    inline bool preConnect(void) const  { return CAPSmsStateMask::preConnect(u); }
    inline bool preRelease(void) const  { return CAPSmsStateMask::preRelease(u); }
    inline bool preContinue(void) const { return CAPSmsStateMask::preContinue(u); }
    inline bool preRequest(void) const  { return CAPSmsStateMask::preRequest(u); }
    inline bool preReport(void) const   { return CAPSmsStateMask::preReport(u); }
    inline bool Ended(void) const       { return (u.s.ctrFinished != 0); }
    inline bool Aborted(void) const     { return (u.s.ctrAborted != 0); }
    inline bool Closed(void) const      { return (u.s.ctrFinished || u.s.ctrAborted); }

    const char * Print(void)
    {
        short   i = 0;
        buf[i++] = 0x30 + u.s.ctrInited; 
        buf[i++] = ','; buf[i++] = 0x30 + u.s.ctrReleased;
        buf[i++] = ','; buf[i++] = 0x30 + u.s.ctrContinued;
        buf[i++] = ','; buf[i++] = 0x30 + u.s.ctrRequested;
        buf[i++] = ','; buf[i++] = 0x30 + u.s.ctrReported;
        buf[i++] = ','; buf[i++] = 0x30 + u.s.ctrFinished;
        buf[i++] = ','; buf[i++] = 0x30 + u.s.ctrAborted;
        buf[i] = 0;
        return buf;
    }
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAP3SMS_FSM__ */

