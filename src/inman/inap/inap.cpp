static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/comp/acdefs.hpp"
#include "inman/inap/inap.hpp"
#include "inman/interaction/inerrcodes.hpp"

#define CAP_OPER_INITED 1
#define CAP_OPER_DONE   2

#define TCAP_DLG_COMP_LAST 2
#define TCAP_DLG_COMP_WAIT 1

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::InapOpCode;

/* ************************************************************************** *
 * class InapOpResListener implementation:
 * ************************************************************************** */
InapOpResListener::InapOpResListener(Invoke * op, Inap * pInap)
    : orgInv(op), orgInap(pInap)
{ 
    assert(op && pInap);
}

void InapOpResListener::error(TcapEntity* resE)
{
    orgInv->setListener(NULL);
    orgInap->onOperationError(orgInv, resE);
}

void InapOpResListener::lcancel()
{
    orgInv->setListener(NULL);
    orgInap->onOperationLCancel(orgInv);
}


Invoke * InapOpResListener::getOrgInv() const
{
    return orgInv;
}

/* ************************************************************************** *
 * class Inap implementation:
 * ************************************************************************** */
Inap::Inap(Session* pSession, SSFhandler * ssfHandler,
           USHORT_T timeout/* = 0*/, Logger * uselog/* = NULL*/)
    : ssfHdl(ssfHandler), session(pSession), logger(uselog)
{
    assert(ssfHandler && pSession);
    _capState.value = _dlgState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Inap");
    dialog = session->openDialog(id_ac_cap3_sms_AC);
    assert(dialog);
    dialog->setInvokeTimeout(timeout);
    dialog->addListener(this);
}

Inap::~Inap()
{
    dialog->removeListener(this);

    //this is the optimized variant of releaseAllOperations()
    ResultHandlersMAP::const_iterator it;
    for (it = resHdls.begin(); it != resHdls.end(); it++) {
        InapOpResListener * ires = (*it).second;
        (ires->getOrgInv())->setListener(NULL);
        delete ires;
    }
    //All remaining Invokes will be deleted by ~Dialog()
    delete dialog;
}

Invoke* Inap::initOperation(UCHAR_T opcode)
{
    Invoke* op = dialog->initInvoke(opcode);
    assert( op );
    InapOpResListener * ires = new InapOpResListener(op, this);
    assert(ires);
    op->setListener(ires);
    resHdls.insert(ResultHandlersMAP::value_type(op->getId(), ires));
    smsc_log_debug(logger, "Inap: initiated Invoke[%u] (opcode = %u), respType: %d",
                   op->getId(), op->getOpcode(), op->getResponseType());
    return op;
}

void Inap::releaseAllOperations(void)
{
    ResultHandlersMAP::const_iterator it;
    for (it = resHdls.begin(); it != resHdls.end(); it++) {
        InapOpResListener * ires = (*it).second;
        Invoke * inv = ires->getOrgInv();
        inv->setListener(NULL);
        delete ires;
        smsc_log_debug(logger, "Inap: releasing Invoke[%u] (opcode = %u), respType: %d, status: %d",
                       inv->getId(), inv->getOpcode(), inv->getResponseType(), inv->getStatus());
        dialog->releaseInvoke(inv->getId());
    }
    resHdls.clear();
}

void Inap::releaseOperation(Invoke * inv)
{
    USHORT_T invId = inv->getId();
    UCHAR_T  opcode = inv->getOpcode();
    smsc_log_debug(logger, "Inap: releasing Invoke[%u] (opcode = %u), respType: %d, status: %d",
                   invId, opcode, inv->getResponseType(), inv->getStatus());

    ResultHandlersMAP::const_iterator it = resHdls.find(invId);
    if (it != resHdls.end()) {
        dialog->releaseInvoke(invId);
        InapOpResListener * ires = (*it).second;
        resHdls.erase(invId);
        delete ires;
    }
}

//Called if Operation got ResultError
void Inap::onOperationError(Invoke *op, TcapEntity * resE)
{
    UCHAR_T opcode = op->getOpcode();
    smsc_log_error(logger, "Inap: Invoke[%u] (opcode = %u) got a returnError: %d",
                   (unsigned)op->getId(), (unsigned)opcode, (unsigned)resE->getOpcode());
    releaseOperation(op);

    //call operation errors handler
    switch (opcode) {
    case InapOpCode::InitialDPSMS: {
        ssfHdl->onAbortSMS(resE->getOpcode(), false);
    } break;
    case InapOpCode::EventReportSMS: {
        ssfHdl->onAbortSMS(resE->getOpcode(), false);
    } break;
    default:;
    }
}

