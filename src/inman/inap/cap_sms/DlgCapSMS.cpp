#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
/* ************************************************************************* *
 * cap3SMS CONTRACT implementation (over TCAP dialog)
 * ************************************************************************* */
#include <assert.h>

#include "util/vformat.hpp"
using smsc::util::format;

#include "inman/inap/cap_sms/DlgCapSMS.hpp"
using smsc::inman::comp::CapSMSOp;
using smsc::inman::comp::SMSEventDPs;
//using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;

using smsc::inman::comp::_RCS_CAPOpErrors;
using smsc::inman::comp::CAPServiceRC;
using smsc::inman::comp::_RCS_CAPService;

#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::TC_AbortCause;
using smsc::inman::inap::_RCS_TC_Abort;
using smsc::inman::inap::_RCS_TC_Report;

namespace smsc {
namespace inman {
namespace inap {

/* ************************************************************************** *
 * class CapSMSDlg implementation:
 * ************************************************************************** */
CapSMSDlg::CapSMSDlg(TCSessionSR* pSession, CapSMS_SSFhandlerITF * ssfHandler,
           USHORT_T inv_timeout/* = 0*/, const char * scf_ident/* = NULL*/,
            Logger * uselog/* = NULL*/) _THROWS_NONE
    : SMS_SSF_Fsm(EventTypeSMS_sms_CollectedInfo), _logPfx("CapSMS")
    , session(pSession), ssfHdl(ssfHandler), nmScf(scf_ident), logger(uselog)
    , rPCause(0), _timer(0), reportType(MessageType_request), dialog(0)
    , capId(0), invTimeout(inv_timeout)
{
    assert(ssfHandler && pSession);
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.CapSMS");
    if (!nmScf)
        nmScf = "SCF";
    strcpy(_logId, _logPfx);
}

CapSMSDlg::~CapSMSDlg()
{
    MutexGuard  grd(_sync);
    ssfHdl = NULL;
    endCapSMS();
    reportType = MessageType_notification; //do PREARRANED_END if TC dialog is still active
    endTCap(); 
}

void CapSMSDlg::abortSMS(void)
{
    MutexGuard  grd(_sync);
    ssfHdl = NULL;
    endCapSMS();
}

RCHash CapSMSDlg::Init(void) _THROWS_NONE
{
    //check preconfitions
    if (_fsmState != SMS_SSF_Fsm::fsmIdle)
        return _RCS_CAPService->mkhash(CAPServiceRC::contractViolation);

    //Initialize TCAP dialog
    dialog = session->openDialog();
    if (!dialog)
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit);
    capId = (unsigned)dialog->getId();
    snprintf(_logId, sizeof(_logId)-1, "%s[0x%X]", _logPfx, capId);

