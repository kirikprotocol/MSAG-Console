#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <assert.h>
#include <memory>

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "inman/inap/dialog.hpp"
#include "inman/inap/dispatcher.hpp"

using smsc::inman::comp::ApplicationContextFactory;

namespace smsc  {
namespace inman {
namespace inap  {

/////////////////////////////////////////////////////////////////////////////////////
// Dialog class implementation
/////////////////////////////////////////////////////////////////////////////////////
Dialog::Dialog(const std::string & sess_uid, USHORT_T dlg_id, USHORT_T msg_user_id,
               ACOID::DefinedOIDidx dialog_ac_idx, const SCCP_ADDRESS_T & loc_addr,
               UCHAR_T sender_ssn/* = 0*/, Logger * uselog/* = NULL*/)
  : logger(uselog), _tcSUId(sess_uid), _dId(dlg_id),  ownAddr(loc_addr)
  , qSrvc(EINSS7_I97TCAP_QLT_BOTH), priority(EINSS7_I97TCAP_PRI_HIGH_0)
  , _timeout(_DEFAULT_INVOKE_TIMER), _lastInvId(0)
  , _ac_idx(dialog_ac_idx), msgUserId(msg_user_id)
{
    _state.value = 0;
    dSSN = sender_ssn ? sender_ssn : ownAddr.addr[1];
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.Dialog");
    APP_CONTEXT_T * acPtr = (APP_CONTEXT_T *)ACOID::OIDbyIdx(dialog_ac_idx);
    assert(acPtr);
    ac = *acPtr;
    ac_fact = ApplicationContextFactory::getFactory(_ac_idx);
    assert(ac_fact);
}

Dialog::~Dialog()
{
    MutexGuard  dtmp(dlgGrd);
    releaseAllInvokes(); //Release all invokes awaiting Result or LCancel
}

void Dialog::releaseAllInvokes(void)
{
    MutexGuard  tmp(invGrd);
    clearInvokes();
}

void Dialog::clearInvokes(void)
{
    InvokeMap::const_iterator it;
    for (it = originating.begin(); it != originating.end(); it++) {
        Invoke * inv = (*it).second;
        smsc_log_debug(logger, "Dialog[0x%X]: releasing %s", (unsigned)_dId,
                       inv->strStatus().c_str());
        delete inv;
    }
    originating.clear();
    for (it = terminating.begin(); it != terminating.end(); it++) {
        Invoke * inv = (*it).second;
        smsc_log_debug(logger, "Dialog[0x%X]: releasing %s", (unsigned)_dId,
                       inv->strStatus().c_str());
        delete inv;
    }
    terminating.clear();
}

void Dialog::reset(USHORT_T new_id, const SCCP_ADDRESS_T * rmt_addr/* = NULL*/)
{
    MutexGuard dtmp(dlgGrd);
    unsigned cnt = originating.size() + terminating.size();
    if (cnt) {
        smsc_log_warn(logger, "Dialog[0x%X]: resetting, %u invokes pending!",
                      (unsigned)_dId, cnt);
        releaseAllInvokes();
    }
    _lastInvId = 0;
    _dId = new_id;
    _state.value = 0;
    listeners.clear();
    if (rmt_addr)
        rmtAddr = *rmt_addr;
}

TC_DlgState Dialog::getState(void)
{ 
    MutexGuard  tmp(dlgGrd);
    return _state; 
}

void Dialog::addListener(DialogListener* pListener)
{
    MutexGuard  tmp(dlgGrd);
    listeners.push_back(pListener);
}

void Dialog::removeListener(DialogListener* pListener)
{
    MutexGuard  tmp(dlgGrd);
    listeners.remove(pListener);
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
        throw CustomException(result, descr, getTcapReasonDescription(result));
    }
    return;
}

void Dialog::beginDialog(UCHAR_T* ui/* = NULL*/, USHORT_T uilen/* = 0*/) throw (CustomException)
{
    smsc_log_debug(logger, "BEGIN_REQ -> {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "  App. context: %s\n"
                    "  User info[%u]: %s\n"
                    "}",
                   dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, priority, qSrvc, 
                   DumpHex(rmtAddr.addrLen, rmtAddr.addr, _HexDump_CVSD).c_str(),
                   DumpHex(ownAddr.addrLen, ownAddr.addr, _HexDump_CVSD).c_str(),
                   DumpHex(ac.acLen, ac.ac, _HexDump_CVSD).c_str(),
                   uilen, DumpHex(uilen, ui).c_str()
                   );
    MutexGuard  tmp(dlgGrd);
    USHORT_T result = EINSS7_I97TBeginReq(
                        dSSN, msgUserId, TCAP_INSTANCE_ID,
                        _dId, priority, qSrvc,
                        rmtAddr.addrLen, rmtAddr.addr,
                        ownAddr.addrLen, ownAddr.addr,
                        ac.acLen, ac.ac,
                        uilen, ui);
    checkSS7res("TBeginReq failed", result);
    _state.s.dlgLInited = 1;
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
                    "  Dialog[0x%X]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Org. address: %s\n"
                    "  App. context[%u]: %s\n"
                    "}",
                    dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, priority, qSrvc, 
                    "", // DumpHex(ownAddr.addrLen, ownAddr.addr, _HexDump_CVSD).c_str(),
                    ac.acLen, DumpHex(ac.acLen, ac.ac, _HexDump_CVSD).c_str()
                   );

