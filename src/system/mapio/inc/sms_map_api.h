/*********************************************************************/
/* Don't remove these two lines, contain depends on them!            */
/* Document Number: %Container% Revision: %Revision%				 */
/*                                                                   */
/* sms_map_api.h,v										 */
/*                                                                   */
/* This header file defines all constants, type definitions and      */
/* structures used in the SMS-MAP. It also defines the SMS-MAP-API   */
/* function prototypes as required by an SMS-MAP User Application.   */
/*																	 */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infocom AB 1996                                */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infocom AB. The program may be used and/or  copied only  */
/* with the written permission from Ericsson Infocom  AB or in the   */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:           Rev:                                   */
/* 46/190 55-CAA 201 17 Uen    A                                     */
/*                                                                   */
/* Revision:							     */
/* @EINVER: sms_map_api.h,v 1.3 1998/11/25 10:23:38 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Kevin Glavin                                                      */
/*                                                                   */
/* Purpose:                                                          */
/* Definitions needed by sms_map_api.c module                        */
/*                                                                   */
/* Input/output relations:                                           */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*   00-930611 Kevin Glavin                                          */
/*             Initial issue                                         */
/*   01-940216 Jorgen Trank					     */
/*	       Editorial changes				     */
/*   02-960403 Ulf Melin                                             */
/*             Included USSD services                                */
/*   03-960422 Ulf Melin                                             */
/*             Changed primitives MAP_ERROR_req & MAP_ERROR_ind      */
/*                                                                   */
/*   04-970915 Mikael Blom Ericsson Infotech AB.                     */
/*	Added compile switch EINSS7_FUNC_POINTER for use of local    */
/*	naming of the functions(primitives) in the application.	     */
/*								     */
/*   05-970929 Jòrgen Wallin Ericsson Infotech AB.		     */
/*	       Added functions to solve the problem with little      */
/*	       and big-edian between Intel and Sparc processors.     */
/*								     */
/*   03-981005 Jan Olm						     */
/*             Added USSD functionality                              */
/*			   Map_Proc_Ussd_Data_ind		     */
/*			   Map_Proc_Ussd_Data_resp		     */
/*			   Map_Submit_req			     */
/*			   Map_Submit_conf			     */
/*								     */
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/


#ifndef __SMS_MAP_API_H__
#define __SMS_MAP_API_H__

