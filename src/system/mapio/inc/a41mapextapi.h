/*********************************************************************/
/* GASK Required Information:                                        */
/* -------------------------                                         */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* Comment: Don't remove the above text, container depends on it!    */
/*********************************************************************/
/* COPYRIGHT Ericsson Infotech AB                                    */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infotech AB. The program may be used and/or  copied only */
/* with the written permission from Ericsson Infotech AB or in the   */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                  Rev:                            */
/* 2/190 55-CAA 901 904 Ux            A                              */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/S Patrik Verme & Anders Peltomäki                           */
/*                                                                   */
/* Purpose:                                                          */
/* Provide packing and unpacking of  ANSI-41 MAP parameters.         */
/* Protocol verification of recieved messages and messages to        */
/* be sent.                                                          */
/*                                                                   */
/* Type definitions, parameter constants, return codes and function  */
/* prototypes.                                                       */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-20010204  Patrik Verme (einpawe) & Anders Peltomäki (einpelt)  */
/*              Initial version.                                     */
/*                                                                   */
/* 01-20010716  Anders Peltomäki (einpelt)                           */
/*              Changed Transaction Capability so it can be 2 or 3   */
/*              octets long. (J-STD-036-AD-2) Added 2 new parameters */
/*              (TDMA_MAO_CELLID and TDMA_MAHO_CHANNEL) to           */
/*              EINSS7_A41MAPEXT_ISPOSREQ_INV_T,                     */
/*              EINSS7_A41MAPEXT_ISPOSREQ_RES_T and                  */
/*              EINSS7_A41MAPEXT_ORREQ_INV_T. (J-STD-036-AD-2)       */
/*              Added new operation POSREQ with two structs,         */
/*              prototypes and parameter constants. (IS-848)         */
/*                                                                   */
/* 02-20010726  Anders Peltomäki (einpelt)                           */
/*              Corrected TR2425 (MIN and IMSI length in ESPOSREQ)   */
/*                                                                   */
/* 03-20011022  Jonas Arvidsson (einjarv)                            */
/*              Added data structures and error codes for            */
/*              the new operations, GPOSREQ, GPOSDI and POSROUTREQ.  */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/
#ifndef __A41MAPEXTAPI_H__
#define __A41MAPEXTAPI_H__
#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif
/*********************************************************************/
/*                                                                   */
/*                       I N C L U D E S                             */
/*                                                                   */
/*********************************************************************/

#include "ss7cp.h"

/*********************************************************************/
/*                                                                   */
/*          P A R A M E T E R   C O N S T A N T S                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*                      V A L U E S                                  */
/*                                                                   */
/*********************************************************************/

/* Access Denied Reason */
#define EINSS7_A41MAPEXT_ACCDEN_LEN             1

/* Action Code */
#define EINSS7_A41MAPEXT_ACTION_CODE_LEN        1

/* Address (max digits length) */
#define EINSS7_A41MAPEXT_MAX_DIGITS_ADDR_LEN    17 /* Max number of digits plus length  */
                                                   /* of the Number of Digits-indicator */

/* Alert Code */
#define EINSS7_A41MAPEXT_ALRTCODE_LEN           2

/* Announcement Code */
#define EINSS7_A41MAPEXT_MIN_ANNCODE_LEN        1 /* Min length */
#define EINSS7_A41MAPEXT_MAX_ANNCODE_LEN        4 /* Max length */

/* Authorization Denied  */
#define EINSS7_A41MAPEXT_AUTH_DENIED_LEN        1

/* Bearer Data */
#define EINSS7_A41MAPEXT_MAX_BEARER_DATA_LEN    253

/* BillingId */
#define EINSS7_A41MAPEXT_BILLING_ID_LEN         7

/* Callback Number */
#define EINSS7_A41MAPEXT_MIN_CALLBACK_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_CALLBACK_LEN       20 /* Max length */

/* Calling Party Number Digits 1 */
#define EINSS7_A41MAPEXT_MIN_CPNDGTS1_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_CPNDGTS1_LEN       20 /* Max length */

/* Calling Party Number Digits 2 */
#define EINSS7_A41MAPEXT_MIN_CPNDGTS2_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_CPNDGTS2_LEN       20 /* Max length */

/* Calling Party Number String 1 */
#define EINSS7_A41MAPEXT_MIN_CPNSTRG1_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_CPNSTRG1_LEN       36 /* Max length */

/* Calling Party Number String 2 */
#define EINSS7_A41MAPEXT_MIN_CPNSTRG2_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_CPNSTRG2_LEN       36 /* Max length */

/* Calling Party Subaddress */
#define EINSS7_A41MAPEXT_MIN_CPSUB_LEN          2  /* Min length */
#define EINSS7_A41MAPEXT_MAX_CPSUB_LEN          21 /* Max length */

/* Carrier Digits */
#define EINSS7_A41MAPEXT_MIN_CARDGTS_LEN        6 /* Min length */
#define EINSS7_A41MAPEXT_MAX_CARDGTS_LEN        7 /* Max length */

/* CDMA Channel Data*/
#define EINSS7_A41MAPEXT_MIN_CDMA_CHAN_DATA_LEN 1 /* Min length */
#define EINSS7_A41MAPEXT_MAX_CDMA_CHAN_DATA_LEN 8 /* Max length */

/* CDMA Code Channel */
#define EINSS7_A41MAPEXT_CDMA_CODE_CHAN_LEN     1

/* CDMA Mobile Capabilities */
#define EINSS7_A41MAPEXT_CDMA_MOBILE_CAP_LEN    1

/* CDMA Pilot Strength */
#define EINSS7_A41MAPEXT_CDMA_PILOT_STREN_LEN   1

/* CDMA Pilot Strength Measurement Count */
#define EINSS7_A41MAPEXT_CDMA_PSMM_COUNT_LEN    1

/* CDMA Private Long Code Mask */
#define EINSS7_A41MAPEXT_CDMA_PRIV_LONG_LEN     6

/* CDMA Serving One Way Delay */
#define EINSS7_A41MAPEXT_CDMA_SERV_DELAY_LEN    3

/* CDMA Serving One Way Delay 2 */
#define EINSS7_A41MAPEXT_MIN_CDMA_ONE_DEL_2_LEN 2 /* Min length */
#define EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN 5 /* Max length */

/* CDMA Target One Way Delay */
#define EINSS7_A41MAPEXT_CDMA_TAR_ONE_DEL_LEN   2

/* Channel Data */
#define EINSS7_A41MAPEXT_CHAN_DATA_LEN          3
#define EINSS7_A41MAPEXT_CHAN_DATA_LEN2         0

/* Company Id */
#define EINSS7_A41MAPEXT_MIN_COMP_ID_LEN        1  /* Min length */
#define EINSS7_A41MAPEXT_MAX_COMP_ID_LEN        15 /* Max length */

/* Deny Access  */
#define EINSS7_A41MAPEXT_DENY_ACCESS_LEN        1

/* Destination Digits */
#define EINSS7_A41MAPEXT_MIN_DEST_DGTS_LEN      4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_DEST_DGTS_LEN      20 /* Max length */

/* Digits */
#define EINSS7_A41MAPEXT_MIN_DIGITS_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_DIGITS_LEN         20 /* Max length */

/* DMH Account Code Digits */
#define EINSS7_A41MAPEXT_MIN_ACDGTS_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_ACDGTS_LEN         20 /* Max length */

/* DMH Alternate Billing Digits */
#define EINSS7_A41MAPEXT_MIN_ABDGTS_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_ABDGTS_LEN         20 /* Max length */

/* DMH Billing Digits */
#define EINSS7_A41MAPEXT_MIN_BILLDGTS_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_BILLDGTS_LEN       20 /* Max length */

/* DMH RedirectionIndicator */
#define EINSS7_A41MAPEXT_REDIND_LEN             1

/* DTX Indication */
#define EINSS7_A41MAPEXT_DTX_IND_LEN            1

/* Emergency Services Messages Entity Id */
#define EINSS7_A41MAPEXT_MAX_ESMEID_LEN         15 /* Max length */
#define EINSS7_A41MAPEXT_MIN_ESMEID_LEN         1  /* Min length */

/* Emergency Services Routing Digits */
#define EINSS7_A41MAPEXT_MIN_ESRDIGITS_LEN      4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_ESRDIGITS_LEN      20 /* Max length */

/* Emergency Services Routing Key */
#define EINSS7_A41MAPEXT_MIN_ESRKEY_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_ESRKEY_LEN         20 /* Max length */

/* Error Code */
static const UCHAR_T EINSS7_A41MAPEXT_ERRCODE_LEN  = 1;    /* Length */
static const UCHAR_T EINSS7_A41MAPEXT_ERRCODE_ID_PRIV_TCAP = 0xD4; /* Private TCAP id */

#define EINSS7_A41MAP_ERR_SYS_FAIL              1       /* System Failure (ESP) */
#define EINSS7_A41MAP_ERR_UNAUTH_REQ            2       /* Unauthorized Request (ESP) */
#define EINSS7_A41MAP_ERR_UNEXP_VALUE           3       /* Unexpected Data Value (ESP) */
#define EINSS7_A41MAP_ERR_UNREC_KEY             4       /* Unrecognized Key (ESP) */

#define EINSS7_A41MAPEXT_ERR_UNREC_MIN             0x81    /* Unrecognized MIN */
#define EINSS7_A41MAPEXT_ERR_UNREC_ESN             0x82    /* Unrecogized ESN  */
#define EINSS7_A41MAPEXT_ERR_MSID_HLR_MIS          0x83    /* MSID-HLR Mismatch */
#define EINSS7_A41MAPEXT_ERR_OP_SEQ_PROB           0x84    /* Operation Sequence Problem */
#define EINSS7_A41MAPEXT_ERR_RESRC_SHORT           0x85    /* Resource Shortage */
#define EINSS7_A41MAPEXT_ERR_OP_NOT_SUP            0x86    /* Operation Not Supported */
#define EINSS7_A41MAPEXT_ERR_TRUNK_UNAVAIL         0x87    /* Trunk Unavailable */
#define EINSS7_A41MAPEXT_ERR_PARAM_ERROR           0x88    /* Parameter error */
#define EINSS7_A41MAPEXT_ERR_SYS_FAIL              0x89    /* System Failure */
#define EINSS7_A41MAPEXT_ERR_UNREC_PAR_VAL         0x8A    /* Unrecognized Parameter Value */
#define EINSS7_A41MAPEXT_ERR_FEAT_INACT            0x8B    /* Feature Inactive */
#define EINSS7_A41MAPEXT_ERR_MISS_PARAM            0x8C    /* Missing Parameter */

/* Error parameters */
#define EINSS7_A41MAPEXT_MAX_ERROR_PARAM_LEN       7 /* Max length */

