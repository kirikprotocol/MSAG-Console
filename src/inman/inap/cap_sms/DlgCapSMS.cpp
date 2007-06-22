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
           USHORT_T timeout/* = 0*/, Logger * uselog/* = NULL*/)
    : ssfHdl(ssfHandler), session(pSession), reportType(MessageType_notification)
    , rPCause(0), logger(uselog)
{
    assert(ssfHandler && pSession);
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.CapSMSDlg");
    dialog = session->openDialog();
    if (!dialog)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "capSMS", "unable to create TC dialog");
    capId = (unsigned)dialog->getId();
    snprintf(_logId, sizeof(_logId)-1, "CapSMS[0x%X]", capId);
    
    dialog->setInvokeTimeout(timeout);
    dialog->addListener(this);
}

CapSMSDlg::~CapSMSDlg()
{
    MutexGuard  grd(_sync);
    ssfHdl = NULL;
    endTCap();
}

void CapSMSDlg::endDPSMS(void)
{
    MutexGuard  grd(_sync);
    if (_capState.preReport())
        eventReportSMS(false); //and wait for T_END_IND or L_CANCEL
    else {
        endTCap();
        ssfHdl = NULL;
    }
}

/* ------------------------------------------------------------------------ *
 * SCFcontractor interface
 * ------------------------------------------------------------------------ */
void CapSMSDlg::initialDPSMS(InitialDPSMSArg* arg) throw(CustomException)
{
    MutexGuard  grd(_sync);
    smsc_log_debug(logger, "%s: --> SCF InitialDPSMS", _logId);
    Invoke* op = dialog->initInvoke(CapSMSOp::InitialDPSMS, this);
    op->setParam(arg);
    dialog->sendInvoke(op); //throws

    dialog->beginDialog();  //throws
    _capState.u.s.ctrInited = CAPSmsStateS::operInited;
}

void CapSMSDlg::reportSubmission(bool submitted) throw(CustomException)
{
    MutexGuard  grd(_sync);
    if (!_capState.preReport())
        throw CustomException((int)
            _RCS_CAPService->mkhash(CAPServiceRC::contractViolation),
            _logId, rc2Txt_CapService(CAPServiceRC::contractViolation));
    eventReportSMS(submitted); //throws
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
            _capState.u.s.ctrInited = CAPSmsStateS::operFailed;
        }
        break;
        case CapSMSOp::EventReportSMS: {
            _capState.u.s.ctrReported = CAPSmsStateS::operFailed;
        } break;
        default:;
        }
        endTCap();
    }
    ssfHdl->onEndCapDlg(capId, _RCS_CAPOpErrors->mkhash(resE->getOpcode()));
}

//Called if Operation got L_CANCEL, possibly while waiting result
void CapSMSDlg::onInvokeLCancel(Invoke *op)
{
    RCHash  errcode = 0;
    bool    doAbort = false;
    {
        MutexGuard  grd(_sync);
        if (CapSMSOp::InitialDPSMS == op->getOpcode()) {
            //It's need to check for CapSMSDlg state, to handle possible CAP3 timeout expiration
            if (!(_capState.u.s.ctrReleased
                  || _capState.u.s.ctrContinued || _capState.Closed())) {
                smsc_log_error(logger, "%s: timed out expecting smsProcessingPackage "
                                       "operation, state {%s}(TC: 0x%x)", _logId,
                                _capState.Print(), dialog->getState().value);
                endTCap();
                doAbort = true;
                errcode = _RCS_CAPService->mkhash(CAPServiceRC::noServiceResponse);
            }
        } else if (CapSMSOp::EventReportSMS == op->getOpcode()) {
            //prearranged end condition
            endTCap();
            doAbort = true;
        }
    }
    if (doAbort)
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

//TCAP indicates DialogPAbort: either due to TC layer error on SCF/SSF side or
//because of TC dialog timeout is expired on SSF (no T_END_IND from SCF).
void CapSMSDlg::onDialogPAbort(UCHAR_T abortCause)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _capState.u.s.ctrAborted = 1;
        smsc_log_error(logger, "%s: P_ABORT at state {%s}", _logId, _capState.Print());
        endTCap();
        if ((_capState.u.s.ctrReported != CAPSmsStateS::operInited) && !_capState.u.s.ctrFinished)
            errcode = _RCS_TC_Abort->mkhash(abortCause);
        //else T_END_IND is timed out
    }
    ssfHdl->onEndCapDlg(capId, errcode);
}