#if defined __cplusplus || defined c_plusplus
extern "C" {
#endif

#ifndef __LIMITS_H__
#define __LIMITS_H__
#include <limits.h>  
#endif

/***************************************************************************/
/*              Header files which sms_map_api.h requires.                 */
/***************************************************************************/

#include "ss7osdpn.h"
#include "portss7.h"
#include "ss7tmc.h"
#include "ss7msg.h"



/***************************************************************************/
/*         Constant and flag definitions required by SMS-MAP-API.          */
/***************************************************************************/


#define TL_ID                      0    /* SMS-MAP-API layer IPC ID - this
                                         * value is only temporarily defined
                                         * here for compliation reasons.
                                         */

#define MAP_USER_ID                0    /* SMS-MAP-USER layer IPC ID */

#define MAP_MSG_PRIMITIVE         15    /* SMS-MAP-API to SMS-MAP IPC message
                                         * primitive value - this value is only
                                         * defined here for compliation reasons.
                                         */


#define MAP_API_MAX_ENTRIES       20    /* IPC Message Queue size */

#define MAP_DCS_ANNEX2             0    /* Data coding scheme GSM 03.40 Ax. 2 */

#define MAP_DCS_CHINESE            8    /* Data coding scheme GSM 03.40 Ax. 2 */
/* Added new values for data coding scheme ErAn 960906, CR0064 */
#define MAP_DCS_LANG_UNSP_ANNEX2   0xF0	/* Data coding scheme "Language	*/
					/* unspecified", Annex 2 coding	*/
#define MAP_DCS_LANG_UNSP_MASK	   0xFC	/* Mask for "Language unspecific */
#define MAP_DCS_LANG_UNSP_MASK_2   0xCC	/* Mask for "Language unspecific */

#define MAP_DIALOGUE_NOT_TERMINATED 0
#define MAP_DIALOGUE_TERMINATED    1
#define MAP_PROC_USS_REQ          59
#define MAP_USS_REQ               60
#define MAP_USS_NOTIFY            61
#define MAP_OPNOTFOLLOW            0
#define MAP_OPFOLLOW               1

/* Parameter range values. */
#define MAP_SM_ID_MIN              0	/* SM Identification */
#define MAP_SM_ID_MAX      ULONG_MAX 
#define MAP_DCS_MIN                0    /* Data Coding Scheme */
#define MAP_DCS_MAX              255
#define MAP_USER_DATA_MIN          0    /* Length of SM User Data */
#define MAP_USER_DATA_MAX        200
#define MAP_ADDR_MIN_BYTES         0    /* SME address */
#define MAP_ADDR_MAX_BYTES        10
  /* HeUl */
#define MAP_MLCNO_BYTES            8
#define MAP_MSCNO_BYTES            8
#define MAP_MSISDN_LCS_BYTES       8
#define MAP_GEOGRAPHICAL_MAX_BYTES      13
#define MAX_MLC_DIGITS             16
#define MAX_MSC_DIGITS             16
#define MAX_MSISDN_LCS_DIGITS      16
#define MAP_VP_MIN                 0    /* SM validity period */
#define MAP_VP_MAX               255
#define MAX_ADDR_LEN              24
#define MAX_ORIG_REF_LEN          20
#define MIN_ADDR_LEN               4
#define MAX_MSISDN_LEN            20
#define MAX_GLOBAL_CELL_ID_LEN     7
#define MAX_IMSI_LEN               8
#define MAX_USSD_STR             140
#define MAX_SS_USERDATA_LEN		 200
#define MAP_TIMING_ADV_MAX       255

  /* End of decoding tags */
#define MAP_DECODE_END             1

/* Added by ulme */

#ifdef USSD_STRLEN_0
#define MIN_USSD_STR		   0
#else
#define MIN_USSD_STR		   1
#endif

/* Added by einjowa */
/*
 * Define little/big endian NT/SUN
 */
#define MAP_BIG_ENDIAN		0	/* AmigaOs SunOs AIX... ye*/
#define MAP_LITTLE_ENDIAN	1	/* NT SCO VAX/VMS...  */

#define ROWS			2
#define COLUMNS			2
#define LONG_INT_BYTES		4
#define SHORT_INT_BYTES		2
#define FormatLongReq(x)   	(*LongFuncReq_p[EmapApiUser][StackUser])(x)
#define FormatLongInd(x)   	(*LongFuncInd_p[EmapApiUser][StackUser])(x)
#define FormatShortReq(x)   	(*ShortFuncReq_p[EmapApiUser][StackUser])(x)
#define FormatShortInd(x)   	(*ShortFuncInd_p[EmapApiUser][StackUser])(x)



/* SMS-MAP-API function return values. */
#define MAP_PARAMETER_OK	   	0
#define MAP_INVALID_PARAMETER_VALUES	1
#define MAP_E_OK                   	1
#define MAP_E_GENERAL              	2
#define MAP_E_INVALID_PARAMETER    	3

  /* MapUAbortReq reason erors */
#define MAP_RESOURCE_LIMIT      0x00
#define MAP_RESOURCE_UNAVAIL    0x01
#define MAP_APPL_PROC_CANCEL    0x02
#define MAP_PROC_ERROR          0x03



  /* MapUAbortReq Diagnostic Info */
  /* if MapUAbortReq reason == Resource unavailable reason */
#define  MAP_SHORT_TERM_PROBLEM            0
#define  MAP_LONG_TERM_PROBLEM             1
/* MapUAbortReq Diagnostic Info */
/* if MapUAbortReq reason == Procedure Cancellation reason */
#define  MAP_HANDOVER_CANCELLATION         0
#define  MAP_RADIO_CHANNEL_RELEASE         1
#define  MAP_NETWORK_PATH_RELEASE          2
#define  MAP_CALL_RELEASE                  3
#define  MAP_ASSOCIATED_PROCEDURE_FAILURE  4
#define  MAP_TANDEM_DIALOGUE_RELEASED      5
#define  MAP_REMOTE_OPERATION_FAILURE      6


/* MAP User Error defines */
#define MAP_UE_NO_ERROR                    0
#define MAP_UE_UNKNOWN_SUBSCRIBER          1
#define MAP_UE_NO_REASON                   1
#define MAP_UE_INVALID_DESTREF             2
#define MAP_UE_INVALID_ORIGREF             3
#define MAP_UE_INVALID_SUBSCR              9
#define MAP_UE_INVALID_EQUIP              12
#define MAP_UE_CALL_BARRED                13
#define MAP_UE_ABSENT_SUBSCR              27
#define MAP_UE_SYS_FAILURE                34
#define MAP_UE_DATA_MISSING               35
#define MAP_UE_UNEXP_DATAVALUE            36
#define MAP_UE_UNREC_MESSTYPE             40
#define MAP_UE_UNREC_TRANSID              41
#define MAP_UE_BADLY_FORMATT_TRSCTN       42
#define MAP_UE_INCORR_TRANSACTPORT        43
#define MAP_UE_RESOURCE_LIMIT             44
#define MAP_UE_ABNORMAL_DIALOGUE          45
#define MAP_UE_INVALID_STATE              46
#define MAP_UE_UNKNOWN_ALPHA              71
#define MAP_UE_USSD_BUSY                  72
#define MAP_UE_OP_TIMEOUT                 80
#define MAP_UE_UNREC_COMP                100
#define MAP_UE_MISSP_COMP                101
#define MAP_UE_BADSTRUCT_COMP            102
#define MAP_UE_INV_DUP_INVOKEID          103
#define MAP_UE_INV_UNREC_OP              104
#define MAP_UE_INV_MISSP_PARAM           105
#define MAP_UE_INV_RESOURCE_LIMIT        106
#define MAP_UE_INV_INIT_REL              107
#define MAP_UE_INV_UNREC_LINKEDID        108
#define MAP_UE_INV_LINKRESP_UNEXP        109
#define MAP_UE_INV_UNEXP_LINKEDOP        110
#define MAP_UE_R_R_UNREC_INVOKEID        111
#define MAP_UE_R_R_RETRES_UNEXP          112
#define MAP_UE_R_R_MISSP_PARAM           113
#define MAP_UE_R_E_UNREC_INVOKEID        114
#define MAP_UE_R_E_RETERR_UNEXP          115
#define MAP_UE_R_E_RETERR_UNREC          116
#define MAP_UE_R_E_ERR_UNREC             117
#define MAP_UE_R_E_MISSP_PARAM           118
#define MAP_UE_POSITIONING_NOT_ALLOWED   251
#define MAP_UE_UNAUTHORIZED_REQUESTING_ENTITY 252
#define MAP_UE_POSITIONING_FAILURE       253       /* HeUl add error code 991213 */
#define MAP_UE_USSD_ODB_BARRED           255

  /* Map U/P Abort errors. See provideLCSConf errors */
#define MAP_UABORT_NO_REASON             47
#define MAP_UABORT_APP_CONTEXT_NOT_SUPP  48 
#define MAP_UABORT_VERSION_INCOMPATIBLE  49 
#define MAP_UABORT_INVALID_DESTREF       50
#define MAP_UABORT_INVALID_ORIGREF       51
#define MAP_UABORT_ABNORMAL_DIALOGUE     52
#define MAP_UABORT_INVALID_PDU           53
#define MAP_UABORT_USER_SPECIFIC_REASON  54
#define MAP_UABORT_USER_RESOURCE_LIM     55
#define MAP_UABORT_RESOURCE_UNAVAIL      56
#define MAP_UABORT_APPL_PROC_CANCEL      57
#define MAP_PABORT_ABNORMAL_DIALOGUE     58
#define MAP_PABORT_TRANSACTION_RELEASED  59
#define MAP_PABORT_VERSION_INCOMPATIBILITY     60
#define MAP_PABORT_RESOURCE_LIM          61


  /* Local SSN's */
#define MAP_ALL_OF_MAP                   05
#define MAP_HLR                          06
#define MAP_VLR                          07
#define MAP_MSC                          08
#define MAP_EIR                          09
#define MAP_AU                           10



 /* MAP Provider errors */

#define MAP_PROV_ERR_NOT_USED                0
#define MAP_DUPLICATED_INVOKE_ID             1
#define MAP_NOT_SUPPORTED_SERVICE            2
#define MAP_MISTYPED_PARAMETER               3
#define MAP_RESOURCE_LIMITATION              4
#define MAP_INITIATING_RELEASE               5
#define MAP_UNEXPECTED_RESPONSE_FROM_PEER    6
#define MAP_SERVICE_COMPLETION_FAILURE       7
#define MAP_NO_RESPONSE_FROM_PEER            8
#define MAP_INVALID_RESPONSE_RECEIVED        9
#define MAP_UNEXPECTED_TRANSACTION_ID        10
#define MAP_UNRECOGNIZED_COMPONENT           11
#define MAP_MISSTYPED_COMPONENT              12
#define MAP_BADSTRUCTURED_COMPONENT          13
#define MAP_UNRECOGNIZED_LINKED_ID           14
#define MAP_LINKED_RESPONSE_UNEXPECTED       15
#define MAP_UNEXPECTED_LINKED_OPERATION      16
#define MAP_DIALOGUE_TIMEOUT                 17
#define MAP_PROV_SYSTEM_FAILURE              18

  /* User Errors */


/***************************************************************************/
/*            SMS-MAP-API - Type and Structure Definitions.                */
/***************************************************************************/

#define MAP_SMI_T         ULONG_T       /* SM id (MAP_SM_ID_MIN - MAP_SM_ID_MAX) */
#define MAP_DCS_T         UCHAR_T       /* Data coding scheme (MAP_DCS_ANNEX2 - * MAP_DCS_MAX) */
#define MAP_MMS_T         BOOLEAN_T     /* More messages to send */
#define MAP_PRI_T         BOOLEAN_T     /* SM priority */
#define MAP_UDL_T         UCHAR_T       /* User data length (MAP_USER_DATA_MIN -
                                         * MAP_USER_DATA_MAX) */