    MutexGuard  tmp(dlgGrd);
    USHORT_T result =
        EINSS7_I97TContinueReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId,
                               priority, qSrvc, 0, NULL, //ownAddr.addrLen, ownAddr.addr,
                               ac.acLen, ac.ac, 0, NULL);

    checkSS7res("TContinueReq failed", result);
    _state.s.dlgLContinued = 1;
}


void Dialog::endDialog(bool basicEnd/* = true*/) throw (CustomException)
{
    MutexGuard  tmp(dlgGrd);
    if (!(_state.value & TC_DLG_CLOSED_MASK)) {
        USHORT_T termination = basicEnd ? EINSS7_I97TCAP_TERM_BASIC_END :
            EINSS7_I97TCAP_TERM_PRE_ARR_END;

        smsc_log_debug(logger, "END_REQ -> {"
                        "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                        "  Dialog[0x%X]\n"
                        "  PriOrder: 0x%X, QoS: 0x%X\n"
                        "  Termination: %s\n"
                        "  App. context[%u]: %s\n"
                        "}",
                       dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, priority, qSrvc,
                       basicEnd ? "BASIC" : "PREARRANGED", ac.acLen, 
                       DumpHex(ac.acLen, ac.ac, _HexDump_CVSD).c_str());

        USHORT_T result =
            EINSS7_I97TEndReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId,
                              priority, qSrvc, termination,
                              ac.acLen, ac.ac, 0, NULL);

        checkSS7res("TEndReq failed", result); //throws
        _state.s.dlgLEnded = 1;
    }
    releaseAllInvokes();
    smsc_log_debug(logger, "Dialog[0x%X]: %s (state = 0x%x)", _dId,
        (_state.value & TC_DLG_ABORTED_MASK) ? "aborted" : "ended", _state.value);
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
                "tag=\"LOCAL\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, msgUserId, TCAP_INSTANCE_ID, _dId,
                inv->getId(), linked ? "YES" : "NO", linked ? linked->getId() : 0,
                op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result = 
        EINSS7_I97TInvokeReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, inv->getId(), 
            linked ? EINSS7_I97TCAP_LINKED_ID_USED : EINSS7_I97TCAP_LINKED_ID_NOT_USED,
            linked ? linked->getId() : 0,
            EINSS7_I97TCAP_OP_CLASS_1, invTimeout, EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
            op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("InvokeReq failed", result);
}

