static char const ident[] = "$Id$";

#include <assert.h>
#include <memory>

#include "inman/inap/dialog.hpp"
#include "inman/inap/dispatcher.hpp"
#include "inman/comp/operfactory.hpp"

using smsc::inman::common::format;
using smsc::inman::common::dump;
using smsc::inman::comp::ApplicationContextFactory;
using smsc::inman::comp::OperationFactory;


namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// Dialog class implementation
/////////////////////////////////////////////////////////////////////////////////////
Dialog::Dialog(USHORT_T dlgId, ACOID::DefinedOIDidx dialog_ac_idx, const SCCP_ADDRESS_T & loc_addr,
                const SCCP_ADDRESS_T & rmt_addr, Logger * uselog/* = NULL*/)
  : logger(uselog), _dId(dlgId),  ownAddr(loc_addr), rmtAddr(rmt_addr)
  , qSrvc(EINSS7_I97TCAP_QLT_BOTH), priority(EINSS7_I97TCAP_PRI_HIGH_0)
  , _timeout(_DEFAULT_INVOKE_TIMER), _lastInvId(0)
  , _ac_idx(dialog_ac_idx), _state(Dialog::dlgIdle)
{
    dSSN = ownAddr.addr[1];
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.Dialog");
    APP_CONTEXT_T * acPtr = (APP_CONTEXT_T *)ACOID::OIDbyIdx(dialog_ac_idx);
    assert(acPtr);
    ac = *acPtr;
}

Dialog::~Dialog()
{
    MutexGuard  tmp(invGrd);
    InvokeMap::const_iterator it;
    for (it = originating.begin(); it != originating.end(); it++) {
        Invoke * inv = (*it).second;
        smsc_log_debug(logger, "Dialog[%u]: releasing Invoke[%u] (opcode = %u), respType: %d, status: %d",
                       _dId, inv->getId(), inv->getOpcode(), inv->getResponseType(), inv->getStatus());
        delete inv;
    }
    for (it = terminating.begin(); it != terminating.end(); it++) {
        Invoke * inv = (*it).second;
        smsc_log_debug(logger, "Dialog[%u]: releasing Invoke[%u] (opcode = %u), respType: %d, status: %d",
                       _dId, inv->getId(), inv->getOpcode(), inv->getResponseType(), inv->getStatus());
        delete inv;
    }
}

void Dialog::reset(USHORT_T new_id)
{
    _dId = new_id;
    listeners.clear();
    originating.clear();
    terminating.clear();
    _state = Dialog::dlgIdle;
    _lastInvId = 0;
}


void Dialog::addListener(DialogListener* pListener)
{
    MutexGuard  tmp(invGrd);
    listeners.push_back(pListener);
}

void Dialog::removeListener(DialogListener* pListener)
{
    MutexGuard  tmp(invGrd);
    listeners.remove( pListener );
}


void Dialog::setInvokeTimeout(USHORT_T timeout)
{
    _timeout = timeout ? timeout : _DEFAULT_INVOKE_TIMER;
}

void Dialog::checkSS7res(const char * descr, USHORT_T result) throw(CustomException)
{
    if (result) {
        if ((MSG_BROKEN_CONNECTION == result) || (MSG_NOT_CONNECTED == result))
            TCAPDispatcher::getInstance()->onDisconnect();
        throw CustomException(descr, result, getTcapReasonDescription(result));
    }
    return;
}

void Dialog::beginDialog(UCHAR_T* ui/* = NULL*/, USHORT_T uilen/* = 0*/) throw (CustomException)
{
    smsc_log_debug(logger, "BEGIN_REQ -> {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  DialogID: 0x%X\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "  App. context: %s\n"
                    "  User info[%u]: %s\n"
                    "}",
                   dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId, priority, qSrvc, 
                   dump(rmtAddr.addrLen, rmtAddr.addr).c_str(),
                   dump(ownAddr.addrLen, ownAddr.addr).c_str(),
                   dump(ac.acLen, ac.ac).c_str(),
                   uilen, dump(uilen, ui).c_str()
                   );
    USHORT_T result = EINSS7_I97TBeginReq(
                        dSSN, MSG_USER_ID, TCAP_INSTANCE_ID,
                        _dId, priority, qSrvc,
                        rmtAddr.addrLen, rmtAddr.addr,
                        ownAddr.addrLen, ownAddr.addr,
                        ac.acLen, ac.ac,
                        uilen, ui);
    checkSS7res("TBeginReq failed", result);
    _state = Dialog::dlgInited;
}