#define MAP_VP_T          MAP_SCTS_T    /* SM validity period */
#define MAP_MWS_T         BOOLEAN_T     /* SM waiting set */
#define MAP_SUB_REF_T     ULONG_T       /* Submit Reference Id */



/* SME Address structure. */
typedef struct MAP_ADDRESS_TAG {
    UCHAR_T addressLength,   		/* Note: Number of digits in address
                                         * field see GSM 3.40 Sect 91.2.4 */
            typeOfAddress,
            address[MAP_ADDR_MAX_BYTES];
} MAP_ADDRESS_T;



 

  

  /* HeUl add for MSISDN LCS 1999-12-09 */
typedef struct MAP_MSISDN_LCS_TAG {
    UCHAR_T addressLength,   		
            typeOfAddress,
            address[MAP_MSISDN_LCS_BYTES];
} MAP_MSISDN_LCS_T;

typedef struct MAP_GEOGRAPHICAL_TAG {
  UCHAR_T geographicalInfoLength,                                  
          geographicalInfo[MAP_GEOGRAPHICAL_MAX_BYTES];
} MAP_GEOGRAPHICAL_T;



/* Address type of number. */

#define MAP_ADDR_TYPE_MASK         240    /* 1 (1 1 1) 0 0 0 0 */
#define MAP_UNKNOWN_ADDR_TYPE      128    /* 1 (0 0 0) 0 0 0 0 */
#define MAP_INTL_NUM_ADDR_TYPE     144    /* 1 (0 0 1) 0 0 0 0 */
#define MAP_NATL_NUM_ADDR_TYPE     160    /* 1 (0 1 0) 0 0 0 0 */
#define MAP_NW_SPEC_ADDR_TYPE      176    /* 1 (0 1 1) 0 0 0 0 */
#define MAP_SHORT_NUM_ADDR_TYPE    192    /* 1 (1 0 0) 0 0 0 0 */
#define MAP_ABBREVIATED_NUM_TYPE   224    /* 1 (1 1 0) 0 0 0 0 */ /* HeUl 1999-10-01 */
#define MAP_ALPA_NUM_ADDR_TYPE     208    /* 1 (1 0 1) 0 0 0 0 */

/* Address numbering plan ID's. */
#define MAP_NUM_PLAN_MASK          143    /* 1 0 0 0 (1 1 1 1) */
#define MAP_UNKNOWN_NUM_PLAN       128    /* 1 0 0 0 (0 0 0 0) */
#define MAP_ISDN_NUM_PLAN          129    /* 1 0 0 0 (0 0 0 1) */
#define MAP_DATA_NUM_PLAN          131    /* 1 0 0 0 (0 0 1 1) */
#define MAP_TELX_NUM_PLAN          132    /* 1 0 0 0 (0 1 0 0) */
#define MAP_MOB_NUM_PLAN           134    /* 1 0 0 0 (0 1 1 0) */ /* HeUl 1999-10-01 */
#define MAP_NATL_NUM_PLAN          136    /* 1 0 0 0 (1 0 0 0) */
#define MAP_PRIV_NUM_PLAN          137    /* 1 0 0 0 (1 0 0 1) */
#define MAP_RSVD_NUM_PLAN          143    /* 1 0 0 0 (1 1 1 1) */

/* Address element access masks. */
#define MAP_ADDR_HIGH_DIGIT_MASK   240    /* 1 1 1 1 0 0 0 0 */
#define MAP_ADDR_LOW_DIGIT_MASK     15    /* 0 0 0 0 1 1 1 1 */
/* High order nibble must be '1111' if odd number of digits in address.
 */
#define MAP_ADDR_FILL_DIGIT_MASK   240    /* 1 1 1 1 0 0 0 0 */

/* Parameter to identify operation */
typedef UCHAR_T MAP_INVOKE_ID_T;

/* Parameter to identify a unique dialogue */
typedef USHORT_T MAP_DIALOGUE_ID_T;

/* Parameter containing SubSystem Number */
typedef UCHAR_T MAP_LOCAL_SSN_T;

/* Parameter to indicate if further operations follows */
typedef BOOLEAN_T MAP_IND_OPFOLLOW_T;

typedef UCHAR_T  MAP_USER_REASON_T;
typedef UCHAR_T  MAP_DIAGNOSTIC_INFO_T;


  /* HeUl 991209 Parameter containing user errors reject errors, p and u abort errors */

typedef UCHAR_T REASON_T;