/* ESN */
#define EINSS7_A41MAPEXT_ESN_LEN                4

/* Extended MSCID einpelt 20010716 */
#define EINSS7_A41MAPEXT_EXT_MSCID_LEN          4

/* Extra Optional Parameters */
#define EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN  150 /* Max length */

/* Generalized Time */
#define EINSS7_A41MAPEXT_MIN_GENTIME_LEN        4 /* Min Length */
#define EINSS7_A41MAPEXT_MAX_GENTIME_LEN        6 /* Max Length */

/* Generic Digits */
#define EINSS7_A41MAPEXT_MIN_GENDGTS_LEN        4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_GENDGT_LEN         20 /* Max length */

/* Geographic Position */
#define EINSS7_A41MAPEXT_MIN_GEOPOS_LEN         8  /* Min Length */
#define EINSS7_A41MAPEXT_MAX_GEOPOS_LEN         13 /* Max Length */

/* Group Information */
#define EINSS7_A41MAPEXT_GRPINFO_LEN            4

/* IMSI */
#define EINSS7_A41MAPEXT_MIN_IMSI_LEN           1 /* Min Length */
#define EINSS7_A41MAPEXT_MAX_IMSI_LEN           8 /* Max Length */
#define EINSS7_A41MAPEXT_ESP_IMSI_DIGITS_LEN    12 /* ESP */ /* 20010726 einpelt TR2425 changed from 8 to 12 */

/* Leg Information */
#define EINSS7_A41MAPEXT_LEGINFO_LEN            4

/* Location Area ID */
#define EINSS7_A41MAPEXT_LOC_AREA_ID_LEN        2

/* MIN */
#define EINSS7_A41MAPEXT_MIN_LEN                5 /* ANSI-41 */
#define EINSS7_A41MAPEXT_ESP_MIN_DIGITS_LEN     9 /* ESP */ /* 20010726 einpelt TR2425 changed from 7 to 9 */

/* Mobile Call Status */
#define EINSS7_A41MAPEXT_MOB_CALL_STAT_LEN      1

/* Mobile Directory Number */
#define EINSS7_A41MAPEXT_MIN_MDN_LEN            4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_MDN_LEN            20 /* Max length */

/* Mobile Position Capability */
#define EINSS7_A41MAPEXT_MIN_MOB_POS_CAP_LEN    1 /* Min length */
#define EINSS7_A41MAPEXT_MAX_MOB_POS_CAP_LEN    5 /* Max length */

/* MSCID */
#define EINSS7_A41MAPEXT_MSCID_LEN              3

/* MSC Identification */
#define EINSS7_A41MAPEXT_MIN_MSCIN_LEN          4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_MSCIN_LEN          20 /* Max length */

/* MS Status einpelt 20010716 */
#define EINSS7_A41MAPEXT_MIN_MSSTATUS_LEN       1 /* Min length */
#define EINSS7_A41MAPEXT_MAX_MSSTATUS_LEN       2 /* Max length */

/* NAMPS Channel Data */
#define EINSS7_A41MAPEXT_NAMPS_CHAN_DATA_LEN    1

/* Network TMSI */
#define EINSS7_A41MAPEXT_MIN_NETMSI_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_NETMSI_LEN         21 /* Max length */

/* No Answer Time */
#define EINSS7_A41MAPEXT_NATIME_LEN             1

/* One Time Feature Indicator */
#define EINSS7_A41MAPEXT_OTFI_LEN               2

/* Origination Indicator */
#define EINSS7_A41MAPEXT_ORIGIND_LEN            1

/* Origination Triggers */
#define EINSS7_A41MAPEXT_MIN_ORIGTRIG_LEN       3 /* Min length */
#define EINSS7_A41MAPEXT_MAX_ORIGTRIG_LEN       4 /* Max length */

/* PC_SSN */
#define EINSS7_A41MAPEXT_PC_SSN_LEN             5

/* Pilot Number */
#define EINSS7_A41MAPEXT_MIN_PILOT_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_PILOT_LEN         20 /* Max length */

/* Position Information Code einpelt 10010716 */
#define EINSS7_A41MAPEXT_POSINFOCODE_LEN        1

/* Position Request Type */
#define EINSS7_A41MAPEXT_POSREQTYPE_LEN         1

/* Position Result */
#define EINSS7_A41MAPEXT_POSRESULT_LEN          1

/* Position Source */
#define EINSS7_A41MAPEXT_POSSOURCE_LEN          1

/* PSID_RSID Information einpelt 20010716 */
#define EINSS7_A41MAPEXT_MIN_PRINFO_LEN         3
#define EINSS7_A41MAPEXT_MAX_PRINFO_LEN         7

/* Problem Code */
static const UCHAR_T EINSS7_A41MAPEXT_PROB_CODE_ID = 0xD5; /* Problem code id */
static const UCHAR_T EINSS7_A41MAPEXT_PROB_CODE_LEN = 0x02; /* Length */

/* Problem Type */
static const UCHAR_T EINSS7_A41MAPEXT_PT_GENERAL   = 0x01; /* General */
static const UCHAR_T EINSS7_A41MAPEXT_PT_INVOKE    = 0x02; /* Invoke */

/* Problem Specifier - General */
static const UCHAR_T EINSS7_A41MAPEXT_INCORR_COMP_PTN      = 0x02; /* Incorrect component portion */
static const UCHAR_T EINSS7_A41MAPEXT_BAD_STRCT_COMP_PTN   = 0x03; /* Badly structured component */

/* Problem Specifier - Invoke */
static const UCHAR_T EINSS7_A41MAPEXT_INCORR_PARAM = 0x03; /* Incorrect parameter */

/* Received Signal Quality */
#define EINSS7_A41MAPEXT_REC_SIG_QUAL_LEN       1

/* Redirecting Number Digits */
#define EINSS7_A41MAPEXT_MIN_RNDGTS_LEN         4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_RNDGTS_LEN         20 /* Max length */

/* Redirecting Number String */
#define EINSS7_A41MAPEXT_MIN_RNSTRING_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_RNSTRING_LEN       36 /* Max length */

/* Redirecting Subaddress */
#define EINSS7_A41MAPEXT_MAX_RSUB_LEN           21 /* Max length */

/* Reject parameters */
#define EINSS7_A41MAPEXT_MAX_REJECT_PARAM_LEN      7 /* Max length */

/* Routing Digits */
#define EINSS7_A41MAPEXT_MIN_ROUTDGTS_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_ROUTDGTS_LEN       20 /* Max length */

/* Sender Indetification Number */
#define EINSS7_A41MAPEXT_MIN_SENDERIN_LEN       4  /* Min length */
#define EINSS7_A41MAPEXT_MAX_SENDERIN_LEN       20 /* Max length */

/* Service Indicator  */
#define EINSS7_A41MAPEXT_SERVICE_IND_LEN        1

/* Serving Cell Id */
#define EINSS7_A41MAPEXT_SERV_CELL_ID_LEN       2

/* Signal Quality */
#define EINSS7_A41MAPEXT_SIGNAL_QUAL_LEN        1

/* SMS Cause Code  */
#define EINSS7_A41MAPEXT_SMS_CAUSE_CODE_LEN     1

/* SMS Charge Indicator  */
#define EINSS7_A41MAPEXT_SMS_CHARGE_IND_LEN     1

/* SMS Message count  */
#define EINSS7_A41MAPEXT_SMS_MESSAGE_COUNT_LEN  1

/* SMS Notification indicator  */
#define EINSS7_A41MAPEXT_SMS_NOT_IND_LEN        1

/* SMS Subaddress */
#define EINSS7_A41MAPEXT_MIN_SMS_SUBADDR_LEN    2
#define EINSS7_A41MAPEXT_MAX_SMS_SUBADDR_LEN    21

/* SMS Teleservice Id */
#define EINSS7_A41MAPEXT_SMS_TELESERVICE_ID_LEN 2

/* Subaddress (max info length) */
#define EINSS7_A41MAPEXT_MAX_SUBADDR_LEN        20

/* System Capabilities  */
#define EINSS7_A41MAPEXT_SYSTEM_CAPABILITIES_LEN 1

/* Target Cell Id */
#define EINSS7_A41MAPEXT_TARGET_CELLID_LEN      2

/* Target MM Information */
#define EINSS7_A41MAPEXT_MIN_TAR_MM_INFO_LEN    11

/* TDMA Channel Data */
#define EINSS7_A41MAPEXT_TDMA_CHAN_DATA_LEN     5

/* TDMA_MAHO_CELLID */
#define EINSS7_A41MAPEXT_MIN_TDMA_MAHO_CELLID_LEN  8
#define EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CELLID_LEN  100

/* TDMA_MAHO_CHANNEL */
#define EINSS7_A41MAPEXT_MIN_TDMA_MAHO_CHANNEL_LEN 8
#define EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CHANNEL_LEN 100

/* TDMA_TimeAlignment */
#define EINSS7_A41MAPEXT_TDMA_TIME_ALIGNMENT_LEN 1

/* Teleservice Id */
#define EINSS7_A41MAPEXT_SMS_TELESERV_ID_LEN    2

/* Teleservice Priority  */
#define A41MAPEXT_TELESERV_PRIO_LEN             1

/* Temporary Reference Number */
#define EINSS7_A41MAPEXT_TRN_LEN                20

/* Termination Restriction Code */
#define EINSS7_A41MAPEXT_TERM_RES_LEN           1

/* Termination Treatment */
#define EINSS7_A41MAPEXT_TERMTREAT_LEN          1

/* Termination Triggers */
#define EINSS7_A41MAPEXT_TERMTRIG_LEN           2

/* Transaction Capability */
#define EINSS7_A41MAPEXT_TRANSCAP_MIN_LEN       2
#define EINSS7_A41MAPEXT_TRANSCAP_MAX_LEN       3

/* Voice Privacy Mask */
#define EINSS7_A41MAPEXT_VOICE_PRIV_MASK_LEN    66

/* VoiceMailboxNumber */
#define EINSS7_A41MAPEXT_MIN_VOICE_MB_NBR_LEN   4
#define EINSS7_A41MAPEXT_MAX_VOICE_MB_NBR_LEN   20

/* VoiceMailboxNumber */
#define EINSS7_A41MAPEXT_MIN_VOICE_MB_PIN_LEN   4
#define EINSS7_A41MAPEXT_MAX_VOICE_MB_PIN_LEN   20

/*********************************************************************/
/*                                                                   */
/*                      R E T U R N   C O D E S                      */
/*                                                                   */
/*********************************************************************/

static const USHORT_T EINSS7_A41MAPEXT_OK          = 0;

