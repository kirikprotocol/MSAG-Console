static char const ident[] = "$Id$";
///////////////////////////////////////////////////////////////////////////////////////////
/// Callbacks implementation
///////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "logger/Logger.h"

#include "callbacks.hpp"
#include "factory.hpp"
#include "session.hpp"
#include "dialog.hpp"
#include "util.hpp"

using smsc::logger::Logger;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::Session;
using smsc::inman::inap::Factory;
using smsc::inman::inap::getTcapBindErrorMessage;
using smsc::inman::inap::dump;
using smsc::inman::inap::tcapLogger;
using smsc::inman::inap::inapLogger;

USHORT_T EINSS7_I97TBindConf(   UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                UCHAR_T          bindResult)
{
    smsc_log_debug(tcapLogger,
                   "EINSS7_I97TBindConf(ssn=%d,userId=%d,tcapInstanceId=%d,bindResult=%d(%s) )",
                   ssn,userId,tcapInstanceId,bindResult,getTcapBindErrorMessage(bindResult));
    Session* pSession = Factory::getInstance()->findSession( ssn );
    assert( pSession );
    if( EINSS7_I97TCAP_BIND_OK == bindResult )
    {
        pSession->setState( Session::BOUNDED );
    }
    else
    {
        smsc_log_error(inapLogger, "TCAP BIND request failed. Error code 0x%X", bindResult);
        pSession->setState( Session::ERROR );
    }
    return MSG_OK;
}

USHORT_T EINSS7_I97TUniInd(     UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                UCHAR_T          priOrder,
                                UCHAR_T          qualityOfService,
                                UCHAR_T          destAdrLength,
                                UCHAR_T          *destAdr_p,
                                UCHAR_T          orgAdrLength,
                                UCHAR_T          *orgAdr_p,
                                UCHAR_T          compPresent,
                                UCHAR_T          appContextLength,
                                UCHAR_T          *appContext_p,
                                USHORT_T         userInfoLength,
                                UCHAR_T          *userInfo_p)
{
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TUniInd("
                 "ssn=%d, userId=%d,tcapInstanceId=%d,...)",
                 ssn, userId, tcapInstanceId );
  return MSG_OK;
}

USHORT_T EINSS7_I97TBeginInd(   UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          priOrder,
                                UCHAR_T          qualityOfService,
                                UCHAR_T          destAdrLength,
                                UCHAR_T          *destAdr_p,
                                UCHAR_T          orgAdrLength,
                                UCHAR_T          *orgAdr_p,
                                UCHAR_T          compPresent,
                                UCHAR_T          appContextLength,
                                UCHAR_T          *appContext_p,
                                USHORT_T         userInfoLength,
                                UCHAR_T          *userInfo_p)
{
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TBeginInd("
                 "ssn=%d,userId=%d,tcapInstanceId=%d,dialogueId=%d,...)",
                 ssn, userId, tcapInstanceId, dialogueId );
  Session* pSession = Factory::getInstance()->findSession( ssn );
  assert( pSession );
  Dialog* pDlg = new Dialog( pSession, dialogueId );
  return MSG_OK;
}

USHORT_T EINSS7_I97TContinueInd(UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          priOrder,
                                UCHAR_T          qualityOfService,
                                UCHAR_T          compPresent,
                                UCHAR_T          aclen,
                                UCHAR_T          *ac,
                                USHORT_T         uilen,
                                UCHAR_T          *ui)
{
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TContinueInd(ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                 "priOrder=%d, qualityOfService=%d, compPresent=\"%s\", "
                 "ac[%d]={%s}, ui[%d]={%s})",
                 ssn, userId, tcapInstanceId,
                 dialogueId, priOrder, qualityOfService,
                 compPresent?"TRUE":"FALSE",
                 aclen, dump(aclen,ac).c_str(), uilen, dump(uilen,ui).c_str());
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
                 "ssn=%d, userId=%d,tcapInstanceId=%d,...)",
                 ssn, userId, tcapInstanceId );
  return MSG_OK;
}

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
  smsc_log_debug(inapLogger,"EINSS7_I97TUAbortInd(ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d,...)",
         ssn, userId, tcapInstanceId, dialogueId );
