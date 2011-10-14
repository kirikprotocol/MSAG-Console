#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************* *
 * TCAP dialog implementation (initiated by local point).
 * ************************************************************************* */
#include <assert.h>
#include <memory>

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "inman/inap/dialog.hpp"
#include "inman/inap/dispatcher.hpp"
#include "inman/inap/TCAPErrors.hpp"

using namespace smsc::util;

namespace smsc  {
namespace inman {
namespace inap  {

Dialog::Dialog(const std::string & sess_uid, const TCDialogID & dlg_id, USHORT_T msg_user_id,
               const ROSComponentsFactory * use_fact,
               const SCCP_ADDRESS_OCTS & loc_addr, UCHAR_T sender_ssn/* = 0*/,
               Logger * uselog/* = NULL*/)
    : _tcSUId(sess_uid), ownAddr(loc_addr), msgUserId(msg_user_id)
    , ac(use_fact->acOID()), acFab(use_fact), _dId(dlg_id)
    , _timeout(_DFLT_INVOKE_TIMER), qSrvc(EINSS7_I97TCAP_QLT_BOTH)
    , priority(EINSS7_I97TCAP_PRI_HIGH_0), _lastInvId(0), logger(uselog)
{
    dSSN = sender_ssn ? sender_ssn : ownAddr.addr[1];
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.Dialog");
    snprintf(_logId, sizeof(_logId)-1, "Dialog[%u:%Xh]",
             (unsigned)_dId.tcInstId, (unsigned)_dId.dlgId);
}

Dialog::~Dialog()
{
    MutexGuard  tmp(dlgGrd);
    clearInvokes();
}

void Dialog::releaseAllInvokes(void)
{
    MutexGuard  tmp(dlgGrd);
    clearInvokes();
}

void Dialog::clearInvokes(void)
{
    for (InvokeMap::const_iterator it = invMap.begin(); it != invMap.end(); ++it) {
        smsc_log_debug(logger, "%s: releasing %s", _logId, it->second->strStatus().c_str());
        if (!_state.isClosed())
            EINSS7_I97TUCancelReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId, it->second->getId());
    }
    invMap.clear();
}

void Dialog::reset(const TCDialogID & new_id, const SCCP_ADDRESS_OCTS * rmt_addr/* = NULL*/)
{
    MutexGuard dtmp(dlgGrd);
    unsigned cnt = (unsigned)invMap.size();
    if (cnt || pUser.get()) {
        smsc_log_error(logger, "%s: resetting to [%u:%Xh], %u invokes pending",
                       _logId, (unsigned)new_id.tcInstId, (unsigned)new_id.dlgId,
                       cnt, pUser.get() ? ", user refs exist":"");
        clearInvokes();
    } else if (!(_dId == new_id)) { //otherwise dialog was just created
        smsc_log_debug(logger, "%s: resetting to [%u:%Xh]", _logId,
                       (unsigned)new_id.tcInstId, (unsigned)new_id.dlgId);
    }

    pUser.Reset(NULL);
    _lastInvId = 0;
    _dId = new_id;
    snprintf(_logId, sizeof(_logId)-1, "Dialog[%u:%Xh]",
             (unsigned)_dId.tcInstId, (unsigned)_dId.dlgId);
    _state.value.mask = 0;
    if (rmt_addr)
        rmtAddr = *rmt_addr;
}


void Dialog::setInvokeTimeout(USHORT_T timeout)
{
    MutexGuard  tmp(dlgGrd);
    _timeout = timeout ? timeout : _DFLT_INVOKE_TIMER;
}

void Dialog::checkSS7res(const char * descr, USHORT_T result) throw(CustomException)
{
    if (result) {
        throw CustomException((int)_RCS_TC_APIError->mkhash(result),
                            descr, _RCS_TC_APIError->explainCode(result).c_str());
    }
    return;
}

void Dialog::beginDialog(UCHAR_T* ui/* = NULL*/, USHORT_T uilen/* = 0*/) throw (CustomException)
{
    smsc_log_debug(logger, "T_BEGIN_REQ -> {"
                    "  SSN: %u, UserID: %u, %s\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "  App. context: %s\n"
                    "  User info[%u]: %s\n"
                    "}",
                   dSSN, msgUserId, idStr(), priority, qSrvc, 
                   DumpHex(rmtAddr.addrLen, rmtAddr.addr, _HexDump_CVSD).c_str(),
                   DumpHex(ownAddr.addrLen, ownAddr.addr, _HexDump_CVSD).c_str(),
                   DumpHex(ac.length(), ac.octets(), _HexDump_CVSD).c_str(),
                   uilen, DumpHex(uilen, ui).c_str()
                   );
    MutexGuard  tmp(dlgGrd);
    USHORT_T result = EINSS7_I97TBeginReq(
                        dSSN, msgUserId, _dId.tcInstId,
                        _dId.dlgId, priority, qSrvc,
                        rmtAddr.addrLen, rmtAddr.addr,
                        ownAddr.addrLen, ownAddr.addr,
                        ac.length(), (unsigned char*)ac.octets(),
                        uilen, ui);
    checkSS7res("TBeginReq failed", result);
    _state.value.s.dlgLInited = 1;
}

void Dialog::beginDialog(const SCCP_ADDRESS_OCTS& remote_addr, UCHAR_T* ui/* = NULL*/,
                         USHORT_T uilen/* = 0*/) throw (CustomException)
{
    rmtAddr = remote_addr;    
    beginDialog(ui, uilen);
}


void Dialog::continueDialog(void) throw (CustomException)
{
    smsc_log_debug(logger, "T_CONTINUE_REQ -> {"
                    "  SSN: %u, UserID: %u, %s\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Org. address: %s\n"
                    "  App. context[%u]: %s\n"
                    "}",
                    dSSN, msgUserId, idStr(), priority, qSrvc, 
                    "", // DumpHex(ownAddr.addrLen, ownAddr.addr, _HexDump_CVSD).c_str(),
                    ac.length(), DumpHex(ac.length(), ac.octets(), _HexDump_CVSD).c_str()
                   );

    MutexGuard  tmp(dlgGrd);
    USHORT_T result =
        EINSS7_I97TContinueReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId,
                               priority, qSrvc, 0, NULL, //ownAddr.addrLen, ownAddr.addr,
                               ac.length(), (unsigned char*)ac.octets(), 0, NULL);