/* Failure */
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_NO_ESMEID          = 101;       /* ESPOSREQ */
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_ESMEID     = 102;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_COMP_LEN_PROBLEM   = 103;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_NO_POSREQTYPE      = 104;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_POSREQTYPE = 105;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_ESRKEY     = 106;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_NO_CALLBACK        = 107;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_CALLBACK   = 108;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_NO_ESRDIGITS       = 109;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_ESRDIGITS  = 110;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_NO_ESPR_KEY        = 111;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_SEQ_TAG_PROBLEM    = 112;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_GEOPOS     = 113;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_DATA_POINTER = 114;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_GEN_TIME   = 115;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_IMSI       = 116;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_COMP_ID    = 117;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_EXTRA_OPT_PARAM = 118;
static const USHORT_T EINSS7_A41MAPEXT_ESPOSREQ_INVALID_MIN        = 119;

static const USHORT_T EINSS7_A41MAPEXT_SMDPP_BOTH_MIN_AND_IMSI     = 201;       /* SMDPP */
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_CDMA_SERV_OWDELAY2 = 202;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_IMSI          = 203;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_DEST_ADDR = 204;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_ORIG_DEST_ADDR = 205;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_ORIG_ORIG_ADDR = 206;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_ORIG_ADDR = 207;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_ORIG_DEST_SUBADDR = 208;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_ORIG_ORIG_SUBADDR = 209;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_EXTRA_OPT_PARAM = 210;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_TEMP_REF_NO   = 211;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_NEW_IMSI      = 212;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_BOTH_MS_MIN_AND_MS_IMSI = 213;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_MS_IMSI       = 214;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_BEARER_DATA = 215;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_DATA_POINTER  = 216;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_SET_TAG_PROBLEM       = 217;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_COMP_LEN_PROBLEM      = 218;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_TELESERVICE_ID = 219;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_ACTION_CODE   = 220;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_ESN           = 221;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_MIN           = 222;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_NEW_MIN       = 223;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SERVICE_IND   = 224;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SERVING_CELL_ID = 225;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_CHARGE_IND = 226;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_MESSAGE_COUNT = 227;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_NOTIFICATION_IND = 228;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_TELESERVICE_PRIO = 229;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_NO_SMS_BEARER_DATA    = 230;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_NO_SMS_TELESERVICE_ID = 231;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_AUTH_DENIED   = 232;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_DENY_ACCESS   = 233;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_MS_MIN        = 234;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_MSCID         = 235;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SMS_CAUSE_CODE = 236;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_SYSTEM_CAPABILITIES = 237;
static const USHORT_T EINSS7_A41MAPEXT_SMDPP_INVALID_POSITION_RESULT = 238;

static const USHORT_T EINSS7_A41MAPEXT_CTRPT_SET_TAG_PROBLEM       = 301;       /* CTRPT */
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_COMP_LEN_PROBLEM      = 302;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_BOTH_MIN_IMSI         = 303;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_INVALID_MIN           = 304;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_INVALID_IMSI          = 305;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_INVALID_BILLINGID     = 306;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_NO_BILLINGID          = 307;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_INVALID_EXTRA_OPT_PARAM = 308;
static const USHORT_T EINSS7_A41MAPEXT_CTRPT_INVALID_DATA_POINTER  = 309;

static const USHORT_T EINSS7_A41MAPEXT_ORREQ_SET_TAG_PROBLEM       = 401;       /* ORREQ */
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_COMP_LEN_PROBLEM      = 402;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_BILLID        = 403;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_BILLID             = 404;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DIGITS        = 405;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_DIGITS             = 406;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_ESN           = 407;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_ESN                = 408;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MSID          = 409;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MIN           = 410;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_MSID               = 411;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_IMSI          = 412;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_ORIGTRIG      = 413;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_ORIGTRIG           = 414;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TRANSCAP      = 415;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_TRANSCAP           = 416;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CPNDGTS1      = 417;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CPNDGTS2      = 418;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CPSUB         = 419;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_ESRDIGITS     = 420;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MOB_CALL_STAT = 421;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MDN           = 422;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MSCIN         = 423;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MOB_POS_CAP   = 424;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CHAN_DATA     = 425;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_NAMPS_CHAN_DATA = 426;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DTX_IND       = 427;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_REC_SIG_QUAL  = 428;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TDMA_CHAN_DATA = 429;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_INFO_IN_TAR_MM_LIST = 430;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TAR_MM_LIST   = 431;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_CELLID_IN_TAR_MM_INFO = 432;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TAR_MM_INFO   = 433;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TAR_CELLID    = 434;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_SIGQUA_IN_TAR_MM_INFO = 435;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_SIG_QUAL      = 436;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_VOICE_P_MASK  = 437;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CDMA_CHAN_DATA = 438;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CDMA_CODE_CHAN = 439;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CDMA_PRIV_LONG = 440;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CDMA_SERV_OWDELAY2 = 441;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CDMA_MAHO_LIST = 442;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_INFO_IN_CDMA_MAHO_LIST = 443;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_CELLID_IN_MAHO_INFO = 444;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MAHO_INFO     = 445;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_PILOT_IN_MAHO_INFO = 446;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_PILOT_STR     = 447;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_ONEWAY_IN_MAHO_INFO = 448;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TAR_ONE_WAY_DEL = 449;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_MSCID         = 450;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_PSMM_LIST     = 451;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_DEL_2_IN_PSMM_LIST = 452;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_MAHO_IN_PSMM_LIST  = 453;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_ORIGIND       = 454;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_OFTI          = 455;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_PC_SSN        = 456;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_SENDERIN      = 457;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_SERV_CELLID   = 458;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TERMRES       = 459;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DATA_POINTER  = 460;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_EXTRA_OPT_PARAM = 461;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_ANNCODE       = 462;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CPNSTRG1      = 463;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CPNSTRG2      = 464;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CARDGTS       = 465;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DMH_ACDGTS    = 466;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DMH_ABDGTS    = 467;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DMH_BILLDGTS  = 468;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_GENDGTS       = 469;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_GEOPOS        = 470;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_PILOTNUM      = 471;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_REDIR_NUM_DGTS = 472;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_REDIR_NUM_STRING = 473;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_REDIR_SUB     = 474;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_ROUTDGTS      = 475;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_TERM_IN_TERMLIST   = 476;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_DEST_DGTS     = 477;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_VMBOX_NUM     = 478;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_VMBOX_PIN     = 479;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_NET_TMSI      = 480;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_NO_MSCID              = 481;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_CDMA_MOB_CAP  = 482;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TDMA_MAHO_CELLID = 483;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TDMA_MAHO_CHANNEL = 484;
static const USHORT_T EINSS7_A41MAPEXT_ORREQ_INVALID_TDMA_TIME_ALIGNMENT = 485;

static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_COMP_LEN_PROBLEM   = 501;        /* ISPOSREQ */
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_POSRES     = 502;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_MOB_POS_CAP = 503;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_MSCID      = 504;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_POSINFO    = 505;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_GEN_TIME   = 506;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_GEOPOS     = 507;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_GENTIME_IN_POSINFO = 508;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_GEOPOS_IN_POSINFO = 509;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_POS_SOURCE = 510;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_SERV_CELLID= 511;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_POSINFO         = 512;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CHAN_DATA  = 513;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_NAMPS_CHAN_DATA = 514;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_DTX_IND    = 515;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_REC_SIG_QUAL = 516;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TDMA_CHAN_DATA = 517;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TAR_MM_LIST = 518;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TAR_MM_INFO = 519;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TAR_CELLID = 520;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_SIG_QUAL   = 521;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_VOICE_P_MASK = 522;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_CHAN_DATA = 523;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_CODE_CHAN = 524;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_PRIV_LONG = 525;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_SERV_OWDELAY2 = 526;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_CELLID_IN_TAR_MM_INFO = 527;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_SIGQUA_IN_TAR_MM_INFO = 528;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_MAHO_LIST = 529;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_CELLID_IN_MAHO_INFO = 530;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_MAHO_INFO  = 531;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_PILOT_STR  = 532;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_PILOT_IN_MAHO_INFO = 533;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_ONEWAY_IN_MAHO_INFO = 534;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TAR_ONE_WAY_DEL = 535;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_MAHO_IN_PSMM_LIST = 536;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_DEL_2_IN_PSMM_LIST = 537;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_DATA_POINTER = 538;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_ESRDIGITS  = 539;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_IMSI       = 540;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_MDN        = 541;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_NET_TMSI   = 542;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_POSREQTYPE = 543;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_POSREQTYPE      = 544;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_ESN        = 545;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_MIN        = 546;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_PSMM_COUNT = 547;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_INFO_IN_TAR_MM_LIST = 548;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_INFO_IN_CDMA_MAHO_LIST = 549;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_PSMM_LIST  = 550;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_EXTRA_OPT_PARAM = 551;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_SET_TAG_PROBLEM    = 552;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_CDMA_MOB_CAP    = 553;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TDMA_MAHO_CELLID = 554;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TDMA_MAHO_CHANNEL = 555;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_NO_POS_RESULT = 556;
static const USHORT_T EINSS7_A41MAPEXT_ISPOSREQ_INVALID_TDMA_TIME_ALIGNMENT = 557;

static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_DATA_POINTER = 600; /* POSREQ */
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_MDN          = 601;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_BOTH_MIN_AND_IMSI    = 602;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_IMSI         = 603;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_SENDERIN     = 604;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_EXTRA_OPT_PARAM = 605;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_SET_TAG_PROBLEM      = 606;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_COMP_LEN_PROBLEM     = 607;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_ESN          = 608;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_EXTENDED_MSCID = 609;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_LOC_AREA_ID  = 610;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_MSCIN        = 611;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_MSCID        = 612;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_MIN          = 613;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_MSSTATUS     = 614;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_PC_SSN       = 615;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_PRINFO       = 616;
static const USHORT_T EINSS7_A41MAPEXT_POSREQ_INVALID_SERV_CELLID  = 617;

static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_COMP_LEN_PROBLEM   = 701;        /* GPOSREQ */
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_CDMA_CHAN_DATA = 702;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_CDMA_SERV_OWDELAY2 = 703;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_CHAN_DATA  = 704;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_DATA_POINTER = 705;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_EXTRA_OPT_PARAM = 706;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_GEN_TIME   = 707;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_GEOPOS     = 708;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_IMSI       = 709;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_MOB_POS_CAP = 710;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_NET_TMSI   = 711;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_POSINFO    = 712;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_POSRES     = 713;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_POS_SOURCE = 714;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_TDMA_MAHO_CELLID = 715;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_TDMA_MAHO_CHANNEL = 716;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_NO_DEL_2_IN_PSMM_LIST = 717;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_NO_GENTIME_IN_POSINFO = 718;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_NO_GEOPOS_IN_POSINFO = 719;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_NO_POS_RESULT = 720;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_SET_TAG_PROBLEM    = 721;
static const USHORT_T EINSS7_A41MAPEXT_GPOSREQ_INVALID_TDMA_TIME_ALIGNMENT = 722;

