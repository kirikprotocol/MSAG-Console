/* ************************************************************************** *
 * Structure(s) for linking TCAP API Callbacks with EINSS7 library
 * ************************************************************************** */
#ifndef __EINSS7_TCAPI_LINK_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EINSS7_TCAPI_LINK_HPP

#ifndef EINSS7_FUNC_POINTER
extern "C" {

typedef USHORT_T (*EINSS7_I97TBINDCONF_T)(   UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
                                             UCHAR_T bindResult
#ifdef EIN_HD
#if EIN_HD >= 101
                                             , UCHAR_T sccpStandard
#endif /* EIN_HD >= 101 */
#endif /* EIN_HD */
                                        );

typedef USHORT_T (*EINSS7_I97TUNIIND_T)(     UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
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
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TBEGININD_T)(   UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
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
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TCONTINUEIND_T)(UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T priOrder,
					     UCHAR_T qualityOfService,
#ifdef EIN_HD
#if EIN_HD >= 101
                                             UCHAR_T orgAdrLength,
                                             UCHAR_T *orgAdr_p,
#endif /* EIN_HD >= 101 */
#endif /* EIN_HD */
					     UCHAR_T compPresent,
					     UCHAR_T appContextLength,
					     UCHAR_T *appContext_p,
					     USHORT_T userInfoLength,
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TENDIND_T)(     UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T priOrder,
					     UCHAR_T qualityOfService,
					     UCHAR_T compPresent,
					     UCHAR_T appContextLength,
					     UCHAR_T *appContext_p,
					     USHORT_T userInfoLength,
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TUABORTIND_T)(  UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T priOrder,
					     UCHAR_T qualityOfService,
					     USHORT_T abortInfoLength,
					     UCHAR_T *abortInfo_p,
					     UCHAR_T appContextLength,
					     UCHAR_T *appContext_p,
					     USHORT_T userInfoLength,
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TPABORTIND_T)(  UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T priOrder,
					     UCHAR_T qualityOfService,
					     UCHAR_T abortCause);

typedef USHORT_T (*EINSS7_I97TNOTICEIND_T)(  UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T reportCause,
					     UCHAR_T returnIndicator,
					     USHORT_T relDialogueId,
					     UCHAR_T segmentationIndicator,
					     UCHAR_T destAdrLength,
					     UCHAR_T *destAdr_p,
					     UCHAR_T orgAdrLength,
					     UCHAR_T *orgAdr_p);

typedef USHORT_T (*EINSS7_I97TINVOKEIND_T)(  UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T linkedIdUsed,
					     UCHAR_T linkedId,
					     UCHAR_T operationTag,
					     USHORT_T operationLength,
					     UCHAR_T *operationCode_p,
					     USHORT_T paramLength,
					     UCHAR_T *parameters_p);

typedef USHORT_T (*EINSS7_I97TRESULTNLIND_T)(UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T operationTag,
					     USHORT_T operationLength,
					     UCHAR_T *operationCode_p,
					     USHORT_T paramLength,
					     UCHAR_T *parameters_p);

typedef USHORT_T (*EINSS7_I97TRESULTLIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T operationTag,
					     USHORT_T operationLength,
					     UCHAR_T *operationCode_p,
					     USHORT_T paramLength,
					     UCHAR_T *parameters_p);

typedef USHORT_T (*EINSS7_I97TUERRORIND_T)(  UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T errorCodeTag,
					     USHORT_T errorCodeLength,
					     UCHAR_T *errorCode_p,
					     USHORT_T paramLength,
					     UCHAR_T *parameters_p);

typedef USHORT_T (*EINSS7_I97TUREJECTIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeIdUsed,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T problemCodeTag,
					     UCHAR_T problemCode);

typedef USHORT_T (*EINSS7_I97TLREJECTIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeIdUsed,
					     UCHAR_T invokeId,
					     UCHAR_T problemCodeTag,
					     UCHAR_T problemCode);

typedef USHORT_T (*EINSS7_I97TRREJECTIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeIdUsed,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T problemCodeTag,
					     UCHAR_T problemCode);

typedef USHORT_T (*EINSS7_I97TLCANCELIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeId);

typedef USHORT_T (*EINSS7_I97TSTATEIND_T)(   UCHAR_T ssn,
					     USHORT_T userId,
				             EINSS7INSTANCE_T tcapInstanceId,
					     UCHAR_T userState,
					     UCHAR_T affectedSsn,
					     ULONG_T affectedSpc,
					     ULONG_T localSpc,
					     UCHAR_T subsysMultiplicityInd);

typedef USHORT_T (*EINSS7_I97TINDERROR_T)(   USHORT_T errorCode,
					     MSG_T  *msg_sp);

typedef USHORT_T (*EINSS7_I97TADDRESSIND_T)( UCHAR_T ssn,
                                            USHORT_T userId,
                                            EINSS7INSTANCE_T tcapInstanceId,
                                            USHORT_T dialogueId,
                                            UCHAR_T bitMask,
                                            UCHAR_T addressLength,
                                            UCHAR_T *orgAdr_p);


typedef struct EINSS7_I97TCAPINIT
{
  /* Pointer to a function */
  EINSS7_I97TBINDCONF_T	   EINSS7_I97TBindConf;
  EINSS7_I97TUNIIND_T	   EINSS7_I97TUniInd;
  EINSS7_I97TBEGININD_T	   EINSS7_I97TBeginInd;
  EINSS7_I97TCONTINUEIND_T EINSS7_I97TContinueInd;
  EINSS7_I97TENDIND_T	   EINSS7_I97TEndInd;
  EINSS7_I97TUABORTIND_T   EINSS7_I97TUAbortInd;
  EINSS7_I97TPABORTIND_T   EINSS7_I97TPAbortInd;
  EINSS7_I97TNOTICEIND_T   EINSS7_I97TNoticeInd;
  EINSS7_I97TINVOKEIND_T   EINSS7_I97TInvokeInd;
  EINSS7_I97TRESULTNLIND_T EINSS7_I97TResultNLInd;
  EINSS7_I97TRESULTLIND_T  EINSS7_I97TResultLInd;
  EINSS7_I97TUERRORIND_T   EINSS7_I97TUErrorInd;
  EINSS7_I97TUREJECTIND_T  EINSS7_I97TURejectInd;
  EINSS7_I97TLREJECTIND_T  EINSS7_I97TLRejectInd;
  EINSS7_I97TRREJECTIND_T  EINSS7_I97TRRejectInd;
  EINSS7_I97TLCANCELIND_T  EINSS7_I97TLCancelInd;
  EINSS7_I97TSTATEIND_T	   EINSS7_I97TStateInd;
  EINSS7_I97TINDERROR_T	   EINSS7_I97TIndError;
  EINSS7_I97TADDRESSIND_T  EINSS7_I97TAddressInd;
} EINSS7_I97TCAPINIT_T;

}
#endif /* EINSS7_FUNC_POINTER */

#endif /* __EINSS7_TCAPI_LINK_HPP */

