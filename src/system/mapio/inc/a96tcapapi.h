/*********************************************************************
Don'tremove these two lines, contain depends on them!                    
DocumentNumber: %Container% Revision: %Revision%                         
*/

/********************************************************************/
/*                                                                  */
/*a96tcapapi.h,v                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*								    */
/*COPYRIGHT Ericsson Infotech  AB				    */
/*								    */
/*The copyright to the computer program herein is the property of   */
/*Ericsson Infotech AB. The program may be used and/or  copied only */
/*with the written permission from Ericsson Infotech  AB or in the  */
/*accordance with the terms and conditions stipulated in the        */
/*agreement/contract under which the program has been supplied.     */
/*								    */
/*------------------------------------------------------------------*/
/*                                                                  */
/*Document Number:           Rev:                                   */
/*18/190 55-CAA 201 18 Ux     A                                     */
/*                                                                  */
/*Revision:                                                         */
/*@EINVER: a96tcapapi.h,v 0.0 2000/09/19 09:18:12 Exp    	    */
/*                                                                  */
/*Programmer: Phelim O'Doherty                                      */
/*                                                                  */
/*Purpose: Definitions for tcapapi.c for ANSI                       */
/*------------------------------------------------------------------*/
/*                                                                  */
/*Revision record:                                                  */
/*  00-000919  Phelim O'Doherty                                     */
/*             Initial issue based on 'a92tcapapi.h-(v1.8)'         */
/*                                                                  */
/*  01-010306  Bronagh McElduff                                     */
/*             EINSS7_A96TCAP_TIMEOUT_LIMIT now given in 0.1s       */ 
/*                                                                  */
/*  Notes:                                                          */
/*                                                                  */
/*                                                                  */
/********************************************************************/
/*C++ compability definitions */
#ifndef __A96TCAPAPI_H__
#define __A96TCAPAPI_H__
#if defined (__cplusplus) || (c_plusplus)
extern"C" {
#endif

/********************************************************************/
/*                                                                  */
/*           P A R A M E T E R   C O N S T A N T S                  */
/*                                                                  */
/********************************************************************/

/*Abort Causes */

#define EINSS7_A96TCAP_ABORT_UNREC_PACK       1
#define EINSS7_A96TCAP_ABORT_INCORRECT_TP     2
#define EINSS7_A96TCAP_ABORT_BAD_TP           3
/* xpho: Updated to correlate with 1996 standard */
#define EINSS7_A96TCAP_ABORT_UNASSIGN_ID      4
#define EINSS7_A96TCAP_ABORT_PERM_TO_RELEASE  5
#define EINSS7_A96TCAP_ABORT_RESOURCE_LIMIT   6
#define EINSS7_A96TCAP_ABORT_UNRECOGN_DP_ID   7
#define EINSS7_A96TCAP_ABORT_BADLY_STRUCT_DP  8
#define EINSS7_A96TCAP_ABORT_MISSING_DP       9
#define EINSS7_A96TCAP_ABORT_INCONSISTENT_DP  10


/* Abort Info Identifier.                                           */

#define EINSS7_A96TCAP_USER_ABORT_INFO_ID     0xD8

/*Bind results */

#define EINSS7_A96TCAP_BIND_OK                0
#define EINSS7_A96TCAP_BIND_SSN_IN_USE        1
#define EINSS7_A96TCAP_BIND_PROTOCOL_ERR      2
#define EINSS7_A96TCAP_BIND_NO_RESOURCES      3
#define EINSS7_A96TCAP_BIND_SSN_NOT_ALLOW     4
#define EINSS7_A96TCAP_BIND_SCCP_NOTREADY     5

/*Components present */

#define EINSS7_A96TCAP_COMP_NOT_PRES          0
#define EINSS7_A96TCAP_COMP_PRES              1

/*Correlation id used */

#define EINSS7_A96TCAP_CORR_ID_NOT_USED       0
#define EINSS7_A96TCAP_CORR_ID_USED           0x80

/*Error code identity */

#define EINSS7_A96TCAP_ERR_ID_NATIONAL        0xD3
#define EINSS7_A96TCAP_ERR_ID_PRIVATE         0xD4

/*Invoke id used */

#define EINSS7_A96TCAP_INVOKE_ID_NOT_USED     0
#define EINSS7_A96TCAP_INVOKE_ID_USED         0x80

/*ANSI issue */
/* No Dialogue Portion and reject components    */
#define EINSS7_A96TCAP_ANSI_ISSUE_1           0x01
/* No Dialogue Portion and abort                */
#define EINSS7_A96TCAP_ANSI_ISSUE_2           0x02
/* Dialogue Portion and Abort                   */
#define EINSS7_A96TCAP_ANSI_ISSUE_3           0x03

/*Last component */            

#define EINSS7_A96TCAP_COMPONENT_NOT_LAST     0
#define EINSS7_A96TCAP_COMPONENT_LAST         1

/*Operation code Identifier */

#define EINSS7_A96TCAP_OPERATION_ID_NATIONAL  0xD0
#define EINSS7_A96TCAP_OPERATION_ID_PRIVATE   0xD1
#define EINSS7_A96TCAP_VALID_OPERATION_LENGTH 2
/*Operation Family */

#define EINSS7_A96TCAP_OP_FAMILY_PARAMETER      0x01
#define EINSS7_A96TCAP_OP_FAMILY_CHARGING       0x02
#define EINSS7_A96TCAP_OP_FAMILY_PROVIDE_INSTR  0x03
#define EINSS7_A96TCAP_OP_FAMILY_CONN_CNTRL     0x04
#define EINSS7_A96TCAP_OP_FAMILY_CALLER_INTERA  0x05
#define EINSS7_A96TCAP_OP_FAMILY_SEND_NOTIFIC   0x06
#define EINSS7_A96TCAP_OP_FAMILY_NETW_MANAGE    0x07
#define EINSS7_A96TCAP_OP_FAMILY_PROCEDURAL     0x08
#define EINSS7_A96TCAP_OP_FAMILY_OP_CNTRL       0x09
#define EINSS7_A96TCAP_OP_FAMILY_REPORT_EVENT   0x0A
#define EINSS7_A96TCAP_OP_FAMILY_MISCELLANEOUS  0xFE

/*Operation Specifier */

/*Codes for Operation Family: Parameter */

#define EINSS7_A96TCAP_OP_PA_PROVIDE_VALUE     0x01
#define EINSS7_A96TCAP_OP_PA_SET_VALUE         0x02

/*Codes for Operation Family: Charging */

#define EINSS7_A96TCAP_OP_CH_BILL_CALL         0x01

/*Codes for Operation Family: Provide Instruction */

#define EINSS7_A96TCAP_OP_PI_START             0x01
#define EINSS7_A96TCAP_OP_PI_ASSIST            0x02

/*Codes for Operation Family: Connection Control */

#define EINSS7_A96TCAP_OP_CC_CONNECT           0x01
#define EINSS7_A96TCAP_OP_CC_TEMP_CONN         0x02
#define EINSS7_A96TCAP_OP_CC_DISCONNECT        0x03
#define EINSS7_A96TCAP_OP_CC_FORW_DISC         0x04

/*Codes for Operation Family: Caller Interaction */

#define EINSS7_A96TCAP_OP_CI_PLAY_ANN          0x01
#define EINSS7_A96TCAP_OP_CI_PLAYA_CLDIG       0x02
#define EINSS7_A96TCAP_OP_CI_INFO_WAITING      0x03
#define EINSS7_A96TCAP_OP_CI_INFO_PROV         0x04

/*Codes for Operation Family: Send Notification */

#define EINSS7_A96TCAP_OP_SN_PARTY_FREE        0x01

/*Codes for Operation Family: Network Management */

#define EINSS7_A96TCAP_OP_NM_AUTO_CALL_GAP     0x01

/*Codes for Operation Family: Procedural */

#define EINSS7_A96TCAP_OP_PR_TEMP_HANDOVER     0x01
#define EINSS7_A96TCAP_OP_PR_REPORT_ASS_TERM   0x02

/*Codes for Operation Family: Operation Control */

#define EINSS7_A96TCAP_OP_OC_CANCEL            0x01

/*Codes for Operation Family: Report Event */

#define EINSS7_A96TCAP_OP_RE_VOICE_MESS_AVAIL  0x01
#define EINSS7_A96TCAP_OP_RE_VOICE_MESS_RETR   0x02

/*Codes for Operation Family: Miscallanous */

#define EINSS7_A96TCAP_OP_MI_QUEUE_CALL        0x01
#define EINSS7_A96TCAP_OP_MI_DEQUEUE_CALL      0x02

/*Problem code id */

#define EINSS7_A96TCAP_PROBLEM_TYPE_GENERAL    0x01
#define EINSS7_A96TCAP_PROBLEM_TYPE_INVOKE     0x02
#define EINSS7_A96TCAP_PROBLEM_TYPE_RESULT     0x03
#define EINSS7_A96TCAP_PROBLEM_TYPE_ERROR      0x04
#define EINSS7_A96TCAP_PROBLEM_TYPE_TRPORT     0x05

/*Codes for General Problem */

#define EINSS7_A96TCAP_PR_G_UNREC_COMP         0x01
#define EINSS7_A96TCAP_PR_G_INC_COMP_PORT      0x02
#define EINSS7_A96TCAP_PR_G_BAD_COMP           0x03
#define EINSS7_A96TCAP_PR_G_INCORR_COMP_CODING 0x04
#define EINSS7_A96TCAP_PR_G_MAX_CP_LENGTH      0x81
#define EINSS7_A96TCAP_PR_G_IS_BLOCK           0x83

/*Codes for Invoke Problem */

#define EINSS7_A96TCAP_PR_I_DUP_ID             1
#define EINSS7_A96TCAP_PR_I_UNREC_OP           2
#define EINSS7_A96TCAP_PR_I_INC_PARM           3
#define EINSS7_A96TCAP_PR_I_UNREC_C_ID         4

/*Codes for Return Result Problem */

#define EINSS7_A96TCAP_PR_R_UNASSIGN_C_ID      1
#define EINSS7_A96TCAP_PR_R_RES_UNEXP          2
#define EINSS7_A96TCAP_PR_R_INC_PARM           3

/*Codes for Return Error Problem */

#define EINSS7_A96TCAP_PR_E_UNASSIGN_C_ID      1
#define EINSS7_A96TCAP_PR_E_RET_ERR_UNEXP      2
#define EINSS7_A96TCAP_PR_E_UNREC_ERR          3
#define EINSS7_A96TCAP_PR_E_UNEXP_ERR          4
#define EINSS7_A96TCAP_PR_E_INC_PARM           5

/*Codes for Transaction Portion Problem */

#define EINSS7_A96TCAP_PR_T_UNREC_T_TYPE       1
#define EINSS7_A96TCAP_PR_T_INC_TR_PORT        2
#define EINSS7_A96TCAP_PR_T_BAD_TRANS          3
#define EINSS7_A96TCAP_PR_T_UNASSIGN_TR_ID     4
#define EINSS7_A96TCAP_PR_T_PERM_TO_REL        5
#define EINSS7_A96TCAP_PR_T_RES_UNAVAIL        6

/*Codes for Segmentation Indication */

#define EINSS7_A96TCAP_NOT_SEGMENTED_MSG       0
#define EINSS7_A96TCAP_SEGMENTED_MSG           1

/*Codes for Quality of service */

#define EINSS7_A96TCAP_TCAP_QOS_NOT_USED       0
#define EINSS7_A96TCAP_TCAP_SEQ_DEL_OPT        1
#define EINSS7_A96TCAP_TCAP_MSG_RET_OPT        2
#define EINSS7_A96TCAP_TCAP_ALL_SERVICE        3

/*Codes for report causes */

#define EINSS7_A96TCAP_REP_NO_TR_ADR_NATURE    0
#define EINSS7_A96TCAP_REP_NO_TR_ADR           1
#define EINSS7_A96TCAP_REP_SUBSYST_CONGEST     2
#define EINSS7_A96TCAP_REP_SUBSYST_FAILURE     3
#define EINSS7_A96TCAP_REP_UNEQUIP_USER        4
#define EINSS7_A96TCAP_REP_NETW_FAILURE        5
#define EINSS7_A96TCAP_REP_NETW_CONGEST        6
#define EINSS7_A96TCAP_REP_UNQUALIFIED         7
/*This one is only used for ANSI */
#define EINSS7_A96TCAP_REP_HOP_COUNT_VIOL      8

/*Codes for the Return Indicator */

#define EINSS7_A96TCAP_TCAP_NO_REL_DIAL_NO_COMP    0
#define EINSS7_A96TCAP_TCAP_NO_REL_DIAL_COMP       1
#define EINSS7_A96TCAP_TCAP_REL_DIAL_NO_COMP       2
#define EINSS7_A96TCAP_TCAP_REL_DIAL_COMP          3

/*Codes for Termination */

#define EINSS7_A96TCAP_TERM_BASIC_END          0
#define EINSS7_A96TCAP_TERM_PRE_ARR_END        1

/*Codes for User Status */

#define EINSS7_A96TCAP_USER_AVAILABLE          0
#define EINSS7_A96TCAP_USER_UNAVAILABLE        1


/*Comparison limits */

#define EINSS7_A96TCAP_VALID_OPERATION_LENGTH  2

/* xbmc: TR2060: Timeout limit given in 0.1s              */
#define EINSS7_A96TCAP_TIMEOUT_LIMIT           65000


/* xpho:  Constants for copying definite length data.     */
#define EINSS7_A96TCAP_DEF_LENGTH_2OCT       0x82
#define EINSS7_A96TCAP_DEF_LENGTH_1OCT       0x81

/* xpho: Application Context Identifiers                  */
#define EINSS7_A96TCAP_AC_NAME_INT_TAG       0xDB
#define EINSS7_A96TCAP_AC_NAME_OBJ_TAG       0xDC    

/* xpho: Security Context Identifiers                     */
#define EINSS7_A96TCAP_SEC_CONTEXT_INT_TAG   0x80
#define EINSS7_A96TCAP_SEC_CONTEXT_OBJ_TAG   0x81 

/* xpho: Codes for Priority Order   Low=0, High=3         */
#define EINSS7_A96TCAP_PRI_HIGH_0	        0
#define EINSS7_A96TCAP_PRI_HIGH_1	        1
#define EINSS7_A96TCAP_PRI_HIGH_2	        2
#define EINSS7_A96TCAP_PRI_HIGH_3	        3

/* xpho: Codes for Importance      Low=4, High=11         */
#define EINSS7_A96TCAP_MSG_PRI_IMP_0            4
#define EINSS7_A96TCAP_MSG_PRI_IMP_1            5
#define EINSS7_A96TCAP_MSG_PRI_IMP_2            6
#define EINSS7_A96TCAP_MSG_PRI_IMP_3            7
#define EINSS7_A96TCAP_MSG_PRI_IMP_4            8
#define EINSS7_A96TCAP_MSG_PRI_IMP_5            9
#define EINSS7_A96TCAP_MSG_PRI_IMP_6            10
#define EINSS7_A96TCAP_MSG_PRI_IMP_7            11

/* xpho: set max priority level                           */
#if defined EINSS7_ITU_SCCP
#define EINSS7_A96TCAP_MAX_MSG_PRI_IMP          11
#elif defined EINSS7_TTC_SCCP                
#define EINSS7_A96TCAP_MAX_MSG_PRI_IMP          11
#elif defined EINSS7_TWENTYFOURBITPC
#define EINSS7_A96TCAP_MAX_MSG_PRI_IMP          11
#else
#define EINSS7_A96TCAP_MAX_MSG_PRI_IMP          3
#endif

/* xpho: The max message size                              */
#if defined EINSS7_ITU_SCCP
#define EINSS7_A96_MAX_MSG_SIZE                   3952
#elif defined EINSS7_TTC_SCCP
#define EINSS7_A96_MAX_MSG_SIZE                   3936
/* Chinese and ANSI Max message size are the same          */
#else 
/* Default max message size ANSI                           */
#define EINSS7_A96_MAX_MSG_SIZE                   3904
#endif  



/********************************************************************/
/*                                                                  */
/*          R E Q U E S T   R E T U R N   C O D E S                 */
/*                                                                  */
/********************************************************************/

#define EINSS7_A96TCAP_REQUEST_OK             0

#define EINSS7_A96TCAP_INV_INVOKE_ID_USED     16
#define EINSS7_A96TCAP_INV_DIALOGUE_ID        17
#define EINSS7_A96TCAP_INV_QLT_OF_SERVICE     18
#define EINSS7_A96TCAP_INV_DEST_ADR_LENGTH    19
#define EINSS7_A96TCAP_INV_ORG_ADR_LENGTH     20
#define EINSS7_A96TCAP_INV_TERMINATION        21
#define EINSS7_A96TCAP_INV_CORR_ID_USED       22
#define EINSS7_A96TCAP_INV_OP_SPEC            23
#define EINSS7_A96TCAP_INV_TIMEOUT            24
#define EINSS7_A96TCAP_INV_OP_ID              25
#define EINSS7_A96TCAP_INV_OP_LENGTH          26
#define EINSS7_A96TCAP_INV_ERR_ID             27
#define EINSS7_A96TCAP_INV_ERR_LENGTH         28
#define EINSS7_A96TCAP_INV_PROBLEM_TYPE       29
#define EINSS7_A96TCAP_INV_PROBLEM_SPEC       30
#define EINSS7_A96TCAP_INV_ABORT_INFO_LENGTH  31
#define EINSS7_A96TCAP_INV_OP_FAMILY          32
#define EINSS7_A96TCAP_INV_USER_ID            33
#define EINSS7_A96TCAP_INV_ISSUE              34
#define EINSS7_A96TCAP_INV_PRIORITY_LEVEL     35
/* xpho: Return Codes for invalid Application context Tag and Security Tag */
#define EINSS7_A96TCAP_INV_AC_TAG             36
#define EINSS7_A96TCAP_INV_SEC_TAG            37
/* xpho: Return code for invalid message size                       */                
#define EINSS7_A96TCAP_INV_MSG_SIZE           38

#ifdef EINSS7_FUNC_POINTER
/********************************************************************/
/*                                                                  */
/*          EINSS7_FUNC_POINTER   R E T U R N   C O D E S           */
/*                                                                  */
/********************************************************************/

#define EINSS7_A96TCAP_CALLBACK_FUNC_NOT_SET     250
#define EINSS7_A96TCAP_CALLBACK_FUNC_ALREADY_SET 251
#endif

/********************************************************************/
/*                                                                  */
/*          M A X   A D D R E S S   L E N G T H                     */
/*                                                                  */
/********************************************************************/
/* This one should be the largest between the supported variants    */
#if defined EINSS7_ITU_SCCP
#define EINSS7_A96TCAP_MAX_ADDRESS_LENGTH       18
#elif defined EINSS7_TTC_SCCP
#define EINSS7_A96TCAP_MAX_ADDRESS_LENGTH       18
#elif defined EINSS7_TWENTYFOURTBITPC
#define EINSS7_A96TCAP_MAX_ADDRESS_LENGTH       20
#else
#define EINSS7_A96TCAP_MAX_ADDRESS_LENGTH       19
#endif

/********************************************************************/
/*                                                                  */
/*          S E N D   B U F F E R   S I Z E                         */
/*                                                                  */
/********************************************************************/
/*xcpa: CR0449: added definition for EINSS7_A96TCAP_MAX_SENDBUFSIZE */
#ifdef EINSS7_NO_APISEND
#define EINSS7_A96TCAP_MAX_SENDBUFSIZE          3968
#endif

/********************************************************************/
/*                                                                  */
/*          I N D I C A T I O N   R E T U R N   C O D E S           */
/*                                                                  */
/********************************************************************/

#define EINSS7_A96TCAP_IND_UNKNOWN_CODE      254
#define EINSS7_A96TCAP_IND_LENGTH_ERROR      255

/********************************************************************/
/*                                                                  */
/*          T Y P E D E F   F O R   EINSS7_A96_SENDBUFFER_T         */
/*                                                                  */
/********************************************************************/
#ifdef EINSS7_NO_APISEND
typedef struct EINSS7_A96TCAP_SENDBUFFER_TAG
{
    USHORT_T size;
    UCHAR_T *sendBuffer_p;
} EINSS7_A96TCAP_SENDBUFFER_T;
#endif
 
/********************************************************************/
/*                                                                  */
/*             M E S S A G E   S E N D   P R O T O T Y P E S        */
/*                                                                  */
/********************************************************************/

USHORT_T EINSS7_A96TcapApiSend(MSG_T *msg_sp,
			       USHORT_T  userId,
			       UCHAR_T   *end_p);
 
/********************************************************************/
/*                                                                  */
/*              R E Q U E S T    P R O T O T Y P E S                */
/*                                                                  */
/********************************************************************/


USHORT_T EINSS7_A96TcapBindReq(UCHAR_T,
			       USHORT_T,
			       UCHAR_T);

USHORT_T EINSS7_A96TcapUnBindReq(UCHAR_T, 
				 USHORT_T);

USHORT_T EINSS7_A96TcapUniReq(UCHAR_T,
			      USHORT_T,
			      USHORT_T,
			      UCHAR_T,
			      UCHAR_T,
			      UCHAR_T,
			      UCHAR_T *,
			      UCHAR_T,
			      UCHAR_T *,
			      UCHAR_T,
			      USHORT_T,
			      UCHAR_T *,
			      USHORT_T,
			      UCHAR_T *,
			      UCHAR_T,
			      USHORT_T,
			      UCHAR_T *,
			      USHORT_T,
			      UCHAR_T *);

USHORT_T EINSS7_A96TcapQuerywpReq(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T *,
				  UCHAR_T,
				  UCHAR_T *,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapQuerywopReq(UCHAR_T,
				   USHORT_T,
				   USHORT_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T *,
				   UCHAR_T,
				   UCHAR_T *,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *);

USHORT_T EINSS7_A96TcapConvwpReq(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *);
 
USHORT_T EINSS7_A96TcapConvwopReq(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T *,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapResponseReq(UCHAR_T,
				   USHORT_T,
				   USHORT_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *);

USHORT_T EINSS7_A96TcapUAbortReq(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_A96TcapInvokeLReq(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapInvokeNLReq(UCHAR_T,
				   USHORT_T,
				   USHORT_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *);

USHORT_T EINSS7_A96TcapResultNLReq(UCHAR_T,
				   USHORT_T,
				   USHORT_T,
				   UCHAR_T,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *);
 
USHORT_T EINSS7_A96TcapResultLReq(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapErrorReq(UCHAR_T,
				USHORT_T,
				USHORT_T,
				UCHAR_T,
				UCHAR_T,
				UCHAR_T,
				USHORT_T,
				UCHAR_T*,
				USHORT_T,
				UCHAR_T*);

USHORT_T EINSS7_A96TcapRRejectReq(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapUCancelReq(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T);

/* xcpa: CR0449: added prototype for EINSS7_A96TcapGetSendBuffer()  */
#ifdef EINSS7_NO_APISEND
EINSS7_A96TCAP_SENDBUFFER_T *EINSS7_A96TcapGetSendBuffer(void); 
#endif
 
/********************************************************************/
/*                                                                  */
/*       C O N F I R M A T I O N   A N D  I N D I C A T I O N       */
/*                    P R O T O T Y P E S                           */
/*                                                                  */
/********************************************************************/

USHORT_T EINSS7_A96TcapHandleInd(MSG_T *mqp_sp);

#ifdef EINSS7_FUNC_POINTER

/*creates the datatypes with "pointer to function" */

typedef USHORT_T (*EINSS7_A96TBINDCONF_T)(UCHAR_T,
					  USHORT_T,
					  UCHAR_T);

typedef USHORT_T (*EINSS7_A96TUNIIND_T)(UCHAR_T,
					USHORT_T,
					UCHAR_T,
					UCHAR_T,
					UCHAR_T *,
					UCHAR_T,
					UCHAR_T *,
					UCHAR_T,
					UCHAR_T,
					USHORT_T,
					UCHAR_T *,
					USHORT_T,
					UCHAR_T *,
					UCHAR_T,
					USHORT_T,
					UCHAR_T *,
					USHORT_T,
					UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TQUERYWPIND_T)(UCHAR_T,
      			                    USHORT_T,
				            USHORT_T,
				            UCHAR_T,
				            UCHAR_T,
				            UCHAR_T,
				            UCHAR_T *,
				            UCHAR_T,
				            UCHAR_T*,
				            UCHAR_T,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TQUERYWOPIND_T)(UCHAR_T,
					     USHORT_T,
					     USHORT_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T *,
					     UCHAR_T,
					     UCHAR_T*,
					     UCHAR_T,
					     UCHAR_T,
					     USHORT_T,
					     UCHAR_T *,
					     USHORT_T,
					     UCHAR_T *,
					     UCHAR_T,
					     USHORT_T,
					     UCHAR_T *,
					     USHORT_T,
					     UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TCONVWPIND_T)(UCHAR_T,
					   USHORT_T,
					   USHORT_T,
					   UCHAR_T,
					   UCHAR_T,
					   UCHAR_T,
					   UCHAR_T,
					   USHORT_T,
					   UCHAR_T *,
					   USHORT_T,
					   UCHAR_T *,
					   UCHAR_T,
					   USHORT_T,
					   UCHAR_T *,
					   USHORT_T,
					   UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TCONVWOPIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TRESPONSE_T)(UCHAR_T,
					  USHORT_T,
					  USHORT_T,
					  UCHAR_T,
					  UCHAR_T,
					  UCHAR_T,
					  UCHAR_T,
					  USHORT_T,
					  UCHAR_T *,
					  USHORT_T,
					  UCHAR_T *,
					  UCHAR_T,
					  USHORT_T,
					  UCHAR_T *,
					  USHORT_T,
					  UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TPUABORTIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TPPABORTIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T);

typedef USHORT_T (*EINSS7_A96TNOTICEIND_T)(UCHAR_T,
					   USHORT_T,
					   USHORT_T,
					   UCHAR_T,
					   UCHAR_T,
					   USHORT_T,
					   UCHAR_T);

typedef USHORT_T (*EINSS7_A96TINVOKELIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *,
					    USHORT_T,
					    UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TINVOKENLIND_T)(UCHAR_T,
					     USHORT_T,
					     USHORT_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T,
					     USHORT_T,
					     UCHAR_T *,
					     USHORT_T,
					     UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TRESULTNLIND_T)(UCHAR_T,
					     USHORT_T,
					     USHORT_T,
					     UCHAR_T,
					     UCHAR_T,
					     UCHAR_T,
					     USHORT_T,
					     UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TRESULTLIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TERRORIND_T)(UCHAR_T,
					  USHORT_T,
					  USHORT_T,
					  UCHAR_T,
					  UCHAR_T,
					  UCHAR_T,
					  UCHAR_T,
					  USHORT_T,
					  UCHAR_T*,
					  USHORT_T,
					  UCHAR_T*);

typedef USHORT_T (*EINSS7_A96TRREJECTIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T,
					    USHORT_T,
					    UCHAR_T *);

typedef USHORT_T (*EINSS7_A96TLREJECTIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T,
					    UCHAR_T,
					    UCHAR_T);

typedef USHORT_T (*EINSS7_A96TLCANCELIND_T)(UCHAR_T,
					    USHORT_T,
					    USHORT_T,
					    UCHAR_T);


typedef USHORT_T (*EINSS7_A96TINDERROR_T)(USHORT_T, 
				          MSG_T*);

typedef USHORT_T (*EINSS7_A96TSTATEIND_T)(UCHAR_T,
					  USHORT_T,
					  UCHAR_T,
					  UCHAR_T,
					  ULONG_T,
					  ULONG_T,
					  UCHAR_T);


typedef struct EINSS7_A96TCAPINIT
 {
        /* Pointer to a function */
        EINSS7_A96TBINDCONF_T     EINSS7_A96TcapBindConf;
        EINSS7_A96TUNIIND_T       EINSS7_A96TcapUniInd;
        EINSS7_A96TQUERYWPIND_T   EINSS7_A96TcapQuerywpInd;
        EINSS7_A96TQUERYWOPIND_T  EINSS7_A96TcapQuerywopInd;
        EINSS7_A96TCONVWPIND_T    EINSS7_A96TcapConvwpInd;
        EINSS7_A96TCONVWOPIND_T   EINSS7_A96TcapConvwopInd;
        EINSS7_A96TRESPONSE_T     EINSS7_A96TcapResponseInd;
        EINSS7_A96TPUABORTIND_T   EINSS7_A96TcapUAbortInd;
        EINSS7_A96TPPABORTIND_T   EINSS7_A96TcapPAbortInd;
        EINSS7_A96TNOTICEIND_T    EINSS7_A96TcapNoticeInd;
        EINSS7_A96TINVOKELIND_T   EINSS7_A96TcapInvokeLInd;
        EINSS7_A96TINVOKENLIND_T  EINSS7_A96TcapInvokeNLInd;
        EINSS7_A96TRESULTNLIND_T  EINSS7_A96TcapResultNLInd;
        EINSS7_A96TRESULTLIND_T   EINSS7_A96TcapResultLInd;
        EINSS7_A96TERRORIND_T     EINSS7_A96TcapErrorInd;
        EINSS7_A96TRREJECTIND_T   EINSS7_A96TcapRRejectInd;
        EINSS7_A96TLREJECTIND_T   EINSS7_A96TcapLRejectInd;
        EINSS7_A96TLCANCELIND_T   EINSS7_A96TcapLCancelInd;
        EINSS7_A96TINDERROR_T     EINSS7_A96TcapIndError;
        EINSS7_A96TSTATEIND_T     EINSS7_A96TcapStateInd;
} EINSS7_A96TCAPINIT_T;

#else

USHORT_T EINSS7_A96TcapBindConf(UCHAR_T,
				USHORT_T,
				UCHAR_T);

USHORT_T EINSS7_A96TcapUniInd(UCHAR_T,
			      USHORT_T,
			      UCHAR_T,
			      UCHAR_T,
			      UCHAR_T *,
			      UCHAR_T,
			      UCHAR_T *,
			      UCHAR_T,
			      UCHAR_T,
			      USHORT_T,
			      UCHAR_T *,
			      USHORT_T,
			      UCHAR_T *,
			      UCHAR_T,
			      USHORT_T,
			      UCHAR_T *,
			      USHORT_T,
			      UCHAR_T *);

USHORT_T EINSS7_A96TcapQuerywpInd(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T *,
				  UCHAR_T,
				  UCHAR_T *,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapQuerywopInd(UCHAR_T,
				   USHORT_T,
				   USHORT_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T *,
				   UCHAR_T,
				   UCHAR_T*,
				   UCHAR_T,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *);

USHORT_T EINSS7_A96TcapConvwpInd(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_A96TcapConvwopInd(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapResponseInd(UCHAR_T,
				   USHORT_T,
				   USHORT_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *,
				   UCHAR_T,
				   USHORT_T,
				   UCHAR_T *,
				   USHORT_T,
				   UCHAR_T *);

USHORT_T EINSS7_A96TcapUAbortInd(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_A96TcapPAbortInd(UCHAR_T,
				USHORT_T,
				USHORT_T,
				UCHAR_T,
				UCHAR_T,
				UCHAR_T);

USHORT_T EINSS7_A96TcapNoticeInd(UCHAR_T,
				USHORT_T,
				USHORT_T,
				UCHAR_T,
				UCHAR_T,
				USHORT_T,
				UCHAR_T);

USHORT_T EINSS7_A96TcapInvokeLInd(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *,
				 USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_A96TcapInvokeNLInd(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapResultNLInd(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T,
				  UCHAR_T,
				  UCHAR_T,
				  USHORT_T,
				  UCHAR_T *);

USHORT_T EINSS7_A96TcapResultLInd(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_A96TcapErrorInd(UCHAR_T,
				USHORT_T,
				USHORT_T,
				UCHAR_T,
				UCHAR_T,
				UCHAR_T,
				UCHAR_T,
				USHORT_T,
				UCHAR_T*,
				USHORT_T,
				UCHAR_T*);

USHORT_T EINSS7_A96TcapRRejectInd(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T,
				 USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_A96TcapLRejectInd(UCHAR_T,
				 USHORT_T,
				 USHORT_T,
				 UCHAR_T,
				 UCHAR_T,
				 UCHAR_T);

USHORT_T EINSS7_A96TcapLCancelInd(UCHAR_T,
				  USHORT_T,
				  USHORT_T,
				  UCHAR_T);

USHORT_T EINSS7_A96TcapIndError(USHORT_T, 
				MSG_T*);

USHORT_T EINSS7_A96TcapStateInd(UCHAR_T,
				USHORT_T,
				UCHAR_T,
				UCHAR_T,
				ULONG_T,
				ULONG_T,
				UCHAR_T);

#endif


#ifdef EINSS7_FUNC_POINTER

/*The application must call this function to register the call back funtions. */
USHORT_T EINSS7_A96TcapRegFunc(EINSS7_A96TCAPINIT_T* );

#endif

/*C++ compability definitions */
#if defined (__cplusplus) || (c_plusplus)
}
#endif
#endif /* __A96TCAPAPI_H__ */
