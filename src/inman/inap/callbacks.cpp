static char const ident[] = "$Id$";
///////////////////////////////////////////////////////////////////////////////////////////
/// Callbacks implementation
///////////////////////////////////////////////////////////////////////////////////////////

#include "inman/inap/inss7util.hpp"

#include "inman/inap/infactory.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/common/util.hpp"

using smsc::inman::inap::Dialog;
using smsc::inman::inap::Session;
using smsc::inman::inap::InSessionFactory;
using smsc::inman::inap::getTcapBindErrorMessage;
using smsc::inman::common::dump;
using smsc::inman::common::format;

namespace smsc {
namespace inman {
namespace inap {
    extern Logger* tcapLogger;
}
}
}

using smsc::inman::inap::tcapLogger;

//-------------------------------- Util functions --------------------------------
static Dialog* findDialog(UCHAR_T ssn, USHORT_T dialogueId)
{
    Session* pSession = InSessionFactory::getInstance()->findSession( ssn );
    if (!pSession) {
        smsc_log_warn( tcapLogger, "Invalid SSN: 0x%X", ssn );
        return 0;
    }
    Dialog* pDlg = pSession->findDialog( dialogueId );
    if (!pDlg) {
        smsc_log_warn( tcapLogger, "Invalid dialog ID: 0x%X", dialogueId );
        return 0;
    }
    return pDlg;
}

USHORT_T EINSS7_I97TBindConf(   UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                UCHAR_T          bindResult)
{
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TBindConf(ssn=%d,userId=%d,tcapInstanceId=%d,bindResult=%d(%s))",
                   ssn,userId,tcapInstanceId,bindResult,getTcapBindErrorMessage(bindResult));

  Session* pSession = InSessionFactory::getInstance()->findSession( ssn );
  if( !pSession )
  {
  smsc_log_warn( tcapLogger, "Invalid SSN: 0x%X", ssn );
  return MSG_OK;
  }

  if( EINSS7_I97TCAP_BIND_OK == bindResult )
  {
        pSession->setState( Session::BOUNDED );
  }
  else
  {
        smsc_log_error(tcapLogger, "BIND failed: '%s' (code 0x%X)",  getTcapBindErrorMessage(bindResult), bindResult);
        pSession->setState( Session::ERROR );
  }
    return MSG_OK;
}

USHORT_T EINSS7_I97TBeginInd(   UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,"BEGIN_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priOrder );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qualityOfService );
  smsc_log_debug(tcapLogger," Comp. present: %s", compPresent?"TRUE":"FALSE" );
  smsc_log_debug(tcapLogger," Dest. address: %s", dump(destAddrLength   ,destAddr_p   ).c_str() );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(orgAddrLength    ,orgAddr_p    ).c_str() );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(appContextLength ,appContext_p ).c_str() );
  smsc_log_debug(tcapLogger," User info: %s"    , dump(userInfoLength   ,userInfo_p   ).c_str() );

  //TODO: Send U_ABORT_REQ

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
  smsc_log_debug(tcapLogger,"CONTINUE_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priOrder );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qualityOfService );
  smsc_log_debug(tcapLogger," Comp. present: %s", compPresent?"TRUE":"FALSE" );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(appContextLength ,appContext_p ).c_str() );
  smsc_log_debug(tcapLogger," User info: %s"    , dump(userInfoLength   ,userInfo_p   ).c_str() );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handleContinueDialog();
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
  smsc_log_debug(tcapLogger,"ADDRESS_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," Bitmask: 0x%X", (USHORT_T)bitMask );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(addressLength, orgAdr_p).c_str() );
  return MSG_OK;
}


USHORT_T EINSS7_I97TEndInd(     UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TEndInd("
                 "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                 "prio=%d, qos=%d, "
                 "compPresent=\"%s\", "
                 "appContext[%d]={%s}, "
                 "userInfo[%d]={%s})",
                 ssn, userId, tcapInstanceId, dialogueId,
                 priOrder,qualityOfService,
                 compPresent?"TRUE":"FALSE",
                 appContextLength, dump(appContextLength ,appContext_p ).c_str(),
                 userInfoLength, dump(userInfoLength   ,userInfo_p   ).c_str()
                  );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handleEndDialog(compPresent ? true : false);
  return MSG_OK;
}

