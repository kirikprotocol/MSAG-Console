/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_tcapif.h,v                                                         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_tcapif.h                                                  */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
/*                                                                   */
/* All rights reserved. The copyright of the computer program herein */
/* is the property of Ericsson Infocom Consultants AB, Sweden. The   */
/* program may be used and/or copied only with the permission of     */
/* Ericsson Infocom Consultants AB or in accordance with the         */
/* terms and conditions stipulated in the agreement/contract under   */
/* which the program has been supplied.                              */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                                                  */
/* 16/190 55-CAA 201 45 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.16 2000/04/07 09:05:26 Exp                       */
/*                                                                   */
/* Programmer:                                                       */
/* Michael Mackey                                                    */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Contains structures and constants specific to the tcapif.         */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 23/04/97     M. Mackey        Initial version.                    */
/*                                                                   */
/*                                                                   */
/* Notes:                                                            */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#include "et96map_api_defines.h"

#ifndef __ET96MAP_TCAPIF_H__
#define __ET96MAP_TCAPIF_H__


/* Request Sizes */
/************* Not Used */
/*#define T_BIND_REQ_SIZE     0x05
#define T_BEGIN_REQ_SIZE    0x0c
#define T_CONTINUE_REQ_SIZE 0x0b
#define T_END_REQ_SIZE      0x0b
#define T_INVOKE_REQ_SIZE   0x10
#define T_RESULT_REQ_SIZE   0x0b
#define T_UNBIND_REQ_SIZE   0x02
#define T_U_ABORT_REQ_SIZE  0x0d
#define T_ERROR_REQ_SIZE    0x0b
#define T_U_CANCEL_REQ_SIZE 0x05
#define T_U_REJECT_REQ_SIZE 0x09

* Indication Sizes *

#define T_BIND_CONF_SIZE    0x03
#define T_BEGIN_IND_SIZE    0x08
#define T_CONTINUE_IND_SIZE 0x06
#define T_END_IND_SIZE      0x07
#define T_INVOKE_IND_SIZE   0x0b
#define T_L_RESULT_IND_SIZE 0x09
#define T_U_ERROR_IND_SIZE  0x0c 
#define TU_ABORT_IND_SIZE   0x06
#define T_L_CANCEL_IND_SIZE 0x05
#define TP_ABORT_IND_SIZE   0x07
#define T_L_REJECT_SIZE     0x07
#define T_RU_REJECT_SIZE    0x08
#define T_STATE_SIZE        0x0c
#define T_NOTICE_SIZE       0x0a*/

/******** End Not Used*/





#define ET96MAP_PRI_ORDER           0x00



/* TCAP Constants */

#define ET96MAP_INVOKE_ID_SIZE      0x01
#define ET96MAP_V_TC_USER           0x01
#define ET96MAP_Q_OF_SRV            0x00
#define ET96MAP_MAX_TIME_OUT        0x0c80
#define ET96MAP_IND_LENGTH_ERROR    0x00ff
#define ET96MAP_IND_UNKNOWN_CODE    0x00fe
#define ET96MAP_NO_INVOKE           FALSE
#define ET96MAP_INVOKE              TRUE
#define ET96MAP_NOT_PRESENT         0

#define ET96MAP_NO_INVOKE_NEC       255
#define ET96MAP_OPCODELENGTH3       0x82
#define ET96MAP_OPCODELENGTH2       0x81
#define ET96MAP_OPCODELENGTH1       0x00
#define ET96MAP_SINGLE_ASN1_TAG	    0xA0
#define ET96MAP_AC_NAME_TAG	    0xA1
#define ET96MAP_USER_INFO_TAG	    0xBE
#define ET96MAP_ABORT_SOURCE_TAG    0x80
#define ET96MAP_PARAM_SET           0x31   
#define ET96MAP_PARAM_SEQ           0x30   
#define ET96MAP_NO_TAG              0x00
#define ET96MAP_PROBLEM_CODE_LEN   1

typedef struct {
    UCHAR_T  localSsn;
    USHORT_T senderId;	        /* Request Only */
    UCHAR_T  bindResult;	/* Confirmation Only */
} ET96MAP_TBIND_T;

typedef struct {
    UCHAR_T  localSsn;
    USHORT_T senderId;	        /* Request Only */
    UCHAR_T  bindResult;	/* Confirmation Only */
} ET96MAP_MBIND_T;

