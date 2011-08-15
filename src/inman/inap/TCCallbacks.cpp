#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************* *
 * EINSS7 TCAP API Callbacks implementation
 * ************************************************************************* */

#include "util/BinDump.hpp"
using smsc::util::DumpHex;
using smsc::util::format;

#include "inman/inap/dispatcher.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::TcapEntity;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSNSession;
using smsc::inman::inap::TNoticeParms;
using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::rc2Txt_TC_BindResult;

//-------------------------------- TC API Callbacks link functions -------------
#include "inman/inap/TCCallbacks.hpp"
#include "inman/inap/EINTCLink.hpp"

using namespace smsc::util;

namespace smsc {
namespace inman {
namespace inap {

Mutex  TCCbkLink::_TCCbkSync;

TCCbkLink::TCCbkLink()
    : _disp(0), logger(Logger::getInstance("smsc.inman.inap.TCcb"))
{
    static struct EINSS7_I97TCAPINIT _cbTC;
    _cbTC.EINSS7_I97TBeginInd = EINSS7_I97TBeginInd;
    _cbTC.EINSS7_I97TBindConf = EINSS7_I97TBindConf;
    _cbTC.EINSS7_I97TContinueInd = EINSS7_I97TContinueInd;
    _cbTC.EINSS7_I97TEndInd = EINSS7_I97TEndInd;
    _cbTC.EINSS7_I97TIndError = EINSS7_I97TIndError;
    _cbTC.EINSS7_I97TInvokeInd = EINSS7_I97TInvokeInd;
    _cbTC.EINSS7_I97TLCancelInd = EINSS7_I97TLCancelInd;
    _cbTC.EINSS7_I97TLRejectInd = EINSS7_I97TLRejectInd;
    _cbTC.EINSS7_I97TNoticeInd = EINSS7_I97TNoticeInd;
    _cbTC.EINSS7_I97TPAbortInd = EINSS7_I97TPAbortInd;
    _cbTC.EINSS7_I97TResultLInd = EINSS7_I97TResultLInd;
    _cbTC.EINSS7_I97TResultNLInd = EINSS7_I97TResultNLInd;
    _cbTC.EINSS7_I97TRRejectInd = EINSS7_I97TRRejectInd;
    _cbTC.EINSS7_I97TStateInd = EINSS7_I97TStateInd;
    _cbTC.EINSS7_I97TUAbortInd = EINSS7_I97TUAbortInd;
    _cbTC.EINSS7_I97TUErrorInd = EINSS7_I97TUErrorInd;
    _cbTC.EINSS7_I97TUniInd = EINSS7_I97TUniInd;
    _cbTC.EINSS7_I97TURejectInd = EINSS7_I97TURejectInd;
    _cbTC.EINSS7_I97TAddressInd = EINSS7_I97TAddressInd;
}

} //inap
} //inman
} //smsc

using smsc::inman::inap::TCCbkLink;
using smsc::inman::inap::TCDialogID;

#define tcapLogger TCCbkLink::get().tcapLogger()

#define tccb_log_fatal(...) if (tcapLogger && tcapLogger->isFatalEnabled()) tcapLogger->log_(smsc::logger::Logger::LEVEL_FATAL, __VA_ARGS__)
#define tccb_log_error(...) if (tcapLogger && tcapLogger->isErrorEnabled()) tcapLogger->log_(smsc::logger::Logger::LEVEL_ERROR, __VA_ARGS__)
#define tccb_log_warn(...)  if (tcapLogger && tcapLogger->isWarnEnabled()) tcapLogger->log_(smsc::logger::Logger::LEVEL_WARN, __VA_ARGS__)
#define tccb_log_info(...)  if (tcapLogger && tcapLogger->isInfoEnabled()) tcapLogger->log_(smsc::logger::Logger::LEVEL_INFO, __VA_ARGS__)
#define tccb_log_debug(...) if (tcapLogger && tcapLogger->isDebugEnabled()) tcapLogger->log_(smsc::logger::Logger::LEVEL_DEBUG, __VA_ARGS__)