void Dialog::sendResultLast(TcapEntity* res) throw (CustomException)
{
    RawBuffer   op, params;
    res->encode(op, params); //throws CustomException

    smsc_log_debug(logger, "EINSS7_I97TResultLReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, tag=\"LOCAL\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, res->getId(),
                op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TResultLReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId,
        res->getId(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
        op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("ResultLReq failed", result);
}

void Dialog::sendResultNotLast(TcapEntity* res) throw (CustomException)
{
    RawBuffer   op, params;
    res->encode(op, params); //throws CustomException

    smsc_log_debug(logger, "EINSS7_I97TResultNLReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, tag=\"LOCAL\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, res->getId(), 
                op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TResultNLReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId,
        res->getId(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
        op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("ResultNLReq failed", result);
}


void Dialog::sendResultError(TcapEntity* res) throw (CustomException)
{
    RawBuffer   op, params;
    res->encode(op, params);  //throws CustomException

    smsc_log_debug(logger, "EINSS7_I97TUErrorReq("
                "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                "invokeId=%d, tag=\"LOCAL\", opcode[%d]={%s}, parameters[%d]={%s})",
                dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, res->getId(),
                op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TUErrorReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId,
        res->getId(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
        op.size(), &op[0], params.size(), &params[0]);

    checkSS7res("UErrorReq failed", result);
}


void Dialog::resetInvokeTimer(UCHAR_T invokeId) throw (CustomException)
{
    smsc_log_debug(logger,"EINSS7_I97TTimerResetReq("
                   "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, invokeId=%d",
                   dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, invokeId);

    USHORT_T result = 
        EINSS7_I97TTimerResetReq(dSSN, msgUserId, TCAP_INSTANCE_ID, _dId, invokeId);

    checkSS7res("TTimerResetReq failed", result);
}

Invoke* Dialog::initInvoke(UCHAR_T opcode, InvokeListener * pLst/* = NULL*/,
                           USHORT_T timeout/* = 0*/)
{
    MutexGuard  tmp(invGrd);
    Invoke::InvokeResponse  resp = Invoke::respNone;

    if (ac_fact->hasErrors(opcode))
        resp = Invoke::respError;
    if (ac_fact->hasResult(opcode))
        resp = Invoke::respResultOrError;

    Invoke* inv = new Invoke(getNextInvokeId(), opcode, resp, pLst);
    inv->setTimeout(timeout ? timeout : _timeout);

    //register invoke if OPERATION has RESULT/ERRORS defined,
    //in order to provide possibility to call result/error listeners
    if (resp != Invoke::respNone)
        originating.insert(InvokeMap::value_type(inv->getId(), inv));
    else
        terminating.insert(InvokeMap::value_type(inv->getId(), inv));
    smsc_log_debug(logger, "Dialog[0x%X]: initiated %s", (unsigned)_dId,
                   inv->strStatus().c_str());
    return inv;
}

void Dialog::releaseInvoke(UCHAR_T invId)
{
    MutexGuard  tmp(invGrd);
    Invoke * inv = NULL;
    //search both maps for Invoke and unregister it
    InvokeMap::iterator it = originating.find(invId);
    if (it == originating.end()) {
        it = terminating.find(invId);
        if (it != terminating.end()) {
            inv = (*it).second;
            terminating.erase(it);
        }
    } else {
        inv = (*it).second;
        originating.erase(it);
    }
    if (inv) {
        smsc_log_debug(logger, "Dialog[0x%X]: releasing %s", (unsigned)_dId,
                       inv->strStatus().c_str());
        delete inv;
    } else {
        smsc_log_error(logger, "Dialog[0x%X]: releasing unregistered Invoke[%u]",
                       (unsigned)_dId, (unsigned)invId);
    }
    return;
}

//returns true is dialog has some Invokes pending (awaiting Result or LCancel)
unsigned Dialog::pendingInvokes(void)
{
    MutexGuard  tmp(invGrd);
    return originating.size() + terminating.size();
}

/* ------------------------------------------------------------------------ *
 * Transaction layer callbacks
 * ------------------------------------------------------------------------ */
/* NOTE: Unimplemented yet!
USHORT_T Dialog::handleBeginDialog(bool compPresent)
{
    MutexGuard tmp(dlgGrd);
    _state.s.dlgRInited = compPresent ?
            TCAP_DLG_COMP_WAIT : TCAP_DLG_COMP_LAST;
    //...
    return MSG_OK;
}
*/
USHORT_T Dialog::handleContinueDialog(bool compPresent)
{
    ListenerList    cpList;
    {
        MutexGuard tmp(dlgGrd);
        _state.s.dlgRContinued = compPresent ? TCAP_DLG_COMP_WAIT : TCAP_DLG_COMP_LAST;
        cpList = listeners;
    }
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogContinue(compPresent);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleEndDialog(bool compPresent)
{
    ListenerList cpList;
    {
        MutexGuard tmp(dlgGrd);
        if (!compPresent) {
            _state.s.dlgREnded = TCAP_DLG_COMP_LAST;
            releaseAllInvokes();
        } else //wait for ongoing invoke/result/error
            _state.s.dlgREnded = TCAP_DLG_COMP_WAIT;
        cpList = listeners;
    }
    //NOTE: calling onDialogREnd() may lead to ~Dialog()/reset() being called,
    //so iterate over ListenerList copy.
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogREnd(compPresent);
    }
    return MSG_OK;
}

//Reports dialog abort to dialogListener
USHORT_T Dialog::handlePAbortDialog(UCHAR_T abortCause)
{
    ListenerList cpList;
    {
        MutexGuard dtmp(dlgGrd);
        _state.s.dlgPAborted = 1;
        releaseAllInvokes();
        cpList = listeners;
    }
    //NOTE: calling onDialogPAbort() may lead to ~Dialog()/reset() being called,
    //so iterate over ListenerList copy.
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogPAbort(abortCause);
    }
    return MSG_OK;
}

USHORT_T Dialog::handleUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                          USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    ListenerList cpList;
    {
        MutexGuard dtmp(dlgGrd);
        _state.s.dlgRUAborted = 1;
        releaseAllInvokes();
        cpList = listeners;
    }
    //NOTE: calling onDialogUAbort() may lead to ~Dialog()/reset() being called,
    //so iterate over ListenerList copy.
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogUAbort(abortInfo_len, pAbortInfo, userInfo_len, pUserInfo);
    }
    return MSG_OK;
}


/* ------------------------------------------------------------------------ *
 * Component layer callbacks
 * ------------------------------------------------------------------------ */
//Reports incoming Invoke to dialogListener
USHORT_T Dialog::handleInvoke(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                              USHORT_T pmlen, const UCHAR_T *pm, bool lastComp)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "Dialog[0x%X]: illegal Invoke[%u] opcode: 0x%s",
                       (unsigned)_dId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        return MSG_OK;
    }

    ListenerList cpList;
    {
        MutexGuard tmp(dlgGrd);
        if (lastComp) {
            if (_state.s.dlgRContinued == TCAP_DLG_COMP_WAIT)
                _state.s.dlgRContinued = TCAP_DLG_COMP_LAST;
            else if (_state.s.dlgREnded == TCAP_DLG_COMP_WAIT)
                _state.s.dlgREnded = TCAP_DLG_COMP_LAST;
        }
        cpList = listeners;
    }
    Invoke  invoke(invId, op[0]);

    if (pmlen) { //operation parameters present
        Component* comp = ac_fact->createArg(op[0]);
        if (comp) {
            try {
                std::vector<unsigned char> code( pm, pm + pmlen );
                comp->decode(code); //throws
                invoke.ownParam(comp);
            } catch (std::exception & exc) {
                smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u]{%u}.Arg: %s",
                    (unsigned)_dId, (unsigned)invId, (unsigned)op[0], exc.what());
                delete comp;
            }
        } else {
            smsc_log_fatal(logger, "Dialog[0x%X]: Invoke[%u]{%u}: unregistered Arg",
                (unsigned)_dId, (unsigned)invId, (unsigned)op[0]);
        }
    }
    //NOTE: calling onDialogInvoke() may lead to ~Dialog()/reset() being called,
    //so iterate over ListenerList copy.
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogInvoke(&invoke, lastComp);
    }
    //if Dialog is remotedly ended, no LCancel will arise for invokes
    {
        MutexGuard tmp(dlgGrd);
        if (_state.s.dlgREnded == TCAP_DLG_COMP_LAST)
            clearInvokes();
    }
    return MSG_OK;
}