typedef struct MAP_ERROR_PARAM_TAG
{
    BOOLEAN_T reasonPresent;
    REASON_T  reason;
}MAP_ERROR_PARAM_T;    
   

typedef struct MAP_SUB_LCS_ERROR_TAG
{
     UCHAR_T errorCode;     
     MAP_ERROR_PARAM_T reason_s;
}MAP_SUB_LCS_ERROR_T;
          

typedef struct MAP_ROUTING_LCS_ERROR_TAG
{
     UCHAR_T errorCode;     
     MAP_ERROR_PARAM_T reason_s;
}MAP_ROUTING_LCS_ERROR_T;


  /* HeUl 991209 Parameter containing prov errors discovered in the  EMAP module */

typedef UCHAR_T MAP_PROVIDER_ERROR_T;

  /* User error location Resp */
typedef UCHAR_T MAP_PERFORM_LOCATION_ERROR_T;



/* Parameter for indication if a dialogue has been terminated */
typedef UCHAR_T MAP_DISC_IND_T;


/* Parameter to indicate a specific operation */
typedef UCHAR_T MAP_USSD_OPCODE_T;

/* parameter to identify ErrorCode */
typedef UCHAR_T MAP_ERROR_CODE_T;

/* typedef for struct containing SS7address and the length of the address */
typedef struct MAP_SS7_ADDR
{
  UCHAR_T ss7AddrLen;
  UCHAR_T ss7Addr[MAX_ADDR_LEN];
} MAP_SS7_ADDR_T;

/* typedef for a struct containing the senders address and length of address */
typedef struct MAP_ORIG_REF
{
  UCHAR_T origRefLen;
  UCHAR_T origRef[MAX_ADDR_LEN];
} MAP_ORIG_REF_T;

/* typedef for a struct containing the USSD message */
typedef struct MAP_USSD_MESS
{
  UCHAR_T ussdDcs;
  UCHAR_T ussdStrLen;
  UCHAR_T ussdStr[MAX_USSD_STR];
} MAP_USSD_MESS_T;

/* typedef for a struct containing info to identify a mobile subscriber */
typedef struct MAP_MSISDN
{
  UCHAR_T msisdnLen;
  UCHAR_T msisdn[MAX_MSISDN_LEN];
} MAP_MSISDN_T;


/* TR 16?? */
typedef struct MAP_VLRNO
{
  UCHAR_T vlrNoLen;
  UCHAR_T vlrNo[MAX_MSISDN_LEN];
} MAP_VLRNO_T;

typedef struct MAP_GLOBAL_CELL_ID_TAG
{
  UCHAR_T globalCellIdLength;
  UCHAR_T globalCellId[MAX_GLOBAL_CELL_ID_LEN];
} MAP_GLOBAL_CELL_ID_T;



/* typedef for a struct containing mobile subscribers number */
typedef struct MAP_IMSI
{
  UCHAR_T imsiLen;
  UCHAR_T imsi[MAX_IMSI_LEN];
} MAP_IMSI_T;


/* Protocol ID typedef. Bit 5 set i.e. telematic IW protocols. Bit patterns
 * are contiguous starting from '0 0 1 0 0 0 0 0' up to ' 0 0 1 1 1 1 1 1'.
 */
typedef enum {
    MAP_PRO_IMPLICIT       = 32,
    MAP_PRO_TELEX          = 33,
    MAP_PRO_TELEFAX_G3     = 34,
    MAP_PRO_TELEFAX_G4     = 35,
    MAP_PRO_VOICE          = 36,
    MAP_PRO_RESERVED_1     = 37,
    MAP_PRO_RESERVED_2     = 38,
    MAP_PRO_RESERVED_3     = 39, 
    MAP_PRO_TELETEX_UNSPEC = 40,
    MAP_PRO_TELETEX_PSPDN  = 41,
    MAP_PRO_TELETEX_CSPDN  = 42,
    MAP_PRO_ANALOG_PSTN    = 43,
    MAP_PRO_DIGITAL_ISDN   = 44,
    MAP_PRO_RESERVED_4     = 45,
    MAP_PRO_RESERVED_5     = 46,
    MAP_PRO_RESERVED_6     = 47,
    MAP_PRO_MHF_SC         = 48,
    MAP_PRO_X400_MHS       = 49,
    MAP_PRO_RESERVED_7     = 50,
    MAP_PRO_RESERVED_8     = 51,
    MAP_PRO_RESERVED_9     = 52,
    MAP_PRO_RESERVED_10    = 53,
    MAP_PRO_RESERVED_11    = 54,
    MAP_PRO_RESERVED_12    = 55,
    MAP_PRO_SC_SPECIFIC_1  = 56,
    MAP_PRO_SC_SPECIFIC_2  = 57,
    MAP_PRO_SC_SPECIFIC_3  = 58,
    MAP_PRO_SC_SPECIFIC_4  = 59,
    MAP_PRO_SC_SPECIFIC_5  = 60,
    MAP_PRO_SC_SPECIFIC_6  = 61,
    MAP_PRO_SC_SPECIFIC_7  = 62,
    MAP_PRO_MS_DEFAULT     = 63
} MAP_PID_T;

/* MAP_TIME_T typedef. */
typedef enum {
    MAP_TIME_ABS,
    MAP_TIME_REL,
    MAP_TIME_NOT_PRESENT
} MAP_TIME_T;

/* MAP_SUB_CONF_STATUS_T typedef. */
typedef enum {
    MAP_STAT_OK = 0,
    MAP_SYSTEM_FAIL = 34,
    MAP_UNEXPECTED_DATAVALUE = 36,
    MAP_SC_CONG = 53,
    MAP_MS_NOT_SC_SUBSC = 54,
    MAP_INVALID_SME_ADDR = 55,
    MAP_REJECT_GENERAL_P = 0x80,
    MAP_REJECT_INVOKE_P = 0x81, 
    MAP_REJECT_RR_P = 0x82,
    MAP_REJECT_RE_P = 0x83,
    MAP_REMOTE_REJECT_P = 0x85
} MAP_SUB_CONF_STATUS_T;

/* MAP_SUB_STATUS_T typedef. */
typedef enum {
    MAP_STATUS_OK = 0,
    MAP_SYS_FAILURE = 34,
    MAP_UNEXPECTED_DATA_VAL = 36,
    MAP_SC_CONGESTION = 53,
    MAP_MS_NOT_SC_SUBSCRIBER = 54,
    MAP_INVALID_SME_ADDRESS = 55
} MAP_SUB_STATUS_T;