static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_COMP_LEN_PROBLEM   = 801;        /* GPOSDIR */
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_ESN        = 803;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_GEN_TIME   = 804;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_GEOPOS     = 805;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_IMSI       = 806;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_MIN        = 807;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_NET_TMSI   = 808;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_POSINFO    = 809;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_POS_SOURCE = 810;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_NO_GENTIME_IN_POSINFO = 811;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_NO_GEOPOS_IN_POSINFO = 812;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_NO_POSINFO         = 813;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_SET_TAG_PROBLEM    = 814;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_EXTRA_OPT_PARAM = 815;
static const USHORT_T EINSS7_A41MAPEXT_GPOSDIR_INVALID_DATA_POINTER = 816;

static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_COMP_LEN_PROBLEM   = 901;        /* POSROUTREQ */
static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_NO_GEO_POSITION = 902;
static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_INVALID_DATA_POINTER = 903;
static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_INVALID_EXTRA_OPT_PARAM = 904;
static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_INVALID_DEST_DIGITS = 905;
static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_INVALID_GEO_POSITION = 906;
static const USHORT_T EINSS7_A41MAPEXT_POSROUTREQ_SEQ_TAG_PROBLEM = 907;

/*********************************************************************/
/*                                                                   */
/*              T Y P E   D E F I T I O N S                          */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*              A D D R E S S I N F O                                */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ADDRINFO_TAG
{
    UCHAR_T  typeOfDigits;              /* Type of digits                */
    UCHAR_T  typeOfNumber;              /* Type of number                */
    UCHAR_T  presentationOfNumber;      /* Presentation of number        */
    UCHAR_T  availabilityOfNumber;      /* Availability of number        */
    UCHAR_T  providerOfNumber;          /* Provider of number            */
    UCHAR_T  encoding;                  /* Encoding                      */
    UCHAR_T  numberingPlan;             /* Numbering plan                */
    UCHAR_T  lengthOfAddress;           /* Length of address except the  */
                                        /* first octets. (TypeOfDigits + */
    UCHAR_T  address[EINSS7_A41MAPEXT_MAX_DIGITS_ADDR_LEN];
} EINSS7_A41MAPEXT_ADDRINFO_T;

/*********************************************************************/
/*                                                                   */
/*       A N N O U N C E M E N T  L I S T  P A R A M E T E R S       */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ANNLIST_TAG
{
    UCHAR_T     annCodeLen;
    UCHAR_T     annCode[EINSS7_A41MAPEXT_MAX_ANNCODE_LEN];      /* Announcement Code */
    UCHAR_T     optCodeLen;
    UCHAR_T     optCode[EINSS7_A41MAPEXT_MAX_ANNCODE_LEN];      /* Optional Announcement Code */
} EINSS7_A41MAPEXT_ANNLIST_T;

/*********************************************************************/
/*                                                                   */
/*  C D M A  T A R G E T  M A H O  I N F O  P A R A M E T E R S      */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_CDMA_MAHO_INFO_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     targetCellId[EINSS7_A41MAPEXT_TARGET_CELLID_LEN]; /* Target Cell Id */
    UCHAR_T     cdmaPilotStren;         /* CDMA Pilot Strength */
    UCHAR_T     cdmaTarOneDelay[EINSS7_A41MAPEXT_CDMA_TAR_ONE_DEL_LEN]; /* CDMA Target One Way Delay */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   mscIdUsed;
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID */
} EINSS7_A41MAPEXT_CDMA_MAHO_INFO_T;

/*********************************************************************/
/*                                                                   */
/*  C D M A  T A R G E T  M A H O  L I S T  P A R A M E T E R S      */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_CDMA_MAHO_LIST_TAG
{
    /*---Mandatory---------------------------------------------------*/
    EINSS7_A41MAPEXT_CDMA_MAHO_INFO_T cdmaTargetMAHOInfo_s; /* CDMA Target MAHO Information */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   optInfoUsed;
    EINSS7_A41MAPEXT_CDMA_MAHO_INFO_T optInfo_s; /* Optional CDMA Target MAHO Information */
} EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T;

/*********************************************************************/
/*                                                                   */
/*       C D M A  P S  M M   L I S T  P A R A M E T E R S            */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_CDMA_PSMM_LIST_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     cdmaServOneDelay2Len;
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN]; /* CDMA Serving One Way Delay 2 */
    EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T cdmaTargetMAHOList_s;  /* CDMA Target MAHO List */
    BOOLEAN_T   optListUsed;
    EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T optList_s;  /* Optional CDMA Target MAHO List */
} EINSS7_A41MAPEXT_CDMA_PSMM_LIST_T;

/*********************************************************************/
/*                                                                   */
/*              E R R O R  P A R A M E T E R S                       */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ERROR_TAG
{
    UCHAR_T     errorCodeId;            /* Error code id */
    UCHAR_T     errorCodeLen;           /* Error code length */
    UCHAR_T     errorCode;              /* Error code */
    UCHAR_T     paramLen;               /* Parameter length  */
    UCHAR_T     param[EINSS7_A41MAPEXT_MAX_ERROR_PARAM_LEN]; /* Parameters */
} EINSS7_A41MAPEXT_ERROR_T;

/*********************************************************************/
/*                                                                   */
/* I N T E R S Y S T E M  T E R M I N A T I O N  P A R A M E T E R S */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ISTERM_TAG
{
    /*---Mandatory---------------------------------------------------*/
    EINSS7_A41MAPEXT_ADDRINFO_T destDgts_s;        /* Destination Digits */
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   accDeniedResUsed;
    UCHAR_T     accDeniedRes;                   /* Access Denied Reason */
    BOOLEAN_T   billingIdUsed;
    UCHAR_T     billingId[EINSS7_A41MAPEXT_BILLING_ID_LEN]; /* BillingId */
    BOOLEAN_T   carrierDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T carrierDigits_s;   /* Carrier Digits */
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];  /* ESN */
    BOOLEAN_T   legInfoUsed;
    UCHAR_T     legInfo[EINSS7_A41MAPEXT_LEGINFO_LEN];  /* Leg Information */
    BOOLEAN_T   mdnUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mdn_s;             /* Mobile Directory Number */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];  /* MIN */
    BOOLEAN_T   mscInUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mscIn_s;           /* MSC Identification */
    BOOLEAN_T   routDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T routDgts_s;        /* Routing Digits */
    BOOLEAN_T   termTrigUsed;
    UCHAR_T     termTrig[EINSS7_A41MAPEXT_TERMTRIG_LEN];        /* Termination Triggers */
} EINSS7_A41MAPEXT_ISTERM_T;

/*********************************************************************/
/*                                                                   */
/*       L O C A L  T E R M I N A T I O N  P A R A M E T E R S       */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_LOCALTERM_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];  /* ESN */
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];  /* MIN */
    UCHAR_T     termTreat;              /* Termination Treatment */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   alertCodeUsed;
    UCHAR_T     alertCode[EINSS7_A41MAPEXT_ALRTCODE_LEN];       /* Alert Code */
    BOOLEAN_T   carrierDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T carrierDigits_s;   /* Carrier Digits */
    BOOLEAN_T   destDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T destDgts_s;        /* Destination Digits */
    BOOLEAN_T   legInfoUsed;
    UCHAR_T     legInfo[EINSS7_A41MAPEXT_LEGINFO_LEN];  /* Leg Information */
    BOOLEAN_T   mdnUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mdn_s;             /* Mobile Directory Number */
    BOOLEAN_T   oneTimefeatIndUsed;
    UCHAR_T     oneTimefeatInd[EINSS7_A41MAPEXT_OTFI_LEN];      /* One Time Feature Indicator */
    BOOLEAN_T   routDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T routDgts_s;        /* Routing Digits */
    BOOLEAN_T   termTrigUsed;
    UCHAR_T     termTrig[EINSS7_A41MAPEXT_TERMTRIG_LEN];        /* Termination Triggers */
    BOOLEAN_T   vmBoxNumUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T vmBoxNum_s;        /* Voice Mailbox Number */
    BOOLEAN_T   vmBoxPinUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T vmBoxPin_s;        /* Voice Mailbox PIN */
} EINSS7_A41MAPEXT_LOCALTERM_T;

/*********************************************************************/
/*                                                                   */
/*   P O S I T I O N  I N F O R M A T I O N  P A R A M E T E R S     */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_POSINFO_TAG
{
    UCHAR_T     genTimeLen;
    UCHAR_T     generalizedTime[EINSS7_A41MAPEXT_MAX_GENTIME_LEN];  /* Generalized Time */
    UCHAR_T     geoPositionLen;
    UCHAR_T     geoPosition[EINSS7_A41MAPEXT_MAX_GEOPOS_LEN];   /* Geographic Position */
    BOOLEAN_T   posSourceUsed;
    UCHAR_T     posSource;                      /* Position Source */
} EINSS7_A41MAPEXT_POSINFO_T;

/*********************************************************************/
/*                                                                   */
/*       P S T N  T E R M I N A T I O N  P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_PSTNTERM_TAG
{
    /*---Mandatory---------------------------------------------------*/
    EINSS7_A41MAPEXT_ADDRINFO_T destDgts_s;        /* Destination Digits */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   carrierDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T carrierDigits_s;   /* Carrier Digits */
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];  /* ESN */
    BOOLEAN_T   legInfoUsed;
    UCHAR_T     legInfo[EINSS7_A41MAPEXT_LEGINFO_LEN];  /* Leg Information */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];  /* MIN */
    BOOLEAN_T   routDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T routDgts_s;        /* Routing Digits */
    BOOLEAN_T   termTrigUsed;
    UCHAR_T     termTrig[EINSS7_A41MAPEXT_TERMTRIG_LEN];        /* Termination Triggers */
} EINSS7_A41MAPEXT_PSTNTERM_T;

/*********************************************************************/
/*                                                                   */
/*              R E J E C T   P A R A M E T E R S                    */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_REJECT_TAG
{
    UCHAR_T     problemType;            /* Problem type */
    UCHAR_T     problemSpecifier;       /* Problem specifier */
    UCHAR_T     paramLen;               /* Parameter length  */
    UCHAR_T     param[EINSS7_A41MAPEXT_MAX_REJECT_PARAM_LEN]; /* Parameters */
} EINSS7_A41MAPEXT_REJECT_T;