USHORT_T Dialog::handleLCancelInvoke(UCHAR_T invId)
{
    Invoke* pInv = NULL;
    Invoke  invCp;
    {
        MutexGuard  tmp(invGrd);
        //search both maps for Invoke and copy it onto stack
        InvokeMap::iterator it = originating.find(invId);
        if (it == originating.end()) {
            it = terminating.find(invId);
            if (it != terminating.end()) {
                pInv = (*it).second;
                terminating.erase(it);
                invCp.clone(pInv);
                delete pInv;
            }
        } else {
            pInv = (*it).second;
            originating.erase(it);
            invCp.clone(pInv);
            delete pInv;
        }
    }
    if (pInv) {
        if ((invCp.getStatus() <= Invoke::resNotLast)
            && (invCp.getResultType() == Invoke::respResultOrError))
            smsc_log_error(logger,
                "Dialog[0x%X]: Invoke[%u]{%u} got L_CANCEL expecting returnResult",
                (unsigned)_dId, (unsigned)invId, invCp.getOpcode());
        smsc_log_debug(logger, "Dialog[0x%X]: releasing %s", (unsigned)_dId,
                       invCp.strStatus().c_str());

        try { invCp.notifyResultListener(NULL, Invoke::resLCancel);
        } catch (std::exception & exc) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].LCancel listener: %s",
                (unsigned)_dId, (unsigned)invId, exc.what());
        } catch (...) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].LCancel listener: unknown exception",
                (unsigned)_dId, (unsigned)invId);
        }
    }
    return MSG_OK;
}