/* MAP_BIND_STATUS_T typedef. */

/* HeUl TR1836 99-10-25 */
typedef enum {      
    MAP_BIND_OK = 0,
    MAP_BIND_SSN_ALREADY_IN_USE,
    MAP_BIND_PROTOCOL_ERROR,
    MAP_BIND_NO_RESOURCES,
    MAP_BIND_INVALID_SSN,
    MAP_BIND_SCCP_NOT_READY,
    MAP_BIND_USER_ID_ALREADY_IN_USE,
    MAP_BIND_UNDEFINED_USERID,
    MAP_BIND_NOT_DEFINED_STATUS
    
} MAP_BIND_STATUS_T;



/* MAP_MSG_STAT_T typedef. */
typedef enum {
    MAP_MSG_STATUS_OK = 0,
    MAP_UNKNOWN_SUBSCRIBER = 1,
    MAP_TELE_SRV_NOT_PROVISIONED = 11,
    MAP_CALL_BARRED = 13,
    MAP_FACILITY_NOT_SUPPORTED = 21,
    MAP_ABSENT_SUBSCRIBER = 27,
    MAP_SYSTEM_FAILURE = 34,
    MAP_UNEXPECTED_DATA_VALUE = 36,
    MAP_MEMORY_CAPACITY_EXCEEDED = 52,
    MAP_ERROR_IN_MS = 50,
    MAP_SMS_LL_CAPS_NOT_PROVISIONED = 51,
    MAP_ILLEGAL_MS = 9,
    MAP_REJECT_GP = 0x80,
    MAP_REJECT_INVOKE = 0x81, 
    MAP_REJECT_RR = 0x82,
    MAP_REJECT_RE = 0x83,
    MAP_REMOTE_REJECT = 0x85
} MAP_MSG_STAT_T;

/* MAP_SCTS_T typedef.
 * For discussion of SC time stamp see GMS 03.40, version 3.4.0, section 9.2.6.9
 * (page 43).
 */
typedef struct MAP_SCTS_TAG {
    UCHAR_T year,
            month,
            day,
            hour,
            minute,
            second;
    UCHAR_T timeZone;
} MAP_SCTS_T;

/* MAP_UD_T typedef. */
typedef struct MAP_UD_TAG {
    CHAR_T data[MAP_USER_DATA_MAX];
} MAP_UD_T;

typedef struct MAP_SS_USERDATA_TAG
{
  UCHAR_T ssUserDataLen;
  UCHAR_T ssUserData[MAX_SS_USERDATA_LEN];
} MAP_SS_USERDATA_T;


/* MAP_SERVING_CELL typedef */
typedef struct MAP_SERVING_CELL_TAG {
  UCHAR_T mobileCountryCode[2];
  UCHAR_T mobileNetworkCode;
  UCHAR_T locationAreaCode[2];
  UCHAR_T cellIdentity[2];
} MAP_SERVING_CELL_T;

/* MAP_TIMING_ADVANCE typedef */
typedef struct MAP_TIMING_ADVANCE_TAG {
  BOOLEAN_T used;
  UCHAR_T value;
} MAP_TIMING_ADVANCE_T;

/* MAP_TIME_STAMP typedef */
typedef struct MAP_TIME_STAMP_TAG {
  BOOLEAN_T used;
  UCHAR_T month;
  UCHAR_T day;
  UCHAR_T hour;
  UCHAR_T minute;
} MAP_TIME_STAMP_T;



/***************************************************************************/
/*                   SMS-MAP-API - Primary Functions                       */
/***************************************************************************/

/* Request/Confirmation function prototype definitions. These request functions
 * which are implemented as part of the SMS-MAP-API and are called by the
 * SMS-MAP User Application.
 */
#ifndef __STDC__

    USHORT_T MapDeliverSMReq();
    USHORT_T MapSubmitSMConf();
    USHORT_T MapBeginReq();
    USHORT_T MapEndReq();
    USHORT_T MapErrorReq();
    USHORT_T MapUssdNotifyReq();
    USHORT_T MapUssdReqReq();
    USHORT_T MapProcUssdDataResp();
    USHORT_T MapSubmitReq();
    USHORT_T MapProvideSubscriberLCSReq();
    USHORT_T MapSendRoutingInfoForLCSReq();
    USHORT_T MapPerformLocationLCSResp();
    USHORT_T MapUAbortReq();
    USHORT_T MapForwardMTPositioningReq();
    
