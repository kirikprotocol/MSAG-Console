static char const ident[] = "$Id$";
///////////////////////////////////////////////////////////////////////////////////////////
/// Callbacks implementation
///////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "util/BinDump.hpp"
using smsc::util::DumpHex;
using smsc::util::format;

#include "inman/inap/dispatcher.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::Dialog;
using smsc::inman::inap::SSNSession;
using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::getTcapBindErrorMessage;


#define tcapLogger TCAPDispatcher::getInstance()->TCAPLogger()

//-------------------------------- Util functions --------------------------------
static Dialog* findDialog(UCHAR_T ssn, USHORT_T dialogueId)
{
    TCAPDispatcher *dsp = TCAPDispatcher::getInstance();
    assert(dsp);

    SSNSession* pSession = dsp->findSession(ssn);
    if (!pSession) {
        smsc_log_warn( dsp->TCAPLogger(), "SS7CB: Invalid/inactive session, SSN: %u", ssn);
        return 0;
    }
    Dialog* pDlg = pSession->findDialog(dialogueId);
    if (!pDlg)
        smsc_log_warn(dsp->TCAPLogger(), "SS7CB: Invalid(closed) dialog ID: 0x%X", dialogueId);
    return pDlg;
}

USHORT_T EINSS7_I97TBindConf(UCHAR_T ssn, USHORT_T userId,
                            EINSS7INSTANCE_T tcapInstanceId, UCHAR_T bindResult)
{
    TCAPDispatcher *dsp = TCAPDispatcher::getInstance();
    assert(dsp);

    smsc_log_debug(dsp->TCAPLogger(),
                 "SS7_I97TBindConf(SSN=%u, UserId=%u, TcapInstanceId=%u, bindResult=%u(%s))",
                   ssn, userId, tcapInstanceId, bindResult, getTcapBindErrorMessage(bindResult));

    dsp->confirmSSN(ssn, bindResult);
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
    smsc_log_debug(tcapLogger, 
                   "SS7_I97TStateInd(SSN=%u, UserId=%u, TcapInstanceId=%u, "
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
                                UCHAR_T          compPresent,
                                UCHAR_T          appContextLength,
                                UCHAR_T          *appContext_p,
                                USHORT_T         userInfoLength,
                                UCHAR_T          *userInfo_p)
{
    smsc_log_debug(tcapLogger, "CONTINUE_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
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
    Dialog* dlg = findDialog(ssn, dialogueId);
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
    smsc_log_debug(tcapLogger, "ADDRESS_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
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
    smsc_log_debug(tcapLogger, "END_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
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
    Dialog* dlg = findDialog( ssn, dialogueId );
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
    smsc_log_debug(tcapLogger, "INVOKE_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], LastComponent: %s\n"
                    "  InvokeID: 0x%X, LinkedId: %s\n"
                    "  Operation: %s, Tag: %s\n"
                    "  Params[%u]: 0x%s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    (lastComponent ? "TRUE" : "FALSE"), invokeId, 
                    (linkedIdUsed ? format("0x%X", linkedId).c_str() : "NOT USED"),
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );

    Dialog* dlg = findDialog(ssn, dialogueId);
    if (dlg)
        dlg->handleInvoke(invokeId, tag, opLength, op, paramLength, pm,
                          lastComponent ? true : false);
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
    smsc_log_debug(tcapLogger, "RESULT_NL_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], LastComponent: %s\n"
                    "  InvokeID: 0x%X\n"
                    "  Operation: %s, Tag: %s\n"
                    "  Params[%u]: 0x%s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    (lastComponent ? "TRUE" : "FALSE"), invokeId, 
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    Dialog* dlg = findDialog( ssn, dialogueId );
    if (dlg)
        dlg->handleResultNotLast(invokeId, tag, opLength, op, paramLength, pm);
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
    smsc_log_debug(tcapLogger, "RESULT_L_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], LastComponent: %s\n"
                    "  InvokeID: 0x%X\n"
                    "  Operation: %s, Tag: %s\n"
                    "  Params[%u]: 0x%s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    (lastComponent ? "TRUE" : "FALSE"), invokeId, 
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    Dialog* dlg = findDialog( ssn, dialogueId );
    if (dlg)
        return dlg->handleResultLast(invokeId, tag, opLength, op, paramLength, pm);
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
    smsc_log_debug(tcapLogger, "U_ERROR_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], LastComponent: %s\n"
                    "  InvokeID: 0x%X\n"
                    "  Error: %s, Tag: %s\n"
                    "  Params[%u]: %s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    (lastComponent ? "TRUE" : "FALSE"), invokeId, 
                    DumpHex(opLength, op, _HexDump_CVSD).c_str(),
                    (tag == 0x02 ? "LOCAL" : "GLOBAL"), 
                    paramLength, DumpHex(paramLength, pm).c_str()
                   );
    Dialog* dlg = findDialog( ssn, dialogueId );
    if (dlg)
        dlg->handleResultError(invokeId, tag, opLength, op, paramLength, pm);
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
    smsc_log_debug(tcapLogger, "P_ABORT_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
                    "  PriOrder: 0x%X, QoS: 0x%X\n"
                    "  Abort cause: 0x%X\n"
                    "}",
                   ssn, userId, tcapInstanceId, dialogueId,
                   priOrder, qualityOfService, abortCause
                   );
    Dialog* dlg = findDialog( ssn, dialogueId );
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
    smsc_log_debug(tcapLogger, "U_ABORT_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
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

    Dialog* dlg = findDialog(ssn, dialogueId);
    return !dlg ? MSG_OK :
        dlg->handleUAbort(abortInfoLength, abortInfo_p, userInfoLength, userInfo_p);
}

USHORT_T EINSS7_I97TLCancelInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId)
{
    smsc_log_debug(tcapLogger, "L_CANCEL_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], InvokeId: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId, invokeId
                   );
    Dialog* dlg = findDialog(ssn, dialogueId);
    if (dlg)
        dlg->handleLCancelInvoke(invokeId);
    return MSG_OK;
}

//-------------------------------------------------------------------------------------
// TODO: Implement
//-------------------------------------------------------------------------------------
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
    smsc_log_debug(tcapLogger, "BEGIN_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
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
  // TODO: Implement
  return MSG_OK;
}

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
    smsc_log_debug(tcapLogger, "UNI_IND {"
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
    smsc_log_debug(tcapLogger, "NOTICE_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], RelDialog[0x%X]\n"
                    "  ReportCause: 0x%X, ReturnIndicator: 0x%X\n"
                    "  SegmentationIndicator: 0x%X\n"
                    "  Dest. address: %s\n"
                    "  Org. address: %s\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId, relDialogueId,
                    reportCause, returnIndicator, segmentationIndicator,
                    DumpHex(destAddrLength, destAddr_p, _HexDump_CVSD).c_str(),
                    DumpHex(orgAddrLength, orgAddr_p, _HexDump_CVSD).c_str()
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
    smsc_log_debug(tcapLogger, "U_REJECT_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], LastComponent: %s\n"
                    "  InvokeId: %s\n"
                    "  ProblemCode: 0x%X, ProblemCodeTag: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    lastComponent ? "TRUE" : "FALSE",
                    invokeIdUsed ? format("0x%X", invokeId).c_str() : "NOT USED",
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
    smsc_log_debug(tcapLogger, "L_REJECT_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X]\n"
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
    smsc_log_debug(tcapLogger, "R_REJECT_IND {"
                    "  SSN: %u, UserID: %u, TcapInstanceID: %u\n"
                    "  Dialog[0x%X], LastComponent: %s\n"
                    "  InvokeId: %s\n"
                    "  ProblemCode: 0x%X, ProblemCodeTag: 0x%X\n"
                    "}",
                    ssn, userId, tcapInstanceId, dialogueId,
                    lastComponent ? "TRUE" : "FALSE",
                    invokeIdUsed ? format("0x%X", invokeId).c_str() : "NOT USED",
                    problemCode, problemCodeTag
                   );
    // TODO: Implement
    return MSG_OK;
}


//-------------------------------------------------------------------------------------

USHORT_T EINSS7_I97TIndError(USHORT_T errorCode, MSG_T *msg_sp)
{
    smsc_log_debug(tcapLogger, "IND_ERROR (errorCode: 0x%X)", errorCode);
    // TODO: Implement
    return MSG_OK;
}