/*********************************************************************/
/*                                                                   */
/*          S U B A D D R E S S I N F O                              */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_SUBADDRINFO_TAG
{
    UCHAR_T  oddEvenInd;        /* Odd / even indicator */
    UCHAR_T  typeOfSubaddress;  /* Type of subaddress   */
    UCHAR_T  lengthOfAddress;   /* Length of address    */
    UCHAR_T  address[EINSS7_A41MAPEXT_MAX_SUBADDR_LEN];
} EINSS7_A41MAPEXT_SUBADDRINFO_T;

/*********************************************************************/
/*                                                                   */
/*  T A R G E T  M E A S U R E M E N T  I N F O  P A R A M E T E R S */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_TARGET_MM_INFO_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     targetCellId[EINSS7_A41MAPEXT_TARGET_CELLID_LEN]; /* Target Cell Id */
    UCHAR_T     signalQuality;          /* Signal Quality */
} EINSS7_A41MAPEXT_TARGET_MM_INFO_T;

/*********************************************************************/
/*                                                                   */
/*  T A R G E T  M E A S U R E M E N T  L I S T  P A R A M E T E R S */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_TARGET_MM_LIST_TAG
{
    /*---Mandatory---------------------------------------------------*/
    EINSS7_A41MAPEXT_TARGET_MM_INFO_T targetMMInfo_s; /* TDMA Target MM Information */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   optInfoUsed;
    EINSS7_A41MAPEXT_TARGET_MM_INFO_T optInfo_s; /* Optional TDMA Target MM Information */
} EINSS7_A41MAPEXT_TARGET_MM_LIST_T;

/*********************************************************************/
/*                                                                   */
/*      T E R M I N A T I O N  L I S T   P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_TERMLIST_TAG
{
    BOOLEAN_T   intersysTermUsed;
    EINSS7_A41MAPEXT_ISTERM_T intersysTerm_s;      /* Intersystem Termination */
    BOOLEAN_T   localTermUsed;
    EINSS7_A41MAPEXT_LOCALTERM_T localTerm_s;      /* Local Termination */
    BOOLEAN_T   pstnTermUsed;
    EINSS7_A41MAPEXT_PSTNTERM_T pstnTerm_s;        /* PSTN Termination */
} EINSS7_A41MAPEXT_TERMLIST_T;

/*********************************************************************/
/*                                                                   */
/*                         I N V O K E                               */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*           C T R P T  I N V O K E  P A R A M E T E R S             */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_CTRPT_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     billingId[EINSS7_A41MAPEXT_BILLING_ID_LEN]; /* BillingId */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];          /* MIN */
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];    /* IMSI */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_CTRPT_INV_T;

/*********************************************************************/
/*                                                                   */
/*         E S P O S R E Q  I N V O K E  P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ESPOSREQ_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     esmeIdLen;
    UCHAR_T     esmeId[EINSS7_A41MAPEXT_MAX_ESMEID_LEN]; /* EMSE Id */
    UCHAR_T     posReqType;                             /* Position Request Type */
    BOOLEAN_T   esrKeyUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T esrKey_s;                  /* ESR Key */
    BOOLEAN_T   callbackNumUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T callbackNum_s;             /* Callback Number */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   esrDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T esrDigits_s;               /* ESR Digits */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_ESPOSREQ_INV_T;

/*********************************************************************/
/*                                                                   */
/*         G P O S D I R   I N V O K E   P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_GPOSDIR_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    EINSS7_A41MAPEXT_POSINFO_T posInfo_s; /* Position Information */
    /*---Optional----------------------------------------------------*/
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];
    UCHAR_T     netmsiLen;
    UCHAR_T     netmsi[EINSS7_A41MAPEXT_MAX_NETMSI_LEN]; /* Network TMSI */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_GPOSDIR_INV_T;

/*********************************************************************/
/*                                                                   */
/*          G P O S R E Q  I N V O K E  P A R A M E T E R S          */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_GPOSREQ_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     posReqType;             /* Position Request Type */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];  /* Electronic Serial Number */
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];    /* IMSI */
    UCHAR_T     mobilePosCapLen;
    UCHAR_T     mobilePosCap[EINSS7_A41MAPEXT_MAX_MOB_POS_CAP_LEN]; /* Mobile Position Capability */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];  /* Mobile Identification Number */
    BOOLEAN_T   channelDataUsed;
    UCHAR_T     channelDataLen;
    UCHAR_T     channelData[EINSS7_A41MAPEXT_CHAN_DATA_LEN]; /* Channel Data */
    BOOLEAN_T   dtxIndicationUsed;
    UCHAR_T     dtxIndication;                          /* DTX Indication */
    BOOLEAN_T   recSignalQualUsed;
    UCHAR_T     recSignalQual;                          /* Received Signal Quality */
    UCHAR_T     cdmaChannelLen;
    UCHAR_T     cdmaChanData[EINSS7_A41MAPEXT_MAX_CDMA_CHAN_DATA_LEN];     /* CDMA Channel Data */
    BOOLEAN_T   cdmaCodeChanUsed;
    UCHAR_T     cdmaCodeChan;                                           /* CDMA Code Channel */
    BOOLEAN_T   cdmaMobileCapUsed;
    UCHAR_T     cdmaMobileCap;                          /* CDMA Mobile Capabilities */
    BOOLEAN_T   cdmaPrivLongUsed;
    UCHAR_T     cdmaPrivLong[EINSS7_A41MAPEXT_CDMA_PRIV_LONG_LEN]; /* CDMA Private Long Code Mask */
    UCHAR_T     cdmaServOneDelay2Len;
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN]; /* CDMA Serving One Way Delay 2 */
    BOOLEAN_T   cdmaTargetMAHOUsed;
    EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T cdmaTargetMAHOList_s;     /* CDMA Target MAHO List */

    /* XXXX Not implemented: CDMAServiceOption */

    BOOLEAN_T   cdmaPSMMListUsed;
    EINSS7_A41MAPEXT_CDMA_PSMM_LIST_T cdmaPSMMList_s;           /* CDMA Pilot Strength Measurements List */
    BOOLEAN_T   nampsChannelDataUsed;
    UCHAR_T     nampsChannelData;                       /* NAMPS Channel Data */
    BOOLEAN_T   tdmaChannelDataUsed;
    UCHAR_T     tdmaChannelData[EINSS7_A41MAPEXT_TDMA_CHAN_DATA_LEN]; /* TDMA Channel Data */
    BOOLEAN_T   targetMMListUsed;
    EINSS7_A41MAPEXT_TARGET_MM_LIST_T targetMMList_s;        /* Target Measurement List */
    USHORT_T    tdmaMahoCellIdLen; /* einpelt 20010716 New Param TDMA_MAHO_CELLID */
    UCHAR_T     tdmaMahoCellId[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CELLID_LEN]; /* TDMA_MAHO_CELLID */
    USHORT_T    tdmaMahoChannelLen; /* einpelt 20010716 New Param TDMA_MAHO_CHANNEL */
    UCHAR_T     tdmaMahoChannel[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CHANNEL_LEN]; /* TDMA_MAHO_CHANNEL */
    USHORT_T    tdmaTimeAlignmentUsed; /* einjarv 20011024 New Param TDMA_TimeAlignment */
    UCHAR_T     tdmaTimeAlignment; /* TDMA_TimeAlignment */

    BOOLEAN_T   voicePrivMaskUsed;
    UCHAR_T     voicePrivMask[EINSS7_A41MAPEXT_VOICE_PRIV_MASK_LEN]; /* Voice Privacy Mask */
    BOOLEAN_T   mscIdUsed;
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID (Serving) */
    UCHAR_T     netmsiLen;
    UCHAR_T     netmsi[EINSS7_A41MAPEXT_MAX_NETMSI_LEN];        /* Network TMSI */
    BOOLEAN_T   servingCellIdUsed;
    UCHAR_T     servingCellId[EINSS7_A41MAPEXT_SERV_CELL_ID_LEN]; /* Serving Cell Id */
    BOOLEAN_T   teleservicePrioUsed;
    UCHAR_T     teleservicePrio; /* Teleservice_Priority */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_GPOSREQ_INV_T;

/*********************************************************************/
/*                                                                   */
/*         I S P O S R E Q  I N V O K E  P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ISPOSREQ_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     posReqType;             /* Position Request Type */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];  /* ESN */
    BOOLEAN_T   esrDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T esrDigits_s;       /* ESR Digits */
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];    /* IMSI */
    BOOLEAN_T   mdnUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mdn_s;             /* Mobile Directory Number */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];  /* MIN */
    BOOLEAN_T   channelDataUsed;
    UCHAR_T     channelDataLen;
    UCHAR_T     channelData[EINSS7_A41MAPEXT_CHAN_DATA_LEN]; /* Channel Data */
    BOOLEAN_T   nampsChannelDataUsed;
    UCHAR_T     nampsChannelData;                       /* NAMPS Channel Data */
    BOOLEAN_T   dtxIndicationUsed;
    UCHAR_T     dtxIndication;                          /* DTX Indication */
    BOOLEAN_T   recSignalQualUsed;
    UCHAR_T     recSignalQual;                          /* Received Signal Quality */
    BOOLEAN_T   tdmaChannelDataUsed;
    UCHAR_T     tdmaChannelData[EINSS7_A41MAPEXT_TDMA_CHAN_DATA_LEN]; /* TDMA Channel Data */
    USHORT_T    tdmaMahoCellIdLen; /* einpelt 20010716 New Param TDMA_MAHO_CELLID */
    UCHAR_T     tdmaMahoCellId[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CELLID_LEN]; /* TDMA_MAHO_CELLID */
    USHORT_T    tdmaMahoChannelLen; /* einpelt 20010716 New Param TDMA_MAHO_CHANNEL */
    UCHAR_T     tdmaMahoChannel[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CHANNEL_LEN]; /* TDMA_MAHO_CHANNEL */
    USHORT_T    tdmaTimeAlignmentUsed; /* einjarv 20011024 New Param TDMA_TimeAlignment */
    UCHAR_T     tdmaTimeAlignment; /* TDMA_TimeAlignment */
    BOOLEAN_T   targetMMListUsed;
    EINSS7_A41MAPEXT_TARGET_MM_LIST_T targetMMList_s;   /* Target Measurement List */
    BOOLEAN_T   voicePrivMaskUsed;
    UCHAR_T     voicePrivMask[EINSS7_A41MAPEXT_VOICE_PRIV_MASK_LEN]; /* Voice Privacy Mask */
    UCHAR_T     cdmaChannelLen;
    UCHAR_T     cdmaChanData[EINSS7_A41MAPEXT_MAX_CDMA_CHAN_DATA_LEN];     /* CDMA Channel Data */
    BOOLEAN_T   cdmaCodeChanUsed;
    UCHAR_T     cdmaCodeChan;                                           /* CDMA Code Channel */
    BOOLEAN_T   cdmaMobileCapUsed;
    UCHAR_T     cdmaMobileCap;                          /* CDMA Mobile Capabilities */
    BOOLEAN_T   cdmaPrivLongUsed;
    UCHAR_T     cdmaPrivLong[EINSS7_A41MAPEXT_CDMA_PRIV_LONG_LEN]; /* CDMA Private Long Code Mask */
    UCHAR_T     cdmaServOneDelay2Len;
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN]; /* CDMA Serving One Way Delay 2 */
    BOOLEAN_T   cdmaTargetMAHOUsed;
    EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T cdmaTargetMAHOList_s;     /* CDMA Target MAHO List */
    BOOLEAN_T   cdmaPSMMListUsed;
    EINSS7_A41MAPEXT_CDMA_PSMM_LIST_T cdmaPSMMList_s;           /* CDMA Pilot Strength Measurements List */
    UCHAR_T     mobilePosCapLen;
    UCHAR_T     mobilePosCap[EINSS7_A41MAPEXT_MAX_MOB_POS_CAP_LEN]; /* Mobile Position Capability */
    BOOLEAN_T   mscIdUsed;
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID */
    UCHAR_T     netmsiLen;
    UCHAR_T     netmsi[EINSS7_A41MAPEXT_MAX_NETMSI_LEN];
    UCHAR_T     cdmaPSMMCountUsed;
    UCHAR_T     cdmaPSMMCount;          /* CDMA Pilot Strength Measurement Count */
    BOOLEAN_T   servingCellIdUsed;
    UCHAR_T     servingCellId[EINSS7_A41MAPEXT_SERV_CELL_ID_LEN]; /* Serving Cell Id */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_ISPOSREQ_INV_T;