#else /* use ANSI C function prototype definition standard */

    USHORT_T MapDeliverSMReq(MAP_SMI_T,
                             MAP_ADDRESS_T *,
                             MAP_ADDRESS_T *,
                             MAP_PID_T,
                             MAP_DCS_T,
                             MAP_MMS_T,
                             MAP_SCTS_T *,
                             MAP_PRI_T,
                             MAP_UDL_T,
                             MAP_UD_T *);

    USHORT_T MapSubmitSMConf(MAP_SUB_REF_T,
                             MAP_SUB_STATUS_T);

    USHORT_T MapBeginReq(MAP_LOCAL_SSN_T,
			 MAP_DIALOGUE_ID_T,
			 MAP_INVOKE_ID_T,
			 MAP_IMSI_T *,
                         MAP_VLRNO_T *,
			 MAP_MSISDN_T *,
			 MAP_SS7_ADDR_T *,
			 MAP_USSD_OPCODE_T,
			 MAP_USSD_MESS_T *);

    USHORT_T MapEndReq(MAP_LOCAL_SSN_T,
		       MAP_DIALOGUE_ID_T,
		       MAP_IND_OPFOLLOW_T,
		       MAP_INVOKE_ID_T,
		       MAP_USSD_MESS_T *);

    USHORT_T MapErrorReq(MAP_LOCAL_SSN_T,
			 MAP_DIALOGUE_ID_T,
			 MAP_INVOKE_ID_T,
			 MAP_ERROR_CODE_T,
			 MAP_DISC_IND_T);

    USHORT_T MapUssdNotifyReq(MAP_LOCAL_SSN_T,
			      MAP_DIALOGUE_ID_T,
			      MAP_INVOKE_ID_T,
			      MAP_USSD_MESS_T *);

    USHORT_T MapUssdReqReq(MAP_LOCAL_SSN_T,
			   MAP_DIALOGUE_ID_T,
			   MAP_INVOKE_ID_T,
			   MAP_USSD_MESS_T *);

    USHORT_T MapProcUssdDataResp(MAP_LOCAL_SSN_T,
			         MAP_DIALOGUE_ID_T,
			         MAP_INVOKE_ID_T,
			         MAP_SS_USERDATA_T *,
			         MAP_MSG_STAT_T);

    USHORT_T MapSubmitReq(MAP_SMI_T,
			  MAP_ADDRESS_T *,
			  MAP_ADDRESS_T *, 
			  MAP_PID_T,
			  MAP_DCS_T,
			  MAP_TIME_T,
			  MAP_SCTS_T *,
			  MAP_UDL_T,
			  MAP_UD_T *);

     USHORT_T MapProvideSubscriberLCSReq(MAP_LOCAL_SSN_T,
		     MAP_DIALOGUE_ID_T,
		     MAP_INVOKE_ID_T ,
                     MAP_SS7_ADDR_T *,
		     MAP_IMSI_T *,
                     MAP_MSISDN_LCS_T *,
                     BOOLEAN_T privacyOverride);
      USHORT_T MapSendRoutingInfoForLCSReq(MAP_LOCAL_SSN_T,
		     MAP_DIALOGUE_ID_T,
		     MAP_INVOKE_ID_T ,
                     MAP_SS7_ADDR_T *,
		     MAP_MSISDN_LCS_T *,
                     MAP_MSISDN_LCS_T *);

   USHORT_T MapPerformLocationLCSResp(MAP_LOCAL_SSN_T,
		     MAP_DIALOGUE_ID_T,
		     MAP_INVOKE_ID_T invokeId,
		     MAP_GEOGRAPHICAL_T *,
		     MAP_PERFORM_LOCATION_ERROR_T);
   
  USHORT_T MapUAbortReq(MAP_LOCAL_SSN_T localSsn,
		     MAP_DIALOGUE_ID_T dialogueId,
		     MAP_USER_REASON_T ,
                     MAP_DIAGNOSTIC_INFO_T );



USHORT_T MapForwardMTPositioningReq(MAP_SMI_T msgIdentifier,
                                    MAP_SS7_ADDR_T *,
				    MAP_ADDRESS_T *,
				    MAP_ADDRESS_T *,
				    MAP_PID_T,
				    MAP_DCS_T,
				    MAP_MMS_T,
				    MAP_SCTS_T *,
				    MAP_PRI_T,
				    MAP_UDL_T,
				    MAP_UD_T * );



#endif /* __STDC__ */

/* Indication function prototype definitions. These indication functions are
 * only defined as part of the SMS-MAP-API and are implemented by the SMS-MAP
 * User Application. These functions are called by the SMS-MAP.
 */
#ifndef __STDC__

#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
    typedef USHORT_T (*MAPSUBMITSMIND_T)();
    typedef USHORT_T (*MAPREPORTSMIND_T)();
    typedef USHORT_T (*MAPALERTIND_T)();
    typedef USHORT_T (*MAPBEGININD_T)();
    typedef USHORT_T (*MAPERRORIND_T)();
    typedef USHORT_T (*MAPUSSDREQCONF_T)();
    typedef USHORT_T (*MAPUSSDNOTIFYCONF_T)();
    typedef USHORT_T (*MAPPROCUSSDDATAIND_T)();
    typedef USHORT_T (*MAPSUBMITRESCONF_T)();
    typedef USHORT_T (*MAPFORWARDPOSCONF_T)();
    typedef USHORT_T (*MAPPROVSUBSCRIBERLCSCONF_T)();
    typedef USHORT_T (*MAPSENDROUTINGINFOFORLCSCONF_T)();
    typedef USHORT_T (*MAPPERFORMLOCATIONLCSIND_T)();    
#else
    extern USHORT_T MapSubmitSMInd();
    extern USHORT_T MapReportSMInd();
    extern USHORT_T MapAlertInd();
    extern USHORT_T MapBeginInd();
    extern USHORT_T MapErrorInd();
    extern USHORT_T MapUssdReqConf();
    extern USHORT_T MapUssdNotifyConf();
    extern USHORT_T MapProcUssdDataInd();
    extern USHORT_T MapSubmitConf();
    extern USHORT_T MapForwardMTPositioningConf();
    extern USHORT_T MapProvideSubscriberLCSConf();
    extern USHORT_T MapSendRoutingInfoForLCSConf();
    extern USHORT_T MapPerformLocationLCSInd();
#endif /* EINSS7_FUNC_POINTER */