USHORT_T Dialog::handleResultLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                                  USHORT_T pmlen, const UCHAR_T *pm)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "Dialog[0x%X]: illegal Invoke[%u] Result opcode: 0x%s",
                       (unsigned)_dId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        return MSG_OK;
    }

    Invoke* pInv = NULL;
    Invoke  invCp;
    {
        MutexGuard dtmp(dlgGrd);
        //search for originating Invoke and copy it onto stack
        MutexGuard itmp(invGrd);
        InvokeMap::iterator it = originating.find(invId);
        if (it != originating.end()) {
            pInv = (*it).second;
            originating.erase(it);
            invCp.clone(pInv);
            delete pInv;
        } else
            smsc_log_error(logger, "Dialog[0x%X]: Result[%u] for unknown Invoke[%u]",
                (unsigned)_dId, (unsigned)op[0], (unsigned)invId);

        if (_state.s.dlgRContinued == TCAP_DLG_COMP_WAIT)
            _state.s.dlgRContinued = TCAP_DLG_COMP_LAST;
        else if (_state.s.dlgREnded == TCAP_DLG_COMP_WAIT) {
            _state.s.dlgREnded = TCAP_DLG_COMP_LAST;
            clearInvokes();
        }
    }
    if (pInv) { //prepare result and call invoke result listeners
        TCResult  result(invId, op[0]);

        Component* resParm = ac_fact->createRes(op[0]);
        if (resParm) {
            try {
                std::vector<unsigned char> code(pm, pm + pmlen);
                resParm->decode(code);
                result.ownParam(resParm);
            } catch (std::exception & exc) {
                smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].Result[%u]: %s",
                    (unsigned)_dId, (unsigned)invId, (unsigned)op[0], exc.what());
                delete resParm;
            }
        } else if (pmlen)
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u]: unregistered Result[%u]",
                            (unsigned)_dId, (unsigned)invId, (unsigned)op[0]);
        //NOTE: notifyResultListeners() may lead to ~Dialog()/reset() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        unsigned dlgId = (unsigned)_dId;
        try { invCp.notifyResultListener(&result, Invoke::resLast);
        } catch (std::exception & exc) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].Result[%u] listener: %s",
                dlgId, (unsigned)invId, (unsigned)op[0], exc.what());
        } catch (...) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].Result[%u] listener: unknown exception", 
                dlgId, (unsigned)invId, (unsigned)op[0]);
        }
        smsc_log_debug(logger, "Dialog[0x%X]: releasing %s", dlgId, invCp.strStatus().c_str());
    }
    return MSG_OK;
}

USHORT_T Dialog::handleResultNotLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                                     USHORT_T pmlen, const UCHAR_T *pm)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "Dialog[0x%X]: illegal Invoke[%u] ResultNL opcode: 0x%s",
                       (unsigned)_dId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        return MSG_OK;
    }


    MutexGuard  tmp(invGrd);
    //search for originating Invoke, prepare result and call invoke result listeners
    InvokeMap::const_iterator it = originating.find(invId);
    if (it != originating.end()) {
        TCResultNL  result(invId, op[0]);

        Component* resParm = ac_fact->createRes(op[0]);
        if (resParm) {
            try {
                std::vector<unsigned char> code(pm, pm + pmlen);
                resParm->decode(code);
                result.ownParam(resParm);
            } catch (std::exception & exc) {
                smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].ResultNL[%u]: %s",
                    (unsigned)_dId, (unsigned)invId, (unsigned)op[0], exc.what());
                delete resParm;
            }
        } else if (pmlen)
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u]: unregistered ResultNL[%u]",
                            (unsigned)_dId, (unsigned)invId, (unsigned)op[0]);

        Invoke* inv = (*it).second;
        //NOTE: notifyResultNListeners() may lead to ~Dialog()/reset() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        try { inv->notifyResultListener(&result, Invoke::resNotLast);
        } catch (std::exception & exc) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].ResultNL[%u] listener: %s",
                (unsigned)_dId, (unsigned)invId, (unsigned)op[0], exc.what());
        } catch (...) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].ResultNL[%u] listener: unknown exception", 
                (unsigned)_dId, (unsigned)invId, (unsigned)op[0]);
        }
    } else
        smsc_log_warn(logger, "Dialog[0x%X]: ResultNL[%u] for unknown Invoke[%u]",
                (unsigned)_dId, (unsigned)op[0], (unsigned)invId);
    return MSG_OK;
}