//Called if Operation got L_CANCEL, possibly while waiting result
void Inap::onOperationLCancel(Invoke *op)
{
    UCHAR_T opcode = op->getOpcode();

    if ((op->getResponseType() == Invoke::respResultOrError)
        && (op->getStatus() == Invoke::resWait))
        smsc_log_error(logger,
                "Inap: Invoke[%u] (opcode = %u) got L_CANCEL expecting returnResult",
                (unsigned)op->getId(), (unsigned)opcode);
    else
        smsc_log_debug(logger, "Inap: Invoke[%u] (opcode = %u) got L_CANCEL",
                   (unsigned)op->getId(), (unsigned)opcode);
    releaseOperation(op);

    switch (opcode) {
    case InapOpCode::InitialDPSMS: {
        _capState.s.ctrInited = CAP_OPER_DONE;
        //It's need to check for Inap state, to handle possible CAP3 timeout expiration
        //NOTE: Dialog indications preceed Invoke indications, so it's enough to just
        //check the _dlgState for remote continuation/end
        if (!(_dlgState.s.dlgRContinued || _dlgState.s.dlgREnded)) {
            smsc_log_error(logger,
                "Inap: (state cap: %u, dlg: %u) still has not got any Operation of smsProcessingPackage",
                _capState.value, _dlgState.value);
            ssfHdl->onAbortSMS(smsc::inman::tcapResourceLimitation, true);
        }
    } break;
    case InapOpCode::EventReportSMS: { //normal situation
    } break;
    default:;
    }
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void Inap::onDialogContinue(bool compPresent)
{
    _dlgState.s.dlgRContinued = compPresent ?
                        TCAP_DLG_COMP_WAIT : TCAP_DLG_COMP_LAST;
}

//SCF sent DialogPAbort (some system error on SSF side).
void Inap::onDialogPAbort(UCHAR_T abortCause)
{
    _capState.s.ctrAborted = _dlgState.s.dlgAborted = 1;
    releaseAllOperations();
    ssfHdl->onAbortSMS(abortCause, true);
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void Inap::onDialogREnd(bool compPresent)
{
    if (!compPresent) {
        _dlgState.s.dlgREnded = TCAP_DLG_COMP_LAST;
        if (!_capState.s.ctrReported)
            onDialogPAbort(smsc::inman::tcapResourceLimitation);
        else {
            _capState.s.ctrFinished = 1;
            releaseAllOperations();
        }
    } else
        _dlgState.s.dlgREnded = TCAP_DLG_COMP_WAIT; //handle ongoing Invoke
}


void Inap::onDialogInvoke(Invoke* op, bool lastComp)
{
    assert(op);

    if (lastComp) {
        if (_dlgState.s.dlgRContinued == TCAP_DLG_COMP_WAIT)
            _dlgState.s.dlgRContinued == TCAP_DLG_COMP_LAST;
        else if (_dlgState.s.dlgREnded == TCAP_DLG_COMP_WAIT)
            _dlgState.s.dlgREnded == TCAP_DLG_COMP_LAST;
    }

    switch (op->getOpcode()) {
    case InapOpCode::FurnishChargingInformationSMS: {
        assert(op->getParam());
        ssfHdl->onFurnishChargingInformationSMS(static_cast<FurnishChargingInformationSMSArg*>
                                                (op->getParam()));
    }   break;
    case InapOpCode::ConnectSMS: {
        assert(op->getParam());
        ssfHdl->onConnectSMS(static_cast<ConnectSMSArg*>(op->getParam()));
    }   break;
    case InapOpCode::RequestReportSMSEvent: {
        assert(op->getParam());
        _capState.s.ctrRequested = 1;
        ssfHdl->onRequestReportSMSEvent(static_cast<RequestReportSMSEventArg*>(op->getParam()));
    }   break;
    case InapOpCode::ContinueSMS: {
        _capState.s.ctrContinued = 1;
        ssfHdl->onContinueSMS();
    }   break;
    case InapOpCode::ReleaseSMS: {
        assert(op->getParam());
        _capState.s.ctrReleased = 1;
        ssfHdl->onReleaseSMS(static_cast<ReleaseSMSArg*>(op->getParam()));
    }   break;
    case InapOpCode::ResetTimerSMS: {
        assert(op->getParam());
        ssfHdl->onResetTimerSMS(static_cast<ResetTimerSMSArg*>(op->getParam()));
    }   break;
    default:;
    }
}

/* ------------------------------------------------------------------------ *
 * SCFcontractor interface
 * ------------------------------------------------------------------------ */
void Inap::initialDPSMS(InitialDPSMSArg* arg)
{
    assert(arg);
    Invoke* op = initOperation(InapOpCode::InitialDPSMS);

    op->setParam(arg);
    op->send();
    dialog->beginDialog();
    _capState.s.ctrInited = CAP_OPER_INITED;
    _dlgState.s.dlgInited = 1;
}

void Inap::eventReportSMS(EventReportSMSArg* arg)
{
    assert(arg);
    Invoke* op = initOperation(InapOpCode::EventReportSMS);

    op->setParam(arg);
    op->send();
    dialog->continueDialog();
    _capState.s.ctrReported = CAP_OPER_INITED;
    _dlgState.s.dlgLContinued = 1;
}

} //inap
} //inman
} //smsc