#else /* use ANSI C function prototype definition standard */

 #ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
    typedef USHORT_T (*MAPSUBMITSMIND_T)(MAP_SUB_REF_T,
                                         MAP_ADDRESS_T *,
					 MAP_ADDRESS_T *,
					 MAP_PID_T,
					 MAP_DCS_T,
					 MAP_TIME_T,
					 MAP_VP_T,
					 MAP_UDL_T,
					 MAP_UD_T *);

    typedef USHORT_T (*MAPREPORTSMIND_T)(MAP_SMI_T,
					 MAP_MSG_STAT_T,
					 MAP_MWS_T);

    typedef USHORT_T (*MAPALERTIND_T)(MAP_ADDRESS_T *);

    typedef USHORT_T (*MAPBEGININD_T)(MAP_LOCAL_SSN_T,
		                      MAP_DIALOGUE_ID_T,
				      MAP_INVOKE_ID_T,
				      MAP_IMSI_T *,
                                      MAP_VLRNO_T *,
				      MAP_ORIG_REF_T *,
				      MAP_MSISDN_T *,
				      MAP_SS7_ADDR_T *,
				      MAP_USSD_MESS_T *);

    typedef USHORT_T (*MAPERRORIND_T)(MAP_LOCAL_SSN_T,
				      MAP_DIALOGUE_ID_T,
				      MAP_INVOKE_ID_T,
				      MAP_ERROR_CODE_T,
				      MAP_DISC_IND_T);


    typedef USHORT_T (*MAPUSSDREQCONF_T)(MAP_LOCAL_SSN_T,
				         MAP_DIALOGUE_ID_T,
				         MAP_INVOKE_ID_T,
				         MAP_USSD_MESS_T *);

    typedef USHORT_T (*MAPUSSDNOTIFYCONF_T)(MAP_LOCAL_SSN_T,
				            MAP_DIALOGUE_ID_T,
				            MAP_INVOKE_ID_T);

    typedef USHORT_T (*MAPPROCUSSDDATAIND_T)(MAP_LOCAL_SSN_T,
					     MAP_DIALOGUE_ID_T,
				             MAP_INVOKE_ID_T,
					     MAP_SS7_ADDR_T *,
					     MAP_ADDRESS_T *,
					     MAP_ADDRESS_T *,
					     MAP_IMSI_T *,
					     MAP_ADDRESS_T *,
					     MAP_SS_USERDATA_T *);

    typedef USHORT_T (*MAPSUBMITRESCONF_T)(MAP_SMI_T,
                                        MAP_SUB_CONF_STATUS_T,
                                        CHAR_T,
                                        CHAR_T *);

   typedef USHORT_T (*MAPFORWARDPOSCONF_T)(MAP_SMI_T,
                                                MAP_SERVING_CELL_T,
                                                MAP_TIMING_ADVANCE_T,
                                                MAP_TIME_STAMP_T,
                                                CHAR_T);
   

  typedef USHORT_T (*MAPPROVSUBSCRIBERLCSCONF_T)(MAP_LOCAL_SSN_T,
				         MAP_DIALOGUE_ID_T,
				         MAP_INVOKE_ID_T,
                                         MAP_GEOGRAPHICAL_T *,
                                         MAP_SUB_LCS_ERROR_T *,
                                         MAP_PROVIDER_ERROR_T);

  typedef USHORT_T (*MAPSENDROUTINGINFOFORLCSCONF_T)(MAP_LOCAL_SSN_T,
				         MAP_DIALOGUE_ID_T,
				         MAP_INVOKE_ID_T,
                                         MAP_IMSI_T *,
                                         MAP_MSISDN_LCS_T *,
                                         MAP_ROUTING_LCS_ERROR_T*,
                                         MAP_PROVIDER_ERROR_T);

  typedef USHORT_T (*MAPPERFORMLOCATIONLCSIND_T)(MAP_LOCAL_SSN_T,
				  MAP_DIALOGUE_ID_T,
				  MAP_INVOKE_ID_T,
				  MAP_SS7_ADDR_T*,
                                  MAP_GLOBAL_CELL_ID_T*,
                                  MAP_TIMING_ADVANCE_T *);
                


#else

    extern USHORT_T MapSubmitSMInd(MAP_SUB_REF_T,
                                   MAP_ADDRESS_T *,
                                   MAP_ADDRESS_T *,
                                   MAP_PID_T,
                                   MAP_DCS_T,
                                   MAP_TIME_T,
                                   MAP_VP_T,
                                   MAP_UDL_T,
                                   MAP_UD_T *);

    extern USHORT_T MapReportSMInd(MAP_SMI_T,
                                   MAP_MSG_STAT_T,
                                   MAP_MWS_T);

    extern USHORT_T MapAlertInd(MAP_ADDRESS_T *);

    extern USHORT_T MapBeginInd(MAP_LOCAL_SSN_T,
				MAP_DIALOGUE_ID_T,
				MAP_INVOKE_ID_T,
				MAP_IMSI_T *,
                                MAP_VLRNO_T *,
				MAP_ORIG_REF_T *,
				MAP_MSISDN_T *,
				MAP_SS7_ADDR_T *,
				MAP_USSD_MESS_T *);

    extern USHORT_T MapErrorInd(MAP_LOCAL_SSN_T,
				MAP_DIALOGUE_ID_T,
				MAP_INVOKE_ID_T,
				MAP_ERROR_CODE_T,
				MAP_DISC_IND_T);


    extern USHORT_T MapUssdReqConf(MAP_LOCAL_SSN_T,
				   MAP_DIALOGUE_ID_T,
				   MAP_INVOKE_ID_T,
				   MAP_USSD_MESS_T *);

    extern USHORT_T MapUssdNotifyConf(MAP_LOCAL_SSN_T,
				      MAP_DIALOGUE_ID_T,
				      MAP_INVOKE_ID_T);

    extern USHORT_T MapProcUssdDataInd(MAP_LOCAL_SSN_T,
				      MAP_DIALOGUE_ID_T,
				      MAP_INVOKE_ID_T,
				      MAP_SS7_ADDR_T *,
				      MAP_ADDRESS_T *,
				      MAP_ADDRESS_T *,
				      MAP_IMSI_T *,
				      MAP_ADDRESS_T *,
				      MAP_SS_USERDATA_T *);

    extern USHORT_T MapSubmitConf(MAP_SMI_T,
                                  MAP_SUB_CONF_STATUS_T,
                                  CHAR_T,
                                  CHAR_T *);


    extern USHORT_T MapForwardMTPositioningConf(MAP_SMI_T,
                                                MAP_SERVING_CELL_T,
                                                MAP_TIMING_ADVANCE_T,
                                                MAP_TIME_STAMP_T,
                                                CHAR_T);

   
    extern USHORT_T MapProvideSubscriberLCSConf(MAP_LOCAL_SSN_T,
				         MAP_DIALOGUE_ID_T,
				         MAP_INVOKE_ID_T,
                                         MAP_GEOGRAPHICAL_T *,
                                         MAP_SUB_LCS_ERROR_T *,
                                         MAP_PROVIDER_ERROR_T);

    extern USHORT_T MapSendRoutingInfoForLCSConf(MAP_LOCAL_SSN_T,
				         MAP_DIALOGUE_ID_T,
				         MAP_INVOKE_ID_T,
                                         MAP_IMSI_T *,
                                         MAP_MSISDN_LCS_T *,
                                         MAP_ROUTING_LCS_ERROR_T*,
                                         MAP_PROVIDER_ERROR_T);



    extern USHORT_T MapPerformLocationLCSInd(MAP_LOCAL_SSN_T,
				  MAP_DIALOGUE_ID_T,
				  MAP_INVOKE_ID_T,
				  MAP_SS7_ADDR_T*,
                                  MAP_GLOBAL_CELL_ID_T*,
                                  MAP_TIMING_ADVANCE_T *);



#endif /* EINSS7_FUNC_POINTER */

