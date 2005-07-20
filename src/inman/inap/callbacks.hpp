#include "ss7cp.h"
#include "et94inapcs1plusapi.h"

extern "C" 
{

USHORT_T E94InapAbortInd(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                ABORT_T         *abort_sp );

USHORT_T E94InapPAbortInd(      UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         pAbortCause );

USHORT_T E94InapBeginInd(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                SS7_ADDRESS_T   *destAddress_sp,
                                SS7_ADDRESS_T   *origAddress_sp,
                                AC_NAMEREQ_T    *acName_sp,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

USHORT_T E94InapBindConf(       UCHAR_T         ssn, 
                                UCHAR_T         bindResult );

USHORT_T E94InapDataInd(        UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

USHORT_T E94InapEndInd(         UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

USHORT_T E94InapErrorInd(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         einReason,
                                BOOLEAN_T       invokeIdUsed,
                                UCHAR_T         invokeId,
                                UCHAR_T         disconnectIndicator );

USHORT_T E94InapNoticeInd(      UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                UCHAR_T         reportCause,
                                UCHAR_T         returnIndicator,
                                USHORT_T        relatedDialogueId,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

USHORT_T E94InapStateInd(       UCHAR_T         ssn,
                                UCHAR_T         userStatus,
                                UCHAR_T         affectedSsn,
                                ULONG_T         affectedSpc,
                                ULONG_T         localSpc );

USHORT_T E94InapIndError(       USHORT_T        indErrorCode,
                                MSG_T           *msg_sp );

}