    dialog->setInvokeTimeout(invTimeout);
    dialog->addListener(this);
    _fsmState = SMS_SSF_Fsm::fsmWaitReq;
    return 0;
}

/* ------------------------------------------------------------------------ *
 * SCFcontractor interface
 * ------------------------------------------------------------------------ */
RCHash CapSMSDlg::initialDPSMS(const InitialDPSMSArg* arg) _THROWS_NONE
{
    MutexGuard  grd(_sync);
    //check preconfitions
    if (_fsmState == SMS_SSF_Fsm::fsmIdle) {
        RCHash rval = Init();
        if (rval)
            return rval;
    }
    if (_fsmState != SMS_SSF_Fsm::fsmWaitReq)
        return _RCS_CAPService->mkhash(CAPServiceRC::contractViolation);

    //begin TCAP dialog with given component
    smsc_log_debug(logger, "%s: --> %s{%u}: InitialDPSMS", _logId, nmScf, arg->ServiceKey());
    try {
        //Create and send IDPSm invoke, start Tssf
        _timer = dialog->sendInvoke(CapSMSOp::InitialDPSMS, arg, this); //throws
        dialog->beginDialog();  //throws
    } catch (const CustomException & c_exc) {
        _timer = 0;
        smsc_log_error(logger, "%s: %s", _logId, c_exc.what());
        return (RCHash)(c_exc.errorCode());
    } catch (...) {
        _timer = 0;
        smsc_log_error(logger, "%s: <unknown> exception", _logId);
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgParam);
    }
    //set postconditions 
    _capState.s.smsIDP = CAPSmsStateS::operInited;
    _fsmState = SMS_SSF_Fsm::fsmWaitInstr;
    _relation = SMS_SSF_Fsm::relControl;
    return 0;
}

RCHash CapSMSDlg::reportSubmission(bool submitted) _THROWS_NONE
{
    MutexGuard  grd(_sync);
    //check preconfitions
    if (_fsmState != SMS_SSF_Fsm::fsmMonitoring)
        return _RCS_CAPService->mkhash(CAPServiceRC::contractViolation);

    return eventReportSMS(submitted);
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
//Called if Operation got ResultError
void CapSMSDlg::onInvokeError(Invoke *op, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        UCHAR_T opcode = op->getOpcode();
        smsc_log_error(logger, "%s: Invoke[%u]{%u} got a returnError: %u",
            _logId, (unsigned)op->getId(), (unsigned)opcode, (unsigned)resE->getOpcode());
    
        //call operation errors handler
        switch (opcode) {
        case CapSMSOp::InitialDPSMS: {
            _capState.s.smsIDP = CAPSmsStateS::operFailed;
        }
        break;
        case CapSMSOp::EventReportSMS: {
            _capState.s.smsReport = CAPSmsStateS::operFailed;
        } break;
        default:;
        }
        endTCap(true); //send U_ABORT
    }
    if (ssfHdl)
        ssfHdl->onEndCapDlg(capId, _RCS_CAPOpErrors->mkhash(resE->getOpcode()));
}

//Called if initiated Operation got L_CANCEL (possibly while waiting result)
void CapSMSDlg::onInvokeLCancel(Invoke *op)
{
    RCHash  errcode = 0;
    bool    doAbort = false;
    {
        MutexGuard  grd(_sync);
        //NOTE:  14.1.2.2.2 gsmSSF/gsmSRF/ -to-gsmSCF messages
        //on expiration of application timer Tssf or Tsrf, dialogue shall be
        //terminated by means of by TC-U-ABORT primitive with an Abort reason,
        //regardless of TC dialogue is established or not.
        switch (_fsmState) {
        case SMS_SSF_Fsm::fsmWaitInstr: {
            errcode = _RCS_CAPService->mkhash(CAPServiceRC::noServiceResponse);
            endTCap(true);  //send U_ABORT
            doAbort = true;
        } break;

        case SMS_SSF_Fsm::fsmMonitoring: {
            errcode = _RCS_CAPService->mkhash(CAPServiceRC::monitoringTimeOut);
            endTCap(true);  //send U_ABORT
            doAbort = true;
        } break;

        case SMS_SSF_Fsm::fsmDone: { //prearranged end condition
            endTCap();
            doAbort = true;
        } break;
        default:;
        }
        //check if current timer is expired
        if (_timer == op->getId())
            _timer = 0;
    }
    if (doAbort && ssfHdl)
        ssfHdl->onEndCapDlg(capId, errcode);
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void CapSMSDlg::onDialogContinue(bool compPresent)
{
    MutexGuard  grd(_sync);
    if (!compPresent)
        smsc_log_error(logger, "%s: missing component in TC_CONT_IND", _logId);
    //else wait for ongoing Invoke result/error
    return;
}

//TCAP indicates DialogPAbort: either due to TC layer error on SSF side or
//because of TC dialog timeout is expired on SSF (no T_END_IND from SCF).
void CapSMSDlg::onDialogPAbort(UCHAR_T abortCause)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _capState.s.smsAbort |= CAPSmsStateS::idSSF;
        smsc_log_error(logger, "%s: P_ABORT{%s} at state %s", _logId, 
                        _RCS_TC_Abort->explainCode(abortCause).c_str(),
                        State2Str().c_str());
        if (_fsmState != SMS_SSF_Fsm::fsmDone)
            errcode = _RCS_TC_Abort->mkhash(abortCause);
        //else time out on T_END_IND from SCF
        endTCap();
    }
    if (ssfHdl)
        ssfHdl->onEndCapDlg(capId, errcode);
}

//SCF sent DialogUAbort (some logic error on SCF side).
void CapSMSDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    {
        MutexGuard  grd(_sync);
        _capState.s.smsAbort |= CAPSmsStateS::idSCF;
        smsc_log_error(logger, "%s: U_ABORT_IND at state %s", _logId,
                        State2Str().c_str());
        endTCap();
    }
    if (ssfHdl)
        ssfHdl->onEndCapDlg(capId, _RCS_TC_Abort->mkhash(TC_AbortCause::userAbort));
}