USHORT_T EINSS7_I97TInvokeInd(  UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,"INVOKE_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeID: 0x%X", invokeId );
  smsc_log_debug(tcapLogger," Tag: 0x%X", (tag==0x02?"LOCAL":"GLOBAL") );
  smsc_log_debug(tcapLogger," LastComponent: %s", (lastComponent?"TRUE":"FALSE"));
  smsc_log_debug(tcapLogger," LinkedId: %s", (linkedIdUsed ? format("0x%X", linkedId ).c_str() : "NOT USED") );
  smsc_log_debug(tcapLogger," Operation: %s" , dump(opLength ,op ).c_str() );
  smsc_log_debug(tcapLogger," Params: %s", dump(paramLength,pm ).c_str() );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handleInvoke( invokeId, tag, opLength, op, paramLength, pm );
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
  smsc_log_debug(tcapLogger,"RESULT_NL_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeID: 0x%X", invokeId );
  smsc_log_debug(tcapLogger," Tag: 0x%X", (tag==0x02?"LOCAL":"GLOBAL") );
  smsc_log_debug(tcapLogger," LastComponent: %s", (lastComponent?"TRUE":"FALSE"));
  smsc_log_debug(tcapLogger," Operation: %s" , dump(opLength ,op ).c_str() );
  smsc_log_debug(tcapLogger," Params: %s", dump(paramLength,pm ).c_str() );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handleResultNotLast( invokeId, tag, opLength, op, paramLength, pm );
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
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TResultLInd("
                 "ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                 "invokeId=%d, lastComponent=\"%s\", "
                 "tag=\"%s\", "
                 "opcode[%d]={%s}, "
                 "parameters[%d]={%s})",
                 ssn, userId, tcapInstanceId, dialogueId,
                 invokeId,lastComponent?"TRUE":"FALSE",
                 tag==0x02?"LOCAL":"GLOBAL",
                 opLength,dump(opLength ,op ).c_str(),
                 paramLength,dump(paramLength,pm ).c_str()
                );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) return dlg->handleResultLast( invokeId, tag, opLength, op, paramLength, pm );

  return MSG_OK;
}

USHORT_T EINSS7_I97TUErrorInd(  UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,"U_ERROR_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeID: 0x%X", invokeId );
  smsc_log_debug(tcapLogger," Tag: 0x%X", (tag==0x02?"LOCAL":"GLOBAL") );
  smsc_log_debug(tcapLogger," LastComponent: %s", (lastComponent?"TRUE":"FALSE"));
  smsc_log_debug(tcapLogger," Errorcode: %s" , dump(opLength ,op ).c_str() );
  smsc_log_debug(tcapLogger," Params: %s", dump(paramLength,pm ).c_str() );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handleUserError( invokeId, tag, opLength, op, paramLength, pm );
  return MSG_OK;
}



USHORT_T EINSS7_I97TUniInd(     UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,"UNI_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priOrder );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qualityOfService );
  smsc_log_debug(tcapLogger," Comp. present: %s", compPresent?"TRUE":"FALSE" );
  smsc_log_debug(tcapLogger," Dest. address: %s", dump(destAddrLength   ,destAddr_p   ).c_str() );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(orgAddrLength    ,orgAddr_p    ).c_str() );
  smsc_log_debug(tcapLogger," App. context: %s" , dump(appContextLength ,appContext_p ).c_str() );
  smsc_log_debug(tcapLogger," User info: %s"    , dump(userInfoLength   ,userInfo_p   ).c_str() );
  return MSG_OK;
}

//----------------------------------- U_ABORT_ind ----------------------------------
// TODO: Implement
//-------------------------------------------------------------------------------------
USHORT_T EINSS7_I97TUAbortInd(  UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TUAbortInd(ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                 "PriOrder=%d, qos=%d, "
                 "abortinfo[%d]={%s}, "
                 "appContext[%d]={%s}, "
                 "userInfo[%d]={%s})",
                 ssn, userId, tcapInstanceId, dialogueId,
                 priOrder, qualityOfService,
                 abortInfoLength, dump(abortInfoLength, abortInfo_p ).c_str(),
                 appContextLength, dump(appContextLength ,appContext_p ).c_str(),
                 userInfoLength, dump(userInfoLength   ,userInfo_p   ).c_str()
                 );
  return MSG_OK;
}