    checkSS7res("TContinueReq failed", result);
    _state.value.s.dlgLContinued = 1;
}

//bool basicEnd/* = true*/
void Dialog::endDialog(Dialog::Ending type/* = endBasic*/) throw (CustomException)
{
    MutexGuard  tmp(dlgGrd);
    if (!_state.isClosed()) {
        if (type == Dialog::endUAbort) {
            //NOTE: do not set abortInfo, so the EINSS7 selects appropriate
            //variant of TCAP PDU: either ABRT-apdu or AARE-apdu with 
            //Associate-source-diagnostic set to  dialogue-service-user : null(0)
            smsc_log_debug(logger, "T_U_ABORT_REQ {"
                            "  SSN: %u, UserID: %u, %s\n"
                            "  PriOrder: 0x%X, QoS: 0x%X\n"
                            "  Abort info: \n"
                            "  App. context: \n"
                            "  User info: \n"
                            "}",
                           dSSN, msgUserId, idStr(), priority, qSrvc);
            USHORT_T result =
                EINSS7_I97TUAbortReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId,
                                priority, qSrvc, 0, NULL, 0, NULL, 0, NULL);

            checkSS7res("TUAbortReq failed", result); //throws
            _state.value.s.dlgLUAborted = 1;
        } else {
            USHORT_T termination = (type == Dialog::endBasic) ?
                EINSS7_I97TCAP_TERM_BASIC_END : EINSS7_I97TCAP_TERM_PRE_ARR_END;
    
            smsc_log_debug(logger, "T_END_REQ -> {"
                            "  SSN: %u, UserID: %u, %s\n"
                            "  PriOrder: 0x%X, QoS: 0x%X\n"
                            "  Termination: %s\n"
                            "  App. context[%u]: %s\n"
                            "}",
                           dSSN, msgUserId, idStr(), priority, qSrvc,
                           (type == Dialog::endBasic) ? "BASIC" : "PREARRANGED", ac.length(), 
                           DumpHex(ac.length(), ac.octets(), _HexDump_CVSD).c_str());
    
            USHORT_T result =
                EINSS7_I97TEndReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId,
                                  priority, qSrvc, termination,
                                  ac.length(), (unsigned char*)ac.octets(), 0, NULL);
    
            checkSS7res("TEndReq failed", result); //throws
            _state.value.s.dlgLEnded = 1;
        }
    }
    clearInvokes();
    smsc_log_debug(logger, "%s: %s (state {%s})", _logId,
        _state.isAborted() ? "aborted" : "ended", _state.Print().c_str());
}


