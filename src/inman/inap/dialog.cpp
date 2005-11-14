static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>
#include <memory>

#include "inman/inap/inss7util.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/infactory.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/results.hpp"
#include "inman/comp/acdefs.hpp"
#include "inman/common/util.hpp"

using std::runtime_error;
using std::auto_ptr;

using smsc::inman::common::format;
using smsc::inman::common::dump;
using smsc::inman::comp::ApplicationContextFactory;
using smsc::inman::comp::OperationFactory;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// Dialog class
/////////////////////////////////////////////////////////////////////////////////////

extern Logger* tcapLogger;

Dialog::Dialog(Session* pSession, USHORT_T dlgId, unsigned dialog_ac_idx)
  : logger(Logger::getInstance("smsc.inman.inap.Dialog"))
  , did( dlgId )
  , session( pSession )
  , qSrvc(EINSS7_I97TCAP_QLT_BOTH)
  , priority(EINSS7_I97TCAP_PRI_HIGH_0)
  , timeout( 30 )
  , invokeId( 1 )
  , _ac_idx(dialog_ac_idx)
{
  ac = *smsc::ac::ACOID::OIDbyIdx(dialog_ac_idx);
  if(session)
  {
    ownAddr    = session->ssfAddr;
    remoteAddr = session->scfAddr;
  }
}

Dialog::~Dialog()
{
    InvokeMap::const_iterator it;
    for (it = originating.begin(); it != originating.end(); it++) {
        Invoke * inv = (*it).second;
        delete inv;
    }
    for (it = terminating.begin(); it != terminating.end(); it++) {
        Invoke * inv = (*it).second;
        delete inv;
    }
}

void Dialog::beginDialog(UCHAR_T* ui, USHORT_T uilen)
{
    smsc_log_debug(tcapLogger,"BEGIN_REQ");
    smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
    smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
    smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
    smsc_log_debug(tcapLogger," DialogID: 0x%X", did );
    smsc_log_debug(tcapLogger," PriOrder: 0x%X", priority );
    smsc_log_debug(tcapLogger," QoS: 0x%X", qSrvc );
    smsc_log_debug(tcapLogger," Dest. address: %s", dump(remoteAddr.addrLen ,remoteAddr.addr  ).c_str() );
    smsc_log_debug(tcapLogger," Org. address: %s" , dump(ownAddr.addrLen ,ownAddr.addr  ).c_str() );
    smsc_log_debug(tcapLogger," App. context: %s" , dump(ac.acLen ,ac.ac ).c_str() );
    if (ui && uilen)
        smsc_log_debug(tcapLogger," User info: %s" , dump(uilen ,ui).c_str() );

    USHORT_T result = EINSS7_I97TBeginReq(
                        session->getSSN(), MSG_USER_ID, TCAP_INSTANCE_ID,
                        did, priority, qSrvc,
                        remoteAddr.addrLen, remoteAddr.addr,
                        ownAddr.addrLen, ownAddr.addr,
                        ac.acLen, ac.ac,
                        uilen, ui);

    if (result != 0)
        throw runtime_error( format("BeginReq failed with code %d (%s)", result,
                                                getTcapReasonDescription(result)));
}

void Dialog::beginDialog(const SCCP_ADDRESS_T& remote_addr, UCHAR_T* ui, USHORT_T uilen)
{
    remoteAddr = remote_addr;
    beginDialog(ui, uilen);
}


void Dialog::beginDialog(const SCCP_ADDRESS_T& remote_addr)
{
    remoteAddr = remote_addr;
    beginDialog(NULL, 0);
}

void Dialog::beginDialog()
{
    beginDialog(NULL, 0);
}

