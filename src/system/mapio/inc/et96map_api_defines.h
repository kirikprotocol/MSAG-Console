/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_api_defines.h,v                                                         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_api_defines.h                                             */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech  AB 1998	                 	     */
/*								     */
/* All rights reserved. The copyright of the computer program herein */
/* is the property of Ericsson Infocom Consultants AB, Sweden. The   */
/* program may be used and/or copied only with the permission of     */
/* Ericsson Infocom Consultants AB or in accordance with the 	     */
/* terms and conditions stipulated in the agreement/contract under   */
/* which the program has been supplied.				     */
/*								     */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                                                  */
/* 2/190 55-CAA 201 45 Ux                                            */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.22 2000/04/27 09:24:22 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Lambkin                                                      */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* <the purpose of this module, references to standard documents>    */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 971105	Ulf Melin Added: #define MAP_E_OK    1               */
/* 			         #define MAP_E_GENERAL 2             */
/*				 #define MAP_E_INVALID_PARAMETER    3*/
/*                                                                   */
/* Revision record:                                                  */
/* 980406	Ulf Hellsten                                         */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/


#ifndef __ET96MAP_API_DEFINES_H__
#define __ET96MAP_API_DEFINES_H__




/*********************************************************************/
/* Below are the fundamental datatypes used by the MAP API and the   */
/* rest of map.                                                      */
/*********************************************************************/

/* TimerDefines */

#define ET96MAP_SMALL_TIMER 13
#define ET96MAP_MEDIUM_TIMER 14
#define ET96MAP_MEDIUM_LONG_TIMER 15
#define ET96MAP_BIND_TIMER 16
#define ET96MAP_GUARD_TIMER 17



/* for test of AC in Mapmodule */
#define ET96MAP_AC_OK              0
#define ET96MAP_AC_VERSION_1      1
#define ET96MAP_VERSION_TO_HIGH   2
#define ET96MAP_AC_NOT_SUPPORTED      3
#define ET96MAP_INCORRECT_AC_LEN      4
#define ET96MAP_INCORRECT_AC          5


/* local SSN values */

#define ET96MAP_HLR         0x06
#define ET96MAP_VLR         0x07
#define ET96MAP_MSC         0x08
#define ET96MAP_EIR         0x09
#define ET96MAP_AUC         0x0A

/* HeUl 300 -> 3500 */
#define ET96MAP_ADDRESS_ALREADY_IN_FILE  0x01
#define ET96MAP_ADDRESS_WAS_NOT_IN_FILE  0x00

#define ET96MAP_MAX_PK_BUF_SIZE 3500
#define ET96MAP_MAX_REFUSE_REASON 6
#define ET96MAP_MAX_LCS_APN_LEN   64

#define ET96MAP_MAX_RP_SMEA_ADDRESS_LENGTH 10


/* the highest value for the priority parameter to/from TCAP */
#define ET96MAP_HIGHEST_PRIORITY 3

/* Address Length ranges */

#ifdef  EINSS7_ANSI_SCCP
#define ET96MAP_MAX_USER_PH2_DATA 200
#else
#define ET96MAP_MAX_USER_PH2_DATA 3000
#endif
#define ET96MAP_MAX_RETURNED_OPERATION   200
#define ET96MAP_MAX_USER_DATA 200
#define ET96MAP_MAX_TMSI_LEN 4
#define ET96MAP_MAX_LMSI_LEN 4
#define ET96MAP_MAX_NAME_STRING_LEN 63
#define ET96MAP_MAX_IMEI_LEN  8
#define ET96MAP_MAX_GEOGRAPHICAL_INFO_LEN 20
#define ET96MAP_MAX_ADD_GEOGRAPHICAL_INFO_LEN 90

/* Msisdn types and numbering plan*/
#define ET96MAP_UNKNOWN_ADDR_TYPE   0x80
#define ET96MAP_INTL_NUM_ADDR_TYPE  0x90
#define ET96MAP_NATL_NUM_ADDR_TYPE  0xA0
#define ET96MAP_NW_SPEC_ADDR_TYPE   0xB0
#define ET96MAP_SHORT_NUM_ADDR_TYPE 0xC0
#define ET96MAP_UNKNOWN_NUM_PLAN    0x80
#define ET96MAP_ISDN_NUM_PLAN       0x81
#define ET96MAP_DATA_NUM_PLAN       0x83
#define ET96MAP_TELX_NUM_PLAN       0x84
#define ET96MAP_NATL_NUM_PLAN       0x88
#define ET96MAP_PRIV_NUM_PLAN       0x89
#define ET96MAP_RSVD_NUM_PLAN       0x8F 

#define ET96MAP_MAX_SIGNAL_INFO_LEN 200

/* AePONA */
#define ET96MAP_MAX_SS_USER_DATA_LEN 200
#define ET96MAP_MAX_USSD_STR_LEN 160
#define ET96MAP_MAX_ORIGINATING_ENTITY_NUM_LEN 18
/* AePONA */