USHORT_T Dialog::handleResultError(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                                 USHORT_T pmlen, const UCHAR_T *pm)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "Dialog[0x%X]: illegal Invoke[%u] Error opcode: 0x%s",
                       (unsigned)_dId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        return MSG_OK;
    }

    Invoke* pInv = NULL;
    Invoke  invCp;
    {
        MutexGuard dtmp(dlgGrd);
        //search for originating Invoke and copy it onto stack
        MutexGuard itmp(invGrd);
        InvokeMap::iterator it = originating.find(invId);
        if (it != originating.end()) {
            pInv = (*it).second;
            originating.erase(it);
            invCp.clone(pInv);
            delete pInv;
        } else
            smsc_log_error(logger, "Dialog[0x%X]: Error[%u] for unknown Invoke[%u]",
                (unsigned)_dId, (unsigned)op[0], (unsigned)invId);

        if (_state.s.dlgRContinued == TCAP_DLG_COMP_WAIT)
            _state.s.dlgRContinued = TCAP_DLG_COMP_LAST;
        else if (_state.s.dlgREnded == TCAP_DLG_COMP_WAIT) {
            _state.s.dlgREnded = TCAP_DLG_COMP_LAST;
            clearInvokes();
        }
    }
    if (pInv) { //prepare resultError and call invoke result listener
        TCError  resErr(invId, op[0]);

        if (ac_fact->hasError(pInv->getOpcode(), op[0])) {
            Component* resParm = ac_fact->createErr(op[0]);
            if (resParm) {
                try {
                    std::vector<unsigned char> code(pm, pm + pmlen);
                    resParm->decode(code); //throws
                    resErr.ownParam(resParm);
                } catch (std::exception & exc) {
                    smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].Error[%u]: %s",
                        (unsigned)_dId, (unsigned)invId, (unsigned)op[0], exc.what());
                    delete resParm;
                }
            } else if (pmlen)
                smsc_log_warn(logger, "Dialog[0x%X]: Invoke[%u]: unregistered Error[%u] parametr",
                                (unsigned)_dId, (unsigned)invId, (unsigned)op[0]);
        } else
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u]: unregistered Error[%u]",
                            (unsigned)_dId, (unsigned)invId, (unsigned)op[0]);
        //NOTE: notifyResultListeners() may lead to ~Dialog()/reset() being called !!!
        //ResultListener should copy 'resParm', not take its ownership !
        unsigned dlgId = (unsigned)_dId;
        try { invCp.notifyResultListener(&resErr, Invoke::resError);
        } catch (std::exception & exc) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].Error[%u] listener: %s",
                dlgId, (unsigned)invId, (unsigned)op[0], exc.what());
        } catch (...) {
            smsc_log_error(logger, "Dialog[0x%X]: Invoke[%u].Error[%u] listener: unknown exception", 
                dlgId, (unsigned)invId, (unsigned)op[0]);
        }
        smsc_log_debug(logger, "Dialog[0x%X]: releasing %s", dlgId, invCp.strStatus().c_str());
    }
    return MSG_OK;
}

USHORT_T Dialog::handleNoticeInd(UCHAR_T reportCause,
                TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                UCHAR_T invokeId/* = 0*/, USHORT_T oplen/* = 0*/, const UCHAR_T *op/* = NULL*/)
{
    //if component is returned, it must have only local operation tag
    assert(!oplen || (oplen == 1));
    ListenerList cpList;
    {
        MutexGuard dtmp(dlgGrd);
        cpList = listeners;
    }
    //NOTE: calling onDialogNotice() may lead to ~Dialog()/reset() being called,
    //so iterate over ListenerList copy.
    for (ListenerList::iterator it = cpList.begin(); it != cpList.end(); it++) {
        DialogListener* ptr = *it;
        ptr->onDialogNotice(reportCause, comp_kind, invokeId, !oplen ? 0 : op[0]);
    }
    return MSG_OK;

}

} // namespace inap
} // namespace inman
} // namespace smsc