typedef struct{
    UCHAR_T  localSsn; 
    USHORT_T dialogueId; 
    UCHAR_T  destAddrLen; 
    UCHAR_T  destAddr[ET96MAP_MAX_ADDR_LEN]; /*TR1367*/
    UCHAR_T  origAddrLen; 
    UCHAR_T  origAddr[ET96MAP_MAX_ADDR_LEN]; /*TR1367*/
    UCHAR_T  compPresent;         /* Indication Only */
    UCHAR_T  appContextLen;
    UCHAR_T  appContext[ET96MAP_AC_LEN]; /*TR1367*/	
    USHORT_T userInfoLen;
    UCHAR_T  userInfo[ET96MAP_MAX_USER_PH2_DATA]; /*TR1367*/
    
    UCHAR_T  priorityOrder; 
    UCHAR_T  qualityOfService;  
} ET96MAP_TBEGIN_T;

typedef struct {

    UCHAR_T  localSsn;
    USHORT_T dialogueId; 
    UCHAR_T  compPresent; 	/* Indication Only */	
    UCHAR_T  origAddrLen; 	/* Request Only    */
    UCHAR_T  origAddr[ET96MAP_MAX_ADDR_LEN]; /*TR1367*/	/* Request Only    */
    UCHAR_T  appContextLen; 
    UCHAR_T  appContext[ET96MAP_AC_LEN]; /*TR1367*/ 
    USHORT_T userInfoLen; 
    UCHAR_T  userInfo[ET96MAP_MAX_USER_PH2_DATA]; /*TR1367*/
   
    UCHAR_T priorityOrder;
    UCHAR_T  qualityOfService;  
} ET96MAP_TCONTINUE_T;

typedef struct{	

    UCHAR_T  localSsn; 
    USHORT_T dialogueId;
    UCHAR_T  compPresent;
    UCHAR_T  termination;
    UCHAR_T  appContextLen; 
    UCHAR_T  appContext[ET96MAP_AC_LEN]; /*TR1367*/
    USHORT_T userInfoLen; 
    UCHAR_T  userInfo[ET96MAP_MAX_USER_PH2_DATA]; /*TR1367*/
  
    UCHAR_T priorityOrder;
    UCHAR_T  qualityOfService;  
} ET96MAP_TEND_T;

typedef struct{
    UCHAR_T  localSsn; 
    USHORT_T dialogueId; 
    UCHAR_T  invokeId;
    UCHAR_T  lastComponent;
    UCHAR_T  linkedId;
    UCHAR_T  operErrCodeTag;
    UCHAR_T  operErrCodeLen;
    UCHAR_T  operErrCode[ET96MAP_MAX_OP_ERR_CODE_LEN]; /*TR1367*/
    USHORT_T paramLen;
    UCHAR_T  parameters[ET96MAP_MAX_REPORT_PARAM]; /*TR1367*/
    USHORT_T timeOut;
}ET96MAP_TINV_RES_ERR_T;

typedef struct{	
    UCHAR_T  localSsn; 
    USHORT_T dialogueId;
    USHORT_T abortInfoLen;
    UCHAR_T  abortInfo[ET96MAP_MAX_ABORT_INFO_LEN]; /*TR1367*/
    UCHAR_T  appContextLen; 
    UCHAR_T  appContext[ET96MAP_AC_LEN]; /*TR1367*/
    USHORT_T userInfoLen; 
    UCHAR_T  userInfo[ET96MAP_MAX_USER_PH2_DATA]; /*TR1367*/
   
    UCHAR_T priorityOrder;
} ET96MAP_TU_ABORT_T;

typedef struct{
    UCHAR_T  localSsn; 
    USHORT_T dialogueId; 
    UCHAR_T  invokeId;
} ET96MAP_T_CANCEL_T;  

typedef struct{
    UCHAR_T  localSsn; 
    USHORT_T dialogueId; 
    UCHAR_T  invokeIdUsed;
    UCHAR_T  invokeId;
    UCHAR_T  lastComponent;
    UCHAR_T  problemCodeTag;
    UCHAR_T  problemCode;
} ET96MAP_TREJECT_T;