static SSNSession* findSSNsession(UCHAR_T ssn)
{
    SSNSession* pSession = NULL;
    TCAPDispatcher * dsp = TCCbkLink::get().tcapDisp();
    if (dsp && !(pSession = dsp->findSession(ssn)))
        tccb_log_warn("SS7TC: Invalid/inactive session, SSN[%u]", (unsigned)ssn);
    return pSession;
}

static Dialog* findDialog(UCHAR_T ssn, EINSS7INSTANCE_T tcapInstanceId,
                          USHORT_T dialogueId)
{
    Dialog* pDlg = NULL;
    SSNSession* pSession = findSSNsession(ssn);
    if (pSession && !(pDlg = pSession->findDialog(TCDialogID(tcapInstanceId, dialogueId))))
        tccb_log_warn("SS7TC: Invalid(closed) Dialog[%u:%Xh]",
                      (unsigned)tcapInstanceId, (unsigned)dialogueId);
    return pDlg;
}

//-------------------------------- Callbacks implementation -----------------------
/* function to be called when a connection is broken */
USHORT_T onEINSS7CpConnectBroken(USHORT_T from_usrID, USHORT_T to_usrID, UCHAR_T inst_id)
{
  TCAPDispatcher * dsp = TCCbkLink::get().tcapDisp();
  if (dsp)
    dsp->onDisconnect(from_usrID, to_usrID, inst_id);
  return MSG_OK;
}


USHORT_T EINSS7_I97TBindConf(UCHAR_T ssn, USHORT_T userId,
                            EINSS7INSTANCE_T tcapInstanceId, UCHAR_T bindResult
#ifdef EIN_HD
#if EIN_HD >= 101
                            , UCHAR_T sccpStandard
#endif /* EIN_HD >= 101 */
#endif /* EIN_HD */
                            )
{
#ifndef EIN_HD
    tccb_log_debug("SS7_I97TBindConf(SSN=%u, UserId=%u, TcapInstanceId=%u, bindResult=%u(%s))",
                   (unsigned)ssn, (unsigned)userId, (unsigned)tcapInstanceId,
                   (unsigned)bindResult, rc2Txt_TC_BindResult(bindResult));
#elif EIN_HD >= 101
    tccb_log_debug("SS7_I97TBindConf(SSN=%u, UserId=%u, TcapInstanceId=%u, bindResult=%u(%s), sccpStd=%u)",
                   (unsigned)ssn, (unsigned)userId, (unsigned)tcapInstanceId,
                   (unsigned)bindResult, rc2Txt_TC_BindResult(bindResult), (unsigned)sccpStandard);
#endif /* EIN_HD */


    TCAPDispatcher * dsp = TCCbkLink::get().tcapDisp();
    if (dsp)
        dsp->confirmSSN(ssn, tcapInstanceId, bindResult);
    return MSG_OK;
}

USHORT_T EINSS7_I97TStateInd(UCHAR_T          ssn,
                             USHORT_T         userId,
                             EINSS7INSTANCE_T tcapInstanceId,
                             UCHAR_T          userState,
                             UCHAR_T          affectedSsn,
                             ULONG_T          affectedSpc,
                             ULONG_T          localSpc,
                             UCHAR_T          subsysMultiplicityInd)
{
    tccb_log_debug("SS7_I97TStateInd(SSN=%u, UserId=%u, TcapInstanceId=%u, "
                    "UserState=%u, AffectedSsn=%u, AffectedSpc=%u, LocalSpc =%u, "
                    "SubsysMultiplicityInd=%u)",
                    ssn, userId, tcapInstanceId, userState, affectedSsn,
                    affectedSpc, localSpc, subsysMultiplicityInd);
    return MSG_OK;
}