UCHAR_T /*inv_id*/ Dialog::sendInvoke(UCHAR_T opcode, const Component *p_arg,
                                        USHORT_T timeout/* = 0*/) throw(CustomException)
{
    RawBuffer   op, params;
    op.push_back(opcode);
    try {
        if (p_arg)
            p_arg->encode(params); //throws CustomException
    } catch (const std::exception & exc) {
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::invCompEnc),
                _RCS_TC_Dialog->explainCode(TC_DlgError::invCompEnc).c_str(), exc.what());
    }

    Invoke::Response_e  resp = Invoke::respNone;
    if (acFab->hasErrors(opcode))
        resp = Invoke::respError;
    if (acFab->hasResult(opcode))
        resp = Invoke::respResultOrError;

    UCHAR_T invId = getNextInvokeId();
    std::auto_ptr<Invoke> inv(new Invoke(invId, opcode, resp));
    inv->setTimeout(timeout ? timeout : _timeout);
   
    const Invoke * linked = inv->getLinkedTo();
    smsc_log_debug(logger, "T_INVOKE_REQ -> {"
                    "  SSN: %u, UserID: %u, %s\n"
                    "  invId: %u, opcode[%u]: %s, timeout: %u\n"
                    "  lunked: \"%s\", linkedId: %u\n"
                    "  parameters[%u]: %s\n"
                    "}",
                   dSSN, msgUserId, idStr(), inv->getId(),
                   op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                   inv->getTimeout(), linked ? "YES" : "NO", linked ? linked->getId() : 0,
                   params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result = 0;
    {
        MutexGuard  tmp(dlgGrd);
        result = EINSS7_I97TInvokeReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId, inv->getId(),
                linked ? EINSS7_I97TCAP_LINKED_ID_USED : EINSS7_I97TCAP_LINKED_ID_NOT_USED,
                linked ? linked->getId() : 0,
                EINSS7_I97TCAP_OP_CLASS_1, inv->getTimeout(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
                (USHORT_T)op.size(), &op[0], (USHORT_T)params.size(), params.empty() ? NULL : &params[0]);
        if (!result) {
            smsc_log_debug(logger, "%s: initiated %s", _logId, inv->strStatus().c_str());
            invMap.insert(InvokeMap::value_type(inv->getId(), InvokeRFP(inv.get())));
            inv.release();
        }
    }
    checkSS7res("InvokeReq failed", result); //throws
    return invId;
}

void Dialog::sendResultLast(TcapEntity* res) throw (CustomException)
{
    RawBuffer   op, params;
    try { res->encode(op, params); //throws CustomException    
    } catch (const std::exception & exc) {
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::resCompEnc),
                            _RCS_TC_Dialog->explainCode(TC_DlgError::resCompEnc).c_str(), exc.what());
    }

    smsc_log_debug(logger, "T_RESULT_REQ -> {"
                    "  SSN: %u, UserID: %u, %s\n"
                    "  invId: %u, opcode[%u]: %s\n"
                    "  parameters[%u]: %s\n"
                    "}",
                   dSSN, msgUserId, idStr(), res->getId(),
                   op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                   params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TResultLReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId,
        res->getId(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
        (USHORT_T)op.size(), &op[0], (USHORT_T)params.size(), &params[0]);

    checkSS7res("ResultLReq failed", result);
}