#endif /* __STDC__ */


/***************************************************************************/
/*                  SMS-MAP-API - Secondary Functions                      */
/***************************************************************************/

/* Request function prototype definitions. These request functions are
 * implemented as part of the SMS-MAP-API and are called by the SMS-MAP
 * User Application.
 */
#ifndef __STDC__

    USHORT_T MapBindReq();
    USHORT_T MapUnBindReq();

#else /* use ANSI C function prototype definition standard */

    USHORT_T MapBindReq(MAP_LOCAL_SSN_T, USHORT_T);

    USHORT_T MapUnbindReq(MAP_LOCAL_SSN_T);

#endif /* __STDC__ */

/* Confirmation function prototype definitions. These confirmation functions are
 * only defined as part of the SMS-MAP-API and are implemented by the SMS-MAP
 * User Application. These functions are called by the SMS-MAP.
 */
#ifndef __STDC__
#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
    typedef USHORT_T (*MAPBINDCONF_T)();
#else
	extern USHORT_T MapBindConf();
#endif /* EINSS7_FUNC_POINTER */

#else /* use ANSI C function prototype definition standard */

#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
    typedef USHORT_T (*MAPBINDCONF_T)(MAP_LOCAL_SSN_T,
				MAP_BIND_STATUS_T);
#else
	extern USHORT_T MapBindConf(MAP_LOCAL_SSN_T,
				MAP_BIND_STATUS_T);
#endif /* EINSS7_FUNC_POINTER */
#endif /* __STDC__ */


/***************************************************************************/
/*                    SMS-MAP-API - Service Functions                      */
/***************************************************************************/

#ifndef __STDC__

    USHORT_T MapHandleMsg();
    USHORT_T MapHandleIndication();

#else /* use ANSI C function prototype definition standard */

    USHORT_T MapHandleMsg(MSG_T *);
    USHORT_T MapHandleIndication(MSG_T *);

#endif /* __STDC__ */

/* Indication function prototype definitions. These indication functions are
 * only defined as part of the SMS-MAP-API and are implemented by the SMS-MAP
 * User Application. These functions are called by the SMS-MAP.
 */
#ifndef __STDC__
#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
    typedef USHORT_T (*MAPINDICATIONERROR_T)();
#else
	extern USHORT_T MapIndicationError();
#endif /* EINSS7_FUNC_POINTER */

#else /* use ANSI C function prototype definition standard */
#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
    typedef USHORT_T (*MAPINDICATIONERROR_T)(USHORT_T, CHAR_T *);
#else
	extern USHORT_T MapIndicationError(USHORT_T, CHAR_T *);
#endif /* EINSS7_FUNC_POINTER */
#endif /* __STDC__ */


#ifdef EINSS7_FUNC_POINTER
  typedef struct MAPINIT
  {
	MAPSUBMITSMIND_T      MapSubmitSMInd;			/* Pointer to a function */
        MAPREPORTSMIND_T      MapReportSMInd;
	MAPALERTIND_T	      MapAlertInd;
	MAPBEGININD_T	      MapBeginInd;
	MAPERRORIND_T	      MapErrorInd;
	MAPUSSDREQCONF_T      MapUssdReqConf;
	MAPUSSDNOTIFYCONF_T   MapUssdNotifyConf;
	MAPBINDCONF_T	      MapBindConf;
	MAPINDICATIONERROR_T  MapIndicationError;
	MAPPROCUSSDDATAIND_T  MapProcUssdDataInd;
	MAPSUBMITRESCONF_T    MapSubmitConf;
        MAPFORWARDPOSCONF_T   MapForwardMTPositioningConf;
        MAPPROVSUBSCRIBERLCSCONF_T MapProvideSubscriberLCSConf;
        MAPSENDROUTINGINFOFORLCSCONF_T MapSendRoutingInfoForLCSConf;
        MAPPERFORMLOCATIONLCSIND_T MapPerformLocationLCSInd;
  }MAPINIT_T;
/* The application must call this function to register the call back funtions. */ 
	USHORT_T MapRegFunc(MAPINIT_T *);
#endif /* EINSS7_FUNC_POINTER */

/***************************************************************************/

/* mapApiId - this variable contains the UserId of the layer that is using the 
 *         api to communicate with the map layer. The appropiate value from the 
 *         portss7.h file should be placed in this variable before any attempt 
 *         to use the api functionality is made. 
 */

extern USHORT_T mapApiId;

/* 
 * numberApiMsg hold the value of the number of messages that the api may 
 * have in its internal queue, this value is originally set to 10 but may 
 * be changed prior to calling the MapBindReq() function. 
 */
extern USHORT_T  mapApiNumberMsg;

/* used during the XmemInit */
extern USHORT_T mapApiBlockSize;
extern USHORT_T mapApiPoolSize;
extern USHORT_T mapApiAddSize;

/* used for TimeInit */
extern USHORT_T mapApiMaxTimers;

/* Added by einjowa */
/*
 * This function determines which platforms the EMAP_API and the SS#7 stack is
 * running on. The following combinations are relevant: (EMAP_api - SS#7_stack)
 * 0-0: SUN-SUN
 * 0-1: SUN-NT
 * 1-0: NT -SUN
 * 1-1: NT -NT   
 */			
USHORT_T MapSetEndian (UCHAR_T EmapApi, UCHAR_T Stack);


/* Added by einjowa */
USHORT_T DoNothingShort(USHORT_T value);
ULONG_T DoNothingLong(ULONG_T value);


/*
 * This function switches the bytes to make it possible to send Requests
 * NT -> SUN. long-int
 */
ULONG_T MakeLittleToBigLongInt (ULONG_T value);


/*
 * This function switches the bytes to make it possible to send Indications
 * NT <- SUN. long-int
 */
ULONG_T MakeBigToLittleLongInt (ULONG_T value);



/*
 * This function switches the bytes to make it possible to send Requests
 * NT -> SUN. short-int
 */
USHORT_T MakeLittleToBigShortInt (USHORT_T value);


/*
 * This function switches the bytes to make it possible to send Indications
 * NT <- SUN. short-int
 */
USHORT_T MakeBigToLittleShortInt (USHORT_T value);

#if defined __cplusplus || defined c_plusplus
}
#endif
#endif /* __SMS_MAP_API_H__ */
