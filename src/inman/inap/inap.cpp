static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/inap/inap.hpp"

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

Invoke * InapOpResListener::getOrgInv() const
{
    return orgInv;
}

/* ************************************************************************** *
 * class Inap implementation:
 * ************************************************************************** */
Inap::Inap(Dialog* dlg, SSF * ssfHandler)
    : dialog(dlg)
    , ssfHdl(ssfHandler)
    , logger(Logger::getInstance("smsc.inman.inap.Inap"))
{
    assert(ssfHandler);
    assert(dialog);
    dialog->addListener(this);
}

Inap::~Inap()
{
    dialog->removeListener(this);

    ResultHandlersMAP::const_iterator it;
    for (it = resHdls.begin(); it != resHdls.end(); it++) {
        InapOpResListener * ires = (*it).second;
        (ires->getOrgInv())->setListener(NULL);
        //Invoke will be deleted by Dialog
        delete ires;
    }
}

void Inap::onOperationError(Invoke *op, TcapEntity * resE)
{
    smsc_log_error(logger, "Inap: Invoke[%u] (opcode = %u) got an error: %d",
                   (unsigned)op->getId(), (unsigned)op->getOpcode(),
                   (unsigned)resE->getOpcode());
    switch(op->getOpcode()) {
    case InapOpCode::InitialDPSMS: {
        ssfHdl->abortSMS(resE->getOpcode());
    } break;
//    case InapOpCode::EventReportSMS: {
//    } break;
    default:;
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
        ssfHdl->requestReportSMSEvent(static_cast<RequestReportSMSEventArg*>(op->getParam()));
    }   break;
    case InapOpCode::ContinueSMS: {
        ssfHdl->continueSMS();
    }   break;
    case InapOpCode::ReleaseSMS: {
        assert(op->getParam());
        ssfHdl->releaseSMS(static_cast<ReleaseSMSArg*>(op->getParam()));
    }   break;
    case InapOpCode::ResetTimerSMS: {
        assert(op->getParam());
        ssfHdl->resetTimerSMS(static_cast<ResetTimerSMSArg*>(op->getParam()));
    }   break;
    default:;
    }
}

void Inap::initialDPSMS(InitialDPSMSArg* arg)
{
    Invoke* op = dialog->invoke( InapOpCode::InitialDPSMS );
    assert( op );
    assert( arg );

    InapOpResListener * ires = new InapOpResListener(op, this);
    resHdls.insert(ResultHandlersMAP::value_type(op->getId(), ires));
    op->setListener(ires);

    op->setParam( arg );
    op->send(dialog);
}

void Inap::eventReportSMS(EventReportSMSArg* arg)
{
    Invoke* op = dialog->invoke( InapOpCode::EventReportSMS );
    assert( op );
    assert( arg );

    InapOpResListener * ires = new InapOpResListener(op, this);
    resHdls.insert(ResultHandlersMAP::value_type(op->getId(), ires));
    op->setListener(ires);

    op->setParam( arg );
    op->send( dialog );
}

}
}
}