void Dialog::sendResultNotLast(TcapEntity* res) throw (CustomException)
{
    RawBuffer   op, params;
    try { res->encode(op, params); //throws CustomException    
    } catch (const std::exception & exc) {
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::resCompEnc),
                            _RCS_TC_Dialog->explainCode(TC_DlgError::resCompEnc).c_str(), exc.what());
    }

    smsc_log_debug(logger, "T_RESULT_NL_REQ -> {"
                    "  SSN: %u, UserID: %u, %s\n"
                    "  invId: %u, opcode[%u]: %s\n"
                    "  parameters[%u]: %s\n"
                    "}",
                   dSSN, msgUserId, idStr(), res->getId(),
                   op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                   params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TResultNLReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId,
        res->getId(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
        (USHORT_T)op.size(), &op[0], (USHORT_T)params.size(), &params[0]);

    checkSS7res("ResultNLReq failed", result); //throws
}


void Dialog::sendResultError(TcapEntity* res) throw (CustomException)
{
    RawBuffer   op, params;
    try { res->encode(op, params); //throws CustomException    
    } catch (const std::exception & exc) {
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::resCompEnc),
                            _RCS_TC_Dialog->explainCode(TC_DlgError::resCompEnc).c_str(), exc.what());
    }

    smsc_log_debug(logger, "T_U_ERROR_REQ -> {"
                    "  SSN: %u, UserID: %u, %s\n"
                    "  invId: %u, opcode[%u]: %s\n"
                    "  parameters[%u]: %s\n"
                    "}",
                   dSSN, msgUserId, idStr(), res->getId(),
                   op.size(), DumpHex(op.size(), &op[0], _HexDump_CVSD).c_str(),
                   params.size(), DumpHex(params.size(), &params[0]).c_str());

    USHORT_T result =
        EINSS7_I97TUErrorReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId,
        res->getId(), EINSS7_I97TCAP_OPERATION_TAG_LOCAL,
        (USHORT_T)op.size(), &op[0], (USHORT_T)params.size(), &params[0]);

    checkSS7res("UErrorReq failed", result);
}

//Returns TC_APIError
USHORT_T Dialog::resetInvokeTimer(UCHAR_T inv_id)
{
    MutexGuard  tmp(dlgGrd);
    InvokeMap::iterator it = invMap.find(inv_id);
    if (it != invMap.end()) {
        smsc_log_debug(logger, "T_TIMER_RESET_REQ -> {"
                        "  SSN: %u, UserID: %u, %s, invId: %u\n"
                        "}",
                       dSSN, msgUserId, idStr(), inv_id);
        return EINSS7_I97TTimerResetReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId, inv_id);
    }
    return EINSS7_I97TCAP_INV_INVOKE_ID_USED;
}

void Dialog::releaseInvoke(UCHAR_T invId)
{
    MutexGuard  tmp(dlgGrd);
    InvokeMap::iterator it = invMap.find(invId);
    if (it != invMap.end()) {
        Invoke* inv = it->second.get();
        smsc_log_debug(logger, "%s: releasing %s", _logId, inv->strStatus().c_str());
        EINSS7_I97TUCancelReq(dSSN, msgUserId, _dId.tcInstId, _dId.dlgId, invId);
        invMap.erase(it);
    } else {
        smsc_log_error(logger, "%s: releasing unregistered Invoke[%u]", _logId, (unsigned)invId);
    }
    return;
}


/* ------------------------------------------------------------------------ *
 * Transaction layer callbacks
 * ------------------------------------------------------------------------ */