/*********************************************************************/
/*                                                                   */
/*           O R R E Q  I N V O K E  P A R A M E T E R S             */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ORREQ_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     billingId[EINSS7_A41MAPEXT_BILLING_ID_LEN]; /* BillingId */
    EINSS7_A41MAPEXT_ADDRINFO_T digits_s;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];          /* ESN */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];          /* MIN */
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];    /* IMSI */
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID */
    UCHAR_T     origTrigLen;
    UCHAR_T     origTrig[EINSS7_A41MAPEXT_MAX_ORIGTRIG_LEN]; /* Origination Triggers */
    UCHAR_T     transCapLen;
    UCHAR_T     transCap[EINSS7_A41MAPEXT_TRANSCAP_MAX_LEN];
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   callPartyNumDig1Used;
    EINSS7_A41MAPEXT_ADDRINFO_T callPartyNumDig1_s; /* Calling Party Number Digits 1 */
    BOOLEAN_T   callPartyNumDig2Used;
    EINSS7_A41MAPEXT_ADDRINFO_T callPartyNumDig2_s; /* Calling Party Number Digits 2 */
    BOOLEAN_T   callPartySubaddUsed;
    EINSS7_A41MAPEXT_SUBADDRINFO_T callPartySubadd_s; /* Calling Party Subaddress */
    BOOLEAN_T   esrDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T esrDigits_s;               /* ESR Digits */
    BOOLEAN_T   mobCallStatUsed;
    UCHAR_T     mobCallStat;                    /* Mobile Call Status */
    BOOLEAN_T   mdnUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mdn_s;             /* Mobile Directory Number */
    BOOLEAN_T   mscInUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mscIn_s;           /* MSC Identification */
    UCHAR_T     mobilePosCapLen;
    UCHAR_T     mobilePosCap[EINSS7_A41MAPEXT_MAX_MOB_POS_CAP_LEN]; /* Mobile Position Capability */
    BOOLEAN_T   channelDataUsed;
    UCHAR_T     channelDataLen;
    UCHAR_T     channelData[EINSS7_A41MAPEXT_CHAN_DATA_LEN]; /* Channel Data */
    BOOLEAN_T   nampsChannelDataUsed;
    UCHAR_T     nampsChannelData;                       /* NAMPS Channel Data */
    BOOLEAN_T   dtxIndicationUsed;
    UCHAR_T     dtxIndication;                          /* DTX Indication */
    BOOLEAN_T   recSignalQualUsed;
    UCHAR_T     recSignalQual;                          /* Received Signal Quality */
    BOOLEAN_T   tdmaChannelDataUsed;
    UCHAR_T     tdmaChannelData[EINSS7_A41MAPEXT_TDMA_CHAN_DATA_LEN]; /* TDMA Channel Data */
    USHORT_T    tdmaMahoCellIdLen; /* einpelt 20010716 New Param TDMA_MAHO_CELLID */
    UCHAR_T     tdmaMahoCellId[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CELLID_LEN]; /* TDMA_MAHO_CELLID */
    USHORT_T    tdmaMahoChannelLen; /* einpelt 20010716 New Param TDMA_MAHO_CHANNEL */
    UCHAR_T     tdmaMahoChannel[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CHANNEL_LEN]; /* TDMA_MAHO_CHANNEL */
    USHORT_T    tdmaTimeAlignmentUsed; /* einjarv 20011024 New Param TDMA_TimeAlignment */
    UCHAR_T     tdmaTimeAlignment; /* TDMA_TimeAlignment */
    BOOLEAN_T   targetMMListUsed;
    EINSS7_A41MAPEXT_TARGET_MM_LIST_T targetMMList_s;        /* Target Measurement List */
    BOOLEAN_T   voicePrivMaskUsed;
    UCHAR_T     voicePrivMask[EINSS7_A41MAPEXT_VOICE_PRIV_MASK_LEN]; /* Voice Privacy Mask */
    UCHAR_T     cdmaChannelLen;
    UCHAR_T     cdmaChanData[EINSS7_A41MAPEXT_MAX_CDMA_CHAN_DATA_LEN];     /* CDMA Channel Data */
    BOOLEAN_T   cdmaCodeChanUsed;
    UCHAR_T     cdmaCodeChan;                                           /* CDMA Code Channel */
    BOOLEAN_T   cdmaMobileCapUsed;
    UCHAR_T     cdmaMobileCap;                          /* CDMA Mobile Capabilities */
    BOOLEAN_T   cdmaPrivLongUsed;
    UCHAR_T     cdmaPrivLong[EINSS7_A41MAPEXT_CDMA_PRIV_LONG_LEN]; /* CDMA Private Long Code Mask */
    UCHAR_T     cdmaServOneDelay2Len;
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN]; /* CDMA Serving One Way Delay 2 */
    BOOLEAN_T   cdmaTargetMAHOUsed;
    EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T cdmaTargetMAHOList_s;  /* CDMA Target MAHO List */
    BOOLEAN_T   cdmaPSMMListUsed;
    EINSS7_A41MAPEXT_CDMA_PSMM_LIST_T cdmaPSMMList_s;        /* CDMA Pilot Strength Measurements List */
    UCHAR_T     netmsiLen;
    UCHAR_T     netmsi[EINSS7_A41MAPEXT_MAX_NETMSI_LEN];        /* Network TMSI */
    BOOLEAN_T   origIndUsed;
    UCHAR_T     origInd;                /* Origination Indicator */
    BOOLEAN_T   oneTimefeatIndUsed;
    UCHAR_T     oneTimefeatInd[EINSS7_A41MAPEXT_OTFI_LEN];      /* One Time Feature Indicator */
    BOOLEAN_T   pc_ssnUsed;             /* PC_SSN */
    UCHAR_T     pc_ssn[EINSS7_A41MAPEXT_PC_SSN_LEN];
    BOOLEAN_T   senderInUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T senderIn_s;        /* Sender Identification Number */
    BOOLEAN_T   servingCellIdUsed;
    UCHAR_T     servingCellId[EINSS7_A41MAPEXT_SERV_CELL_ID_LEN]; /* Serving Cell Id */
    BOOLEAN_T   termResCodeUsed;
    UCHAR_T     termResCode;            /* Termination Restriction Code */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_ORREQ_INV_T;

/*********************************************************************/
/*                                                                   */
/*       P O S R O U T R E Q  I N V O K E  P A R A M E T E R S       */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_POSROUTREQ_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     geoPositionLen;
    UCHAR_T     geoPosition[EINSS7_A41MAPEXT_MAX_GEOPOS_LEN];
    /*---Optional----------------------------------------------------*/
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_POSROUTREQ_INV_T;

/*********************************************************************/
/*                                                                   */
/*           S M D P P  I N V O K E  P A R A M E T E R S             */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_SMDPP_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     smsBearerDataLen;
    UCHAR_T     smsBearerData[EINSS7_A41MAPEXT_MAX_BEARER_DATA_LEN];
    BOOLEAN_T   smsTeleserviceIdUsed;/*SMSTeleserviceIdentifier used*/
    USHORT_T    smsTeleserviceId;
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   actionCodeUsed;
    UCHAR_T     actionCode;
    UCHAR_T     cdmaServOneDelay2Len; /* CDMA Serving One Way Delay length */
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN];
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];/* ESN */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN]; /* MIN */
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];
    BOOLEAN_T   newMinUsed; /* Newly Assigned MIN used */
    UCHAR_T     newMin[EINSS7_A41MAPEXT_MIN_LEN]; /* New MIN */
    UCHAR_T     newImsiLen; /* Newly Assigned IMSI length */
    UCHAR_T     newImsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];
    BOOLEAN_T   serviceIndUsed; /* Service Indicator used*/
    UCHAR_T     serviceInd;
    BOOLEAN_T   servingCellIdUsed;
    USHORT_T    servingCellId;
    BOOLEAN_T   smsChargeIndUsed; /* SMS Charge Indication used*/
    UCHAR_T     smsChargeInd;
    BOOLEAN_T   smsDestAddrUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T smsDestAddr_s;
    BOOLEAN_T   smsMsgCountUsed; /* SMS Message count used */
    UCHAR_T     smsMsgCount;
    BOOLEAN_T   smsNotIndUsed; /* SMS Notification indicator used */
    UCHAR_T     smsNotInd;
    BOOLEAN_T   smsOrigDestAddrUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T smsOrigDestAddr_s;
    BOOLEAN_T   smsOrigDestSubaddrUsed;
    EINSS7_A41MAPEXT_SUBADDRINFO_T smsOrigDestSubaddr_s;
    BOOLEAN_T   smsOrigOrigAddrUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T smsOrigOrigAddr_s;
    BOOLEAN_T   smsOrigOrigSubaddrUsed;
    EINSS7_A41MAPEXT_SUBADDRINFO_T smsOrigOrigSubaddr_s;
    BOOLEAN_T   smsOrigAddrUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T smsOrigAddr_s;
    BOOLEAN_T   teleservicePrioUsed;
    UCHAR_T     teleservicePrio;
    UCHAR_T     tempRefNoLen; /* Temporary referense number length */
    UCHAR_T     tempRefNo[EINSS7_A41MAPEXT_TRN_LEN];
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_SMDPP_INV_T;