//SCF sent DialogUAbort (some logic error on SCF side).
void CapSMSDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    {
        MutexGuard  grd(_sync);
        _capState.u.s.ctrAborted = 1;
        smsc_log_error(logger, "%s: U_ABORT(%s) at state {%s}", _logId,
                        !abortInfo_len ? "userInfo" : _RCS_TC_Abort->code2Txt(*pAbortInfo),
                        _capState.Print());
        endTCap();
    }
    ssfHdl->onEndCapDlg(capId, _RCS_TC_Abort->mkhash(!abortInfo_len ?
                        TC_AbortCause::userAbort : *pAbortInfo));
}

//Underlying layer unable to deliver message, just abort dialog
void CapSMSDlg::onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        UCHAR_T invId/* = 0*/, UCHAR_T opCode/* = 0*/)
{
    {
        MutexGuard  grd(_sync);
        _capState.u.s.ctrAborted = 1;
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
        smsc_log_error(logger, "%s: NOTICE_IND at state {%s}, %s", _logId,
                       _capState.Print(), dstr.c_str());
        endTCap();
    }
    ssfHdl->onEndCapDlg(capId, _RCS_TC_Report->mkhash(reportCause));
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void CapSMSDlg::onDialogREnd(bool compPresent)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _capState.u.s.ctrFinished = 1;
        if (!compPresent) {
            endTCap();
            if ((_capState.u.s.ctrReported != CAPSmsStateS::operInited) && !_capState.u.s.ctrReleased) {
                smsc_log_error(logger, "%s: T_END_IND, state {%s}", _logId, _capState.Print());
                errcode =_RCS_CAPService->mkhash(CAPServiceRC::noServiceResponse);
            }
        }
    }
    if (!compPresent)
        ssfHdl->onEndCapDlg(capId, errcode);
    //else wait for ongoing Invoke
}