/*
  if( abortInfo_p ) {
    UCHAR_T *bb = mkHexBuf( (UCHAR_T*)abortInfo_p, sizeof(abortInfoLength) );
    printf("abort info[%d]: %s\n", abortInfoLength, bb );
    free( bb );
  }
  if( userInfo_p ) {
    UCHAR_T *bb = mkHexBuf( (UCHAR_T*)userInfo_p, sizeof(userInfoLength) );
    printf("user info[%d]: %s\n", userInfoLength, bb );
    free( bb );
  }
*/
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
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
  return MSG_OK;
}

USHORT_T EINSS7_I97TNoticeInd(  UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          reportCause,
                                UCHAR_T          returnIndicator,
                                USHORT_T         relDialogueId,
                                UCHAR_T          segmentationIndicator,
                                UCHAR_T          destAdrLength,
                                UCHAR_T          *destAdr_p,
                                UCHAR_T          orgAdrLength,
                                UCHAR_T          *orgAdr_p)
{
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
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
                                UCHAR_T          operationTag,
                                USHORT_T         oplen,
                                UCHAR_T          *op,
                                USHORT_T         pmlen,
                                UCHAR_T          *pm)
{
  smsc_log_debug(tcapLogger,
                 "EINSS7_I97TInvokeInd(ssn=%d, userId=%d, tcapInstanceId=%d, dialogueId=%d, "
                 "invokeId=%d, lastComponent=\"%s\", linkedIdUsed=\"%s\", linkedId=%d, "
                 "operationTag=\"%s\", "
                 "op[%d]={%s}, pm[%d]={%s})",
                 ssn, userId, tcapInstanceId, dialogueId,
                 invokeId, lastComponent?"YES":"NO", linkedIdUsed?"USED":"NOT USED", linkedId,
                 operationTag==0x02?"LOCAL":"GLOBAL",
                 oplen, dump(oplen,op).c_str(), pmlen, dump(pmlen,pm).c_str());
  return MSG_OK;
}

USHORT_T EINSS7_I97TResultNLInd(UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          operationTag,
                                USHORT_T         operationLength,
                                UCHAR_T          *operationCode_p,
                                USHORT_T         paramLength,
                                UCHAR_T          *parameters_p)
{
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
  return MSG_OK;
}

USHORT_T EINSS7_I97TResultLInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          operationTag,
                                USHORT_T         operationLength,
                                UCHAR_T          *operationCode_p,
                                USHORT_T         paramLength,
                                UCHAR_T          *parameters_p)
{
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
  return MSG_OK;
}

USHORT_T EINSS7_I97TUErrorInd(  UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId,
                                UCHAR_T          lastComponent,
                                UCHAR_T          errorCodeTag,
                                USHORT_T         errorCodeLength,
                                UCHAR_T          *errorCode_p,
                                USHORT_T         paramLength,
                                UCHAR_T          *parameters_p)
{
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
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
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
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
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
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
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
  return MSG_OK;
}

USHORT_T EINSS7_I97TLCancelInd( UCHAR_T          ssn,
                                USHORT_T         userId,
                                EINSS7INSTANCE_T tcapInstanceId,
                                USHORT_T         dialogueId,
                                UCHAR_T          invokeId)
{
  printf("%s: ssn=%d, userId=%d, iId=%d, dlg=%X04F\n", __func__,
         (int)ssn, (int)userId, (int)tcapInstanceId, dialogueId );
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
                 "EINSS7_I97TStateInd(ssn=%d, userId=%d, iId=%d, userState=%d, "
                 "affectedSsn=%d, affectedSpc=%d, localSpc=%d, subsysMultiplicityInd=%d",
                 ssn, userId, tcapInstanceId,
                 userState, affectedSsn, affectedSpc,
                 localSpc, subsysMultiplicityInd);
  return MSG_OK;
}

USHORT_T EINSS7_I97TIndError(   USHORT_T         errorCode,
                                MSG_T            *msg_sp)
{
  printf("%s: err=%d: %s\n", __func__, errorCode, msg_sp );
  return MSG_OK;
}
