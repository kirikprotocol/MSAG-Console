/*********************************************************************
Don't remove these two lines, contain depends on them!
Document Number: %Container% Revision: %Revision%
*/

/*********************************************************************/
/*								     */
/* i97tcapapi.h,v */
/*								     */
/*-------------------------------------------------------------------*/
/*							O	     */
/* COPYRIGHT Ericsson Infotech Consultants AB               	     */
/*								     */
/* All rights reserved. The copyright of the computer program herein */
/* is the property of Ericsson Infotech Consultants AB, Sweden. The  */
/* program may be used and/or copied only with the permission of     */
/* Ericsson Infotech Consultants AB or in accordance with the 	     */
/* terms and conditions stipulated in the agreement/contract under   */
/* which the program has been supplied.				     */
/*								     */
/*-------------------------------------------------------------------*/
/*								     */
/* Document Number:           Rev:                                   */
/* 24/190 55-CAA 201 19 Ux     A                                     */
/*								     */
/* Revision:							     */
/* @EINVER: i93tcapapi.h,v 1.19 1998/01/30 07:43:51 Exp 	     */
/*								     */
/* Programmer:							     */
/* Göran Olausson, Cap Programator AB.				     */
/*								     */
/* Purpose:							     */
/* To define function prototypes for transaction portion procedures  */
/* used by White TCAP.						     */
/*								     */
/*-------------------------------------------------------------------*/
/*								     */
/* Revision record:						     */
/* 00-950301	Göran Olausson, Cap Programator AB.		     */
/*		Original version.				     */
/*								     */
/* 01-951020	Tomas Strand Ericsson Infotech Consultants AB.	     */
/*		Added REP_HOPCOUNT_VIOL.			     */
/*								     */
/* 02-970305	Jan Fernqvist Ericsson Infotech AB.		     */
/*		Changes according to the development of the          */
/*		TCAP ITU R2A, ITU-T (03/93), version of TCAP.	     */
/*		(See the Implementation Proposal for details)	     */
/*								     */
/* 03-970403	Tomas Strand Ericsson Infotech AB.                   */
/*		Removed Priority order in TNoticeInd.                */
/*								     */
/* 04-970910	Hans Andersson Ericsson Infotech AB.		     */
/*		Added ifdef CHINESE_SCCP to handle Chinese SCCP	     */
/*								     */
/* 05-970915	Mikael Blom Ericsson Infotech AB.                    */
/*		Added compile switch EINSS7_FUNC_POINTER             */
/*		for use of local naming of the functions(primitives) */
/*		in the application.                                  */
/*								     */
/* 06-980129    Mikael Blom Ericsson Infotech AB                     */
/*              Added define CALLBACK_FUNC_NOT_SET	             */
/*								     */
/* 07-980506    Conrad Patton (xcpa)                                 */
/*              Added prefix EINSS7 to functions and constants.      */
/*                                                                   */
/* 08-990723    Conrad Patton (xcpa)                                 */
/*              Corrected Operation Class values.                    */
/*                                                                   */
/* 09-000127    C Patton                                             */
/*              Changed Dialogue Ids from USHORT_Ts to ULONG_Ts in   */
/*              function prototypes.                                 */
/*                                                                   */
/* 09-000313    Additional coding to allow merge of TCAP TTC and     */
/*              TCAP ITU/ETSI.                                       */
/*                                                                   */
/* 11-000324    Bronagh McElduff                                     */
/*              Added definitions for new Importance parameter.      */
/*                                                                   */
/* 12-000403    Bronagh McElduff                                     */
/*              Inclusion of the called and calling addresses in the */
/*              definition of the EINSS7_I97TNoticeInd function.     */
/*                                                                   */
/* 13-000605    Bronagh McElduff                                     */
/*              Updated after review.                                */
/*                                                                   */
/* Notes:							     */
/*								     */
/*********************************************************************/