/* NOTE: Unimplemented yet!
USHORT_T Dialog::handleBeginDialog(bool compPresent)
{
    MutexGuard tmp(dlgGrd);
    _state.value.s.dlgRInited = compPresent ?
            TCAP_DLG_COMP_WAIT : TCAP_DLG_COMP_LAST;
    //...
    return MSG_OK;
}
*/
USHORT_T Dialog::handleContinueDialog(bool compPresent)
{
    {
        MutexGuard tmp(dlgGrd);
        _state.value.s.dlgRContinued = compPresent ? TCAP_DLG_COMP_WAIT : TCAP_DLG_COMP_LAST;
        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }
    pUser->onDialogContinue(compPresent);
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleEndDialog(bool compPresent)
{
    {
        MutexGuard tmp(dlgGrd);
        if (!compPresent) {
            _state.value.s.dlgREnded = TCAP_DLG_COMP_LAST;
            clearInvokes();
        } else //wait for ongoing invoke/result/error
            _state.value.s.dlgREnded = TCAP_DLG_COMP_WAIT;
        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }
    pUser->onDialogREnd(compPresent);
    unlockUser();
    return MSG_OK;
}

//Reports dialog abort to dialogListener
USHORT_T Dialog::handlePAbortDialog(UCHAR_T abortCause)
{
    {
        MutexGuard dtmp(dlgGrd);
        _state.value.s.dlgPAborted = 1;
        clearInvokes();
        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }
    pUser->onDialogPAbort(abortCause);
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                          USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
        {
        MutexGuard  dtmp(dlgGrd);
        _state.value.s.dlgRUAborted = 1;
        clearInvokes();
        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }
    pUser->onDialogUAbort(abortInfo_len, pAbortInfo, userInfo_len, pUserInfo);
    unlockUser();
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
        smsc_log_error(logger, "%s: illegal Invoke[%u] opcode: 0x%s",
                       _logId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        //TODO: send TUReject
        return MSG_OK;
    }

    Invoke  invoke(invId, op[0]);
    {
        MutexGuard tmp(dlgGrd);
        if (pmlen) { //operation parameters present
            Component* comp = acFab->createArg(op[0], logger);
            if (comp) {
                try {
                    std::vector<unsigned char> code( pm, pm + pmlen );
                    comp->decode(code); //throws
                    invoke.ownParam(comp);
                } catch (std::exception & exc) {
                    smsc_log_error(logger, "%s: Invoke[%u:%u].Arg: %s",
                                   _logId, (unsigned)invId, (unsigned)op[0], exc.what());
                    delete comp;
                }
            } else {
                smsc_log_fatal(logger, "%s: Invoke[%u:%u]: unregistered Arg",
                               _logId, (unsigned)invId, (unsigned)op[0]);
            }
        }
        if (!pUser.get()) {
            if (lastComp)
                updateState(true);
            return MSG_OK;
        }
        //postponing invokes cleanUp on T_END until return from TC User callback
        if (lastComp)
            updateState();
        pUser.Lock();
    }
    pUser->onDialogInvoke(&invoke, lastComp);
    if (lastComp)
    {
        MutexGuard dtmp(dlgGrd);
        cleanUpInvokes();
    }
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleLCancelInvoke(UCHAR_T invId)
{
    InvokeRFP pInv(NULL);
    {
        MutexGuard  dtmp(dlgGrd);
        InvokeMap::iterator it = invMap.find(invId);
        if (it == invMap.end())
            return MSG_OK;
        pInv = it->second;  //increases invoke ref count
        pInv->setStatus(Invoke::resLCancel);
        invMap.erase(it);
        if ((pInv->getStatus() <= Invoke::resNotLast)
            && (pInv->getResultType() == Invoke::respResultOrError)) {
            smsc_log_error(logger, "%s: %s got L_CANCEL expecting returnResult",
                           _logId, pInv->idStr().c_str());
        }
        smsc_log_debug(logger, "%s: releasing %s", _logId,
                       pInv->strStatus().c_str());

        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }
    try { pUser->onInvokeLCancel(pInv);
    } catch (const std::exception & exc) {
        smsc_log_error(logger, "%s: %s.LCancel, listener: %s",
                       _logId, pInv->idStr().c_str(), exc.what());
    } catch (...) {
        smsc_log_error(logger, "%s: %s.LCancel, listener: unknown exception",
                       _logId, pInv->idStr().c_str());
    }
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleResultLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                                  USHORT_T pmlen, const UCHAR_T *pm)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "%s: illegal Invoke[%u] Result opcode: 0x%s",
                       _logId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        //TODO: send TUReject
        return MSG_OK;
    }

    InvokeRFP   pInv(NULL);
    TCResult    result(invId, op[0]);
    {
        MutexGuard dtmp(dlgGrd);
        InvokeMap::iterator it = invMap.find(invId);
        if (it == invMap.end()) {
            smsc_log_error(logger, "%s: Result[%u] for unknown Invoke[%u]",
                                _logId, (unsigned)op[0], (unsigned)invId);
            updateState(true);
            return MSG_OK;
        }
        pInv = it->second; //increases invoke ref count
        pInv->setStatus(Invoke::resLast);
        invMap.erase(it); //remove invoke from list of monitored ones
        
        //prepare result
        Component* resParm = acFab->createRes(op[0], logger);
        if (resParm) {
            try {
                std::vector<unsigned char> code(pm, pm + pmlen);
                resParm->decode(code);
                result.ownParam(resParm);
            } catch (const std::exception & exc) {
                smsc_log_error(logger, "%s: Invoke[%u].Result[%u]: %s",
                               _logId, (unsigned)invId, (unsigned)op[0], exc.what());
                delete resParm;
            }
        } else if (pmlen) {
            smsc_log_error(logger, "%s: Invoke[%u]: unregistered Result[%u]",
                            _logId, (unsigned)invId, (unsigned)op[0]);
        }

        if (!pUser.get()) {
            updateState(true);
            return MSG_OK;
        }
        updateState(); //postponing invokes cleanUp on T_END until return from TC User callback
        pUser.Lock();
    }

    try { pUser->onInvokeResult(pInv, &result);
    } catch (const std::exception & exc) {
        smsc_log_error(logger, "%s: Invoke[%u].Result[%u] listener: %s",
                       _logId, (unsigned)invId, (unsigned)op[0], exc.what());
    } catch (...) {
        smsc_log_error(logger, "%s: Invoke[%u].Result[%u] listener: unknown exception", 
                       _logId, (unsigned)invId, (unsigned)op[0]);
    }
    {
        MutexGuard dtmp(dlgGrd);
        cleanUpInvokes();
    }
    smsc_log_debug(logger, "%s: releasing %s", _logId, pInv->strStatus().c_str());
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleResultNotLast(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                                     USHORT_T pmlen, const UCHAR_T *pm)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "%s: illegal Invoke[%u] ResultNL opcode: 0x%s",
                       _logId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        //TODO: send TUReject
        return MSG_OK;
    }
    
    InvokeRFP   pInv(NULL);
    TCResultNL  result(invId, op[0]);
    {   //search for originating Invoke
        MutexGuard dtmp(dlgGrd);
        InvokeMap::iterator it = invMap.find(invId);
        if (it == invMap.end()) {
            smsc_log_error(logger, "%s: ResultNL[%u] for unknown Invoke[%u]",
                           _logId, (unsigned)op[0], (unsigned)invId);
            return MSG_OK;

        }
        pInv = it->second;
        pInv->setStatus(Invoke::resNotLast);

        //prepare result
        Component* resParm = acFab->createRes(op[0], logger);
        if (resParm) {
            try {
                std::vector<unsigned char> code(pm, pm + pmlen);
                resParm->decode(code);
                result.ownParam(resParm);
            } catch (const std::exception & exc) {
                smsc_log_error(logger, "%s: Invoke[%u].ResultNL[%u]: %s",
                               _logId, (unsigned)invId, (unsigned)op[0], exc.what());
                delete resParm;
            }
        } else if (pmlen) {
            smsc_log_error(logger, "%s: Invoke[%u]: unregistered ResultNL[%u]",
                            _logId, (unsigned)invId, (unsigned)op[0]);
        }

        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }

    try { pUser->onInvokeResultNL(pInv, &result);
    } catch (const std::exception & exc) {
        smsc_log_error(logger, "%s: Invoke[%u].ResultNL[%u] listener: %s",
                       _logId, (unsigned)invId, (unsigned)op[0], exc.what());
    } catch (...) {
        smsc_log_error(logger, "%s: Invoke[%u].ResultNL[%u] listener: unknown exception", 
                       _logId, (unsigned)invId, (unsigned)op[0]);
    }
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleResultError(UCHAR_T invId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op,
                                 USHORT_T pmlen, const UCHAR_T *pm)
{
    if ((tag != EINSS7_I97TCAP_OPERATION_TAG_LOCAL) || (oplen != 1)) {
        smsc_log_error(logger, "%s: illegal Invoke[%u] Error opcode: 0x%s",
                       _logId, (unsigned)invId,
                       !oplen ? "00" : DumpHex(oplen, op).c_str());
        //TODO: send TUReject
        return MSG_OK;
    }

    InvokeRFP   pInv(NULL);
    TCError  resErr(invId, op[0]);
    {
        MutexGuard dtmp(dlgGrd);
        InvokeMap::iterator it = invMap.find(invId);
        if (it == invMap.end()) {
            smsc_log_error(logger, "%s: Error[%u] for unknown Invoke[%u]",
                                _logId, (unsigned)op[0], (unsigned)invId);
            updateState(true);
            return MSG_OK;
        }
        pInv = it->second;
        pInv->setStatus(Invoke::resError);
        invMap.erase(it);
        
        //prepare resultError
        if (acFab->hasError(pInv->getOpcode(), op[0])) {
            Component* resParm = acFab->createErr(op[0], logger);
            if (resParm) {
                try {
                    std::vector<unsigned char> code(pm, pm + pmlen);
                    resParm->decode(code); //throws
                    resErr.ownParam(resParm);
                } catch (const std::exception & exc) {
                    smsc_log_error(logger, "%s: Invoke[%u].Error[%u]: %s",
                                   _logId, (unsigned)invId, (unsigned)op[0], exc.what());
                    delete resParm;
                }
            } else if (pmlen) {
                smsc_log_warn(logger, "%s: Invoke[%u]: unregistered Error[%u] parameter",
                                _logId, (unsigned)invId, (unsigned)op[0]);
            }
        } else {
            smsc_log_error(logger, "%s: Invoke[%u]: unregistered Error[%u]",
                            _logId, (unsigned)invId, (unsigned)op[0]);
        }
        if (!pUser.get()) {
            updateState(true);
            return MSG_OK;
        }
        updateState();  //postponing invokes cleanUp on T_END
        pUser.Lock();
    }

    try { pUser->onInvokeError(pInv, &resErr);
    } catch (const std::exception & exc) {
        smsc_log_error(logger, "%s: Invoke[%u].Error[%u] listener: %s",
                       _logId, (unsigned)invId, (unsigned)op[0], exc.what());
    } catch (...) {
        smsc_log_error(logger, "%s: Invoke[%u].Error[%u] listener: unknown exception", 
                       _logId, (unsigned)invId, (unsigned)op[0]);
    }
    {
        MutexGuard dtmp(dlgGrd);
        cleanUpInvokes();
    }
    smsc_log_debug(logger, "%s: releasing %s", _logId, pInv->strStatus().c_str());
    unlockUser();
    return MSG_OK;
}

USHORT_T Dialog::handleNoticeInd(UCHAR_T reportCause,
                TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                UCHAR_T invokeId/* = 0*/, USHORT_T oplen/* = 0*/, const UCHAR_T *op/* = NULL*/)
{
    //if component is returned, it must have only local operation tag,
    //otherwise it's a misslinkage issue.
    assert(!oplen || (oplen == 1));
    {
        MutexGuard dtmp(dlgGrd);
        if (!pUser.get())
            return MSG_OK;
        pUser.Lock();
    }
    pUser->onDialogNotice(reportCause, comp_kind, invokeId, !oplen ? 0 : op[0]);
    unlockUser();
    return MSG_OK;
}

} // namespace inap
} // namespace inman
} // namespace smsc
