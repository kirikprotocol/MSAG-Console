#include "i97tcapapi.h"

extern "C"{

USHORT_T EINSS7_I97TBindConf(   UCHAR_T ssn,
        USHORT_T userId,
        UCHAR_T bindResult)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TUniInd( UCHAR_T ssn,
        USHORT_T userId,
        UCHAR_T priOrder,
        UCHAR_T qualityOfService,
        UCHAR_T destAdrLength,
        UCHAR_T *destAdr_p,
        UCHAR_T orgAdrLength,
        UCHAR_T *orgAdr_p,
        UCHAR_T compPresent,
        UCHAR_T appContextLength,
        UCHAR_T *appContext_p,
        USHORT_T userInfoLength,
        UCHAR_T *userInfo_p)
{
  return RETURN_VALUE;
}

/*
USHORT_T EINSS7_I97TBeginInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T priOrder,
        UCHAR_T qualityOfService,
        UCHAR_T destAdrLength,
        UCHAR_T *destAdr_p,
        UCHAR_T orgAdrLength,
        UCHAR_T *orgAdr_p,
        UCHAR_T compPresent,
        UCHAR_T appContextLength,
        UCHAR_T *appContext_p,
        USHORT_T userInfoLength,
        UCHAR_T *userInfo_p)
{
  return RETURN_VALUE;
}
*/

USHORT_T EINSS7_I97TContinueInd(UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T priOrder,
        UCHAR_T qualityOfService,
        UCHAR_T compPresent,
        UCHAR_T appContextLength,
        UCHAR_T *appContext_p,
        USHORT_T userInfoLength,
        UCHAR_T *userInfo_p)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TEndInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T priOrder,
        UCHAR_T qualityOfService,
        UCHAR_T compPresent,
        UCHAR_T appContextLength,
        UCHAR_T *appContext_p,
        USHORT_T userInfoLength,
        UCHAR_T *userInfo_p)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TUAbortInd(  UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T priOrder,
        UCHAR_T qualityOfService,
        USHORT_T abortInfoLength,
        UCHAR_T *abortInfo_p,
        UCHAR_T appContextLength,
        UCHAR_T *appContext_p,
        USHORT_T userInfoLength,
        UCHAR_T *userInfo_p)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TPAbortInd(  UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T priOrder,
        UCHAR_T qualityOfService,
        UCHAR_T abortCause)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TNoticeInd(  UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T reportCause,
        UCHAR_T returnIndicator,
        USHORT_T relDialogueId,
        UCHAR_T segmentationIndicator,
        UCHAR_T destAdrLength,
        UCHAR_T *destAdr_p,
        UCHAR_T orgAdrLength,
        UCHAR_T *orgAdr_p)
{
  return RETURN_VALUE;
}

/*
USHORT_T EINSS7_I97TInvokeInd(  UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeId,
        UCHAR_T lastComponent,
        UCHAR_T linkedIdUsed,
        UCHAR_T linkedId,
        UCHAR_T operationTag,
        USHORT_T operationLength,
        UCHAR_T *operationCode_p,
        USHORT_T paramLength,
        UCHAR_T *parameters_p)
{
  return RETURN_VALUE;
}
*/

USHORT_T EINSS7_I97TResultNLInd(UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeId,
        UCHAR_T lastComponent,
        UCHAR_T operationTag,
        USHORT_T operationLength,
        UCHAR_T *operationCode_p,
        USHORT_T paramLength,
        UCHAR_T *parameters_p)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TResultLInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeId,
        UCHAR_T lastComponent,
        UCHAR_T operationTag,
        USHORT_T operationLength,
        UCHAR_T *operationCode_p,
        USHORT_T paramLength,
        UCHAR_T *parameters_p)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TUErrorInd(  UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeId,
        UCHAR_T lastComponent,
        UCHAR_T errorCodeTag,
        USHORT_T errorCodeLength,
        UCHAR_T *errorCode_p,
        USHORT_T paramLength,
        UCHAR_T *parameters_p)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TURejectInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeIdUsed,
        UCHAR_T invokeId,
        UCHAR_T lastComponent,
        UCHAR_T problemCodeTag,
        UCHAR_T problemCode)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TLRejectInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeIdUsed,
        UCHAR_T invokeId,
        UCHAR_T problemCodeTag,
        UCHAR_T problemCode)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TRRejectInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeIdUsed,
        UCHAR_T invokeId,
        UCHAR_T lastComponent,
        UCHAR_T problemCodeTag,
        UCHAR_T problemCode)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TLCancelInd( UCHAR_T ssn,
        USHORT_T userId,
        USHORT_T dialogueId,
        UCHAR_T invokeId)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TStateInd( UCHAR_T ssn,
        USHORT_T userId,
        UCHAR_T userState,
        UCHAR_T affectedSsn,
        ULONG_T affectedSpc,
        ULONG_T localSpc)
{
  return RETURN_VALUE;
}

USHORT_T EINSS7_I97TIndError( USHORT_T errorCode,
        MSG_T  *msg_sp)
{
  return RETURN_VALUE;
}

};