/*********************************************************************/
/*                                                                   */
/*         P O S R E Q  I N V O K E  P A R A M E T E R S             */
/*                      added: einpelt 20010716                      */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_POSREQ_INV_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID */
    UCHAR_T     posInfoCode;     /* Position Information Code */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];  /* ESN */
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];    /* IMSI */
    BOOLEAN_T   mdnUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mdn_s;             /* Mobile Directory Number */
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];  /* MIN */
    BOOLEAN_T   senderInUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T senderIn_s;        /* Sender Identification Number */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_POSREQ_INV_T;

/*********************************************************************/
/*                                                                   */
/*                         R E S U L T                               */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*           C T R P T  R E S U L T  P A R A M E T E R S             */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_CTRPT_RES_TAG
{
    /*---Optional----------------------------------------------------*/
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_CTRPT_RES_T;

/*********************************************************************/
/*                                                                   */
/*        E S P O S R E Q  R E S U L T  P A R A M E T E R S          */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ESPOSREQ_RES_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     positionResult;                 /* Position Result */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   posInfoUsed;
    EINSS7_A41MAPEXT_POSINFO_T posInfo_s;          /* Position Information */
    BOOLEAN_T   callbackNumUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T callbackNum_s;     /* Callback Number */
    BOOLEAN_T   esrDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T esrDigits_s;       /* ESR Digits */
    UCHAR_T     genTimeLen;
    UCHAR_T     generalizedTime[EINSS7_A41MAPEXT_MAX_GENTIME_LEN];  /* Generalized Time */
    BOOLEAN_T   minUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T min_s;     /* MIN */
    BOOLEAN_T   imsiUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T imsi_s;    /* IMSI */
    BOOLEAN_T   mobCallStatUsed;
    UCHAR_T     mobCallStat;                    /* Mobile Call Status */
    BOOLEAN_T   companyIdLen;
    UCHAR_T     companyId[EINSS7_A41MAPEXT_MAX_COMP_ID_LEN];
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_ESPOSREQ_RES_T;

/*********************************************************************/
/*                                                                   */
/*         G P O S D I R  R E S U L T  P A R A M E T E R S           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_GPOSDIR_RES_TAG
{
    /*---Optional----------------------------------------------------*/
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_GPOSDIR_RES_T;

/*********************************************************************/
/*                                                                   */
/*         G P O S R E Q  R E S U L T  P A R A M E T E R S           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_GPOSREQ_RES_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     positionResult;           /* Position Result */
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   posInfoUsed;
    EINSS7_A41MAPEXT_POSINFO_T posInfo_s; /* Position Information */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_GPOSREQ_RES_T;

/*********************************************************************/
/*                                                                   */
/*          I S P O S R E Q  R E S U L T  P A R A M E T E R S        */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ISPOSREQ_RES_TAG
{
    /*---Mandatory---------------------------------------------------*/
    UCHAR_T     positionResult;                         /* Position Result */
    /*---Optional----------------------------------------------------*/
    UCHAR_T     mobilePosCapLen;
    UCHAR_T     mobilePosCap[EINSS7_A41MAPEXT_MAX_MOB_POS_CAP_LEN]; /* Mobile Position Capability */
    BOOLEAN_T   channelDataUsed;
    UCHAR_T     channelDataLen;
    UCHAR_T     channelData[EINSS7_A41MAPEXT_CHAN_DATA_LEN]; /* Channel Data */
    BOOLEAN_T   nampsChannelDataUsed;
    UCHAR_T     nampsChannelData;                       /* NAMPS Channel Data */
    BOOLEAN_T   dtxIndicationUsed;
    UCHAR_T     dtxIndication;                          /* DTX Indication */
    BOOLEAN_T   recSignalQualUsed;
    UCHAR_T     recSignalQual;                          /* Received Signal Quality */
    BOOLEAN_T   tdmaChannelDataUsed;
    UCHAR_T     tdmaChannelData[EINSS7_A41MAPEXT_TDMA_CHAN_DATA_LEN]; /* TDMA Channel Data */
    USHORT_T    tdmaMahoCellIdLen; /* einpelt 20010716 New Param TDMA_MAHO_CELLID */
    UCHAR_T     tdmaMahoCellId[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CELLID_LEN]; /* TDMA_MAHO_CELLID */
    USHORT_T    tdmaMahoChannelLen; /* einpelt 20010716 New Param TDMA_MAHO_CHANNEL */
    UCHAR_T     tdmaMahoChannel[EINSS7_A41MAPEXT_MAX_TDMA_MAHO_CHANNEL_LEN]; /* TDMA_MAHO_CHANNEL */
    USHORT_T    tdmaTimeAlignmentUsed; /* einjarv 20011024 New Param TDMA_TimeAlignment */
    UCHAR_T     tdmaTimeAlignment; /* TDMA_TimeAlignment */
    BOOLEAN_T   targetMMListUsed;
    EINSS7_A41MAPEXT_TARGET_MM_LIST_T targetMMList_s;        /* Target Measurement List */
    BOOLEAN_T   voicePrivMaskUsed;
    UCHAR_T     voicePrivMask[EINSS7_A41MAPEXT_VOICE_PRIV_MASK_LEN]; /* Voice Privacy Mask */
    UCHAR_T     cdmaChannelLen;
    UCHAR_T     cdmaChanData[EINSS7_A41MAPEXT_MAX_CDMA_CHAN_DATA_LEN];     /* CDMA Channel Data */
    BOOLEAN_T   cdmaCodeChanUsed;
    UCHAR_T     cdmaCodeChan;                                           /* CDMA Code Channel */
    BOOLEAN_T   cdmaMobileCapUsed;
    UCHAR_T     cdmaMobileCap;                          /* CDMA Mobile Capabilities */
    BOOLEAN_T   cdmaPrivLongUsed;
    UCHAR_T     cdmaPrivLong[EINSS7_A41MAPEXT_CDMA_PRIV_LONG_LEN]; /* CDMA Private Long Code Mask */
    UCHAR_T     cdmaServOneDelay2Len;
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN]; /* CDMA Serving One Way Delay 2 */
    BOOLEAN_T   cdmaTargetMAHOUsed;
    EINSS7_A41MAPEXT_CDMA_MAHO_LIST_T cdmaTargetMAHOList_s;  /* CDMA Target MAHO List */
    BOOLEAN_T   cdmaPSMMListUsed;
    EINSS7_A41MAPEXT_CDMA_PSMM_LIST_T cdmaPSMMList_s;        /* CDMA Pilot Strength Measurements List */
    BOOLEAN_T   mscIdUsed;
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];      /* MSCID */
    BOOLEAN_T   posInfoUsed;
    EINSS7_A41MAPEXT_POSINFO_T posInfo_s;                  /* Position Information */
    BOOLEAN_T   servingCellIdUsed;
    UCHAR_T     servingCellId[EINSS7_A41MAPEXT_SERV_CELL_ID_LEN]; /* Serving Cell Id */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_ISPOSREQ_RES_T;

/*********************************************************************/
/*                                                                   */
/*            O R R E Q  R E S U L T  P A R A M E T E R S            */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_ORREQ_RES_TAG
{
    BOOLEAN_T   accDeniedResUsed;
    UCHAR_T     accDeniedRes;           /* Access Denied Reason */
    BOOLEAN_T   actionCodeUsed;
    UCHAR_T     actionCode;             /* Action Code */
    BOOLEAN_T   annListUsed;
    EINSS7_A41MAPEXT_ANNLIST_T annList_s;  /* Announcement List */
    BOOLEAN_T   callPartyNumStr1Used;
    EINSS7_A41MAPEXT_ADDRINFO_T callPartyNumStr1_s; /* Calling Party Number String 1 */
    BOOLEAN_T   callPartyNumStr2Used;
    EINSS7_A41MAPEXT_ADDRINFO_T callPartyNumStr2_s; /* Calling Party Number String 2 */
    BOOLEAN_T   callPartySubaddUsed;
    EINSS7_A41MAPEXT_SUBADDRINFO_T callPartySubadd_s; /* Calling Party Subaddress */
    BOOLEAN_T   carrierDigitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T carrierDigits_s;   /* Carrier Digits */
    BOOLEAN_T   digitsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T digits_s;          /* Digits */
    BOOLEAN_T   dmh_accCodeDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T dmh_accCodeDgts_s; /* DMH Account Code Digits */
    BOOLEAN_T   dmh_altBillDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T dmh_altBillDgts_s; /* DMH Alternate Billing Digits */
    BOOLEAN_T   dmh_billDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T dmh_billDgts_s;    /* DMH Billing Digits */
    BOOLEAN_T   dmh_redirIndUsed;
    UCHAR_T     dmh_redirInd;                   /* DMH RedirectionIndicator */
    BOOLEAN_T   genDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T genDgts_s;         /* Generic Digits */
    UCHAR_T     geoPositionLen;
    UCHAR_T     geoPosition[EINSS7_A41MAPEXT_MAX_GEOPOS_LEN];   /* Geographic Position */
    BOOLEAN_T   groupInfoUsed;
    UCHAR_T     groupInfo[EINSS7_A41MAPEXT_GRPINFO_LEN];        /* Group Information */
    BOOLEAN_T   mdnUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mdn_s;                  /* Mobile Directory Number */
    BOOLEAN_T   noAnswerTimeUsed;
    UCHAR_T     noAnswerTime;                           /* No Answer Time */
    BOOLEAN_T   oneTimeFeatIndUsed;
    UCHAR_T     oneTimeFeatInd[EINSS7_A41MAPEXT_OTFI_LEN];      /* One Time Feature Indicator */
    BOOLEAN_T   pilotNumUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T pilotNum_s;             /* Pilot Number */
    BOOLEAN_T   redirNumDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T redirNumDgts_s;         /* Redirecting Number Digits */
    BOOLEAN_T   redirNumStrUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T redirNumStr_s;          /* Redirecting Number String */
    BOOLEAN_T   redirSubaddUsed;
    EINSS7_A41MAPEXT_SUBADDRINFO_T redirSubadd_s;       /* Redirecting Subaddress */
    BOOLEAN_T   routDgtsUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T routDgts_s;             /* Routing Digits */
    BOOLEAN_T   termListUsed;
    EINSS7_A41MAPEXT_TERMLIST_T termList_s;             /* Termination List */
    BOOLEAN_T   termTrigUsed;
    UCHAR_T     termTrig[EINSS7_A41MAPEXT_TERMTRIG_LEN];        /* Termination Triggers */
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_ORREQ_RES_T;

/*********************************************************************/
/*                                                                   */
/*       P O S R O U T R E Q  R E S U L T  P A R A M E T E R S       */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_POSROUTREQ_RES_TAG
{
    /*---Optional----------------------------------------------------*/
    EINSS7_A41MAPEXT_ADDRINFO_T destDigits_s;
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_POSROUTREQ_RES_T;

/*********************************************************************/
/*                                                                   */
/*           S M D P P  R E S U L T  P A R A M E T E R S             */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_SMDPP_RES_TAG
{
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   authorizationDeniedUsed;
    UCHAR_T     authorizationDenied;
    UCHAR_T     denyAccessUsed;
    UCHAR_T     denyAccess;
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];
    BOOLEAN_T   msMinUsed; /* Mobile Station MIN used */
    UCHAR_T     msMin[EINSS7_A41MAPEXT_MIN_LEN];
    UCHAR_T     msImsiLen; /* Mobile Station IMSI length */
    UCHAR_T     msImsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];
    BOOLEAN_T   mscIdUsed;
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];
    UCHAR_T     smsBearerDataLen;
    UCHAR_T     smsBearerData[EINSS7_A41MAPEXT_MAX_BEARER_DATA_LEN];
    BOOLEAN_T   smsCauseCodeUsed; /* SMS Cause Code used*/
    UCHAR_T     smsCauseCode;
    BOOLEAN_T   systemCapabilitiesUsed;
    UCHAR_T     systemCapabilities;
    UCHAR_T     cdmaServOneDelay2Len; /* CDMA Serving One Way Delay 2 length */
    UCHAR_T     cdmaServOneDelay2[EINSS7_A41MAPEXT_MAX_CDMA_ONE_DEL_2_LEN];
    BOOLEAN_T   servingCellIdUsed;
    USHORT_T    servingCellId;
    BOOLEAN_T   positionResultUsed;
    UCHAR_T     positionResult;
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_SMDPP_RES_T;

