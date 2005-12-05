static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/comp/acdefs.hpp"
#include "inman/inap/inap.hpp"
#include "inman/interaction/inerrcodes.hpp"

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
    assert(op);
    assert(pInap);
}

void InapOpResListener::error(TcapEntity* resE)
{
    orgInap->onOperationError(orgInv, resE);
}

void InapOpResListener::lcancel()
{
    if ((orgInv->getResponseType() == Invoke::respResultOrError)
        && (orgInv->getStatus() == Invoke::resWait)) {
        orgInap->onOperationLCancel(orgInv);
    } else { //just release original Invoke and its Listener
        orgInv->setListener(NULL);
        orgInap->releaseOperation(orgInv);
    }
}


Invoke * InapOpResListener::getOrgInv() const
{
    return orgInv;
}

/* ************************************************************************** *
 * class Inap implementation:
 * ************************************************************************** */
Inap::Inap(Session* pSession, SSF * ssfHandler,
           USHORT_T timeout/* = 0*/, Logger * uselog/* = NULL*/)
    : ssfHdl(ssfHandler), session(pSession)
    , logger(uselog), _state(Inap::ctrIdle)
{
    assert(ssfHandler);
    assert(pSession);
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
    smsc_log_debug(logger, "Inap: releasing Invoke[%u] (opcode = %u), respType: %d, status: %d",
                   invId, inv->getOpcode(), inv->getResponseType(), inv->getStatus());

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
    smsc_log_error(logger, "Inap: Invoke[%u] (opcode = %u) got an error: %d",
                   (unsigned)op->getId(), (unsigned)opcode, (unsigned)resE->getOpcode());
    releaseOperation(op);

    //call operation errors handler
    switch(opcode) {
    case InapOpCode::InitialDPSMS: {
        ssfHdl->abortSMS(resE->getOpcode(), false);
    } break;
//    case InapOpCode::EventReportSMS: {
//    } break;
    default:;
    }
}

//Called if Operation got L_CANCEL while waiting result
void Inap::onOperationLCancel(Invoke *op)
{
    UCHAR_T opcode = op->getOpcode();
    smsc_log_error(logger, "Inap: Invoke[%u] (opcode = %u) got L_CANCEL",
                   (unsigned)op->getId(), (unsigned)opcode);
    releaseOperation(op);

    //call operation errors handler
    switch(opcode) {
    case InapOpCode::InitialDPSMS: {
        ssfHdl->abortSMS(smsc::inman::tcapResourceLimitation, true);
    } break;
//    case InapOpCode::EventReportSMS: {
//    } break;
    default:;
    }
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SSF sent DialogPAbort (some system error on SSF side).
void Inap::onDialogPAbort(UCHAR_T abortCause)
{
    _state = Inap::ctrAborted;
    releaseAllOperations();
    ssfHdl->abortSMS(abortCause, true);
}

//SSF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void Inap::onDialogREnd(bool compPresent)
{
    if (_state < Inap::ctrReported)
        onDialogPAbort(smsc::inman::tcapResourceLimitation);
    else {
        _state = Inap::ctrFinished;
        releaseAllOperations();
    }
}


void Inap::onDialogInvoke(Invoke* op)
{
    assert(op);
    // SSF handler keeps (Billing*) casted to (SSF*)
    switch(op->getOpcode()) {
    case InapOpCode::FurnishChargingInformationSMS: {
        assert(op->getParam());
        ssfHdl->furnishChargingInformationSMS(static_cast<FurnishChargingInformationSMSArg*>
                                                (op->getParam()));
    }   break;
    case InapOpCode::ConnectSMS: {
        assert(op->getParam());
        ssfHdl->connectSMS(static_cast<ConnectSMSArg*>(op->getParam()));
    }   break;
    case InapOpCode::RequestReportSMSEvent: {
        assert(op->getParam());
        _state = Inap::ctrRequested;
        ssfHdl->requestReportSMSEvent(static_cast<RequestReportSMSEventArg*>(op->getParam()));
    }   break;
    case InapOpCode::ContinueSMS: {
        _state = Inap::ctrContinued;
        ssfHdl->continueSMS();
    }   break;
    case InapOpCode::ReleaseSMS: {
        assert(op->getParam());
        _state = Inap::ctrReleased;
        ssfHdl->releaseSMS(static_cast<ReleaseSMSArg*>(op->getParam()));
    }   break;
    case InapOpCode::ResetTimerSMS: {
        assert(op->getParam());
        ssfHdl->resetTimerSMS(static_cast<ResetTimerSMSArg*>(op->getParam()));
    }   break;
    default:;
    }
}

/* ------------------------------------------------------------------------ *
 * SCF interface
 * ------------------------------------------------------------------------ */
void Inap::initialDPSMS(InitialDPSMSArg* arg)
{
    assert(arg);
    Invoke* op = initOperation(InapOpCode::InitialDPSMS);

    op->setParam(arg);
    op->send();
    dialog->beginDialog();
    _state = Inap::ctrInited;
}

void Inap::eventReportSMS(EventReportSMSArg* arg)
{
    assert(arg);
    Invoke* op = initOperation(InapOpCode::EventReportSMS);

    op->setParam(arg);
    op->send();
    dialog->continueDialog();
    _state = Inap::ctrReported;
}

} //inap
} //inman
} //smsc