//See cap3SMS CONTRACT description comment in CapSMSDlg.hpp
void CapSMSDlg::onDialogInvoke(Invoke* op, bool lastComp)
{
    bool    doAbort = false;
    bool    doReport = false;
    RCHash  err = 0;
    {
        MutexGuard  grd(_sync);
    
        if (!op->getParam() && (op->getOpcode() != CapSMSOp::ContinueSMS))
            smsc_log_error(logger, "%s: Invoke(opcode = %u) arg missed!", _logId, op->getOpcode());
    
        switch (op->getOpcode()) {
        case CapSMSOp::ConnectSMS: {
            if (_capState.preConnect())
                smsc_log_debug(logger, "%s: <-- SCF ConnectSMS", _logId);
            else
                doAbort = true;
        }   break;
    
        case CapSMSOp::ReleaseSMS: {
            if (_capState.preRelease() && op->getParam()) {
                _capState.u.s.ctrReleased = 1;
                ReleaseSMSArg * arg = static_cast<ReleaseSMSArg*>(op->getParam());
                smsc_log_debug(logger, "%s: <-- SCF ReleaseSMS, RP cause: %u",
                                _logId, (unsigned)arg->rPCause);
                rPCause = arg->rPCause;
                doReport = true;
            } else
                doAbort = true;
        }   break;
    
        case CapSMSOp::ContinueSMS: {
            if (_capState.preContinue()) {
                _capState.u.s.ctrContinued = 1;
                smsc_log_debug(logger, "%s: <-- SCF ContinueSMS", _logId);
                if (_capState.u.s.ctrRequested)
                    doReport = true;
                //else wait RequestReportSMSEvent
            } else
                doAbort = true;
        }   break;
    
        case CapSMSOp::RequestReportSMSEvent: {
            if (_capState.preRequest() && op->getParam()) {
                _capState.u.s.ctrRequested = 1;
                rrse.reset(static_cast<RequestReportSMSEventArg*>(op->getParam()));
                op->setParam(NULL); //take ownership
                std::string dump;
                smsc_log_debug(logger, "%s: <-- SCF RequestReportSMSEvent (%s)",
                               _logId, (rrse->printEvents(dump)).c_str());
                if (_capState.u.s.ctrContinued)
                    doReport = true;
                //else wait ContinueSMS
            } else
                doAbort = true;
        }   break;
    
        case CapSMSOp::ResetTimerSMS: {
            ResetTimerSMSArg* arg = static_cast<ResetTimerSMSArg*>(op->getParam());
            smsc_log_debug(logger, "%s: <-- SCF ResetTimerSMS, value: %lu secs",
                           _logId, (long)(arg->timerValue));
        }   break;
    
        case CapSMSOp::FurnishChargingInformationSMS: {
            smsc_log_debug(logger, "%s: <-- SCF FurnishChargingInformationSMS", _logId);
        }   break;

        default:
            smsc_log_error(logger, "%s: illegal Invoke(opcode = %u)",
                _logId, op->getOpcode());
        }
        if (doAbort) {
            /* NOTE: there is a known problem with Alcatel IN: it sends *
             * END_REQ carrying excessive ContinueSMS component.        */
            logger->log(_capState.u.s.ctrFinished ? Logger::LEVEL_WARN : Logger::LEVEL_ERROR,
                        "%s: inconsistent %s, state {%s}", _logId,
                        CapSMSOp::code2Name(op->getOpcode()), _capState.Print());
            if (!_capState.u.s.ctrFinished)
                err = _RCS_CAPService->mkhash(CAPServiceRC::contractViolation);
        }
        if (_capState.u.s.ctrFinished)
            doAbort = true; //just end dialog with errOk
    }
    if (doReport)
        ssfHdl->onDPSMSResult(capId, rPCause);
    if (doAbort) {
        endTCap();
        ssfHdl->onEndCapDlg(capId, err);
    }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
void CapSMSDlg::eventReportSMS(bool submitted) throw(CustomException)
{
    EventTypeSMS_e eventType = submitted ? EventTypeSMS_o_smsSubmission : 
                                            EventTypeSMS_o_smsFailure ;
    {
        SMSEventDPs::const_iterator cit = rrse->SMSEvents().find(eventType);
        MonitorMode_e mMode = (cit != rrse->SMSEvents().end())
                                ? (*cit).second : MonitorMode_notifyAndContinue;
        reportType = (mMode == MonitorMode_interrupted)
                     ? MessageType_request : MessageType_notification;
    }

    EventReportSMSArg    arg(eventType, reportType);
    std::string dump;
    smsc_log_debug(logger, "%s: --> SCF EventReportSMS %s", _logId, arg.print(dump).c_str());

    Invoke* op = dialog->initInvoke(CapSMSOp::EventReportSMS, this,
                                    (reportType == MessageType_request)
                                    ? 0/*dflt*/ : CAPSMS_END_TIMEOUT);
    op->setParam(&arg);
    dialog->sendInvoke(op);  //throws
    dialog->continueDialog();  //throws
    _capState.u.s.ctrReported = CAPSmsStateS::operInited;
}

//ends TC dialog, releases Dialog()
void CapSMSDlg::endTCap(void)
{
    if (dialog) {
        dialog->removeListener(this);
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            //see 3GPP 29.078 14.1.2.1.3 smsSSF-to-gsmSCF SMS related messages
            try {  // do TC_BasicEnd if still active
                dialog->endDialog((bool)(reportType != MessageType_notification));
                smsc_log_debug(logger, "%s: T_END_REQ, state {%s}", _logId,
                                _capState.Print());
            } catch (std::exception & exc) {
                smsc_log_error(logger, "%s: T_END_REQ: %s", _logId, exc.what());
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

