static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/inap/DlgCapSMS.hpp"

using smsc::inman::comp::CapSMSOpCode;

namespace smsc {
namespace inman {
namespace inap {

/* ************************************************************************** *
 * class CapSMSDlg implementation:
 * ************************************************************************** */
CapSMSDlg::CapSMSDlg(SSNSession* pSession, SSFhandler * ssfHandler,
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
    assert(arg && dialog);
    Invoke* op = dialog->initInvoke(CapSMSOpCode::InitialDPSMS, this);
    op->setParam(arg);
    dialog->sendInvoke(op);

    dialog->beginDialog();
    _capState.s.ctrInited = CAP_OPER_INITED;
}


void CapSMSDlg::eventReportSMS(EventReportSMSArg* arg) throw(CustomException)
{
    assert(arg && dialog);
    Invoke* op = dialog->initInvoke(CapSMSOpCode::EventReportSMS, this, CAPSMS_END_TIMEOUT);
    op->setParam(arg);
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
    smsc_log_error(logger, "CapSMS[%u]: Invoke[%u].(%u) got a returnError: %u",
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
    ssfHdl->onAbortSMS(resE->getOpcode(), smsc::inman::errCAP3);
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
            ssfHdl->onAbortSMS(CapSMSDlg::smsContractViolation, smsc::inman::errCAPuser);
        }
    } else if (CapSMSOpCode::EventReportSMS == op->getOpcode()) {
        endTCap();
        ssfHdl->onEndSMS(false);
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
        ssfHdl->onEndSMS(false); //T_END_IND is timed out
    else
        ssfHdl->onAbortSMS(abortCause, smsc::inman::errTCAP);
}

//SCF sent DialogUAbort (some logic error on SCF side).
void CapSMSDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    MutexGuard  grd(_sync);
    _capState.s.ctrAborted = 1;
    smsc_log_error(logger, "CapSMS[%u]: U_ABORT at state 0x%x", capId, _capState.value);
    endTCap();
    ssfHdl->onAbortSMS(Dialog::tcUserGeneralError, smsc::inman::errTCuser);
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
            ssfHdl->onEndSMS(true);
        } else{
            smsc_log_error(logger, "CapSMS[%u]: T_END_IND, state 0x%x", capId, _capState.value);
            ssfHdl->onAbortSMS(CapSMSDlg::smsContractViolation, smsc::inman::errCAPuser);
        }
    } //else wait for ongoing Invoke
}

//See cap3SMS CONTRACT description comment in CapSMSDlg.hpp
void CapSMSDlg::onDialogInvoke(Invoke* op, bool lastComp)
{
    bool    doAbort = false;
    MutexGuard  grd(_sync);

    switch (op->getOpcode()) {
    case CapSMSOpCode::ConnectSMS: {
        assert(op->getParam());
        if (_capState.value ^ PRE_CONNECT_SMS_MASK)
            doAbort = true;
        else
            ssfHdl->onConnectSMS(static_cast<ConnectSMSArg*>(op->getParam()));
    }   break;
    case CapSMSOpCode::ReleaseSMS: {
        assert(op->getParam());
        if ((_capState.value ^ PRE_RELEASE_SMS_MASK1)
            && (_capState.value ^ PRE_RELEASE_SMS_MASK2)) {
            doAbort = true;
        } else {
            _capState.s.ctrReleased = 1;
            ssfHdl->onReleaseSMS(static_cast<ReleaseSMSArg*>(op->getParam()));
        }
    }   break;
    case CapSMSOpCode::ContinueSMS: {
        if ((_capState.value ^ PRE_CONTINUE_SMS_MASK1)
            && (_capState.value ^ PRE_CONTINUE_SMS_MASK2)) {
            doAbort = true;
        } else {
            _capState.s.ctrContinued = 1;
            ssfHdl->onContinueSMS();
        }
    }   break;
    case CapSMSOpCode::RequestReportSMSEvent: {
        assert(op->getParam());
        if ((_capState.value ^ PRE_REQUEST_EVENT_MASK1)
            && (_capState.value ^ PRE_REQUEST_EVENT_MASK2)) {
            doAbort = true;
        } else {
            _capState.s.ctrRequested = 1;
            ssfHdl->onRequestReportSMSEvent(static_cast<RequestReportSMSEventArg*>(op->getParam()));
        }
    }   break;
    case CapSMSOpCode::FurnishChargingInformationSMS: {
        assert(op->getParam());
        ssfHdl->onFurnishChargingInformationSMS(static_cast<FurnishChargingInformationSMSArg*>
                                                (op->getParam()));
    }   break;
    case CapSMSOpCode::ResetTimerSMS: {
        assert(op->getParam());
        ssfHdl->onResetTimerSMS(static_cast<ResetTimerSMSArg*>(op->getParam()));
    }   break;
    default:
        smsc_log_error(logger, "CapSMS[%u]: illegal Invoke(opcode = %u)",
            capId, op->getOpcode());
    }
    if (doAbort)
        smsc_log_error(logger, "CapSMS[%u]: state(0x%x) inconsistent with Invoke(opcode = %u)",
                        capId, _capState.value, op->getOpcode());
    if (_capState.s.ctrFinished) {
        endTCap(); //ReleaseSMS 
        ssfHdl->onEndSMS(true);
    } else if (doAbort) {
        endTCap();
        ssfHdl->onAbortSMS(CapSMSDlg::smsContractViolation, smsc::inman::errCAPuser);
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

