/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_consts.h,v                                                         */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infocom AB. The program may be used and/or  copied only  */
/* with the written permission from Ericsson Infocom  AB or in the   */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* et96map_consts.h                                                  */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                                                  */
/* 33/190 55-CAA 201 45 Ux                                            */
/* Revision:                                                         */
/* @EINVER: et96map_consts.h,v 1.19 2000/04/07 09:03:59 Exp                      */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/P <Your Name>                                               */
/*                                                                   */
/* Purpose:                                                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/


#ifndef __ET96MAP_CONSTS_H__
#define __ET96MAP_CONSTS_H_

#include "portss7.h"
#ifdef ETSIMAP_ID_2
#define ETSIMAPMODULEID ETSIMAP2_ID
#define USEDMAPMODULEID 8  
#else
#define ETSIMAPMODULEID ETSIMAP_ID
#define USEDMAPMODULEID 4
#endif

#ifdef ITUTCAP_ID_2
#define ITUTCAPMODULEID /* for future use */ 
#define USEDTCAPMODULEID 32   
#else
#define ITUTCAPMODULEID TCAP_ID
#define USEDTCAPMODULEID 16 
#endif
 
#define ITUFLAG   00
#define CHINAFLAG 01
#define ANSIFLAG 03


#ifdef EINSS7_ANSI_SCCP
#define COMPILETIMEFLAG ANSIFLAG;
#elif EINSS7_TWENTYFOURBITPC
#define COMPILETIMEFLAG CHINAFLAG;
#else
#define COMPILETIMEFLAG ITUFLAG;
#endif

#define ETSITAG 8










/*********************************************************************/
/*                                                                   */
/*                        COMMON MAP DEFINES                         */
/*                                                                   */
/*********************************************************************/

/* MAP sleep timer after receipt of terminate order */
#define ET96MAP_SLEEP_TIME 5

/* max number of MAP dialogues */


/* max number of MAP users */
#define ET96MAP_MAX_USERS 255 /* Changed from 253 -> 254, TR1367, qinxolj*/

/* default number of maximum number of transactions allowed. This is */
/* used when a phase 1 configuration file is being used and a value  */
/* for this parameter is not given in the config file. The USSD srvcs*/
/* need to use the transaction machine hence the need for this.      */
#define ET96MAP_MAX_DEFAULT_TRANS 20

/* MAP Error reasons taken from the MAP R4 FS */
/*#define MAP_ERROR_OFFSET 17000*/ /* applies in portss7.h */

/* Maximum number of timers alllowed for MAP */
/* #define ET96MAP_MAX_TIMERS 20 replaced by entry in configurationfile, TR1367*/

/* max size of buffer used to hold result of ASN1 parser */
#define ET96MAP_MAX_ASN1_BUF 1024

/* Define constants for ASN.1 */
#define ET96MAP_IMSI_TAG       0xc0
#define ET96MAP_ORIGREF_TAG    0xc1
#define ET96MAP_MSISDN_TAG     0xc2

/* component last indicator used in tinvoke indication */
#define ET96MAP_COMP_NOT_LAST 0
#define ET96MAP_COMP_LAST 1

/* error code tag */
#define ET96MAP_TAG_LOCAL 0x02


/* AePONA */
#define ET96MAP_DCS_LENGTH 1
#define ET96MAP_ALERTING_PATTERN_LENGTH 1
/* AePONA */





/*********************************************************************/
/*                                                                   */
/*                          TCAP API DEFINES                         */
/*                                                                   */
/*********************************************************************/

/* The following values have been taken from the tcapapi.h file     */
/* in paul's home directory. The tcapapi.h file is not being used   */
/* because they contain the fn headers for the tcap fns which would */
/* conflict with the MAP's tcap If fn headers. This conflict must   */
/* be resolved at or before integration test. */

/* Max users on one SSN */
#define ET96MAP_MAX_USERS_ON_SSN               10


/* Abort Cause  						     */
#define ET96MAP_ABORT_UNREC_MES	       	         0
#define ET96MAP_ABORT_UNREC_ID	       	         1
#define ET96MAP_ABORT_BAD_TP	       	         2
#define ET96MAP_ABORT_INCORRECT_TP               3
#define ET96MAP_ABORT_RESOURCE_LIMIT             4
#define ET96MAP_ABORT_ABNORMAL_DIALOGUE          5
#define ET96MAP_ABORT_NO_COMMON_DIALOGUE_PORTION 6