USHORT_T EINSS7_I97TPAbortInd(  UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          priOrder,
                                UCHAR_T          qualityOfService,
                                UCHAR_T          abortCause)
{
  smsc_log_debug(tcapLogger,"P_ABORT_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," PriOrder: 0x%X", priOrder );
  smsc_log_debug(tcapLogger," QoS: 0x%X", qualityOfService );
  smsc_log_debug(tcapLogger," Abort cause: 0x%X", abortCause );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handlePAbortDialog(abortCause);

  return MSG_OK;
}

//----------------------------------- T_NOTICE_ind ----------------------------------
// TODO: Implement
//-------------------------------------------------------------------------------------
USHORT_T EINSS7_I97TNoticeInd(  UCHAR_T          ssn,
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
  smsc_log_debug(tcapLogger,"NOTICE_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," RelDialogID: 0x%X", relDialogueId );
  smsc_log_debug(tcapLogger," ReportCause: 0x%X", reportCause );
  smsc_log_debug(tcapLogger," ReturnIndicator: 0x%X", returnIndicator );
  smsc_log_debug(tcapLogger," SegmentationIndicator: 0x%X", segmentationIndicator );
  smsc_log_debug(tcapLogger," Dest. address: %s", dump(destAddrLength   ,destAddr_p   ).c_str() );
  smsc_log_debug(tcapLogger," Org. address: %s" , dump(orgAddrLength    ,orgAddr_p    ).c_str() );
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
  smsc_log_debug(tcapLogger,"U_REJECT_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeId: %s", (invokeIdUsed ? format("0x%X", invokeId ).c_str() : "NOT USED") );
  smsc_log_debug(tcapLogger," LastComponent: %s", (lastComponent?"TRUE":"FALSE"));
  smsc_log_debug(tcapLogger," ProblemCodeTag: 0x%X", problemCodeTag );
  smsc_log_debug(tcapLogger," ProblemCode: 0x%X", problemCode );
  return MSG_OK;
}

USHORT_T EINSS7_I97TLRejectInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeIdUsed,
                                UCHAR_T          invokeId,
                                UCHAR_T          problemCodeTag,
                                UCHAR_T          problemCode)
{
  smsc_log_debug(tcapLogger,"L_REJECT_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeId: %s", (invokeIdUsed ? format("0x%X", invokeId ).c_str() : "NOT USED") );
  smsc_log_debug(tcapLogger," ProblemCodeTag: 0x%X", problemCodeTag );
  smsc_log_debug(tcapLogger," ProblemCode: 0x%X", problemCode );
  return MSG_OK;
}

USHORT_T EINSS7_I97TRRejectInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeIdUsed,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          problemCodeTag,
                                UCHAR_T          problemCode)
{
  smsc_log_debug(tcapLogger,"R_REJECT_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeId: %s", (invokeIdUsed ? format("0x%X", invokeId ).c_str() : "NOT USED") );
  smsc_log_debug(tcapLogger," LastComponent: %s", (lastComponent?"TRUE":"FALSE"));
  smsc_log_debug(tcapLogger," ProblemCodeTag: 0x%X", problemCodeTag );
  smsc_log_debug(tcapLogger," ProblemCode: 0x%X", problemCode );
  return MSG_OK;
}

USHORT_T EINSS7_I97TLCancelInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId)
{
  smsc_log_debug(tcapLogger,"L_CANCEL_IND");
  smsc_log_debug(tcapLogger," SSN: 0x%X", ssn);
  smsc_log_debug(tcapLogger," UserID: 0x%X", userId );
  smsc_log_debug(tcapLogger," TcapInstanceID: 0x%X", tcapInstanceId );
  smsc_log_debug(tcapLogger," DialogID: 0x%X", dialogueId );
  smsc_log_debug(tcapLogger," InvokeId: 0x%X", invokeId );

  Dialog* dlg = findDialog( ssn, dialogueId );
  if( dlg ) dlg->handleLCancelInvoke(invokeId);

  return MSG_OK;
}

USHORT_T EINSS7_I97TStateInd(   UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                UCHAR_T          userState,
                                UCHAR_T          affectedSsn,
                                ULONG_T          affectedSpc,
                                ULONG_T          localSpc,
                                UCHAR_T          subsysMultiplicityInd)
{
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TStateInd(ssn=%d, userId=%d, tcapInstanceId=%d, userState=%d, "
                 "affectedSsn=%d, affectedSpc=%d, localSpc=%d, subsysMultiplicityInd=%d",
                 ssn, userId, tcapInstanceId,
                 userState, affectedSsn, affectedSpc,
                 localSpc, subsysMultiplicityInd);

  return MSG_OK;
}

//-------------------------------------------------------------------------------------

USHORT_T EINSS7_I97TIndError(   USHORT_T         errorCode,
                                MSG_T            *msg_sp)
{
  smsc_log_debug(tcapLogger, "IND_ERROR (errorCode: 0x%X)", errorCode );
  return MSG_OK;
}