USHORT_T EINSS7_I97TContinueInd(UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          priOrder,
                                UCHAR_T          qualityOfService,
#ifdef EIN_HD
#if EIN_HD >= 101
                                UCHAR_T         orgAdrLength,
                                UCHAR_T         *orgAdr_p,
#endif /* EIN_HD >= 101 */
#endif /* EIN_HD */
                                UCHAR_T          compPresent,
                                UCHAR_T          appContextLength,
                                UCHAR_T          *appContext_p,
                                USHORT_T         userInfoLength,
                                UCHAR_T          *userInfo_p)
{
#ifndef EIN_HD
    tccb_log_debug("CONTINUE_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Comp. present: %s\n"
                    "  App. context: %s\n"
                    "  User info: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId, priOrder,
                   qualityOfService, compPresent ? "TRUE":"FALSE",
                   DumpHex(appContextLength, appContext_p, _HexDump_CVSD).c_str(),
                   DumpHex(userInfoLength, userInfo_p, _HexDump_CVSD).c_str()
                   );
#elif EIN_HD >= 101
    tccb_log_debug("CONTINUE_IND {"
                    "  SSN: %u, UserID: %u,  Dialog[%u:%Xh]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Comp. present: %s\n"
                    "  App. context: %s\n"
                    "  User info: %s\n"
                    "  Org. adr: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId, priOrder,
                   qualityOfService, compPresent ? "TRUE":"FALSE",
                   DumpHex(appContextLength, appContext_p, _HexDump_CVSD).c_str(),
                   DumpHex(userInfoLength, userInfo_p, _HexDump_CVSD).c_str(),
                   DumpHex(orgAdrLength, orgAdr_p, _HexDump_CVSD).c_str()
                   );
#endif /* EIN_HD */
    Dialog* dlg = findDialog(ssn, tcapInstanceId, dialogueId);
    if (dlg)
        dlg->handleContinueDialog(compPresent ? true : false);
    return MSG_OK;
}


USHORT_T EINSS7_I97TAddressInd(UCHAR_T ssn,
				USHORT_T userId,
		                EINSS7INSTANCE_T tcapInstanceId,
				USHORT_T dialogueId,
				UCHAR_T bitMask,
				UCHAR_T addressLength,
				UCHAR_T *orgAdr_p)
{
    tccb_log_debug("ADDRESS_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  Bitmask: 0x%X\n"
                    "  Org. address: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId, (USHORT_T)bitMask,
                   DumpHex(addressLength, orgAdr_p, _HexDump_CVSD).c_str()
                   );
    return MSG_OK;
}


USHORT_T EINSS7_I97TEndInd(UCHAR_T          ssn,
                           USHORT_T         userId,
                           EINSS7INSTANCE_T tcapInstanceId,
                           USHORT_T         dialogueId,
                           UCHAR_T          priOrder,
                           UCHAR_T          qualityOfService,
                           UCHAR_T          compPresent,
                           UCHAR_T          appContextLength,
                           UCHAR_T          *appContext_p,
                           USHORT_T         userInfoLength,
                           UCHAR_T          *userInfo_p)
{
    tccb_log_debug("END_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Comp. present: %s\n"
                    "  App. context: %s\n"
                    "  User info: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId, priOrder,
                   qualityOfService, compPresent ? "TRUE":"FALSE",
                   DumpHex(appContextLength, appContext_p, _HexDump_CVSD).c_str(),
                   DumpHex(userInfoLength, userInfo_p, _HexDump_CVSD).c_str()
                   );
    Dialog* dlg = findDialog(ssn, tcapInstanceId, dialogueId);
    if (dlg)
        dlg->handleEndDialog(compPresent ? true : false);
    return MSG_OK;
}

