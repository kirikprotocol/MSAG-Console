// cap3SMS CONTRACT implementation (over TCAP dialog)
static char const ident[] = "$Id$";

#include <assert.h>

#include "util/vformat.hpp"
using smsc::util::format;

#include "inman/inap/cap_sms/DlgCapSMS.hpp"
using smsc::inman::comp::CapSMSOpCode;

//using smsc::inman::comp::ConnectSMSArg;
using smsc::inman::comp::ReleaseSMSArg;
using smsc::inman::comp::RequestReportSMSEventArg;
using smsc::inman::comp::ResetTimerSMSArg;
using smsc::inman::comp::EventReportSMSArg;

namespace smsc {
namespace inman {
namespace inap {

/* ************************************************************************** *
 * class CapSMSDlg implementation:
 * ************************************************************************** */
CapSMSDlg::CapSMSDlg(TCSessionSR* pSession, CapSMS_SSFhandlerITF * ssfHandler,
           USHORT_T timeout/* = 0*/, Logger * uselog/* = NULL*/)
    : ssfHdl(ssfHandler), session(pSession), logger(uselog)
{
    assert(ssfHandler && pSession);
    _capState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.CapSMSDlg");
    dialog = session->openDialog();
    if (!dialog)
        throw CustomException("Unable to create TC Dialog");
    
    dialog->setInvokeTimeout(timeout);
    dialog->addListener(this);
    capId = (unsigned)dialog->getId();
}

CapSMSDlg::~CapSMSDlg()
{
    endDPSMS();
}

void CapSMSDlg::endDPSMS(void)
{
    MutexGuard  grd(_sync);
    ssfHdl = NULL;
    endTCap();
}

/* ------------------------------------------------------------------------ *
 * SCFcontractor interface
 * ------------------------------------------------------------------------ */
void CapSMSDlg::initialDPSMS(InitialDPSMSArg* arg) throw(CustomException)
{
    smsc_log_debug(logger, "CapSMS[%u]: --> SCF InitialDPSMS", capId);
    Invoke* op = dialog->initInvoke(CapSMSOpCode::InitialDPSMS, this);
    op->setParam(arg);
    dialog->sendInvoke(op);

    dialog->beginDialog();
    _capState.s.ctrInited = CAP_OPER_INITED;
}


void CapSMSDlg::eventReportSMS(bool submitted) throw(CustomException)
{
    EventTypeSMS_e eventType = submitted ? EventTypeSMS_o_smsSubmission : 
                                            EventTypeSMS_o_smsFailure ;
    smsc_log_debug(logger, "CapSMS[%u]: --> SCF EventReportSMS "
                   "( eventType: o_sms%s (0x%X), messageType: notification(0x%X) )",
                   capId, (submitted) ? "Submission" : "Failure",
                   eventType, MessageType_notification);
    
    EventReportSMSArg    arg(eventType, MessageType_notification);
    Invoke* op = dialog->initInvoke(CapSMSOpCode::EventReportSMS, this, CAPSMS_END_TIMEOUT);
    op->setParam(&arg);
    dialog->sendInvoke(op);

    dialog->continueDialog();
    _capState.s.ctrReported = CAP_OPER_INITED;
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
//Called if Operation got ResultError
void CapSMSDlg::onInvokeError(Invoke *op, TcapEntity * resE)
{
    MutexGuard  grd(_sync);
    UCHAR_T opcode = op->getOpcode();
    smsc_log_error(logger, "CapSMS[%u]: Invoke[%u]{%u} got a returnError: %u",
        capId, (unsigned)op->getId(), (unsigned)opcode, (unsigned)resE->getOpcode());

    //call operation errors handler
    switch (opcode) {
    case CapSMSOpCode::InitialDPSMS: {
        _capState.s.ctrInited = CAP_OPER_FAIL;
    }
    break;
    case CapSMSOpCode::EventReportSMS: {
        _capState.s.ctrReported = CAP_OPER_FAIL;
    } break;
    default:;
    }
    endTCap();
    ssfHdl->onEndCapDlg(resE->getOpcode(), smsc::inman::errCAP3);
}

//Called if Operation got L_CANCEL, possibly while waiting result
void CapSMSDlg::onInvokeLCancel(Invoke *op)
{
    MutexGuard  grd(_sync);
    if (CapSMSOpCode::InitialDPSMS == op->getOpcode()) {
        //It's need to check for CapSMSDlg state, to handle possible CAP3 timeout expiration
        if (!(_capState.s.ctrReleased
              || _capState.s.ctrContinued || _capState.s.ctrAborted)) {
            smsc_log_error(logger,
                "CapSMS[%u]: state 0x%x(TC: 0x%x): has not got any Operation of smsProcessingPackage",
                capId, _capState.value, dialog->getState().value);
            endTCap();
            ssfHdl->onEndCapDlg(CapSMSDlg::smsContractViolation, smsc::inman::errCAPuser);
        }
    } else if (CapSMSOpCode::EventReportSMS == op->getOpcode()) {
        endTCap();
        ssfHdl->onEndCapDlg();
    }
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void CapSMSDlg::onDialogContinue(bool compPresent)
{
    MutexGuard  grd(_sync);
    if (!compPresent)
        smsc_log_error(logger, "CapSMS[%u]: missing component in TC_CONT_IND", capId);
    return;
}

//TCAP indicates DialogPAbort: either due to TC layer error on SCF/SSF side or
//because of TC dialog timeout is expired on SSF (no T_END_IND from SCF).
void CapSMSDlg::onDialogPAbort(UCHAR_T abortCause)
{
    MutexGuard  grd(_sync);
    _capState.s.ctrAborted = 1;
    smsc_log_error(logger, "CapSMS[%u]: P_ABORT at state 0x%x", capId, _capState.value);
    endTCap();
    if ((_capState.s.ctrReported == CAP_OPER_INITED) || _capState.s.ctrFinished)
        ssfHdl->onEndCapDlg(); //T_END_IND is timed out
    else
        ssfHdl->onEndCapDlg(abortCause, smsc::inman::errTCAP);
}

//SCF sent DialogUAbort (some logic error on SCF side).
void CapSMSDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    MutexGuard  grd(_sync);
    _capState.s.ctrAborted = 1;
    smsc_log_error(logger, "CapSMS[%u]: U_ABORT at state 0x%x", capId, _capState.value);
    endTCap();
    ssfHdl->onEndCapDlg(Dialog::tcUserGeneralError, smsc::inman::errTCuser);
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void CapSMSDlg::onDialogREnd(bool compPresent)
{
    MutexGuard  grd(_sync);
    _capState.s.ctrFinished = 1;
    if (!compPresent) {
        endTCap();
        if ((_capState.s.ctrReported == CAP_OPER_INITED) || _capState.s.ctrReleased) {
            ssfHdl->onEndCapDlg();
        } else{
            smsc_log_error(logger, "CapSMS[%u]: T_END_IND, state 0x%x", capId, _capState.value);
            ssfHdl->onEndCapDlg(CapSMSDlg::smsContractViolation, smsc::inman::errCAPuser);
        }
    } //else wait for ongoing Invoke
}

//See cap3SMS CONTRACT description comment in CapSMSDlg.hpp
void CapSMSDlg::onDialogInvoke(Invoke* op, bool lastComp)
{
    bool    doAbort = false;
    MutexGuard  grd(_sync);

    if (!op->getParam() && (op->getOpcode() != CapSMSOpCode::ContinueSMS))
        smsc_log_error(logger, "CapSMS[%u]: Invoke(opcode = %u) arg missed!", capId, op->getOpcode());

    switch (op->getOpcode()) {
    case CapSMSOpCode::ConnectSMS: {
        if (_capState.value ^ PRE_CONNECT_SMS_MASK)
            doAbort = true;
        else
            smsc_log_debug(logger, "CapSMS[%u]: <-- SCF ConnectSMS", capId);
    }   break;

    case CapSMSOpCode::ReleaseSMS: {
        if ((_capState.value ^ PRE_RELEASE_SMS_MASK1)
            && (_capState.value ^ PRE_RELEASE_SMS_MASK2)) {
            doAbort = true;
        } else {
            _capState.s.ctrReleased = 1;
            ReleaseSMSArg * arg = static_cast<ReleaseSMSArg*>(op->getParam());
            smsc_log_debug(logger, "CapSMS[%u]: <-- SCF ReleaseSMS, RP cause: %u",
                            capId, (unsigned)arg->rPCause);
            ssfHdl->onDPSMSResult(arg->rPCause);
        }
    }   break;

    case CapSMSOpCode::ContinueSMS: {
        if ((_capState.value ^ PRE_CONTINUE_SMS_MASK1)
            && (_capState.value ^ PRE_CONTINUE_SMS_MASK2)) {
            doAbort = true;
        } else {
            _capState.s.ctrContinued = 1;
            smsc_log_debug(logger, "CapSMS[%u]: <-- SCF ContinueSMS", capId);
            ssfHdl->onDPSMSResult(0);
        }
    }   break;

    case CapSMSOpCode::RequestReportSMSEvent: {
        assert(op->getParam());
        if ((_capState.value ^ PRE_REQUEST_EVENT_MASK1)
            && (_capState.value ^ PRE_REQUEST_EVENT_MASK2)) {
            doAbort = true;
        } else {
            _capState.s.ctrRequested = 1;
            if (logger->isDebugEnabled()) {
                RequestReportSMSEventArg* arg = static_cast<RequestReportSMSEventArg*>(op->getParam());
                const RequestReportSMSEventArg::SMSEventVector& dps = arg->getSMSEvents();
                std::string dump;
                RequestReportSMSEventArg::SMSEventVector::const_iterator it;
                for (it = dps.begin(); it != dps.end(); it++) {
                    RequestReportSMSEventArg::SMSEvent dp = *it;
                    format(dump, "{eventType 0x%X, monitorMode 0x%X} ", dp.event, dp.monitorType);
                }
                smsc_log_debug(logger, "CapSMS[%u]: <-- SCF RequestReportSMSEvent, %s",
                               capId, dump.c_str());
            }
        }
    }   break;

    case CapSMSOpCode::FurnishChargingInformationSMS: {
        smsc_log_error(logger, "CapSMS[%u]: <-- SCF FurnishChargingInformationSMS",
                       capId);
    }   break;

    case CapSMSOpCode::ResetTimerSMS: {
        ResetTimerSMSArg* arg = static_cast<ResetTimerSMSArg*>(op->getParam());
        smsc_log_debug(logger, "CapSMS[%u]: <-- SCF ResetTimerSMS, value: %lu secs",
                       capId, (long)(arg->timerValue));
    }   break;

    default:
        smsc_log_error(logger, "CapSMS[%u]: illegal Invoke(opcode = %u)",
            capId, op->getOpcode());
    }
    if (doAbort) {
        /* NOTE: there is a known problem with Alcatel IN: it sends END_REQ carrying
         * excessive ContinueSMS component.
         */
        logger->log(_capState.s.ctrFinished ? Logger::LEVEL_WARN : Logger::LEVEL_ERROR,
                    "CapSMS[%u]: inconsistent Invoke(opcode = %u), state(0x%x)",
                    capId, op->getOpcode(), _capState.value);
    }
    if (_capState.s.ctrFinished) {
        endTCap(); //ReleaseSMS 
        ssfHdl->onEndCapDlg();
    } else if (doAbort) {
        endTCap();
        ssfHdl->onEndCapDlg(CapSMSDlg::smsContractViolation, smsc::inman::errCAPuser);
    }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//ends TC dialog, releases Dialog()
void CapSMSDlg::endTCap(void)
{
    if (dialog) {
        dialog->removeListener(this);
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            //see 3GPP 29.078 14.1.2.1.3 smsSSF-to-gsmSCF SMS related messages
            try {
                dialog->endDialog(); // do TC_BasicEnd if still active
                smsc_log_debug(logger, "CapSMS[%u]: T_END_REQ, state: 0x%x", capId,
                                _capState.value);
            } catch (std::exception & exc) {
                smsc_log_error(logger, "CapSMS[%u]: T_END_REQ: %s", capId, exc.what());
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