/* AePONA */
#define ET96MAP_USER_AVAILABLE_CONGEST_LEVEL_1	0x02
#define ET96MAP_USER_AVAILABLE_CONGEST_LEVEL_2	0x03  
#define ET96MAP_USER_AVAILABLE_CONGEST_LEVEL_3	0x04
/* AePONA */


/* AePONA */
#define ET96MAP_UE_USSD_BUSY                  72
/* AePONA */

#define ET96MAP_MAX_USSD_STR 160

#define ETSIMAX_USSD_PH2_STR 3000
#define ET96MAP_MAX_MSISDN_LEN  18
#define ET96MAP_ADDRESS_LEN  19
#define ET96MAP_MAX_SCA_LEN      38
#define ET96MAP_DCS_MIN 0
#define ET96MAP_DCS_MAX 255
#define ET96MAP_MAX_REPORT_PARAM 220
 
#define ET96MAP_MIN_USER_ID 0x01
#define ET96MAP_MAX_USER_ID 0xFF

#define ET96MAP_MIN_SSN 0x02
#define ET96MAP_MAX_SSN 0xFF

#define ET96MAP_MIN_SUBMIT_REF 0x1
#define ET96MAP_MAX_SUBMIT_REF 0xFFFFFFFF

#define ET96MAP_STATUS_OK                       0x00

#define ET96MAP_INVAL_SME_ADDRESS                0x37

#define ET96MAP_MIN_IMSI_LEN                        0x03
#define ET96MAP_MAX_IMSI_LEN                        0x08

#define ET96MAP_AC_LEN                          0x07
#define ET96MAP_MAX_OP_ERR_CODE_LEN             0x01
#define ET96MAP_MAX_ABORT_INFO_LEN              0x01

#define ET96MAP_MIN_ADDR_LEN                        0x04
#define ET96MAP_MAX_ADDR_LEN                        0x18
/* HeUl add define */
#define ET96MAP_CUG_CODE_LEN                    0x04 
#define ET96MAP_MIN_INVOKES                         0x00
#define ET96MAP_MAX_INVOKES                         0xFF

#define ET96MAP_DIALOGUE_NOT_TERMINATED         0x00
#define ET96MAP_DIALOGUE_TERMINATED             0x01

#define ET96MAP_MIN_DIALOGUES                       0x00
#define ET96MAP_MAX_DIALOGUES                       0xFFFF

#define ET96MAP_MIN_MSG_IDENT                       0x01
#define ET96MAP_MAX_MSG_IDENT                       0xFFFFFFFF

#define ET96MAP_USER_AVAILABLE                      0x00
#define ET96MAP_USER_UNAVAILABLE                    0x01
#define ET96MAP_MAX_SM_RP_SMEA_ADDRESS_LENGTH  10

/* Map User errors */

#define ET96MAP_UE_NO_ERROR                    0
#define ET96MAP_UE_UNKNOWN_SUBSCRIBER          1
#define ET96MAP_UE_INVALID_DESTREF             2
#define ET96MAP_UE_INVALID_ORIGREF             3
#define ET96MAP_UE_UNID_SUBSCR                 5 /* anwa TR1351 */
#define ET96MAP_UE_ABSENT_SUBSCR_SM            6 /* New for version 3 */
#define ET96MAP_UE_INVALID_SUBSCR              9
#define ET96MAP_UE_TELES_NOT_PROVISIONED      11 /* anwa TR1351 */
#define ET96MAP_UE_INVALID_EQUIP              12
#define ET96MAP_UE_CALL_BARRED                13
#define ET96MAP_UE_CUG_REJECT                 15 
#define ET96MAP_UE_FACIL_NOT_SUPP             21 /* anwa TR1351 */
#define ET96MAP_UE_ABSENT_SUBSCR              27
#define ET96MAP_UE_SUBSCR_BUSY_FOR_MT_SMS     31 /* anwa TR1351 */
#define ET96MAP_UE_SM_DELIV_FAILURE           32 /* anwa TR1351 */
#define ET96MAP_UE_MSG_WAITING_LIST_FULL      33 /* anwa TR1351 */
#define ET96MAP_UE_SYS_FAILURE                34
#define ET96MAP_UE_DATA_MISSING               35
#define ET96MAP_UE_UNEXP_DATAVALUE            36
#define ET96MAP_UE_UNREC_MESSTYPE             40
#define ET96MAP_UE_UNREC_TRANSID              41
#define ET96MAP_UE_BADLY_FORMATT_TRSCTN       42
#define ET96MAP_UE_INCORR_TRANSACTPORT        43
#define ET96MAP_UE_ABNORMAL_DIALOGUE          45
#define ET96MAP_UE_INVALID_STATE              46
#define ET96MAP_UE_UNAUTHORIZED_REQUESTING_NETWORK_FAILURE 52
#define ET96MAP_UE_UNAUTHORIZED_LCS_DIAGNOSTIC_FAILURE 53
#define ET96MAP_UE_POS_METHOD_FAILURE 54
#define ET96MAP_UE_UNKNOWN_OR_UNREACHABLE_LCS_CLIENT 58
#define ET96MAP_UE_UNKNOWN_ALPHA              71
#define ET96MAP_UE_USSD_BUSY                  72
#define ET96MAP_UE_OP_TIMEOUT                 80
#define ET96MAP_UE_RESOURCE_LIMIT            106
#define ET96MAP_UE_INIT_REL                  107 
#define ET96MAP_UE_NO_REASON                 122



 
#define ET96MAP_UE_USSD_ODB_BARRED           255 /* reject error on an USSD */