USHORT_T EINSS7_I97TInvokeInd(UCHAR_T          ssn,
                              USHORT_T         userId,
                              EINSS7INSTANCE_T tcapInstanceId,
                              USHORT_T         dialogueId,
                              UCHAR_T          invokeId,
                              UCHAR_T          lastComponent,
                              UCHAR_T          linkedIdUsed,
                              UCHAR_T          linkedId,
                              UCHAR_T          tag,
                              USHORT_T         opLength,
                              UCHAR_T          *op,
                              USHORT_T         paramLength,
                              UCHAR_T          *pm)
{
    tccb_log_debug("INVOKE_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeID: 0x%X, LastComponent: %s, LinkedId: %s\n"
                    "  Operation: %s, Tag: %s\n"
                    "  Params[%u]: 0x%s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeId, (lastComponent ? "TRUE" : "FALSE"),
                    (linkedIdUsed ? format("0x%X", linkedId).c_str() : "NOT USED"),
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    SSNSession* pSession = findSSNsession(ssn);
    if (pSession) {
        TCDialogID    dlgId(tcapInstanceId, dialogueId);
        Dialog* dlg = pSession->findDialog(dlgId);
        if (dlg)
            dlg->handleInvoke(invokeId, tag, opLength, op, paramLength, pm, (bool)lastComponent);
        else { //look in noticed dialogs
            TNoticeParms    parms;
            if (pSession->noticeParms(dlgId, parms)) {
                if ((dlg = pSession->findDialog(parms.relId))) {
                    dlg->handleNoticeInd(parms.reportCause, TcapEntity::tceInvoke,
                                         invokeId, opLength, op);
                }
                EINSS7_I97TEndReq(ssn, userId, tcapInstanceId, dialogueId,
                                  0, 0, EINSS7_I97TCAP_TERM_PRE_ARR_END, 0, 0, 0, NULL);
                pSession->releaseDialog(dlgId);
            }
        }
    }
    return MSG_OK;
}

USHORT_T EINSS7_I97TResultNLInd(UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          tag,
                                USHORT_T         opLength,
                                UCHAR_T          *op,
                                USHORT_T         paramLength,
                                UCHAR_T          *pm)
{
    tccb_log_debug("RESULT_NL_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeID: 0x%X, LastComponent: %s\n"
                    "  Operation: %s, Tag: %s\n"
                    "  Params[%u]: 0x%s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeId, (lastComponent ? "TRUE" : "FALSE"),
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    SSNSession* pSession = findSSNsession(ssn);
    if (pSession) {
        TCDialogID    dlgId(tcapInstanceId, dialogueId);
        Dialog* dlg = pSession->findDialog(dlgId);
        if (dlg)
            dlg->handleResultNotLast(invokeId, tag, opLength, op, paramLength, pm);
        else { //look in noticed dialogs
            TNoticeParms    parms;
            if (pSession->noticeParms(dlgId, parms)) {
                if ((dlg = pSession->findDialog(parms.relId))) {
                    dlg->handleNoticeInd(parms.reportCause, TcapEntity::tceResultNL,
                                         invokeId, opLength, op);
                }
                EINSS7_I97TEndReq(ssn, userId, tcapInstanceId, dialogueId,
                                  0, 0, EINSS7_I97TCAP_TERM_PRE_ARR_END, 0, 0, 0, NULL);
                pSession->releaseDialog(dlgId);
            }
        }
    }
    return MSG_OK;
}

USHORT_T EINSS7_I97TResultLInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          tag,
                                USHORT_T         opLength,
                                UCHAR_T          *op,
                                USHORT_T         paramLength,
                                UCHAR_T          *pm)
{
    tccb_log_debug("RESULT_L_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeID: 0x%X, LastComponent: %s\n"
                    "  Operation: %s, Tag: %s\n"
                    "  Params[%u]: 0x%s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeId, (lastComponent ? "TRUE" : "FALSE"),
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    SSNSession* pSession = findSSNsession(ssn);
    if (pSession) {
        TCDialogID    dlgId(tcapInstanceId, dialogueId);
        Dialog* dlg = pSession->findDialog(dlgId);
        if (dlg)
            dlg->handleResultLast(invokeId, tag, opLength, op, paramLength, pm);
        else { //look in noticed dialogs
            TNoticeParms    parms;
            if (pSession->noticeParms(dlgId, parms)) {
                if ((dlg = pSession->findDialog(parms.relId))) {
                    dlg->handleNoticeInd(parms.reportCause, TcapEntity::tceResult,
                                         invokeId, opLength, op);
                }
                EINSS7_I97TEndReq(ssn, userId, tcapInstanceId, dialogueId,
                                  0, 0, EINSS7_I97TCAP_TERM_PRE_ARR_END, 0, 0, 0, NULL);
                pSession->releaseDialog(dlgId);
            }
        }
    }
    return MSG_OK;
}

USHORT_T EINSS7_I97TUErrorInd(UCHAR_T          ssn,
                              USHORT_T         userId,
                              EINSS7INSTANCE_T tcapInstanceId,
                              USHORT_T         dialogueId,
                              UCHAR_T          invokeId,
                              UCHAR_T          lastComponent,
                              UCHAR_T          tag,
                              USHORT_T         opLength,
                              UCHAR_T          *op,
                              USHORT_T         paramLength,
                              UCHAR_T          *pm)
{
    tccb_log_debug("U_ERROR_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeID: 0x%X, LastComponent: %s\n"
                    "  Error: %s, Tag: %s\n"
                    "  Params[%u]: %s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeId, (lastComponent ? "TRUE" : "FALSE"),
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    SSNSession* pSession = findSSNsession(ssn);
    if (pSession) {
        TCDialogID    dlgId(tcapInstanceId, dialogueId);
        Dialog* dlg = pSession->findDialog(dlgId);
        if (dlg)
            dlg->handleResultError(invokeId, tag, opLength, op, paramLength, pm);
        else { //look in noticed dialogs
            TNoticeParms    parms;
            if (pSession->noticeParms(dlgId, parms)) {
                if ((dlg = pSession->findDialog(parms.relId))) {
                    dlg->handleNoticeInd(parms.reportCause, TcapEntity::tceError,
                                         invokeId, opLength, op);
                }
                EINSS7_I97TEndReq(ssn, userId, tcapInstanceId, dialogueId,
                                  0, 0, EINSS7_I97TCAP_TERM_PRE_ARR_END, 0, 0, 0, NULL);
                pSession->releaseDialog(dlgId);
            }
        }
    }
    return MSG_OK;
}

USHORT_T EINSS7_I97TPAbortInd(UCHAR_T          ssn,
                              USHORT_T         userId,
                              EINSS7INSTANCE_T tcapInstanceId,
                              USHORT_T         dialogueId,
                              UCHAR_T          priOrder,
                              UCHAR_T          qualityOfService,
                              UCHAR_T          abortCause)
{
    tccb_log_error("P_ABORT_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Abort cause: 0x%X\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId,
                   priOrder, qualityOfService, abortCause
                   );
    Dialog* dlg = findDialog(ssn, tcapInstanceId, dialogueId);
    if (dlg)
        dlg->handlePAbortDialog(abortCause);
    return MSG_OK;
}

USHORT_T EINSS7_I97TUAbortInd(UCHAR_T          ssn,
                              USHORT_T         userId,
                              EINSS7INSTANCE_T tcapInstanceId,
                              USHORT_T         dialogueId,
                              UCHAR_T          priOrder,
                              UCHAR_T          qualityOfService,
                              USHORT_T         abortInfoLength,
                              UCHAR_T          *abortInfo_p,
                              UCHAR_T          appContextLength,
                              UCHAR_T          *appContext_p,
                              USHORT_T         userInfoLength,
                              UCHAR_T          *userInfo_p)
{
    tccb_log_error("U_ABORT_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Abort info: %s\n"
                    "  App. context: %s\n"
                    "  User info: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId,
                   priOrder, qualityOfService, 
                   DumpHex(abortInfoLength, abortInfo_p, _HexDump_CVSD).c_str(),
                   DumpHex(appContextLength, appContext_p, _HexDump_CVSD).c_str(),
                   DumpHex(userInfoLength, userInfo_p, _HexDump_CVSD).c_str()
                   );

    Dialog* dlg = findDialog(ssn, tcapInstanceId, dialogueId);
    return !dlg ? MSG_OK :
        dlg->handleUAbort(abortInfoLength, abortInfo_p, userInfoLength, userInfo_p);
}

USHORT_T EINSS7_I97TLCancelInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId)
{
    tccb_log_debug("L_CANCEL_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeId: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId, invokeId
                   );
    Dialog* dlg = findDialog(ssn, tcapInstanceId, dialogueId);
    if (dlg)
        dlg->handleLCancelInvoke(invokeId);
    return MSG_OK;
}

USHORT_T EINSS7_I97TNoticeInd(UCHAR_T          ssn,
                              USHORT_T         userId,
                              EINSS7INSTANCE_T tcapInstanceId,
                              USHORT_T         dialogueId,
                              UCHAR_T          reportCause,
                              UCHAR_T          returnIndicator,
                              USHORT_T         relDialogueId,
                              UCHAR_T          segmentationIndicator,
                              UCHAR_T          destAddrLength,
                              UCHAR_T          *destAddr_p,
                              UCHAR_T          orgAddrLength,
                              UCHAR_T          *orgAddr_p)
{
    tccb_log_error("NOTICE_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  RelDialog[%u:%Xh]\n"
                    "  ReportCause: 0x%X, ReturnIndicator: 0x%X\n"
                    "  SegmentationIndicator: 0x%X\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    tcapInstanceId, relDialogueId,
                    reportCause, returnIndicator, segmentationIndicator,
                    DumpHex(destAddrLength, destAddr_p, _HexDump_CVSD).c_str(),
                    DumpHex(orgAddrLength, orgAddr_p, _HexDump_CVSD).c_str()
                   );
    //Notify related dialog if it's present
    if (returnIndicator & EINSS7_I97TCAP_IND_NOCOMP_DIALOG) {
        SSNSession* pSession = findSSNsession(ssn);
        if (pSession) {
            TCDialogID    relId(tcapInstanceId, relDialogueId);
            Dialog* dlg = pSession->findDialog(relId);
            if (dlg) { //check for incoming component
                if (returnIndicator & EINSS7_I97TCAP_IND_COMP_NODIALOG) {
                    pSession->noticeInd(TCDialogID(tcapInstanceId, dialogueId), relId, reportCause);
                    return MSG_OK; //handle in TInvokeInd()
                }
                dlg->handleNoticeInd(reportCause);
            }
        }
    }
    EINSS7_I97TEndReq(ssn, userId, tcapInstanceId, dialogueId,
                      0, 0, EINSS7_I97TCAP_TERM_PRE_ARR_END, 0, 0, 0, NULL);
    return MSG_OK;
}

USHORT_T EINSS7_I97TBeginInd(UCHAR_T          ssn,
                             USHORT_T         userId,
                             EINSS7INSTANCE_T tcapInstanceId,
                             USHORT_T         dialogueId,
                             UCHAR_T          priOrder,
                             UCHAR_T          qualityOfService,
                             UCHAR_T          destAddrLength,
                             UCHAR_T          *destAddr_p,
                             UCHAR_T          orgAddrLength,
                             UCHAR_T          *orgAddr_p,
                             UCHAR_T          compPresent,
                             UCHAR_T          appContextLength,
                             UCHAR_T          *appContext_p,
                             USHORT_T         userInfoLength,
                             UCHAR_T          *userInfo_p)
{
    tccb_log_debug("BEGIN_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Comp. present: %s\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "  App. context: %s\n"
                    "  User info: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId, priOrder,
                   qualityOfService, compPresent ? "TRUE":"FALSE",
                   DumpHex(destAddrLength, destAddr_p, _HexDump_CVSD).c_str(),
                   DumpHex(orgAddrLength, orgAddr_p, _HexDump_CVSD).c_str(),
                   DumpHex(appContextLength, appContext_p, _HexDump_CVSD).c_str(),
                   DumpHex(userInfoLength, userInfo_p, _HexDump_CVSD).c_str()
                   );

    //NOTE: dialog initiation by remote peer is not supported yet,
    //so just send AARE-apdu with Associate-source-diagnostic set to
    //dialogue-service-user : application-context-name-not-supported(2)
    UCHAR_T abInfo = 0x02;
    tccb_log_debug("U_ABORT_REQ {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Abort info: 0x02\n"
                    "  App. context: \n"
                    "  User info: \n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId,
                   priOrder, qualityOfService);

    EINSS7_I97TUAbortReq(ssn, userId, tcapInstanceId, dialogueId,
                        priOrder, qualityOfService, 1, &abInfo, 0, NULL, 0, NULL);
    return MSG_OK;
}

//------------------------------------------------------------------------------
//-- TODO: Implement
//------------------------------------------------------------------------------

USHORT_T EINSS7_I97TUniInd(UCHAR_T          ssn,
                           USHORT_T         userId,
                           EINSS7INSTANCE_T tcapInstanceId,
                           UCHAR_T          priOrder,
                           UCHAR_T          qualityOfService,
                           UCHAR_T          destAddrLength,
                           UCHAR_T          *destAddr_p,
                           UCHAR_T          orgAddrLength,
                           UCHAR_T          *orgAddr_p,
                           UCHAR_T          compPresent,
                           UCHAR_T          appContextLength,
                           UCHAR_T          *appContext_p,
                           USHORT_T         userInfoLength,
                           UCHAR_T          *userInfo_p)
{
    tccb_log_debug("UNI_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Comp. present: %s\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "  App. context: %s\n"
                    "  User info: %s\n"
                    "}",
                   ssn, userId, tcapInstanceId, priOrder,
                   qualityOfService, compPresent ? "TRUE":"FALSE",
                   DumpHex(destAddrLength, destAddr_p, _HexDump_CVSD).c_str(),
                   DumpHex(orgAddrLength, orgAddr_p, _HexDump_CVSD).c_str(),
                   DumpHex(appContextLength, appContext_p, _HexDump_CVSD).c_str(),
                   DumpHex(userInfoLength, userInfo_p, _HexDump_CVSD).c_str()
                   );
    // TODO: Implement
    return MSG_OK;
}

USHORT_T EINSS7_I97TURejectInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeIdUsed,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          problemCodeTag,
                                UCHAR_T          problemCode)
{
    tccb_log_debug("U_REJECT_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeId: %s, LastComponent: %s\n"
                    "  ProblemCode: 0x%X, ProblemCodeTag: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeIdUsed ? format("0x%X", invokeId).c_str() : "NOT USED",
                    lastComponent ? "TRUE" : "FALSE",
                    problemCode, problemCodeTag
                   );
    // TODO: Implement
    return MSG_OK;
}

USHORT_T EINSS7_I97TLRejectInd(UCHAR_T          ssn,
                               USHORT_T         userId,
                               EINSS7INSTANCE_T tcapInstanceId,
                               USHORT_T         dialogueId,
                               UCHAR_T          invokeIdUsed,
                               UCHAR_T          invokeId,
                               UCHAR_T          problemCodeTag,
                               UCHAR_T          problemCode)
{
    tccb_log_debug("L_REJECT_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeId: %s\n"
                    "  ProblemCode: 0x%X, ProblemCodeTag: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeIdUsed ? format("0x%X", invokeId).c_str() : "NOT USED",
                    problemCode, problemCodeTag
                   );
    // TODO: Implement
    return MSG_OK;
}

USHORT_T EINSS7_I97TRRejectInd(UCHAR_T          ssn,
                               USHORT_T         userId,
                               EINSS7INSTANCE_T tcapInstanceId,
                               USHORT_T         dialogueId,
                               UCHAR_T          invokeIdUsed,
                               UCHAR_T          invokeId,
                               UCHAR_T          lastComponent,
                               UCHAR_T          problemCodeTag,
                               UCHAR_T          problemCode)
{
    tccb_log_debug("R_REJECT_IND {"
                    "  SSN: %u, UserID: %u, Dialog[%u:%Xh]\n"
                    "  InvokeId: %s, LastComponent: %s\n"
                    "  ProblemCode: 0x%X, ProblemCodeTag: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    invokeIdUsed ? format("0x%X", invokeId).c_str() : "NOT USED",
                    lastComponent ? "TRUE" : "FALSE",
                    problemCode, problemCodeTag
                   );
    // TODO: Implement
    return MSG_OK;
}

USHORT_T EINSS7_I97TIndError(USHORT_T errorCode, MSG_T *msg_sp)
{
    tccb_log_debug("IND_ERROR (errorCode: 0x%X)", errorCode);
    // TODO: Implement
    return MSG_OK;
}