/* Bind Result	 						     */
#define ET96MAP_BIND_OK		       	0
#define ET96MAP_BIND_SSN_IN_USE		1
#define ET96MAP_BIND_PROTOCOL_ERR	2
#define ET96MAP_BIND_NO_RESOURCES	3
#define ET96MAP_BIND_INVALID_SSN	4
#define ET96MAP_BIND_SCCP_NOT_READY	5
#define ET96MAP_BIND_USERID_IN_USE      6
#define ET96MAP_BIND_USERID_NOT_ALLOWED 6
#define ET96MAP_BIND_NOT_DEFINED_RESULT 7
#define ET96MAP_BIND_TIMER_EXPIRED      8
#define ET96MAP_BIND_NO_PREV_CONF       9
#define ET96MAP_BIND_USERS_RESOURCES_UNAVAIL 10

/* Operation Class 						     */
/* HeUl Change values on operation classes 1 -> 0 and so on */
#define ET96MAP_OP_CLASS_1	       	0
#define ET96MAP_OP_CLASS_2	       	1
#define ET96MAP_OP_CLASS_3	       	2
#define ET96MAP_OP_CLASS_4	       	3

/* Components Present 						     */
#define ET96MAP_COMP_NOT_PRES	       	0
#define ET96MAP_COMP_PRES	       	1

/* Error code tag 						     */
#define ET96MAP_ERR_TAG_LOCAL	       	2
#define ET96MAP_ERR_TAG_GLOBAL	       	6

/* Invoke id used 						     */
#define ET96MAP_INVOKE_ID_USED	       	128
#define ET96MAP_INVOKE_ID_NOT_USED     	0

/* Last component 						     */
#define ET96MAP_COMPONENT_NOT_LAST     	0
#define ET96MAP_COMPONENT_LAST	       	1

/* Linked ID used 						     */
#define ET96MAP_LINKED_ID_NOT_USED     	0
#define ET96MAP_LINKED_ID_USED	       	128

/* Operation code tag 						     */
#define ET96MAP_OPERATION_TAG_LOCAL	2
#define ET96MAP_OPERATION_TAG_GLOBAL	6

/* Problem code tag 						     */
#define ET96MAP_PROBLEM_TAG_GENERAL     128
#define ET96MAP_PROBLEM_TAG_INVOKE	129
#define ET96MAP_PROBLEM_TAG_RESULT	130
#define ET96MAP_PROBLEM_TAG_ERROR	131

/* Codes for General Problem 					     */
#define ET96MAP_PR_G_UNREC_COMP        	0
#define ET96MAP_PR_G_MISTYPED_COMP     	1
#define ET96MAP_PR_G_BAD_COMP	       	2

/* Codes for Invoke Problem 					     */
#define ET96MAP_PR_I_DUP_ID	       	0
#define ET96MAP_PR_I_UNREC_OP	       	1
#define ET96MAP_PR_I_MIST_PARM	       	2
#define ET96MAP_PR_I_RES_LIMIT	       	3
#define ET96MAP_PR_I_INIT_RELEASE      	4
#define ET96MAP_PR_I_UNREC_L_ID        	5
#define ET96MAP_PR_I_RESP_UNEXP        	6
#define ET96MAP_PR_I_UNEXP_OP	       	7

/* Codes for Return Result Problem 				     */
#define ET96MAP_PR_R_UNREC_I_ID        0
#define ET96MAP_PR_R_RES_UNEXP	       1
#define ET96MAP_PR_R_MIST_PARM	       2

/* Codes for Return Error Problem 				     */
#define ET96MAP_PR_E_UNREC_I_ID        0
#define ET96MAP_PR_E_RET_ERR_UNEXP     1
#define ET96MAP_PR_E_UNREC_ERR	       2
#define ET96MAP_PR_E_UNEXP_ERR	       3
#define ET96MAP_PR_E_MIST_PARM	       4

/* Codes for report causes 					     */
#define ET96MAP_REP_NO_TR_ADR_NATURE	0
#define ET96MAP_REP_NO_TR_ADR		1
#define ET96MAP_REP_SUBSYST_CONGEST	2
#define ET96MAP_REP_SUBSYST_FAILURE	3
#define ET96MAP_REP_UNEQUIP_USER	4
#define ET96MAP_EP_NETW_FAILURE	5
#define ET96MAP_REP_NETW_CONGEST	6
#define ET96MAP_EP_UNQUALIFIED		7
#define ET96MAP_REP_HOPCOUNT_VIOL	8

/* Codes for the Return Indicator 				     */
#define ET96MAP_IND_NOCOMP_NODIALOG	0
#define ET96MAP_IND_COMP_NODIALOG	1
#define ET96MAP_IND_NOCOMP_DIALOG	2
#define ET96MAP_IND_COMP_DIALOG		3