typedef struct{
    UCHAR_T  localSsn; 
    USHORT_T dialogueId; 
    UCHAR_T  abortCause;
    UCHAR_T priorityOrder;
} ET96MAP_TP_ABORT_T;

typedef struct{    
    UCHAR_T  localSsn; 
    UCHAR_T  userState;
    UCHAR_T  affectedSsn;
    ULONG_T  affectedSpc;
    ULONG_T  localSpc;
} ET96MAP_TSTATE_T;

typedef struct{
    UCHAR_T  localSsn; 
    USHORT_T dialogueId;
    UCHAR_T  reportCause;
    USHORT_T relDialogueId;
    UCHAR_T  returnIndicator;
    UCHAR_T  SegmentationIndicator;
} ET96MAP_T_NOTICE_T;

void MAP_CallTcapMsg(MSG_T *msg_s);
void Et96MapTBindConf(MSG_T bindConf_s);
void Et96MapTBeginInd(MSG_T beginInd_s);
void Et96MapTContinueInd(MSG_T continueInd_s);
void Et96MapTEndInd(MSG_T endInd_s);
void Et96MapTInvOrResOrErrInd (MSG_T invOrResInd_s);
void Et96MapTUAbortInd(MSG_T tUAbortInd_s);
void Et96MapTLCancelInd(MSG_T tLCancelInd_s);
void Et96MapTRejectInd(MSG_T tRejectInd_s);
void Et96MapTPAbortInd(MSG_T tPAbortInd_s);
void Et96MapTStateInd (MSG_T tStateInd_s);
void Et96MapTNoticeInd (MSG_T tNoticeInd_s);

void Et96MapBldHeader(UCHAR_T **header_pp, UCHAR_T localSsn, 
	       USHORT_T dialogueId, UCHAR_T invokeId, 
	       BOOLEAN_T invoke,USHORT_T *count);

void Et96MapBldHeaderUabort(UCHAR_T **header_pp, UCHAR_T localSsn, 
	       USHORT_T dialogueId, UCHAR_T priorityOrder, 
	       USHORT_T *count);

void Et96MapBldHeaderBegContEnd(UCHAR_T **header_pp, UCHAR_T localSsn, 
	       USHORT_T dialogueId, UCHAR_T priorityOrder, 
	       UCHAR_T qualityOfService,USHORT_T *count);

void Et96MapBldOptionalString(UCHAR_T **string1_pp, UCHAR_T *string2_p,
		       UCHAR_T lengt, USHORT_T *count);
void Et96MapBldOperation (UCHAR_T **header_pp, UCHAR_T codeTag, 
		   UCHAR_T len, UCHAR_T *operation,
		   USHORT_T *count);
void Et96MapBldLongOptionalString(UCHAR_T **string1_pp,
			   UCHAR_T *string2_p, USHORT_T length,
			   BOOLEAN_T noLength, USHORT_T *count);


void Et96MapStripHeader(UCHAR_T **header_pp, UCHAR_T *localSsn_p, 
		 USHORT_T *dialogueId_p, UCHAR_T *invokeId_p,UCHAR_T *priorityOrder, UCHAR_T priQServ);


void Et96MapStripOptionalString(UCHAR_T **string1_pp, UCHAR_T *string2_p, 
			 UCHAR_T *length);
void Et96MapStripOperation(UCHAR_T **string_pp, UCHAR_T *codeTag, 
		    UCHAR_T *len, UCHAR_T *operation_p);

void Et96MapStripLongOptionalString(UCHAR_T **srcString_pp, UCHAR_T *desString_p, 
		             USHORT_T *desStringLen,UCHAR_T thisTag,
			     UCHAR_T *lenCodeSize);

void Et96MapBldOptionalBcdString(UCHAR_T **string1_pp, UCHAR_T *string2_p, UCHAR_T length);

void Et96MapStripOptionalBcdString(UCHAR_T **string1_pp, UCHAR_T *string2_p, 
			 UCHAR_T *length);

void Et96MapTIndError (USHORT_T errorCode, MSG_T *msg_sp);

void Et96MapCopyIndataToParameters(UCHAR_T **inbuff,UCHAR_T *parameters,USHORT_T *paramLen);


void Et96MapIndefLengthToDefiniteLength(UCHAR_T *inbuff,
                              USHORT_T *paramLen);


#endif /* __ET96MAP_TCAPIF_H__ */
