void Dialog::beginDialog(const SCCP_ADDRESS_T& remote_addr, UCHAR_T* ui/* = NULL*/,
                         USHORT_T uilen/* = 0*/) throw (CustomException)
{
    rmtAddr = remote_addr;    
    beginDialog(ui, uilen);
}


void Dialog::continueDialog(void) throw (CustomException)
{
    smsc_log_debug(logger, "CONTINUE_REQ -> {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  DialogID: 0x%X\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Org. address: %s\n"
                    "  App. context[%u]: %s\n"
                    "}",
                   dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId, priority, qSrvc, 
//                   dump(ownAddr.addrLen, ownAddr.addr).c_str(),
		   "",
                   ac.acLen, dump(ac.acLen, ac.ac).c_str()
/*
                   (_state < Dialog::dlgConfirmed) ? ac.acLen : 0,
                   (_state < Dialog::dlgConfirmed) ? dump(ac.acLen, ac.ac).c_str() : ""
*/                   
                   );

    USHORT_T result =
        EINSS7_I97TContinueReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
//                               priority, qSrvc, ownAddr.addrLen, ownAddr.addr,
                               priority, qSrvc, 0, NULL,
                               ac.acLen, ac.ac,
/*
                               (_state < Dialog::dlgConfirmed) ? ac.acLen : 0,
                               (_state < Dialog::dlgConfirmed) ? ac.ac : NULL, 
*/                               
                               0, NULL);

    checkSS7res("ContinueReq failed", result);
    if (_state < Dialog::dlgConfirmed)
        _state  = Dialog::dlgContinued;
}

void Dialog::endDialog(USHORT_T termination) throw (CustomException)
{
    smsc_log_debug(logger, "END_REQ -> {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  DialogID: 0x%X\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Termination: 0x%X\n"
                    "  App. context[%u]: %s\n"
                    "}",
                   dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId, priority, qSrvc,
                   termination, ac.acLen, dump(ac.acLen, ac.ac).c_str()
                   );

    USHORT_T result =
        EINSS7_I97TEndReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
                          priority, qSrvc, termination,
                          ac.acLen, ac.ac, 0, NULL);

    checkSS7res("EndReq failed", result);
    _state  = Dialog::dlgEnded;
}


void Dialog::sendInvoke(Invoke * inv) throw (CustomException)
{
    RawBuffer   op, params;
    inv->encode(op, params); //throws CustomException

    const Invoke * linked = inv->getLinkedTo();
    USHORT_T invTimeout = inv->getTimeout();

    if (!invTimeout)
        invTimeout = _timeout;

    smsc_log_debug(logger, "EINSS7_I97TInvokeReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, lunkedused=\"%s\", linkedid=%d, "
                "tag=\"%s\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
                inv->getId(), linked ? "YES" : "NO", linked ? linked->getId() : 0,
                (inv->getTag() == EINSS7_I97TCAP_OPERATION_TAG_LOCAL) ? "LOCAL" : "GLOBAL",
                op.size(), dump(op.size(), &op[0]).c_str(),
                params.size(), dump(params.size(), &params[0]).c_str());

    USHORT_T result = 
        EINSS7_I97TInvokeReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId, inv->getId(), 
            linked ? EINSS7_I97TCAP_LINKED_ID_USED : EINSS7_I97TCAP_LINKED_ID_NOT_USED,
            linked ? linked->getId() : 0,
            EINSS7_I97TCAP_OP_CLASS_1, invTimeout, inv->getTag(),
            op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("InvokeReq failed", result);
}