/* Codes for the Segmentation Indicator 			     */
#define ET96MAP_NOT_SEGMENTED_MSG	0
#define ET96MAP_SEGMENTED_MSG		1

/* Codes for Termination 					     */
#define ET96MAP_TERM_BASIC_END		0
#define ET96MAP_TERM_PRE_ARR_END	1

/* Codes for Quality of service 				     */
#define ET96MAP_QLT_NOT_USED		0
#define ET96MAP_QLT_IN_SEQUENCE		1
#define ET96MAP_QLT_MESS_RET		2
#define ET96MAP_QLT_BOTH		3

/* Codes for Priority Order	 	Low=0, High=3		     */
#define ET96MAP_PRI_HIGH_0	       0
#define ET96MAP_PRI_HIGH_1	       1
#define ET96MAP_PRI_HIGH_2	       2
#define ET96MAP_PRI_HIGH_3	       3

/* Codes for Kind of TCAP		White / Blue		     */
#define ET96MAP_BLUE_USER	       0
#define ET96MAP_WHITE_USER	       1





/* The max size of the local and remote address field.		     */
/* Excluding the lengthfield, but in some cases including some SCCP  */
/* overhead							     */

/***************** Not Used
#ifdef EINSS7_ANSI_SCCP
#define ET96MAP_MAX_ADDRESS_LENGTH	12
#else
#define ET96MAP_MAX_ADDRESS_LENGTH	18
#endif
End Not Used ****************************/


/* The max length of User Information data.			     */
#define ET96MAP_MAX_UI_LENGTH		3096


/*********************************************************************/
/*                                                                   */
/*                      ASN1 COMPILER DEFINES                        */
/*                                                                   */
/*********************************************************************/

/* ASN1 operation codes */


/* AePONA */
#define ET96MAP_PROCESS_UNSTRUCTURED_SS_DATA 19 /* 0x13 */
#define ET96MAP_BEGIN_SUBSCRIBER_ACTIVITY 54 /* 0x36 */
/* AePONA */

#define ET96MAP_PROCESS_UNSTRUCTURED_SS_REQUEST 59
#define ET96MAP_UNSTRUCTURED_SS_REQUEST 60
#define ET96MAP_UNSTRUCTURED_SS_NOTIFY 61
#define ET96MAP_SEND_ROUTING_INFO_FOR_SM 45
#define ET96MAP_FORWARD_SM 46
#define ET96MAP_REPORT_SM_DELIVERY_STATUS 47
#define ET96MAP_ALERT_SERVICE_CENTRE_V1 49
#define ET96MAP_ALERT_SERVICE_CENTRE 64
#define ET96MAP_INFORM_SERVICE_CENTRE 63
#define ET96MAP_READY_FOR_SM 66
#define ET96MAP_V3_FORWARD_SM_MT 44
#define ET96MAP_PROVIDE_SUBSCRIBER_INFO 70

/* Added By DON */
#define ET96MAP_ANY_TIME_INTERROGATION 71
/* Added By DON */


/* phase 1 ASN1 operation codes */

#define ET96MAP_OPCODE_FWD_MO_SM     249 
#define ET96MAP_OPCODE_FWD_MT_SM     250
#define ET96MAP_OPCODE_ALERT_SC      251


/* phase 3 ASN1 operation codes */

#define ET96MAP_SEND_ROUTING_INFO_FOR_LCS 85
#define ET96MAP_PROVIDE_SUBSCRIBER_LCS 83
#define ET96MAP_SUBSCRIBER_LOCATION_REPORT_LCS 86

/* ASN1 Error local Valuese */
/* Not Used ****************
#define SYSTEM_FAILURE 34
#define DATA_MISSING 35
#define UNEXPECTED_DATA_VALUE 36
#define UNKNOWN_ALPHABET 71
#define CALL_BARRED 13
#define ABSENT_SUBSCRIBER 27
#define ILLEGAL_SUBSCRIBER 9
#define ILLEGAL_EQUIPMENT 12
#define USSD_BUSY 72
#define FACILITY_NOT_SUPPORTED 21
#define TELESERVICE_NOT_PROVISIONED 11
#define UNIDENTIFIED_SUBSCRIBER 5
#define SUBSCRIBER_BUSY_FOR_MTSMS 31
#define SM_DELIVERY_FAILURE 32
#define MESSAGE_WAITING_LIST_FULL 33
End not used *************************/



#endif /* __ET96MAP_CONSTS_H__ */