/* C++ compability definitions */
#ifndef __I97TCAPAPI_H__
#define __I97TCAPAPI_H__
#if defined  (__cplusplus) || (c_plusplus)
extern "C" {
#endif

/* Abort Cause  						     */
#define EINSS7_I97TCAP_ABORT_UNREC_MES	       	0
#define EINSS7_I97TCAP_ABORT_UNREC_ID	       	1
#define EINSS7_I97TCAP_ABORT_BAD_TP	       	2
#define EINSS7_I97TCAP_ABORT_INCORRECT_TP       3
#define EINSS7_I97TCAP_ABORT_RESOURCE_LIMIT     4

/* Bind Result	 						     */
#define EINSS7_I97TCAP_BIND_OK		       	0
#define EINSS7_I97TCAP_BIND_SSN_IN_USE		1
#define EINSS7_I97TCAP_BIND_PROTOCOL_ERR	2
#define EINSS7_I97TCAP_BIND_NO_RESOURCES	3
#define EINSS7_I97TCAP_BIND_INVALID_SSN         4
#define EINSS7_I97TCAP_BIND_SCCP_NOT_READY	5
#define EINSS7_I97TCAP_BIND_USER_ID_ERR         6

/* Operation Class 						     */
#define EINSS7_I97TCAP_OP_CLASS_1	       	0
#define EINSS7_I97TCAP_OP_CLASS_2	       	1
#define EINSS7_I97TCAP_OP_CLASS_3	       	2
#define EINSS7_I97TCAP_OP_CLASS_4	       	3

/* Components Present 						     */
#define EINSS7_I97TCAP_COMP_NOT_PRES	       	0
#define EINSS7_I97TCAP_COMP_PRES	       	1

/* Error code tag 						     */
#define EINSS7_I97TCAP_ERR_TAG_LOCAL	       	2
#define EINSS7_I97TCAP_ERR_TAG_GLOBAL	       	6

/* Invoke id used 						     */
#define EINSS7_I97TCAP_INVOKE_ID_USED	       	128
#define EINSS7_I97TCAP_INVOKE_ID_NOT_USED     	0

/* Last component 						     */
#define EINSS7_I97TCAP_COMPONENT_NOT_LAST     	0
#define EINSS7_I97TCAP_COMPONENT_LAST	       	1

/* Linked ID used 						     */
#define EINSS7_I97TCAP_LINKED_ID_NOT_USED     	0
#define EINSS7_I97TCAP_LINKED_ID_USED	       	128

/* Operation code tag 						     */
#define EINSS7_I97TCAP_OPERATION_TAG_LOCAL	2
#define EINSS7_I97TCAP_OPERATION_TAG_GLOBAL	6

/* Problem code tag 						     */
#define EINSS7_I97TCAP_PROBLEM_TAG_GENERAL      128
#define EINSS7_I97TCAP_PROBLEM_TAG_INVOKE	129
#define EINSS7_I97TCAP_PROBLEM_TAG_RESULT	130
#define EINSS7_I97TCAP_PROBLEM_TAG_ERROR	131

/* Codes for General Problem 					     */
#define EINSS7_I97TCAP_PR_G_UNREC_COMP        	0
#define EINSS7_I97TCAP_PR_G_MISTYPED_COMP     	1
#define EINSS7_I97TCAP_PR_G_BAD_COMP	       	2

/* Codes for Invoke Problem 					     */
#define EINSS7_I97TCAP_PR_I_DUP_ID	       	0
#define EINSS7_I97TCAP_PR_I_UNREC_OP	       	1
#define EINSS7_I97TCAP_PR_I_MIST_PARM	       	2
#define EINSS7_I97TCAP_PR_I_RES_LIMIT	       	3
#define EINSS7_I97TCAP_PR_I_INIT_RELEASE      	4
#define EINSS7_I97TCAP_PR_I_UNREC_L_ID        	5
#define EINSS7_I97TCAP_PR_I_RESP_UNEXP        	6
#define EINSS7_I97TCAP_PR_I_UNEXP_OP	       	7

/* Codes for Return Result Problem 				     */
#define EINSS7_I97TCAP_PR_R_UNREC_I_ID          0
#define EINSS7_I97TCAP_PR_R_RES_UNEXP	        1
#define EINSS7_I97TCAP_PR_R_MIST_PARM	        2

/* Codes for Return Error Problem 				     */
#define EINSS7_I97TCAP_PR_E_UNREC_I_ID          0
#define EINSS7_I97TCAP_PR_E_RET_ERR_UNEXP       1
#define EINSS7_I97TCAP_PR_E_UNREC_ERR	        2
#define EINSS7_I97TCAP_PR_E_UNEXP_ERR	        3
#define EINSS7_I97TCAP_PR_E_MIST_PARM           4

/* Codes for report causes 					     */
#define EINSS7_I97TCAP_REP_NO_TR_ADR_NATURE	0
#define EINSS7_I97TCAP_REP_NO_TR_ADR		1
#define EINSS7_I97TCAP_REP_SUBSYST_CONGEST	2
#define EINSS7_I97TCAP_REP_UNEQUIP_USER         4
#define EINSS7_I97TCAP_REP_NETW_FAILURE         5
#define EINSS7_I97TCAP_REP_NETW_CONGEST         6
#define EINSS7_I97TCAP_REP_UNQUALIFIED		7
#define EINSS7_I97TCAP_REP_HOPCOUNT_VIOL	8

/* Codes for the Return Indicator 				     */
#define EINSS7_I97TCAP_IND_NOCOMP_NODIALOG	0
#define EINSS7_I97TCAP_IND_COMP_NODIALOG	1
#define EINSS7_I97TCAP_IND_NOCOMP_DIALOG	2
#define EINSS7_I97TCAP_IND_COMP_DIALOG		3

/* Codes for the Segmentation Indicator 			     */
#define EINSS7_I97TCAP_NOT_SEGMENTED_MSG	0
#define EINSS7_I97TCAP_SEGMENTED_MSG		1

/* Codes for Termination 					     */
#define EINSS7_I97TCAP_TERM_BASIC_END		0
#define EINSS7_I97TCAP_TERM_PRE_ARR_END         1

/* Codes for Quality of service 				     */
#define EINSS7_I97TCAP_QLT_NOT_USED		0
#define EINSS7_I97TCAP_QLT_IN_SEQUENCE		1
#define EINSS7_I97TCAP_QLT_MESS_RET		2
#define EINSS7_I97TCAP_QLT_BOTH                 3

/* Codes for Priority Order	 	Low=0, High=3		     */
#define EINSS7_I97TCAP_PRI_HIGH_0	        0
#define EINSS7_I97TCAP_PRI_HIGH_1	        1
#define EINSS7_I97TCAP_PRI_HIGH_2	        2
#define EINSS7_I97TCAP_PRI_HIGH_3	        3

/* Codes for Importance                 Low=4, High=11               */
#define EINSS7_I97TCAP_MSG_PRI_IMP_0            4
#define EINSS7_I97TCAP_MSG_PRI_IMP_1            5
#define EINSS7_I97TCAP_MSG_PRI_IMP_2            6
#define EINSS7_I97TCAP_MSG_PRI_IMP_3            7
#define EINSS7_I97TCAP_MSG_PRI_IMP_4            8
#define EINSS7_I97TCAP_MSG_PRI_IMP_5            9
#define EINSS7_I97TCAP_MSG_PRI_IMP_6            10
#define EINSS7_I97TCAP_MSG_PRI_IMP_7            11

/* xcpa: define maximum priority level for different networks.       */
#if defined EINSS7_ANSI_SCCP
#define EINSS7_I97TCAP_MAX_PRIORITY   EINSS7_I97TCAP_PRI_HIGH_3 
#else
#define EINSS7_I97TCAP_MAX_PRIORITY   EINSS7_I97TCAP_MSG_PRI_IMP_7
#endif

/* Codes for Kind of TCAP		White / Blue		     */
#define EINSS7_I97TCAP_BLUE_USER	        0
#define EINSS7_I97TCAP_WHITE_USER	        1

/* The max size of the local and remote address field.		     */
/* Excluding the lengthfield, but in some cases including some SCCP  */
/* overhead							     */
#if defined EINSS7_ANSI_SCCP
/*xcpa: Changed maximum address length to from 12 to 19 octets.      */
#define EINSS7_I97TCAP_MAX_ADDRESS_LENGTH	19

#elif defined EINSS7_TWENTYFOURBITPC
#define EINSS7_I97TCAP_MAX_ADDRESS_LENGTH	20

#elif defined EINSS7_TTC_SCCP
#define EINSS7_I97TCAP_MAX_ADDRESS_LENGTH       18

#else
#define EINSS7_I97TCAP_MAX_ADDRESS_LENGTH	18

#endif

/* The max length of User Information data.			     */
/* xcpa: Changed maximum UI from 3092 to 3096 to account since       */
/* external data tag is now coded by TCAP.                           */
#define EINSS7_I97TCAP_MAX_UI_LENGTH		3092

/*********************************************************************/
/*								     */
/*	     R E Q U E S T   R E T U R N   C O D E S		     */
/*								     */
/*********************************************************************/

#define EINSS7_I97TCAP_INV_INVOKE_ID_USED     16
#define EINSS7_I97TCAP_INV_DIALOGUE_ID        17
#define EINSS7_I97TCAP_INV_QLT_OF_SERVICE     18
#define EINSS7_I97TCAP_INV_DEST_ADR_LENGTH    19

#define EINSS7_I97TCAP_INV_ORG_ADR_LENGTH     20
#define EINSS7_I97TCAP_INV_TERMINATION        21
#define EINSS7_I97TCAP_INV_LINKED_ID_USED     22
#define EINSS7_I97TCAP_INV_CLASS	      23
#define EINSS7_I97TCAP_INV_TIMEOUT	      24

#define EINSS7_I97TCAP_INV_OP_TAG	      25
#define EINSS7_I97TCAP_INV_OP_LENGTH	      26
#define EINSS7_I97TCAP_INV_ERR_TAG	      27
#define EINSS7_I97TCAP_INV_ERR_LENGTH	      28
#define EINSS7_I97TCAP_INV_PROBLEM_TAG        29

#define EINSS7_I97TCAP_INV_PROBLEM_CODE       30
#define EINSS7_I97TCAP_INV_ABORTINFO_LENGTH   31
#define EINSS7_I97TCAP_INV_PRIORITY_LEVEL     32
#define EINSS7_I97TCAP_INV_USER_VERSION       33
#define EINSS7_I97TCAP_INV_AC_LENGTH          34
#define EINSS7_I97TCAP_INV_UI_LENGTH          35
#define EINSS7_I97TCAP_INV_USER_ID            36

#ifdef EINSS7_FUNC_POINTER
/*********************************************************************/
/*								     */
/*	     EINSS7_FUNC_POINTER   R E T U R N	 C O D E S           */
/*								     */
/*********************************************************************/

#define EINSS7_I97TCAP_CALLBACK_FUNC_NOT_SET     250
#define EINSS7_I97TCAP_CALLBACK_FUNC_ALREADY_SET 251

#endif
/*********************************************************************/
/*								     */
/*	     I N D I C A T I O N   R E T U R N	 C O D E S	     */
/*								     */
/*********************************************************************/

#define EINSS7_I97TCAP_IND_UNKNOWN_CODE       254
#define EINSS7_I97TCAP_IND_LENGTH_ERROR       255

/*********************************************************************/
/*								     */
/*		 R E Q U E S T	  P R O T O T Y P E S		     */
/*								     */
/*********************************************************************/


USHORT_T EINSS7_I97TBindReq(    UCHAR_T subSystNbr,
			        USHORT_T userId,
			        UCHAR_T versionOfTCuser);

USHORT_T EINSS7_I97TUnBindReq(  UCHAR_T subSystNbr,
			        USHORT_T userId);

USHORT_T EINSS7_I97TUniReq(     UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T destAdrLength,
				UCHAR_T *destAdr_p,
				UCHAR_T orgAdrLength,
				UCHAR_T *orgAdr_p,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
			        UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TBeginReq(   UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T destAdrLength,
				UCHAR_T *destAdr_p,
				UCHAR_T orgAdrLength,
				UCHAR_T *orgAdr_p,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
			        UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TContinueReq(UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T orgAdrLength,
				UCHAR_T *orgAdr_p,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TEndReq(     UCHAR_T subSystNbr,
			        USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T termination,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TUAbortReq(  UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				USHORT_T abortInfoLength,
				UCHAR_T *abortInfo_p,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TInvokeReq(  UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T linkedIdUsed,
				UCHAR_T linkedId,
				UCHAR_T opClass,
				USHORT_T timeOut,
				UCHAR_T operationTag,
				USHORT_T operationLength,
				UCHAR_T *operationCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TResultNLReq(UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T operationTag,
				USHORT_T operationLength,
				UCHAR_T *operationCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TResultLReq( UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T operationTag,
				USHORT_T operationLength,
				UCHAR_T *operationCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TUErrorReq(  UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T errorCodeTag,
				USHORT_T errorCodeLength,
				UCHAR_T *errorCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TURejectReq( UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeIdUsed,
				UCHAR_T invokeId,
				UCHAR_T problemCodeTag,
				UCHAR_T problemCode);

USHORT_T EINSS7_I97TUCancelReq( UCHAR_T subSystNbr,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId);

USHORT_T EINSS7_I97TTimerResetReq( UCHAR_T subSystNtbr,
				   USHORT_T userId,
				   USHORT_T dialogueId,
				   UCHAR_T invokeId);


/*********************************************************************/
/*								     */
/*	  C O N F I R M A T I O N   A N D  I N D I C A T I O N	     */
/*		       P R O T O T Y P E S			     */
/*								     */
/*********************************************************************/

USHORT_T EINSS7_I97THandleInd(MSG_T *mqp_sp);

#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */

typedef USHORT_T (*EINSS7_I97TBINDCONF_T)(   UCHAR_T ssn,
					     USHORT_T userId,
					     UCHAR_T bindResult);

typedef USHORT_T (*EINSS7_I97TUNIIND_T)(     UCHAR_T ssn,
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
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TBEGININD_T)(   UCHAR_T ssn,
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
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TCONTINUEIND_T)(UCHAR_T ssn,
					     USHORT_T userId,
					     USHORT_T dialogueId,
					     UCHAR_T priOrder,
					     UCHAR_T qualityOfService,
					     UCHAR_T compPresent,
					     UCHAR_T appContextLength,
					     UCHAR_T *appContext_p,
					     USHORT_T userInfoLength,
					     UCHAR_T *userInfo_p);

typedef USHORT_T (*EINSS7_I97TENDIND_T)(     UCHAR_T ssn,
					     USHORT_T userId,
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
					     USHORT_T dialogueId,
					     UCHAR_T priOrder,
					     UCHAR_T qualityOfService,
					     UCHAR_T abortCause);

typedef USHORT_T (*EINSS7_I97TNOTICEIND_T)(  UCHAR_T ssn,
					     USHORT_T userId,
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
					     USHORT_T dialogueId,
					     UCHAR_T invokeIdUsed,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T problemCodeTag,
					     UCHAR_T problemCode);

typedef USHORT_T (*EINSS7_I97TLREJECTIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeIdUsed,
					     UCHAR_T invokeId,
					     UCHAR_T problemCodeTag,
					     UCHAR_T problemCode);

typedef USHORT_T (*EINSS7_I97TRREJECTIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeIdUsed,
					     UCHAR_T invokeId,
					     UCHAR_T lastComponent,
					     UCHAR_T problemCodeTag,
					     UCHAR_T problemCode);

typedef USHORT_T (*EINSS7_I97TLCANCELIND_T)( UCHAR_T ssn,
					     USHORT_T userId,
					     USHORT_T dialogueId,
					     UCHAR_T invokeId);

typedef USHORT_T (*EINSS7_I97TSTATEIND_T)(   UCHAR_T ssn,
					     USHORT_T userId,
					     UCHAR_T userState,
					     UCHAR_T affectedSsn,
					     ULONG_T affectedSpc,
					     ULONG_T localSpc);

typedef USHORT_T (*EINSS7_I97TINDERROR_T)(   USHORT_T errorCode,
					     MSG_T  *msg_sp);

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
} EINSS7_I97TCAPINIT_T;

#else

USHORT_T EINSS7_I97TBindConf(   UCHAR_T ssn,
				USHORT_T userId,
				UCHAR_T bindResult);

USHORT_T EINSS7_I97TUniInd(	UCHAR_T ssn,
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
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TBeginInd(	UCHAR_T ssn,
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
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TContinueInd(UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T compPresent,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TEndInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T compPresent,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TUAbortInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				USHORT_T abortInfoLength,
				UCHAR_T *abortInfo_p,
				UCHAR_T appContextLength,
				UCHAR_T *appContext_p,
				USHORT_T userInfoLength,
				UCHAR_T *userInfo_p);

USHORT_T EINSS7_I97TPAbortInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T priOrder,
				UCHAR_T qualityOfService,
				UCHAR_T abortCause);

USHORT_T EINSS7_I97TNoticeInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T reportCause,
				UCHAR_T returnIndicator,
				USHORT_T relDialogueId,
				UCHAR_T segmentationIndicator,
				UCHAR_T destAdrLength,
				UCHAR_T *destAdr_p,
				UCHAR_T orgAdrLength,
				UCHAR_T *orgAdr_p);

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
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TResultNLInd(UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T lastComponent,
				UCHAR_T operationTag,
				USHORT_T operationLength,
				UCHAR_T *operationCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TResultLInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T lastComponent,
				UCHAR_T operationTag,
				USHORT_T operationLength,
				UCHAR_T *operationCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TUErrorInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId,
				UCHAR_T lastComponent,
				UCHAR_T errorCodeTag,
				USHORT_T errorCodeLength,
				UCHAR_T *errorCode_p,
				USHORT_T paramLength,
				UCHAR_T *parameters_p);

USHORT_T EINSS7_I97TURejectInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeIdUsed,
				UCHAR_T invokeId,
				UCHAR_T lastComponent,
				UCHAR_T problemCodeTag,
				UCHAR_T problemCode);

USHORT_T EINSS7_I97TLRejectInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeIdUsed,
				UCHAR_T invokeId,
				UCHAR_T problemCodeTag,
				UCHAR_T problemCode);

USHORT_T EINSS7_I97TRRejectInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeIdUsed,
				UCHAR_T invokeId,
				UCHAR_T lastComponent,
				UCHAR_T problemCodeTag,
				UCHAR_T problemCode);

USHORT_T EINSS7_I97TLCancelInd(	UCHAR_T ssn,
				USHORT_T userId,
				USHORT_T dialogueId,
				UCHAR_T invokeId);

USHORT_T EINSS7_I97TStateInd(	UCHAR_T ssn,
				USHORT_T userId,
				UCHAR_T userState,
				UCHAR_T affectedSsn,
				ULONG_T affectedSpc,
				ULONG_T localSpc);

USHORT_T EINSS7_I97TIndError(	USHORT_T errorCode,
				MSG_T  *msg_sp);

#endif


#ifdef EINSS7_FUNC_POINTER
/* The application must call this function to register the call back funtions. */ 
	USHORT_T EINSS7_I97TcapRegFunc(EINSS7_I97TCAPINIT_T *);
#endif

/* C++ compability definitions */
#if defined (__cplusplus) || (c_plusplus)
}
#endif

#endif /* __I97TCAPAPI_H__ */

/*------------------- End of header file i97tcapapi.h ----------------*/