/* MAP Provider errors */

#define ET96MAP_PROV_ERR_NOT_USED                0
#define ET96MAP_DUPLICATED_INVOKE_ID             1
#define ET96MAP_NOT_SUPPORTED_SERVICE            2
#define ET96MAP_MISTYPED_PARAMETER               3
#define ET96MAP_RESOURCE_LIMITATION              4
#define ET96MAP_INITIATING_RELEASE               5
#define ET96MAP_UNEXPECTED_RESPONSE_FROM_PEER    6
#define ET96MAP_SERVICE_COMPLETION_FAILURE       7
#define ET96MAP_NO_RESPONSE_FROM_PEER            8
#define ET96MAP_INVALID_RESPONSE_RECEIVED        9
#define ET96MAP_UNEXPECTED_TRANSACTION_ID        10
#define ET96MAP_UNRECOGNIZED_COMPONENT           11
#define ET96MAP_MISSTYPED_COMPONENT              12
#define ET96MAP_BADSTRUCTURED_COMPONENT          13
#define ET96MAP_UNRECOGNIZED_LINKED_ID           14
#define ET96MAP_LINKED_RESPONSE_UNEXPECTED       15
#define ET96MAP_UNEXPECTED_LINKED_OPERATION      16





#define ET96MAP_INCLUDED                         0x00
#define ET96MAP_NOT_INCLUDED                     0x01

/* returns if error from Et96MapReqFunc */

#define ET96MAP_CALLBACK_FUNC_NOT_SET        0x02

/* Map User Reason */

#define ET96MAP_RESOURCE_LIMIT      0x00
#define ET96MAP_RESOURCE_UNAVAIL    0x01
#define ET96MAP_APPL_PROC_CANCEL    0x02
#define ET96MAP_PROC_ERROR          0x03

/* Map api results */

#define ET96MAP_E_OK                   1
#define ET96MAP_E_GENERAL              2
#define ET96MAP_E_INVALID_PARAMETER    3

#define ET96MAP_NORMAL_RELEASE                0x00
#define ET96MAP_PREARRANGED_END               0x01

/* type of message - used by MAP_SM_RP_OA_T and MAP_SM_RP_DA_T */

#define ET96MAP_ADDRTYPE_IMSI          0
#define ET96MAP_ADDRTYPE_LMSI          1
#define ET96MAP_ADDRTYPE_MSISDN        2 
#define ET96MAP_ADDRTYPE_ROAMING_NUM   3
#define ET96MAP_ADDRTYPE_SCADDR        4
#define ET96MAP_ADDRTYPE_NO_SM_RP_DA   5
#define ET96MAP_ADDRTYPE_NO_SM_RP_OA   5
/* SM PDU Type */

#define ET96MAP_SMS_DELIVER_PDU   0
#define ET96MAP_SMS_SUBMIT_PDU    1
#define ET96MAP_SMS_STATUS_PDU    2


/* timer setting if timer expired because of wait for user or remote */

#define ET96MAP_WAIT_USER    1
#define ET96MAP_WAIT_REMOTE  2

/* if ForwardSM is a MO or a MT message */
#define ET96MAP_MOBORG       1
#define ET96MAP_MOBTERM      2



/* Added by DON */


/* ATI MapUser Errors*/

#define ET96MAP_UE_ATI_NOT_ALLOWED       49
#define ET96MAP_UE_UNKNOWN_SUBSCRIBER    1


/* ATI geographical info lengths */
#define ET96MAP_DEGREES_LATITUDE_LEN     3
#define ET96MAP_DEGREES_LONGITUDE_LEN    3

/* ATI CELLID or LAI lengths */
#define ET96MAP_CELL_ID_LEN              7
#define ET96MAP_LAI_ID_LEN               5


#define ET96MAP_GEODETIC_INFO_LEN  10
#define ET96MAP_LOCATION_NUMBER_LEN      10
#define ET96MAP_MIN_LOCATION_NUMBER_LEN      2

#define ET96MAP_SELECTED_LSA_ID_LEN       3      
/* ATI Geographical Info Length */
#define ET96MAP_GEOGRAPHICAL_INFO_LEN    8


/* Values used when enc/dec UCHAR_T buffers
   from ATI types, for optionally present
   values */ 
#define ET96MAP_OPTIONAL_VALUE_ABSENT    0
#define ET96MAP_OPTIONAL_VALUE_PRESENT   1



/* Added by DON  */

#endif /* __ET96MAP_API_DEFINES_H__ */
