void Dialog::continueDialog()
{
  smsc_log_debug(tcapLogger,"CONTINUE_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priority );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qSrvc );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(session->ssfAddr.addrLen ,session->ssfAddr.addr  ).c_str() );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(ac.acLen ,ac.ac ).c_str() );

  USHORT_T result = EINSS7_I97TContinueReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    session->ssfAddr.addrLen,
    session->ssfAddr.addr,
    ac.acLen,
    ac.ac,
    0,
    NULL);

  if(result != 0)
    throw runtime_error( format("ContinueReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}

void Dialog::endDialog(USHORT_T termination)
{
  smsc_log_debug(tcapLogger,"END_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priority );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qSrvc );
  smsc_log_debug(tcapLogger," Termination: 0x%X", termination );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(ac.acLen ,ac.ac ).c_str() );

  USHORT_T result = EINSS7_I97TEndReq(
    session->getSSN(),
    MSG_USER_ID,
    TCAP_INSTANCE_ID,
    did,
    priority,
    qSrvc,
    termination,
    ac.acLen,
    ac.ac,
    0,
    NULL);

  if(result != 0)
    throw runtime_error( format("EndReq failed with code %d (%s)", result,getTcapReasonDescription(result)));
}


void Dialog::timerReset()
{
  smsc_log_debug(tcapLogger,"TIME_RESET_REQ");
  smsc_log_debug(tcapLogger," SSN: 0x%X", session->getSSN());
  smsc_log_debug(tcapLogger," UserID: 0x%X", MSG_USER_ID );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", TCAP_INSTANCE_ID );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", did );

  USHORT_T result = EINSS7_I97TTimerResetReq
  (
      session->getSSN(),
      MSG_USER_ID,
      TCAP_INSTANCE_ID,
      did,
      1
    );

  if(result != 0)
    throw runtime_error( format("TimerResetReq failed with code %d (%s)", result,getTcapReasonDescription(result)));

}

Invoke* Dialog::invoke(UCHAR_T opcode)
{
  //smsc_log_debug(logger, "Dialog: creating Invoke (opcode=%d)", opcode);
  Invoke* invoke = new Invoke();
  invoke->setId( getNextInvokeId() );
  invoke->setTag( EINSS7_I97TCAP_OPERATION_TAG_LOCAL );
  invoke->setOpcode( opcode );

  //register invoke if OPERATION has RESULT/ERRORS defined,
  //in order to provide possibility to call result/error listeners
  OperationFactory * fact = ApplicationContextFactory::getFactory(_ac_idx);
  if (fact->hasResult(opcode) || fact->hasErrors(opcode))
      originating.insert(InvokeMap::value_type(invoke->getId(), invoke));
  else
      terminating.insert(InvokeMap::value_type(invoke->getId(), invoke));
  return invoke;
}

//--------------------------------------- Callbacks ----------------------------------

USHORT_T Dialog::handleBeginDialog()
{
  return MSG_OK;
}

USHORT_T Dialog::handleContinueDialog()
{
  return MSG_OK;
}

USHORT_T Dialog::handleEndDialog()
{
  return MSG_OK;
}

USHORT_T Dialog::handleInvoke(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert( op );
    assert( oplen > 0 );
    assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

    UCHAR_T opcode = op[0];
    Invoke  invoke;

    invoke.setId(invId);
    invoke.setTag(tag);
    invoke.setOpcode(opcode);

    if (pmlen) { //operation parameters present
        Component* comp = ApplicationContextFactory::getFactory(_ac_idx)->createArg( opcode );
        assert(comp);
        std::vector<unsigned char> code( pm, pm + pmlen );
        comp->decode(code);
        invoke.setParam(comp);
    }
    //NOTE: calling onDialogInvoke() may lead to this Dialog destruction,
    //so iterate over ListenerList copy.
    ListenerList cpList = listeners;
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogInvoke(&invoke);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleResultLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert( op );
    assert( oplen > 0 );
    assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        InvokeResultLast  result;
        UCHAR_T opcode = op[0];

        result.setId( invId );
        result.setTag( tag );
        result.setOpcode( opcode );

        Component* resParm = ApplicationContextFactory::getFactory(_ac_idx)->createRes(opcode);
        assert(resParm);
        if (resParm) {
            std::vector<unsigned char> code(pm, pm + pmlen);
            resParm->decode(code);
            result.setParam(resParm);
        }
        Invoke* inv = (*it).second;
        //NOTE: notifyResultListeners() may lead to this ~Dialog() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        inv->notifyResultListener(&result);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleResultNotLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert( op );
    assert( oplen > 0 );
    assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        InvokeResultNotLast  result;
        UCHAR_T opcode = op[0];

        result.setId( invId );
        result.setTag( tag );
        result.setOpcode( opcode );

        Component* resParm = ApplicationContextFactory::getFactory(_ac_idx)->createRes(opcode);
        assert(resParm);
        if (resParm) {
            std::vector<unsigned char> code(pm, pm + pmlen);
            resParm->decode(code);
            result.setParam(resParm);
        }
        Invoke* inv = (*it).second;
        //NOTE: notifyResultNListeners() may lead to this ~Dialog() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        inv->notifyResultNListener(&result);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleUserError(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert( op );
    assert( oplen > 0 );
    assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        InvokeResultError  resErr;
        UCHAR_T ercode = op[0];

        resErr.setId( invId );
        resErr.setTag( tag );
        resErr.setOpcode( ercode );

        //Error may have no parameters
        Component* errParm = ApplicationContextFactory::getFactory(_ac_idx)->createErr(ercode);
        if (errParm) {
            std::vector<unsigned char> code(pm, pm + pmlen);
            errParm->decode(code);
            resErr.setParam(errParm);
        }
        Invoke* inv = (*it).second;
        //NOTE: notifyErrorListener() may lead to this ~Dialog() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        inv->notifyErrorListener(&resErr);
    }
    return MSG_OK;
}

} // namespace inap
} // namespace inman
} // namespace smsc