/*********************************************************************/
/*                                                                   */
/*         P O S R E Q  R E S U L T  P A R A M E T E R S             */
/*                      added: einpelt 20010716                      */
/*********************************************************************/
typedef struct EINSS7_A41MAPEXT_POSREQ_RES_TAG
{
    /*---Optional----------------------------------------------------*/
    BOOLEAN_T   esnUsed;
    UCHAR_T     esn[EINSS7_A41MAPEXT_ESN_LEN];
    BOOLEAN_T   extendedMscIdUsed;
    UCHAR_T     extendedMscId[EINSS7_A41MAPEXT_EXT_MSCID_LEN];
    UCHAR_T     imsiLen;
    UCHAR_T     imsi[EINSS7_A41MAPEXT_MAX_IMSI_LEN];
    BOOLEAN_T   locationAreaIdUsed;
    UCHAR_T     locationAreaId[EINSS7_A41MAPEXT_LOC_AREA_ID_LEN];
    BOOLEAN_T   minUsed;
    UCHAR_T     min[EINSS7_A41MAPEXT_MIN_LEN];
    BOOLEAN_T   mscIdUsed;
    UCHAR_T     mscId[EINSS7_A41MAPEXT_MSCID_LEN];
    BOOLEAN_T   mscInUsed;
    EINSS7_A41MAPEXT_ADDRINFO_T mscIn_s; /* MSC Identification */
    USHORT_T    msStatusLen;
    UCHAR_T     msStatus[EINSS7_A41MAPEXT_MAX_MSSTATUS_LEN];
    BOOLEAN_T   pc_ssnUsed;
    UCHAR_T     pc_ssn[EINSS7_A41MAPEXT_PC_SSN_LEN];
    USHORT_T    prInfoLen;

    /* PSID_RSIDInformation */
    UCHAR_T     prInfo[EINSS7_A41MAPEXT_MAX_PRINFO_LEN];

    BOOLEAN_T   servingCellIdUsed;
    UCHAR_T     servingCellId[EINSS7_A41MAPEXT_SERV_CELL_ID_LEN];
    UCHAR_T     extraOptParamLen;
    UCHAR_T     extraOptParam[EINSS7_A41MAPEXT_MAX_EXTRAOPTPARAM_LEN];
} EINSS7_A41MAPEXT_POSREQ_RES_T;

/*********************************************************************/
/*                                                                   */
/*               F U N C T I O N   P R O T O T Y P E S               */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*                P A C K I N G  F U N C T I O N S                   */
/*                                                                   */
/*********************************************************************/

/* Invokes */
extern USHORT_T
EINSS7_A41MapExtPackGposreqInvoke(UCHAR_T *dataBuffer_p,
                                  USHORT_T *dataBufferLen_p,
                                  const EINSS7_A41MAPEXT_GPOSREQ_INV_T
                                  *gposreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackIsposreqInvoke(UCHAR_T *dataBuffer_p,
                                   USHORT_T *dataBufferLen_p,
                                   const EINSS7_A41MAPEXT_ISPOSREQ_INV_T
                                   *isposreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackSmdppInvoke(UCHAR_T *dataBuffer_p,
                                USHORT_T *dataBufferLen_p,
                                const EINSS7_A41MAPEXT_SMDPP_INV_T
                                *smdppParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackPosreqInvoke(UCHAR_T *dataBuffer_p,
                                 USHORT_T *dataBufferLen_p,
                                 const EINSS7_A41MAPEXT_POSREQ_INV_T
                                 *posreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackPosroutreqInvoke(UCHAR_T *dataBuffer_p,
                                     USHORT_T *dataBufferLen_p,
                                     const EINSS7_A41MAPEXT_POSROUTREQ_INV_T
                                     *posroutreqParam_sp);

/* Results */
extern USHORT_T
EINSS7_A41MapExtPackCtrptResult(UCHAR_T *dataBuffer_p,
                                USHORT_T *dataBufferLen_p,
                                const EINSS7_A41MAPEXT_CTRPT_RES_T
                                *ctrptParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackEsposreqResult(UCHAR_T *dataBuffer_p,
                                   USHORT_T *dataBufferLen_p,
                                   const EINSS7_A41MAPEXT_ESPOSREQ_RES_T
                                   *esposreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackGposdirResult(UCHAR_T *dataBuffer_p,
                                  USHORT_T *dataBufferLen_p,
                                  const EINSS7_A41MAPEXT_GPOSDIR_RES_T
                                  *gposdirParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackIsposreqResult(UCHAR_T *dataBuffer_p,
                                   USHORT_T *dataBufferLen_p,
                                   const EINSS7_A41MAPEXT_ISPOSREQ_RES_T
                                   *isposreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackSmdppResult(UCHAR_T *dataBuffer_p,
                                USHORT_T *dataBufferLen_p,
                                const EINSS7_A41MAPEXT_SMDPP_RES_T
                                *smdppParam_sp);

extern USHORT_T
EINSS7_A41MapExtPackOrreqResult(UCHAR_T *dataBuffer_p,
                                USHORT_T *dataBufferLen_p,
                                const EINSS7_A41MAPEXT_ORREQ_RES_T
                                *orreqParam_sp);

/*********************************************************************/
/*                                                                   */
/*               F U N C T I O N   P R O T O T Y P E S               */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*               U N P A C K I N G  F U N C T I O N S                */
/*                                                                   */
/*********************************************************************/

/* Invokes */
extern USHORT_T
EINSS7_A41MapExtUnpackCtrptInvoke(const UCHAR_T *dataBuffer_p,
                                  USHORT_T dataBufferLen,
                                  EINSS7_A41MAPEXT_CTRPT_INV_T *ctrptParam_sp,
                                  EINSS7_A41MAPEXT_ERROR_T *errorParam_sp,
                                  EINSS7_A41MAPEXT_REJECT_T *rejectParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackEsposreqInvoke(const UCHAR_T *dataBuffer_p,
                                     USHORT_T dataBufferLen,
                                     EINSS7_A41MAPEXT_ESPOSREQ_INV_T
                                     *esposreqParam_sp,
                                     EINSS7_A41MAPEXT_REJECT_T
                                     *rejectParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackIsposreqInvoke(const UCHAR_T *dataBuffer_p,
                                     USHORT_T dataBufferLen,
                                     EINSS7_A41MAPEXT_ISPOSREQ_INV_T
                                     *isposreqParam_sp,
                                     EINSS7_A41MAPEXT_ERROR_T *errorParam_sp,
                                     EINSS7_A41MAPEXT_REJECT_T
                                     *rejectParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackGposdirInvoke(const UCHAR_T *dataBuffer_p,
                                    USHORT_T dataBufferLen,
                                    EINSS7_A41MAPEXT_GPOSDIR_INV_T
                                    *gposdirParam_sp,
                                    EINSS7_A41MAPEXT_ERROR_T *errorParam_sp,
                                    EINSS7_A41MAPEXT_REJECT_T *rejectParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackOrreqInvoke(const UCHAR_T *dataBuffer_p,
                                  USHORT_T dataBufferLen,
                                  EINSS7_A41MAPEXT_ORREQ_INV_T *orreqParam_sp,
                                  EINSS7_A41MAPEXT_ERROR_T *errorParam_sp,
                                  EINSS7_A41MAPEXT_REJECT_T *rejectParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackSmdppInvoke(const UCHAR_T *dataBuffer_p,
                                  USHORT_T dataBufferLen,
                                  EINSS7_A41MAPEXT_SMDPP_INV_T *smdppParam_sp,
                                  EINSS7_A41MAPEXT_REJECT_T *rejectParam_sp);

/* Results */
extern USHORT_T
EINSS7_A41MapExtUnpackGposreqResult(const UCHAR_T *dataBuffer_p,
                                    USHORT_T dataBufferLen,
                                    EINSS7_A41MAPEXT_GPOSREQ_RES_T
                                    *gposreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackIsposreqResult(const UCHAR_T *dataBuffer_p,
                                     USHORT_T dataBufferLen,
                                     EINSS7_A41MAPEXT_ISPOSREQ_RES_T
                                     *isposreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackSmdppResult(const UCHAR_T *dataBuffer_p,
                                  USHORT_T dataBufferLen,
                                  EINSS7_A41MAPEXT_SMDPP_RES_T *smdppParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackPosreqResult(const UCHAR_T *dataBuffer_p,
                                   USHORT_T dataBufferLen,
                                   EINSS7_A41MAPEXT_POSREQ_RES_T
                                   *posreqParam_sp);

extern USHORT_T
EINSS7_A41MapExtUnpackPosroutreqResult(const UCHAR_T *dataBuffer_p,
                                       USHORT_T dataBufferLen,
                                       EINSS7_A41MAPEXT_POSROUTREQ_RES_T
                                       *posroutreqParam_sp);

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif
#endif