void Dialog::sendResultLast(InvokeResultLast* res) throw (CustomException)
{
    RawBuffer   op, params;
    res->encode(op, params); //throws CustomException

    smsc_log_debug(logger, "EINSS7_I97TResultLReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, tag=\"%s\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
                res->getId(), 
                (res->getTag() == EINSS7_I97TCAP_OPERATION_TAG_LOCAL) ? "LOCAL" : "GLOBAL",
                op.size(), dump(op.size(), &op[0]).c_str(),
                params.size(), dump(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TResultLReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
    	res->getId(), res->getTag(),
        op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("ResultLReq failed", result);
}

void Dialog::sendResultNotLast(InvokeResultNotLast* res) throw (CustomException)
{
    RawBuffer   op, params;
    res->encode(op, params); //throws CustomException

    smsc_log_debug(logger, "EINSS7_I97TResultNLReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, tag=\"%s\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
                res->getId(), 
                (res->getTag() == EINSS7_I97TCAP_OPERATION_TAG_LOCAL) ? "LOCAL" : "GLOBAL",
                op.size(), dump(op.size(), &op[0]).c_str(),
                params.size(), dump(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TResultNLReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
        res->getId(), res->getTag(),
        op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("ResultNLReq failed", result);
}


void Dialog::sendResultError(InvokeResultError* res) throw (CustomException)
{
    RawBuffer   op, params;
    res->encode(op, params);  //throws CustomException

    smsc_log_debug(logger, "EINSS7_I97TUErrorReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, tag=\"%s\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
                res->getId(), 
                (res->getTag() == EINSS7_I97TCAP_OPERATION_TAG_LOCAL) ? "LOCAL" : "GLOBAL",
                op.size(), dump(op.size(), &op[0]).c_str(),
                params.size(), dump(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TUErrorReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId,
        res->getId(), res->getTag(),
        op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("UErrorReq failed", result);
}


void Dialog::resetInvokeTimer(UCHAR_T invokeId) throw (CustomException)
{
    smsc_log_debug(logger,"EINSS7_I97TTimerResetReq("
                   "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, invokeId=%d",
                   dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId, invokeId);

    USHORT_T result = 
        EINSS7_I97TTimerResetReq(dSSN, MSG_USER_ID, TCAP_INSTANCE_ID, _dId, invokeId);

    checkSS7res("TTimerResetReq failed", result);
}

Invoke* Dialog::initInvoke(UCHAR_T opcode, USHORT_T timeout/* = 0*/)
{
    MutexGuard  tmp(invGrd);
    Invoke::InvokeResponse  resp = Invoke::respNone;
    OperationFactory * fact = ApplicationContextFactory::getFactory(_ac_idx);

    if (fact->hasErrors(opcode))
        resp = Invoke::respError;
    if (fact->hasResult(opcode))
        resp = Invoke::respResultOrError;

    Invoke* invoke = new Invoke(this, getNextInvokeId(),
                              EINSS7_I97TCAP_OPERATION_TAG_LOCAL, opcode, resp);
    invoke->setTimeout(timeout ? timeout : _timeout);

    //register invoke if OPERATION has RESULT/ERRORS defined,
    //in order to provide possibility to call result/error listeners
    if (resp != Invoke::respNone)
        originating.insert(InvokeMap::value_type(invoke->getId(), invoke));
    else
        terminating.insert(InvokeMap::value_type(invoke->getId(), invoke));
    return invoke;
}

void Dialog::releaseInvoke(UCHAR_T invId)
{
    MutexGuard  tmp(invGrd);
    //search both maps for Invoke and unregister it
    InvokeMap::iterator it = originating.find(invId);
    if (it == originating.end()) {
        it = terminating.find(invId);
        if (it != terminating.end()) {
            Invoke * inv = (*it).second;
            terminating.erase(it);
            delete inv;
        }
    } else {
        Invoke * inv = (*it).second;
        originating.erase(it);
        delete inv;
    }
}

//returns true is dialog has some Invokes pending (awaiting Result or LCancel)
unsigned Dialog::pendingInvokes(void)
{
    MutexGuard  tmp(invGrd);
    return originating.size() + terminating.size();
}

/* ------------------------------------------------------------------------ *
 * Transaction level callbacks
 * ------------------------------------------------------------------------ */
USHORT_T Dialog::handleBeginDialog()
{
    _state = Dialog::dlgInited;
    return MSG_OK;
}

USHORT_T Dialog::handleContinueDialog(bool compPresent)
{
    if (_state == Dialog::dlgInited)
        _state = Dialog::dlgContinued;
    else
        _state = Dialog::dlgConfirmed;
    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogContinue(compPresent);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleEndDialog(bool compPresent)
{
    _state = Dialog::dlgEnded;
    //NOTE: calling onDialogREnd() may lead to this Dialog destruction,
    //so iterate over ListenerList copy.
    ListenerList cpList = listeners;
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogREnd(compPresent);
    }
    return MSG_OK;
}

//Reports dialog abort to dialogListener
USHORT_T Dialog::handlePAbortDialog(UCHAR_T abortCause)
{
    _state = Dialog::dlgEnded;
    //NOTE: calling onDialogPAbort() may lead to this Dialog destruction,
    //so iterate over ListenerList copy.
    ListenerList cpList = listeners;
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogPAbort(abortCause);
    }
    
    return MSG_OK;
}

/* ------------------------------------------------------------------------ *
 * Interaction level callbacks
 * ------------------------------------------------------------------------ */
//Reports incoming Invoke to dialogListener
USHORT_T Dialog::handleInvoke(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                              USHORT_T pmlen, const UCHAR_T *pm, bool lastComp)
{
    assert(op && (oplen > 0));
    assert(tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL);

    Invoke  invoke(this, invId, tag, op[0]);

    if (pmlen) { //operation parameters present
        Component* comp = ApplicationContextFactory::getFactory(_ac_idx)->createArg(op[0]);
        assert(comp);
        std::vector<unsigned char> code( pm, pm + pmlen );
        comp->decode(code);
        invoke.ownParam(comp);
    }
    //NOTE: calling onDialogInvoke() may lead to this Dialog destruction,
    //so iterate over ListenerList copy.
    ListenerList cpList = listeners;
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogInvoke(&invoke, lastComp);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleLCancelInvoke(UCHAR_T invId)
{
    Invoke* inv = NULL;
    invGrd.Lock();
    //search both maps for Invoke and notify it
    InvokeMap::const_iterator it = originating.find(invId);
    if (it == originating.end()) {
        it = terminating.find(invId);
        inv = (it == terminating.end()) ? NULL : (*it).second;
    } else
        inv = (*it).second;
    invGrd.Unlock();

    if (inv)
        inv->notifyLCancelListener();

    return MSG_OK;
}

USHORT_T Dialog::handleResultLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert(op && (oplen > 0));
    assert( tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL );

    invGrd.Lock();
    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        InvokeResultLast  result(this, invId, tag, op[0]);

        Component* resParm = ApplicationContextFactory::getFactory(_ac_idx)->createRes(op[0]);
        assert(resParm);
        if (resParm) {
            std::vector<unsigned char> code(pm, pm + pmlen);
            resParm->decode(code);
            result.ownParam(resParm);
        }
        Invoke* inv = (*it).second;
        //NOTE: notifyResultListeners() may lead to this ~Dialog() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        invGrd.Unlock();
        inv->notifyResultListener(&result);
        return MSG_OK;
    }
    invGrd.Unlock();
    return MSG_OK;
}

USHORT_T Dialog::handleResultNotLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert(op && (oplen > 0));
    assert(tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL);

    invGrd.Lock();
    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        InvokeResultNotLast  result(this, invId, tag, op[0]);

        Component* resParm = ApplicationContextFactory::getFactory(_ac_idx)->createRes(op[0]);
        assert(resParm);
        if (resParm) {
            std::vector<unsigned char> code(pm, pm + pmlen);
            resParm->decode(code);
            result.ownParam(resParm);
        }
        Invoke* inv = (*it).second;
        //NOTE: notifyResultNListeners() may lead to this ~Dialog() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        invGrd.Unlock();
        inv->notifyResultNListener(&result);
        return MSG_OK;
    }
    invGrd.Unlock();
    return MSG_OK;
}

USHORT_T Dialog::handleUserError(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm)
{
    assert(op && (oplen > 0));
    assert(tag == EINSS7_I97TCAP_OPERATION_TAG_LOCAL);

    invGrd.Lock();
    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        InvokeResultError  resErr(this, invId, tag, op[0]);

        //Error may have no parameters
        Component* errParm = ApplicationContextFactory::getFactory(_ac_idx)->createErr(op[0]);
        if (errParm) {
            std::vector<unsigned char> code(pm, pm + pmlen);
            errParm->decode(code);
            resErr.ownParam(errParm);
        }
        Invoke* inv = (*it).second;
        //NOTE: notifyErrorListener() may lead to this ~Dialog() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        invGrd.Unlock();
        inv->notifyErrorListener(&resErr);
        return MSG_OK;
    }
    invGrd.Unlock();
    return MSG_OK;
}

} // namespace inap
} // namespace inman
} // namespace smsc