//Underlying layer unable to deliver message, just abort dialog
void CapSMSDlg::onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        UCHAR_T invId/* = 0*/, UCHAR_T opCode/* = 0*/)
{
    {
        MutexGuard  grd(_sync);
        _capState.s.smsAbort |= CAPSmsStateS::idSSF;
        std::string dstr;
        if (comp_kind != TcapEntity::tceNone) {
            format(dstr, ", Invoke[%u]", invId);
            switch (comp_kind) {
            case TcapEntity::tceError:      dstr += ".Error"; break;
            case TcapEntity::tceResult:     dstr += ".Result"; break;
            case TcapEntity::tceResultNL:   dstr += ".ResultNL"; break;
            default:;
            }
            dstr += " not delivered.";
        }
        smsc_log_error(logger, "%s: NOTICE_IND{%s} at state %s", _logId,
                       dstr.c_str(), State2Str().c_str());
        reportType = MessageType_notification; //do PREARRANED_END
        endTCap();
    }
    if (ssfHdl)
        ssfHdl->onEndCapDlg(capId, _RCS_TC_Report->mkhash(reportCause));
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void CapSMSDlg::onDialogREnd(bool compPresent) //_THROW_NONE
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _capState.s.smsEnd |= CAPSmsStateS::idSSF;
        if (!compPresent) {
            if (_tDPs.front() == EventTypeSMS_sms_CollectedInfo) {
                smsc_log_error(logger, "%s: T_END_IND, state %s", _logId, State2Str().c_str());
                errcode =_RCS_CAPService->mkhash(CAPServiceRC::noServiceResponse);
            }
            endTCap();
        }
    }
    if (!compPresent && ssfHdl)
        ssfHdl->onEndCapDlg(capId, errcode);
    //else wait for ongoing Invoke
}

