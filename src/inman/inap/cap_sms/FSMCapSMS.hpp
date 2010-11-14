/* ************************************************************************* *
 * SSF (Service Switching Function) FSM for CAP3Sms
 * 3GPP TS 23.078 version 6.x.x Release 6, clause 7.5.5
 * 3GPP TS 29.078 version 6.x.x Release 6, clause 12
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_CAP3SMS_FSM__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_CAP3SMS_FSM__

#include "inman/comp/cap_sms/CapSMSComps.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::SMSEventDPs;

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

struct CAPSmsStateS {
    enum OperationState { operNone = 0, operInited = 0x02, operFailed = 0x03 };
    enum FIdentity { idNone = 0, idSCF = 0x01, idSSF = 0x02 };

    unsigned short smsIDP  : 2;       //InitialDPSMS -> SCF
    unsigned short smsRlse : 1;       //ReleaseSMS <- SCF
    unsigned short smsCont : 1;       //ContinueSMS <- SCF
    /* */
    unsigned short smsConn : 1;       //ConnectSMS <- SCF
    unsigned short smsReqEvent : 1;   //RequestReportSMSEvent <- SCF
    unsigned short smsFCI  : 1;       //FurnishCjargingInformationSMS <- SCF
    unsigned short smsTimer  : 1;     //ResetTimerSMS <- SCF
    /* */
    unsigned short smsReport : 2;     //EventReportSMS -> SCF
    unsigned short smsEnd : 2;        //FIdentity for T_END_[REQ|IND]
    /* */
    unsigned short smsAbort : 2;      //FIdentity for T_U_ABORT_[REQ|IND] | T_P_ABORT_IND

    const std::string Print(void) const
    {
        short   i = 0;
        char    buf[sizeof(CAPSmsStateS)*8*2 + 1];

        buf[i++] = 0x30 + smsIDP; 
        buf[i++] = ','; buf[i++] = 0x30 + smsRlse;
        buf[i++] = ','; buf[i++] = 0x30 + smsCont;
        buf[i++] = ','; buf[i++] = 0x30 + smsConn;
        buf[i++] = ','; buf[i++] = 0x30 + smsReqEvent;
        buf[i++] = ','; buf[i++] = 0x30 + smsFCI;
        buf[i++] = ','; buf[i++] = 0x30 + smsTimer;
        buf[i++] = ','; buf[i++] = 0x30 + smsReport;
        buf[i++] = ','; buf[i++] = 0x30 + smsEnd;
        buf[i++] = ','; buf[i++] = 0x30 + smsAbort;
        buf[i] = 0;
        return buf;
    }
};

typedef union {
    unsigned short   value;
    CAPSmsStateS     s;
} CAPSmsStateT;

typedef std::list<EventTypeSMS_e> SMSTriggerDPs;

struct SMS_SSF_Fsm {
    enum SSFState { fsmIdle = 0, fsmWaitReq, fsmWaitInstr, fsmMonitoring, fsmDone };
    enum RelationShip { relNone = 0, relControl, relMonitor };

    SSFState        _fsmState;
    RelationShip    _relation;
    SMSEventDPs     _eDPs;  //monitored event DPs
    SMSTriggerDPs   _tDPs;  //stack of DPs the processing suspended at
    CAPSmsStateT    _capState;

    SMS_SSF_Fsm(EventTypeSMS_e org_dp) : _fsmState(fsmIdle), _relation(relNone)
    {
        _capState.value = 0;
        _tDPs.push_front(org_dp);
    }

    static const char * nmFSMState(enum SSFState fsm_state)
    {
        switch (fsm_state) {
        case SMS_SSF_Fsm::fsmDone:          return "fsmDone";
        case SMS_SSF_Fsm::fsmMonitoring:    return "fsmMonitoring";
        case SMS_SSF_Fsm::fsmWaitInstr:     return "fsmWaitInstr";
        case SMS_SSF_Fsm::fsmWaitReq:       return "fsmWaitReq";
        default:;
        }
        return "fsmIdle";
    }

    static const char * nmRelations(enum RelationShip fsm_rel)
    {
        switch (fsm_rel) {
        case SMS_SSF_Fsm::relMonitor:       return "relMonitor";
        case SMS_SSF_Fsm::relControl:       return "relControl";
        default:;
        }
        return "relNone";
    }

    inline const char * nmFSMState(void) const { return nmFSMState(_fsmState); }
    inline const char * nmRelations(void) const { return nmRelations(_relation); }

    //"%s(%s):{%s}", _fsmState, _relation, _capState 
    std::string State2Str(void) const
    {
        std::string str(nmFSMState());
        str += '('; str += nmRelations(); str += "):{";
        str +=  _capState.s.Print(); str += '}';
        return str;
    }
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_CAP3SMS_FSM__ */