//See cap3SMS CONTRACT description comment in CapSMSDlg.hpp
void CapSMSDlg::onDialogInvoke(Invoke* op, bool lastComp)
{
    bool    doEnd = false;
    bool    doReport = false;
    RCHash  err = 0;
    {
        MutexGuard  grd(_sync);
        if (_capState.s.smsEnd) //TC_END_IND was get
            doEnd = true;

        if (!op->getParam() && (op->getOpcode() != CapSMSOp::ContinueSMS))
            smsc_log_error(logger, "%s: <-- %s: %s { Arg missed !!! }", _logId,
                            nmScf, CapSMSOp::code2Name(op->getOpcode()));
    
        switch (op->getOpcode()) {
        case CapSMSOp::ReleaseSMS: {
            //check preconditions
            if ((_fsmState == SMS_SSF_Fsm::fsmWaitInstr) 
                && (_relation == SMS_SSF_Fsm::relControl)) {
                if (_tDPs.front() == EventTypeSMS_sms_CollectedInfo) {
                    _capState.s.smsRlse = 1;
                    ReleaseSMSArg * arg = static_cast<ReleaseSMSArg*>(op->getParam());
                    rPCause = arg->rPCause;
                    smsc_log_debug(logger, "%s: <-- %s: ReleaseSMS { RP cause: %u }",
                                    _logId, nmScf, (unsigned)rPCause);
                    doReport = doEnd = true;
                    _fsmState = SMS_SSF_Fsm::fsmDone;
                    _relation = SMS_SSF_Fsm::relNone;
                    stopTimer();
                } else { //MonitorMode_interrupted, T_END_IND was awaited
                    doEnd = true;
                    _fsmState = SMS_SSF_Fsm::fsmDone;
                    logBadInvoke(op->getOpcode());
                }
            } else { //dissynchronization, send U_ABORT to SCF
                logBadInvoke(op->getOpcode());
                err = _RCS_CAPService->mkhash(CAPServiceRC::contractViolation);
            }
        }   break;

        case CapSMSOp::ConnectSMS:
            smsParams.reset(static_cast<ConnectSMSArg*>(op->getParam()));
        case CapSMSOp::ContinueSMS: {
            //check preconditions
            if ((_fsmState == SMS_SSF_Fsm::fsmWaitInstr) 
                && (_relation == SMS_SSF_Fsm::relControl)) {
                smsc_log_debug(logger, "%s: <-- %s: %s", _logId, nmScf, 
                                    CapSMSOp::code2Name(op->getOpcode()));
                if (_tDPs.front() == EventTypeSMS_sms_CollectedInfo) {
                    if (op->getOpcode() == CapSMSOp::ContinueSMS)
                        _capState.s.smsCont = 1;
                    else
                        _capState.s.smsConn = 1;
                    //Some IN-points send ContSMS and ReqReportSMSEvent in single
                    //TC message arranged so, that ContSMS is reported by TC layer first
                    if (_capState.s.smsReqEvent || lastComp) {
                        doReport = true;
                        stopTimer();
                        if (!_eDPs.hasMMode(MonitorMode_interrupted))
                            _relation = SMS_SSF_Fsm::relMonitor;
                        _fsmState = SMS_SSF_Fsm::fsmMonitoring;
                    } //else wait RequestReportSMSEvent, but do not reset timer
                } else { //MonitorMode_interrupted, T_END_IND was awaited
                    doEnd = true;
                    _fsmState = SMS_SSF_Fsm::fsmDone;
                }
            } else { //dissynchronization, send U_ABORT to SCF
                logBadInvoke(op->getOpcode());
                err = _RCS_CAPService->mkhash(CAPServiceRC::contractViolation);
            }
        }   break;
    
        case CapSMSOp::RequestReportSMSEvent: {
            //check preconditions
            if ((_fsmState == SMS_SSF_Fsm::fsmWaitInstr) 
                && (_relation == SMS_SSF_Fsm::relControl)) {
                _capState.s.smsReqEvent = 1;
                std::string dump;
                RequestReportSMSEventArg * rrse = static_cast<RequestReportSMSEventArg*>(op->getParam());
                smsc_log_debug(logger, "%s: <-- %s: %s {%s}", _logId, nmScf, 
                                CapSMSOp::code2Name(op->getOpcode()),
                                rrse->SMSEvents().print(dump).c_str());
                //arm/disarm EDPs
                for (SMSEventDPs::const_iterator cit = rrse->SMSEvents().begin();
                                                cit != rrse->SMSEvents().end(); ++cit) {
                    if (cit->second == MonitorMode_transparent) //disarm EDP
                        _eDPs.erase(cit->first);
                    else                                        //arm EDP
                        _eDPs.insert(*cit);
                }
                if (_capState.s.smsCont || _capState.s.smsConn) {
                    doReport = true;
                    stopTimer();
                } else    //wait ContinueSMS|ConnectSMS
                    resetTimer();
            } else 
                logBadInvoke(op->getOpcode());
        }   break;
    
        case CapSMSOp::ResetTimerSMS: {
            //check preconditions
            if (_fsmState == SMS_SSF_Fsm::fsmWaitInstr) {
                _capState.s.smsTimer = 1;
                ResetTimerSMSArg* arg = static_cast<ResetTimerSMSArg*>(op->getParam());
                smsc_log_debug(logger, "%s: <-- %s: ResetTimerSMS { value: %lu secs }",
                               _logId, nmScf, (long)(arg->timerValue));
                resetTimer();
            } else
                logBadInvoke(op->getOpcode());
        }   break;
    
        case CapSMSOp::FurnishChargingInformationSMS: {
            //check preconditions
            if (_fsmState == SMS_SSF_Fsm::fsmWaitInstr) {
                _capState.s.smsFCI = 1;
                smsc_log_debug(logger, "%s: <-- %s: FurnishChargingInformationSMS {}", _logId, nmScf);
                resetTimer();
            } else
                logBadInvoke(op->getOpcode());
        }   break;

        default:
            smsc_log_error(logger, "%s: Illegal Invoke(opcode = %u), state %s(%s), {%s}",
                        _logId, op->getOpcode(), nmFSMState(), nmRelations(),
                        _capState.s.Print().c_str());
        }
    }
    if (doReport && ssfHdl)
        ssfHdl->onDPSMSResult(capId, rPCause, smsParams);
    if (doEnd) {
        endTCap((bool)(err != 0));
        if (ssfHdl)
            ssfHdl->onEndCapDlg(capId, err);
    }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//Returns true if Invoke is sent
//Reports delivery status (continues capSMS dialog)
RCHash CapSMSDlg::eventReportSMS(bool submitted) _THROWS_NONE
{
    EventTypeSMS_e eventType;
    if (submitted) {
        eventType = EventTypeSMS_o_smsSubmission;
        _eDPs.erase(EventTypeSMS_o_smsFailure); //disarm o_smsFailure EDP
    } else {
        eventType = EventTypeSMS_o_smsFailure;  //disarm o_smsSubmission EDP
        _eDPs.erase(EventTypeSMS_o_smsSubmission);
    }
    
    SMSEventDPs::iterator it = _eDPs.find(eventType);
    if (it == _eDPs.end()) { //EDP is not armed !!!
        smsc_log_error(logger, "%s: EDP %s isn't armed", _logId,
                        smsc::inman::comp::_nmEventTypeSMS(eventType));
        _relation = SMS_SSF_Fsm::relNone;
        _fsmState = SMS_SSF_Fsm::fsmDone;
        return 0;
    }

     //disarm this EDP
    _eDPs.erase(eventType);
    _tDPs.push_front(eventType);

    reportType = (it->second == MonitorMode_interrupted) ?
                    MessageType_request : MessageType_notification;

    std::string dump;
    EventReportSMSArg    arg(eventType, reportType);
    smsc_log_debug(logger, "%s: --> %s: EventReportSMS %s", _logId, nmScf, arg.print(dump).c_str());
    UCHAR_T invId = 0;
    try {
        invId = dialog->sendInvoke(CapSMSOp::EventReportSMS, &arg, this,
                (reportType == MessageType_request) ? 0/*dflt*/ : CAPSMS_END_TIMEOUT); //throws
        dialog->continueDialog();   //throws
    } catch (const CustomException & c_exc) {
        smsc_log_error(logger, "%s: %s", _logId, c_exc.what());
        setTimer(0);
        return (RCHash)(c_exc.errorCode());
    } catch (...) {
        smsc_log_error(logger, "%s: <unknown> exception", _logId);
        setTimer(0);
        return _RCS_TC_Dialog->mkhash(TC_DlgError::dlgParam);
    }
    //set postconfitions
    setTimer(invId);
    _capState.s.smsReport = CAPSmsStateS::operInited;
    if (reportType == MessageType_request) {
        _fsmState = SMS_SSF_Fsm::fsmWaitInstr; //control relationship
    } else {
        _fsmState = SMS_SSF_Fsm::fsmDone;
        _relation = SMS_SSF_Fsm::relNone;
    }
    return 0;
}

//Forcedly ends CapSMS dialog: sends to SCF either submission failure report or U_ABORT 
void CapSMSDlg::endCapSMS(void)
{
    //in monitoring state send submission failure report
    //and wait for T_END_IND or L_CANCEL
    if (_fsmState == SMS_SSF_Fsm::fsmMonitoring) {
        if (eventReportSMS(false))
            endTCap(true); //send 
    } else if ((_fsmState == SMS_SSF_Fsm::fsmWaitInstr) && !_capState.s.smsReport)
        endTCap(true); //send U_ABORT to SCF
}

//Ends TC dialog depending on CapSMS state, releases Dialog()
void CapSMSDlg::endTCap(bool u_abort/* = false*/)
{
    _fsmState = SMS_SSF_Fsm::fsmDone;
    _relation = SMS_SSF_Fsm::relNone;
    _timer = 0;
    if (dialog) {
        dialog->removeListener(this);
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            //see 3GPP 29.078 14.1.2 smsSSF-to-gsmSCF SMS related messages
            try {
                if (u_abort) {
                    _capState.s.smsAbort |= CAPSmsStateS::idSSF;
                    smsc_log_error(logger, "%s: U_ABRT_REQ, state %s", _logId,
                                    State2Str().c_str());
                    dialog->endDialog(Dialog::endUAbort);
                } else {
                    _capState.s.smsEnd |= CAPSmsStateS::idSSF;
                    smsc_log_debug(logger, "%s: T_END_REQ, state %s", _logId,
                                    State2Str().c_str());
                    dialog->endDialog((reportType == MessageType_request) ?
                                    Dialog::endBasic : Dialog::endPrearranged);
                }
            } catch (std::exception & exc) {
                smsc_log_error(logger, "%s: %s: %s", _logId, 
                            u_abort ? "U_ABRT_REQ": "T_END_REQ", exc.what());
                dialog->releaseAllInvokes();
            }
        }
        session->releaseDialog(dialog);
        dialog = NULL;
    }
}

} //inap
} //inman
} //smsc

