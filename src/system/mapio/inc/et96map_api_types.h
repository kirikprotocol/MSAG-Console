/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_api_types.h,v                                                         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_api_types.h                                               */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech  AB 1998                              */
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
/* 1/190 55-CAA 201 45 Ux                                            */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.3 1998/05/26 12:04:42 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Lambkin                                                      */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Declare the structures and types used by the ET96 MAP API module. */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 971105       Ulf Melin                                            */
/*                                                                   */
/*              Changed MAP_USSD_MSG_T struvt to                     */
/*              MAP_USSD_MESS_T, for compability reasons             */
/*              Changed MAP_ERR_CODE_T to MAP_ERROR_CODE_T           */
/*                                                                   */
/* Revision record:                                                  */
/* 980406       Ulf Hellsten                                         */
/*                                                                   */
/* 981015       Anders Wallden                                       */
/*              Changed destaddr and orgaddr in struct               */
/*              MAP_OPEN_REQ_IND, from MAP_ADDRESS_T                 */
/*              to MAP_SS7_ADDR_T                                    */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#ifndef __ET96MAP_API_TYPES_H__
#define __ET96MAP_API_TYPES_H__

/***********************/
/* Specific SSN values */
/***********************/

#define  ET96MAP_MINIMUM_SSN  0x04
#define  ET96MAP_ALL_OF_MAP 0x05
#define  ET96MAP_HLR  0x06
#define  ET96MAP_VLR  0x07
#define  ET96MAP_MSC  0x08
#define  ET96MAP_EIR  0x09
#define  ET96MAP_EVOLUTION  0x0A
#define  ET96MAP_MSC_NUMBER  0x00
#define  ET96MAP_SGSN_NUMBER 0x01

/*********************************************************************/
/*                                                                   */
/*                      MAP API TYPE DEFINITIONS                     */
/*                                                                   */
/*********************************************************************/

typedef UCHAR_T ET96MAP_DEF_LOC_EVENT_TYPE_T;

typedef UCHAR_T ET96MAP_SUPPORTED_GAD_SHAPES_T;

typedef UCHAR_T ET96MAP_DEFERRED_LOC_EVENT_TYPE_T;

typedef BOOLEAN_T ET96MAP_DEL_OUTCOME_IND_T;

typedef BOOLEAN_T ET96MAP_GPRS_NODE_IND_T;

typedef UCHAR_T ET96MAP_ABSENT_DIAG_T;

typedef UCHAR_T ET96MAP_ADD_DIAG_T;

typedef UCHAR_T ET96MAP_SM_RP_MTI_T;

typedef BOOLEAN_T ET96MAP_GPRS_SUPP_IND_T;

typedef UCHAR_T ET96MAP_DCS_T;

typedef UCHAR_T ET96MAP_DISC_IND_T;

typedef USHORT_T ET96MAP_DIALOGUE_ID_T;

typedef UCHAR_T ET96MAP_LCS_CLIENT_INTERNAL_ID_T;

typedef BOOLEAN_T ET96MAP_PRIVACY_OVERRIDE_T;

typedef UCHAR_T ET96MAP_LCS_PRIORITY_T; /* if NULL not used OPTIONAL */

typedef UCHAR_T ET96MAP_RESP_TIME_CAT_T;

typedef UCHAR_T ET96MAP_HORIZONTAL_ACCURACY_T;

typedef UCHAR_T ET96MAP_VERTICAL_ACCURACY_T;

typedef USHORT_T ET96MAP_AGE_OF_LOCATION_INFO_T;

typedef UCHAR_T ET96MAP_ERROR_CODE_T;

typedef UCHAR_T ET96MAP_INVOKE_ID_T;

typedef BOOLEAN_T ET96MAP_IND_OPFOLLOW_T;

typedef ULONG_T ET96MAP_SMI_T;

typedef BOOLEAN_T ET96MAP_MWS_T;

typedef BOOLEAN_T ET96MAP_MMS_T;

typedef UCHAR_T ET96MAP_PROV_ERR_T;

typedef BOOLEAN_T ET96MAP_MSISDN_ALERT_T;

typedef UCHAR_T ET96MAP_USSD_OPCODE_T;

typedef BOOLEAN_T ET96MAP_PRI_T;

typedef UCHAR_T  ET96MAP_RELEASE_METHOD_T;

typedef ULONG_T ET96MAP_SUB_REF_T;

typedef UCHAR_T  ET96MAP_USER_REASON_T;

typedef UCHAR_T ET96MAP_UDL_T;
typedef USHORT_T ET96MAP_UDL_PH2_T;

/* values defined in map_consts.h */
typedef UCHAR_T ET96MAP_BIND_STAT_T;

typedef UCHAR_T ET96MAP_DIAGNOSTIC_INFO_T;

typedef UCHAR_T ET96MAP_SM_PDU_TYPE_T;

/* HeUl add def */
typedef UCHAR_T ET96MAP_TELE_CODE_T;
typedef BOOLEAN_T ET96MAP_MWD_SET_T;

/* AePONA */
/* This is found in ValMapAlertingPattern, Et96MapV2UnstructuredSSRequestReq */
/* and Et96MapV2UnstructuredSSNotifyReq functions                            */
typedef UCHAR_T ET96MAP_ALERTING_PATTERN_T;

/* This is used in a number of USSD functions */
typedef UCHAR_T ET96MAP_USSD_DATA_CODING_SCHEME_T;
/* AePONA */

/* Diagnostic Info - Resource Unavailable reason */
#define  ET96MAP_SHORT_TERM_PROBLEM            0
#define  ET96MAP_LONG_TERM_PROBLEM             1

/* Diagnostic Info - Procedure Cancellation reason */
#define  ET96MAP_HANDOVER_CANCELLATION         0
#define  ET96MAP_RADIO_CHANNEL_RELEASE         1
#define  ET96MAP_NETWORK_PATH_RELEASE          2
#define  ET96MAP_CALL_RELEASE                  3
#define  ET96MAP_ASSOCIATED_PROCEDURE_FAILURE  4
#define  ET96MAP_TANDEM_DIALOGUE_RELEASED      5
#define  ET96MAP_REMOTE_OPERATION_FAILURE      6

#define ET96MAP_VER1            1
#define ET96MAP_VER2            2
#define ET96MAP_VER3            3

#define ET96MAP_ALLOC_FAIL          0
#define ET96MAP_NOT_FIND                0

/* HeUl added */
#define ET96MAP_USER_SPECIFIC                      0
#define ET96MAP_USER_SPECIFIC_1                    1
#define ET96MAP_CONTEXT_NOT_SUPP                   2

typedef enum
{
  ET96MAP_NORMAL= 0,
  ET96MAP_ERROR_UNDEFINED,
  ET96MAP_INTERNAL_TIMEOUT,
  ET96MAP_CONGEST,
  ET96MAP_MT_LR_RESTART,
  ET96MAP_PRIVACY_VIOLATION
} ET96MAP_TERM_CAUSE_T;

/* Heul for PAbortInd */
typedef enum
{
  ET96MAP_MAP,
  ET96MAP_TCAP,
  ET96MAP_NETSERV
} ET96MAP_SOURCE_T;

typedef enum
{
  ET96MAP_TIME_ABS,
  ET96MAP_TIME_REL,
  ET96MAP_TIME_NOT_PRESENT

} ET96MAP_TIME_T;

typedef enum
{
  /* AePONA */
  ET96MAP_NETWORK_FUNCTIONAL_SS_CONTEXT = 0x12,      /* version 1 of USSD */
  ET96MAP_NETWORK_UNSTRUCTURED_SS_CONTEXT = 0x13,    /* version 2 of USSD */
  /* AePONA */
  ET96MAP_SHORT_MSG_GATEWAY_CONTEXT = 0x14,         /* version 1,2,3 of to/from HLR */
  ET96MAP_SHORT_MSG_MO_RELAY,                       /* version 1,2,3 of MO_SMS */
  ET96MAP_SHORT_MSG_ALERT = 0x17,                   /* version 1 and 2 from HLR */
  ET96MAP_SHORT_MSG_MT_RELAY = 0x19,                /* version 1,2,3  of MT_SMS */
  ET96MAP_SUBSCRIBER_INFO_ENQUIRY_CONTEXT = 0x1C,   /* version 3  PROVIDE_SUBSCRIBER_INFO */
  ET96MAP_ANY_TIME_INFO_ENQUIRY_CONTEXT = 0x1D,     /* version 3  ATI */
  ET96MAP_LCSVC_GATEWAY_CONTEXT = 0x25,             /* version 3  LCS */
  ET96MAP_LCSVC_ENQUIRY_CONTEXT = 0x26              /* version 3  LCS */

} ET96MAP_APP_CONTEXT_T;

typedef enum
{
  ET96MAP_MS_PRESENT = 0,
  ET96MAP_MEM_AVAILABLE = 1

} ET96MAP_ALERT_REASON_T;

typedef enum
{
  ET96MAP_BAD_EVNT_DETECTED_BY_PEER = 0,
  ET96MAP_RSP_REJECTED_BY_PEER,
  ET96MAP_BAD_EVNT_FROM_PEER,
  ET96MAP_CANNOT_DELIVER

} ET96MAP_DIAGNOSTIC_PROBLEM_T;

typedef enum
{
  ET96MAP_MSG_STAT_OK /* Not yet defined */,
  ET96MAP_UNKNOWN_SUBSCRIBER,
  ET96MAP_ILLEGAL_MS= 0x09,
  ET96MAP_TELE_SRV_NOT_PROVISIONED=0x0B,
  ET96MAP_CALL_BARRED=0x0D,
  ET96MAP_FACILITY_NOT_SUPPORTED=0x15,
  ET96MAP_ABSENT_SUBSCRIBER=0x1B,
  ET96MAP_SYSTEM_FAILURE=0x22,
  ET96MAP_UNEXPECTED_DATAVALUE=0x24,
  ET96MAP_ERR_IN_MS =0x32,
  ET96MAP_SMS_LL_CAPS_NOT_PROVISIONED,
  ET96MAP_MEMORY_CAPACITY_EXCEEDED=0x34,
  ET96MAP_REJECT_GP=0x80,
  ET96MAP_REJECT_INVOKE=0x81,
  ET96MAP_REJECT_RR,
  ET96MAP_REJECT_RE,
  ET96MAP_REJECT_REMOTE=0x85

} ET96MAP_MSG_STAT_T ;

typedef UCHAR_T ET96MAP_LOCAL_SSN_T;

typedef enum
{
  ET96MAP_MEMORY_CAPACITY_OVERRUN = 0,
  ET96MAP_SUBSCRIBER_ABSENT = 1,
  ET96MAP_TRANSFER_SUCCESSFUL = 2

} ET96MAP_DEL_OUTCOME_T;

typedef enum
{
  ET96MAP_PRO_IMPLICIT=0x20,
  ET96MAP_PRO_TELEX,
  ET96MAP_PRO_TELEFAX_G3,
  ET96MAP_PRO_TELEFAX_G4,
  ET96MAP_PRO_VOICE,
  ET96MAP_RESERVED_1,
  ET96MAP_RESERVED_2,
  ET96MAP_RESERVED_3,
  ET96MAP_PRO_TELETEX_UNSPEC,
  ET96MAP_PRO_TELETEX_PSPDN,
  ET96MAP_PRO_TELETEX_CSPDN=0x30,
  ET96MAP_PRO_ANALOG_PSTN,
  ET96MAP_PRO_DIGITAL_ISDN,
  ET96MAP_PRO_RESERVED_4,
  ET96MAP_PRO_RESERVED_5,
  ET96MAP_PRO_RESERVED_6,
  ET96MAP_PRO_MHF_SC,
  ET96MAP_PRO_X400_MHS,
  ET96MAP_PRO_RESERVED_7,
  ET96MAP_PRO_RESERVED_8,
  ET96MAP_PRO_RESERVED_9=0x40,
  ET96MAP_PRO_RESERVED_10,
  ET96MAP_PRO_RESERVED_11,
  ET96MAP_PRO_RESERVED_12,
  ET96MAP_PRO_SC_SPECIFIC_1,
  ET96MAP_PRO_SC_SPECIFIC_2,
  ET96MAP_PRO_SC_SPECIFIC_3,
  ET96MAP_PRO_SC_SPECIFIC_4,
  ET96MAP_PRO_SC_SPECIFIC_5,
  ET96MAP_PRO_SC_SPECIFIC_6,
  ET96MAP_PRO_SC_SPECIFIC_7=0x50,
  ET96MAP_PRO_MS_DEFAULT

} ET96MAP_PID_T;

typedef enum
{
  ET96MAP_RESULT_OK,
  ET96MAP_RESULT_NOT_OK

} ET96MAP_OPEN_RESULT_T;

typedef enum
{
 ET96MAP_ABNORMAL_DIALOGUE,
 ET96MAP_PROVIDER_MALFUNCTION,
 ET96MAP_TRANSACTION_RELEASED,
 ET96MAP_RESOURCE_LIM,
 ET96MAP_MAINTENANCE_ACTIVITY,
 ET96MAP_VERSION_INCOMPATIBILITY,
 ET96MAP_DIALOGUE_TIMEOUT
} ET96MAP_PROV_REASON_T;

typedef enum
{
  ET96MAP_DO_NOT_ATTEMPT_DELIVERY = 0,
  ET96MAP_ATTEMPT_DELIVERY = 1

} ET96MAP_SM_RP_PRI_T;

typedef enum
{
  ET96MAP_STAT_OK,
  ET96MAP_SC_CONGESTION=0x35,
  ET96MAP_MS_NOT_SC_SUBSCRIBER,
  ET96MAP_INVALID_SME_ADDR

} ET96MAP_SUB_STAT_T;

typedef enum
{
  ET96MAP_NO_REASON = 0,          /* Resp/Conf*/
  ET96MAP_INV_DEST_REF,           /* Resp/Conf*/
  ET96MAP_INV_ORIG_REF,           /* Resp/Conf*/
  ET96MAP_APP_CONTEXT_NOT_SUPP,   /* Conf */
  ET96MAP_NODE_NOT_REACHABLE,     /* Conf */
  ET96MAP_VERS_INCOMPATIBLE       /* Conf */
} ET96MAP_REFUSE_REASON_T;

/*********************************************************************/
/*                                                                   */
/*                    MAP API STRUCTURE DEFINITIONS                  */
/*                                                                   */
/*********************************************************************/

typedef struct
{
  UCHAR_T code[ET96MAP_CUG_CODE_LEN];
}ET96MAP_CUG_CODE_T;

typedef struct
{
    USHORT_T specificInfoLen;
    UCHAR_T  specificData[ET96MAP_MAX_USER_PH2_DATA];
}ET96MAP_USERDATA_T;

typedef struct
{
   ET96MAP_APP_CONTEXT_T acType;
   enum
   {
      ET96MAP_VERSION_1 = 1,
      ET96MAP_VERSION_2,
      ET96MAP_VERSION_3
   }version;
}ET96MAP_APP_CNTX_T;

typedef struct
{
  UCHAR_T addressLength;
  UCHAR_T typeOfAddress;
  UCHAR_T address[ET96MAP_ADDRESS_LEN];

} ET96MAP_ADDRESS_T;

typedef struct
{
  UCHAR_T typeOfNumber;
  UCHAR_T addressLength;
  UCHAR_T typeOfAddress;
  UCHAR_T address[ET96MAP_ADDRESS_LEN];

}ET96MAP_LOCATION_INFO_T;

typedef struct
{
  UCHAR_T typeOfNumber;
  UCHAR_T addressLength;
  UCHAR_T typeOfAddress;
  UCHAR_T address[ET96MAP_ADDRESS_LEN];

}ET96MAP_ADDITIONAL_NUMBER_T;

typedef struct
{
  UCHAR_T imsiLen;
  UCHAR_T imsi[ET96MAP_MAX_IMSI_LEN];

} ET96MAP_IMSI_T;

typedef struct
{
  UCHAR_T lmsiLen;
  UCHAR_T lmsi[ET96MAP_MAX_LMSI_LEN];

} ET96MAP_LMSI_T;

typedef struct
{
  UCHAR_T msisdnLen;
  UCHAR_T msisdn[ET96MAP_MAX_MSISDN_LEN];

} ET96MAP_MSISDN_T;

typedef struct
{
  BOOLEAN_T scAddrNotIncl;
  BOOLEAN_T mnrf;
  BOOLEAN_T mcef;
  BOOLEAN_T mnrg;

} ET96MAP_MWD_STATUS_T;

typedef struct
{
  UCHAR_T  year;
  UCHAR_T  month;
  UCHAR_T  day;
  UCHAR_T  hour;
  UCHAR_T  minute;
  UCHAR_T  second;
  UCHAR_T  timeZone;

} ET96MAP_SCTS_T;

typedef struct
{
  UCHAR_T  signalInfoLen;
  UCHAR_T  signalInfo[ET96MAP_MAX_SIGNAL_INFO_LEN];

} ET96MAP_SM_RP_UI_T;

typedef struct
{
  UCHAR_T ss7AddrLen;
  UCHAR_T ss7Addr[ET96MAP_MAX_ADDR_LEN];

} ET96MAP_SS7_ADDR_T;

typedef struct
{

  ET96MAP_LOCAL_SSN_T  localSsn;
  UCHAR_T version;   /* only Conf */
  ET96MAP_APP_CONTEXT_T ac;
  ET96MAP_SS7_ADDR_T ss7Address_s;
}ET96MAP_GET_VERSION_REQ_CONF_T;

typedef struct
{
  UCHAR_T tmsiLen;
  UCHAR_T tmsi[ET96MAP_MAX_TMSI_LEN];

}ET96MAP_TMSI_T;

typedef struct
{
  CHAR_T data[ET96MAP_MAX_USER_DATA];

} ET96MAP_UD_T;

/* HeUl added struct */
typedef struct
{
  UCHAR_T data[ET96MAP_MAX_USER_PH2_DATA];

} ET96MAP_UD_PH2_T;

/* AePONA */
/* This struct is used in Et96MapV1ProcessUnstructuredSSDataInd, Et96MapV1ProcessUnstructuredSSDataResp */
/* ValEt96MapV1UnpackProcessUnstructuredSSDataInd and ValEt96MapV1ProcessUnstructuredSSDataResp         */
typedef struct
{
  UCHAR_T   ssUserDataStrLen;
  UCHAR_T   ssUserDataStr[ET96MAP_MAX_SS_USER_DATA_LEN];

} ET96MAP_SS_USER_DATA_T;

/* This struct is used in a number of USSD primitives */
typedef struct
{
  UCHAR_T   ussdStrLen;
  UCHAR_T   ussdStr[ET96MAP_MAX_USSD_STR_LEN];

} ET96MAP_USSD_STRING_T;

/* End AePONA */

typedef struct
{
  UCHAR_T   ussdDcs;
  UCHAR_T   ussdStrLen;
  UCHAR_T   ussdStr[ET96MAP_MAX_USSD_STR];

} ET96MAP_USSD_MESS_T;

typedef struct
{
  UCHAR_T typeOfAddress;
  UCHAR_T addrLen;
  UCHAR_T addr[ET96MAP_MAX_ADDR_LEN];

} ET96MAP_SM_RP_DA_T;

typedef struct
{
  UCHAR_T typeOfAddress;
  UCHAR_T addrLen;
  UCHAR_T addr[ET96MAP_MAX_ADDR_LEN];

} ET96MAP_SM_RP_OA_T;

typedef struct {
    ET96MAP_LOCAL_SSN_T         localSsn;
    ET96MAP_DIALOGUE_ID_T               dialogueId;
    ET96MAP_INVOKE_ID_T         invokeId;
    ET96MAP_IMSI_T                      imsi_s;
    ET96MAP_SS7_ADDR_T                  origRef_s;      /*  Indication Only */
    ET96MAP_MSISDN_T            ussdMsisdn_s;
    ET96MAP_SS7_ADDR_T          ss7Addr_s;
    ET96MAP_USSD_OPCODE_T               ussdOpcode;     /*  Request Only */
    ET96MAP_USSD_MESS_T         ussdMsg_s;
} ET96MAP_BEGIN_T;

typedef struct{
     ET96MAP_LOCAL_SSN_T                localSsn;
     ET96MAP_BIND_STAT_T                bindStat; /* Cnf Only */
} ET96MAP_BIND_T;

typedef struct {
    ET96MAP_LOCAL_SSN_T             localSsn;
    ET96MAP_DIALOGUE_ID_T               dialogueId;
    ET96MAP_IND_OPFOLLOW_T              indOpFollow;
    ET96MAP_INVOKE_ID_T         invokeId;
    ET96MAP_USSD_MESS_T         ussdMsg_s;
} ET96MAP_END_REQ_T;

typedef struct {
    ET96MAP_LOCAL_SSN_T         localSsn;
    ET96MAP_DIALOGUE_ID_T               dialogueId;
    ET96MAP_INVOKE_ID_T         invokeId;
    ET96MAP_ERROR_CODE_T                errCode;
    ET96MAP_DISC_IND_T          discInd;
} ET96MAP_ERR_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T
localSsn;

} ET96MAP_UNBIND_T;

typedef struct  {
    ET96MAP_ADDRESS_T msisdn;

} ET96MAP_ALERT_IND_T;

typedef struct
{

  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_APP_CNTX_T    appContext_s;
  ET96MAP_SS7_ADDR_T    destAddr_s;  /* anwa */
  ET96MAP_IMSI_T        destRef_s;
  ET96MAP_SS7_ADDR_T    origAddr_s;  /* anwa */
  ET96MAP_ADDRESS_T     origRef_s;
  ET96MAP_UDL_PH2_T        specificInfoLen;
  ET96MAP_UD_PH2_T         specificInfoData_s;
  ET96MAP_USERDATA_T    specificInfo_s;

} ET96MAP_OPEN_REQ_IND_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T       localSsn;
  ET96MAP_DIALOGUE_ID_T     dialogueId;
  UCHAR_T               acPresent;
  ET96MAP_APP_CNTX_T        appContext_s;
  ET96MAP_OPEN_RESULT_T     openResult;
  UCHAR_T               refusePresent;
  ET96MAP_REFUSE_REASON_T   refuseReason;
  ET96MAP_SS7_ADDR_T        ss7RespAddress_s;  /* only Resp */
  ET96MAP_UDL_PH2_T        specificInfoLen;
  ET96MAP_UD_PH2_T         specificInfoData_s;
  ET96MAP_USERDATA_T        specificInfo_s;
  ET96MAP_PROV_ERR_T        provErrorCode;  /* Conf only */
} ET96MAP_OPEN_CONF_RSP_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T      localSsn;
  ET96MAP_DIALOGUE_ID_T    dialogueId;
  ET96MAP_RELEASE_METHOD_T releaseMethod; /* Request */
  ET96MAP_UDL_PH2_T        specificInfoLen;
  ET96MAP_UD_PH2_T         specificInfoData_s;
  ET96MAP_USERDATA_T       specificInfo_s;

  UCHAR_T                 priorityOrder;
  UCHAR_T                 qualityOfService;
} ET96MAP_CLOSE_REQ_IND_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T       localSsn;
  ET96MAP_DIALOGUE_ID_T     dialogueId;
  UCHAR_T               version;
  ET96MAP_USER_REASON_T     userReason;
  UCHAR_T               diagInfoPresent;
  ET96MAP_DIAGNOSTIC_INFO_T infoDiagnostic;
  ET96MAP_UDL_PH2_T        specificInfoLen;
  ET96MAP_UD_PH2_T         specificInfoData_s;
  ET96MAP_USERDATA_T        specificInfo_s;

  UCHAR_T                 priorityOrder;
} ET96MAP_UABORT_REQ_IND_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T    localSsn;
  ET96MAP_DIALOGUE_ID_T  dialogueId;
  ET96MAP_PROV_REASON_T  provReason;
  ET96MAP_SOURCE_T       source;
  UCHAR_T  priorityOrder;
} ET96MAP_PABORT_IND_T;

typedef struct
{
   UCHAR_T returnedOperationLen;
   UCHAR_T returnedOperation[200];
}ET96MAP_RETURNED_OPERATION_T;

/* HeUl add reldialogueId */
typedef struct {
  ET96MAP_LOCAL_SSN_T                  localSsn;
  ET96MAP_DIALOGUE_ID_T        dialogueId;
  ET96MAP_DIALOGUE_ID_T        relDialogueId;
  ET96MAP_DIAGNOSTIC_PROBLEM_T problemDiagnostic;
  ET96MAP_RETURNED_OPERATION_T returnedOperation_s;
} ET96MAP_NOTICE_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_MESS_T    ussdMsg_s;

  /* next 2 fields used by phase 2 only */

  ET96MAP_ERROR_CODE_T    errCode;       /* Conf & Rsp */
  ET96MAP_PROV_ERR_T    provErrCode;   /* Conf only */

} ET96MAP_USSD_T;

/* anwa moved struct MAP_PROC_USSD_REQ_CNF_T to ph1_dialogue.h */
/* anwa TR1351 */
#define ET96MAP_MAX_DIAGNOSTIC               200

/* HeUl TR1351 */
typedef struct ET96MAP_CALLBARR_OR_NON_CUG_TAG
{
        BOOLEAN_T callCugPresent;
        enum
        {
                ET96MAP_INCOMING_CALL_BARRED,
                ET96MAP_NON_CUG_MEMBER
        }callORCug;
}ET96MAP_CALLBARR_OR_NON_CUG_T;

/* HeUl TR1351 */
typedef struct ET96MAP_MWD_SETS_TAG
{
        BOOLEAN_T mwdPresent;
        BOOLEAN_T mwdSet;

}ET96MAP_MWD_SETS_T;

/* MT = MobileTerminated  MO = MobileOriginated */
typedef struct ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_TAG
{
  enum
  {
    ET96MAP_MEM_CAPACITY_EXCEEDED = 0,                 /* MT */
    ET96MAP_PROTOCOL_ERROR,                            /* MT or MO */
    ET96MAP_MOBILE_NOT_SM_EQUIPPED,                    /* MT */
    ET96MAP_MO_UNKNOWN_SERVICE_CENTER_ADDRESS = 3,     /* MO */
    ET96MAP_MO_SERVICE_CENTER_CONGESTION,              /* MO */
    ET96MAP_MO_INVALID_SME_ADDRESS,                    /* MO */
    ET96MAP_MO_SUBSCRIBER_NOT_SC_SUBSCRIBER            /* MO */
  }reason;                            /* MANDATORY */

  BOOLEAN_T signalInfoPresent;
  struct
  {
    UCHAR_T signalInfoLength;
    UCHAR_T signalInfo[ET96MAP_MAX_DIAGNOSTIC];
  }signalInfo_s;                     /* OPTIONAL */
}ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T;

/* anwa TR1351 */
typedef struct ET96MAP_SM_DELIVERY_FAILURE_REASON_MO_TAG
{
  enum
  {
    ET96MAP_PROTOCOL_ERR = 1,
    ET96MAP_UNKNOWN_SERVICE_CENTER_ADDRESS = 3,
    ET96MAP_SERVICE_CENTER_CONGESTION,
    ET96MAP_INVALID_SME_ADDRESS,
    ET96MAP_SUBSCRIBER_NOT_SC_SUBSCRIBER
  }reason;                            /* MANDATORY */

  BOOLEAN_T signalInfoPresent;
  struct
  {
    UCHAR_T signalInfoLength;
    UCHAR_T signalInfo[ET96MAP_MAX_DIAGNOSTIC];
  }signalInfo_s;                     /* OPTIONAL */
}ET96MAP_SM_DELIVERY_FAILURE_REASON_MO_T;

/*anwa TR1351 */
typedef struct ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_TAG
{
  BOOLEAN_T networkResourcePresent;
  enum
  {
    ET96MAP_NRP_PLMN = 0,
    ET96MAP_NRP_HLR,
    ET96MAP_NRP_VLR,
    ET96MAP_NRP_PVLR,
    ET96MAP_NRP_CONTROLLING_MSC,
    ET96MAP_NRP_VMSC,
    ET96MAP_NRP_EIR,
    ET96MAP_NRP_RSS
   }networkResource;       /* OPTIONAL */

}ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T;

/* anwa TR1351 */
typedef struct ET96MAP_CALL_BARRING_CAUSE_TAG
{
  BOOLEAN_T barringCausePresent;
  enum
  {
    ET96MAP_BARRINGSERVICE_ACTIVE = 0,
    ET96MAP_OPERERROR_BARRING
  }barringCause;           /* OPTIONAL */
}ET96MAP_CALL_BARRING_CAUSE_T;

typedef struct ET96MAP_GPRS_CONN_TAG
{
  BOOLEAN_T gprsConnSuspended;
}ET96MAP_GPRS_CONN_T;

typedef struct ET96MAP_SUBSCRIBER_DIAGNOSTIC_TAG
{
  BOOLEAN_T absentSubscriberDiagnosticPresent;
  BOOLEAN_T additionalAbsentSubscriberDiagnosticPresent;
  UCHAR_T   absentSubscriberDiagnostic;
  UCHAR_T   additionalAbsentSubscriberDiagnostic;

}ET96MAP_SUBSCRIBER_DIAGNOSTIC_T;

/* AePONA */
/* This error is found in Et96MapV1ProcessUnstructuredSSDataResp and ValEt96MapV1ProcessUnstructuredSSDataResp */
typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_DATA_T;

/* This error is found in Et96MapV2ProcessUnstructuredSSRequestResp and ValEt96MapV2ProcessUnstructuredSSRequestResp */
typedef struct
{
  UCHAR_T errorCode;
  union
  {
    /* This is triggered by systemFailure error */
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    /* This is triggered by callBarred error    */
    ET96MAP_CALL_BARRING_CAUSE_T callBarringCause_s;
  }u;

}ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_REQUEST_T;
/* AePONA */

/* AePONA */

/* This error is found in Et96MapV2UnstructuredSSRequestConf and ValEt96MapV2UnpackUnstructuredSSRequestConf*/
typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T;

/* This struct is found in Et96MapV1UnpackBeginSubscriberActivityInd */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_IMSI_T        imsi_s;
  ET96MAP_ADDRESS_T     originatingEntityNumber_s;

} ET96MAP_BEGIN_SUBSCRIBER_ACTIVITY_IND_T;

/* This struct is found in Et96MapV1UnpackProcessUnstructuredSSDataInd */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_SS_USER_DATA_T    ssUserData_s;

} ET96MAP_PROCESS_UNSTRUCTURED_SS_DATA_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_SS_USER_DATA_T    ssUserData_s;

  ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_DATA_T errorProcessUnstructuredSSData_s;

} ET96MAP_PROCESS_UNSTRUCTURED_SS_DATA_RSP_T;

/* This struct is found in Et96MapV2UnpackProcessUnstructuredSSRequestInd */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_DATA_CODING_SCHEME_T    ussdDcs;
  ET96MAP_USSD_STRING_T    ussdString_s;

  ET96MAP_ADDRESS_T msisdn_s;

} ET96MAP_PROCESS_UNSTRUCTURED_SS_REQUEST_IND_T;

/* This struct is found in Et96MapUnpackProcessUnstructuredSSRequestRsp */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_DATA_CODING_SCHEME_T    ussdDcs;
  ET96MAP_USSD_STRING_T    ussdString_s;

  ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_REQUEST_T errorProcessUnstructuredSSRequest_s;

} ET96MAP_PROCESS_UNSTRUCTURED_SS_REQUEST_RSP_T;

/* This struct is found in  Et96MapUnpackUnstructuredSSRequestReq */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_DATA_CODING_SCHEME_T    ussdDcs;
  ET96MAP_USSD_STRING_T    ussdString_s;

  UCHAR_T alertingPatternPresent;
  ET96MAP_ALERTING_PATTERN_T alertingPattern_s; /* Conditional */

} ET96MAP_UNSTRUCTURED_SS_REQUEST_REQ_T;

/* This struct is found in  Et96MapUnpackUnstructuredSSNotifyReq */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_DATA_CODING_SCHEME_T    ussdDcs;
  ET96MAP_USSD_STRING_T    ussdString_s;

  UCHAR_T alertingPatternPresent;
  ET96MAP_ALERTING_PATTERN_T alertingPattern_s; /* Conditional */

} ET96MAP_UNSTRUCTURED_SS_NOTIFY_REQ_T;

/* this struct is found in Et96MapV2UnpackUnstructuredSSRequestConf */
typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_DATA_CODING_SCHEME_T    ussdDcs;
  ET96MAP_USSD_STRING_T    ussdString_s;

  ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T errorUnstructuredSSRequest_s;
  ET96MAP_PROV_ERR_T    provErrCode;   /* Conf only */

} ET96MAP_UNSTRUCTURED_SS_REQUEST_CNF_T;
/* AePONA */

/* anwa TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_ALERT_SERVICE_T;

/* anwa TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_READY_FOR_SM_T;

/* anwa TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_CALL_BARRING_CAUSE_T barrier_s;
  }u;

}ET96MAP_ERROR_PROC_USSD_REQ_T;

typedef struct ET96MAP_UNKNOWN_SUBSCRIBER_TAG
{
  BOOLEAN_T unknownSubscriberDiagnosticPresent;
  enum
  {
    ET96MAP_IMSI_UNKNOWN = 0,
    ET96MAP_GPRS_SUBSCRIBER_UNKNOWN
   }unknownSubscriberDiagnostic;       /* OPTIONAL */

}ET96MAP_UNKNOWN_SUBSCRIBER_T;

/* xsro TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  /* Parameter only for version 3 */
  ET96MAP_UNKNOWN_SUBSCRIBER_T unknownSubscriber_s;

}ET96MAP_ERROR_REPORT_SM_DEL_STAT_T;

/* xsro TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_CALL_BARRING_CAUSE_T barrier_s;
    ET96MAP_CALLBARR_OR_NON_CUG_T callBarredORnonCug_s;
    ET96MAP_MWD_SETS_T mwdSetValue_s;
    /* See lower.Only version 3 has paramters*/
    ET96MAP_SUBSCRIBER_DIAGNOSTIC_T subscriberDiagnostic_s;
    ET96MAP_UNKNOWN_SUBSCRIBER_T unknownSubscriber_s;

  }u;

}ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T;

/* HeUl TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T smDeliveryFailureReason_s;
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_MWD_SETS_T mwdSetValue_s;
    /* only version 3 */
    ET96MAP_GPRS_CONN_T gprsConn_s;
    ET96MAP_SUBSCRIBER_DIAGNOSTIC_T subscriberDiagnostic_s;

  }u;
}ET96MAP_ERROR_FORW_SM_MT_T;

/* anwa TR1351 */
typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SM_DELIVERY_FAILURE_REASON_MO_T smDeliveryFailureReason_s;
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_MWD_SETS_T mwdSetValue_s;
  }u;
}ET96MAP_ERROR_FORW_SM_MO_T;

/* xsro TR1351*/
typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_USSD_NOTIFY_T;

/* xsro TR1351*/
typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_USSD_REQ_T;

typedef struct /* anwa added TR1351 */
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_MESS_T    ussdMsg_s;

  /* next 2 fields used by phase 2 only */

  ET96MAP_ERROR_PROC_USSD_REQ_T errorProcUssdReq_s;
  ET96MAP_PROV_ERR_T    provErrCode;   /* Conf only */

} ET96MAP_PROC_USSD_REQ_CNF_T;

typedef struct /* anwa added TR1351 */
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_ERROR_USSD_NOTIFY_T errorUssdNotify_s;
  ET96MAP_PROV_ERR_T    provErrCode;   /* Conf only */

} ET96MAP_USSD_NOTIFY_CNF_T;

typedef struct /* anwa added TR1351 */
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_USSD_MESS_T    ussdMsg_s;

  /* next 2 fields used by phase 2 only */

  ET96MAP_ERROR_USSD_REQ_T errorUssdReq_s;
  ET96MAP_PROV_ERR_T    provErrCode;   /* Conf only */

} ET96MAP_USSD_REQ_CNF_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_IMSI_T        imsi_s;
  ET96MAP_LOCATION_INFO_T locationInfo_s;
  ET96MAP_ADDRESS_T     roamNum_s;    /* version 1 */
  ET96MAP_ADDRESS_T     mscNum_s;      /* version 1 version 2 */
  ET96MAP_LMSI_T        lmsi_s;
  UCHAR_T           mwdSetPresent; /* version1 */
  ET96MAP_MWD_SET_T     mwdSet;        /* version 1 */

  /* Added new parameters for version 3 */
  BOOLEAN_T                   gprsNodeIndicator;
  ET96MAP_ADDRESS_T           networkNodeNumber_s;
  ET96MAP_ADDITIONAL_NUMBER_T additionalNodeNumber_s;

  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T  errorSendRoutingInfoForSm_s;  /* anwa TR1351 */
  ET96MAP_PROV_ERR_T    provErrCode; /* Cnf only  */

} ET96MAP_SND_RINFO_SM_CONF_RSP_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_SM_RP_UI_T    smRpUi_s; /* only version 3 */
  ET96MAP_ERROR_FORW_SM_MO_T errorForwardSMmo_s;   /* anwa TR1351 Response*/
  ET96MAP_ERROR_FORW_SM_MT_T errorForwardSMmt_s;   /* Conf */
  ET96MAP_PROV_ERR_T    provErrCode;               /* Conf */

} ET96MAP_FWD_SM_CONF_RSP_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_ERROR_FORW_SM_MO_T errorForwardSMmoV2_s;   /* Conf */
  ET96MAP_PROV_ERR_T    provErrCode;               /* Conf */
} ET96MAP_FWD_SM_MO_V2_CONF;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_ERROR_READY_FOR_SM_T errorReadyForSm_s; /* anwa TR1351 */

  ET96MAP_PROV_ERR_T    provErrCode; /* Cnf only */

} ET96MAP_RDY_SM_CONF_RSP_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T    localSsn;
  ET96MAP_DIALOGUE_ID_T  dialogueId;
  ET96MAP_INVOKE_ID_T    invokeId;
  ET96MAP_ERROR_ALERT_SERVICE_T errorAlertService_s; /* anwa TR1351 */
  ET96MAP_PROV_ERR_T     provErrCode; /* Cnf only */

} ET96MAP_ALERT_SC_CONF_RSP_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T       localSsn;
  ET96MAP_DIALOGUE_ID_T     dialogueId;
  ET96MAP_INVOKE_ID_T       invokeId;
  ET96MAP_ADDRESS_T          msisdnAlert_s;
  ET96MAP_ERROR_REPORT_SM_DEL_STAT_T  errorReportSmDelStat_s;   /* anwa TR1351 */
  ET96MAP_PROV_ERR_T        provErrCode; /* Cnf Only */

} ET96MAP_RPT_SM_DEL_CONF_RSP_T;

typedef struct  {
  ET96MAP_SMI_T       msgId;
  ET96MAP_ADDRESS_T       destAddr_s;
  ET96MAP_ADDRESS_T       origAddr_s;
  ET96MAP_PID_T               protocolId;
  ET96MAP_DCS_T               smDcs;
  ET96MAP_MMS_T       moreMsgsToSend;
  ET96MAP_SCTS_T              scTimeStamp_s;
  ET96MAP_PRI_T       priority;
  ET96MAP_UDL_T       userDataLen;
  ET96MAP_UD_T        userData_s;

} ET96MAP_DEL_SM_REQ_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T        localSsn;
  ET96MAP_DIALOGUE_ID_T      dialogueId;
  ET96MAP_INVOKE_ID_T        invokeId;
  ET96MAP_ADDRESS_T              msIsdn_s;
  ET96MAP_ADDRESS_T              scAddr_s;
  ET96MAP_DEL_OUTCOME_T      smDelOutcome;
  UCHAR_T                    absentSubscriberDiagSMPresent;
  ET96MAP_ABSENT_DIAG_T      absentSubscriberDiagSM;
  ET96MAP_GPRS_SUPP_IND_T    gprsSuppIndicator;
  ET96MAP_DEL_OUTCOME_IND_T      deliveryOutcomeIndicator;
  UCHAR_T                    additionalSMDeliveryOutcomePresent;
  ET96MAP_DEL_OUTCOME_T         additionalSMDeliveryOutcome;
  UCHAR_T                    additionalAbsentsubscrDiagSMPresent;
  ET96MAP_ADD_DIAG_T         additionalAbsentsubscrDiagSM;

} ET96MAP_RPT_SM_DEL_REQ_IND_T;

typedef struct  {
  ET96MAP_SUB_REF_T          submitRef;
  ET96MAP_SUB_STAT_T     submitStat;

} ET96MAP_SUBMIT_SM_CONF_T;

typedef struct {
  ET96MAP_SUB_REF_T             submitRef;
  ET96MAP_ADDRESS_T         origAddr_s;
  ET96MAP_ADDRESS_T         destAddr_s;
  ET96MAP_PID_T         protocolId;
  ET96MAP_DCS_T         submitDcs;
  ET96MAP_TIME_T                absTime;
  ET96MAP_SCTS_T                validityPeriod_s;
  ET96MAP_UDL_T         userDataLen;
  ET96MAP_UD_T          userData_s;

} ET96MAP_SUBMIT_SM_IND_T;

typedef struct  {
  UCHAR_T      addressLength;
  UCHAR_T      typeOfAddress;
  UCHAR_T      address[ET96MAP_MAX_SM_RP_SMEA_ADDRESS_LENGTH];
} ET96MAP_SM_RP_SMEA_T;

typedef struct {
  ET96MAP_LOCAL_SSN_T       localSsn;
  ET96MAP_DIALOGUE_ID_T         dialogueId;
  ET96MAP_INVOKE_ID_T           invokeId;
  ET96MAP_ADDRESS_T             msIsdn_s;
  ET96MAP_SM_RP_PRI_T       rpPriority;
  ET96MAP_ADDRESS_T         scAddr_s;
  UCHAR_T                cugCodePresent;      /* only version 1 */
  ET96MAP_CUG_CODE_T         cugCode_s;
  UCHAR_T                teleserviceCodePresent;
  ET96MAP_TELE_CODE_T        teleserviceCode;
  UCHAR_T                 smRpMtiPresent;
  ET96MAP_SM_RP_MTI_T        smRpMti;
  UCHAR_T                   smRpSmeaOctettLen;
  ET96MAP_SM_RP_SMEA_T       smRpSmea_s;
  ET96MAP_GPRS_SUPP_IND_T   gprsSuppInd;

} ET96MAP_SND_RINFO_SM_REQ_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T       localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T           invokeId;
  ET96MAP_MMS_T                 moreMsgToSend;
  ET96MAP_SM_RP_OA_T        smRpOa_s;
  ET96MAP_SM_RP_DA_T        smRpDa_s;
  ET96MAP_SM_RP_UI_T        smRpUi_s;
  ET96MAP_SM_PDU_TYPE_T     smPduType;
  ET96MAP_IMSI_T       imsi_s; /* added for version 3 */

} ET96MAP_FWD_SM_REQ_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T    localSsn;
  ET96MAP_DIALOGUE_ID_T  dialogueId;
  ET96MAP_INVOKE_ID_T    invokeId;
  ET96MAP_IMSI_T             imsi_s;
  ET96MAP_ALERT_REASON_T alertReason;

} ET96MAP_RDY_SM_REQ_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T    localSsn;
  ET96MAP_DIALOGUE_ID_T  dialogueId;
  ET96MAP_INVOKE_ID_T    invokeId;
  ET96MAP_ADDRESS_T       msisdn_s;
  ET96MAP_ADDRESS_T          scAddr_s;
} ET96MAP_ALERT_SC_REQ_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T    localSsn;
  ET96MAP_DIALOGUE_ID_T  dialogueId;
  ET96MAP_INVOKE_ID_T    invokeId;
  ET96MAP_ADDRESS_T       msisdn_s;
  UCHAR_T              mwdStatPresent;
  ET96MAP_MWD_STATUS_T     mwdStat_s;
} ET96MAP_INFORM_SC_REQ_IND_T;

typedef struct
{
  ET96MAP_SMI_T  msgId;
  ET96MAP_MSG_STAT_T msgStat;
  ET96MAP_MWS_T  msgWaitingSet;
  CHAR_T         paramLen;
  CHAR_T         param[ET96MAP_MAX_REPORT_PARAM];

} ET96MAP_REPORT_SM_IND_T;

typedef struct
{
     ET96MAP_LOCAL_SSN_T   localSsn;
     ET96MAP_DIALOGUE_ID_T dialogueId;
     ET96MAP_INVOKE_ID_T   invokeId;
     ET96MAP_ADDRESS_T     mlcNumber_s;
     ET96MAP_ADDRESS_T     msisdn_s;
     ET96MAP_IMSI_T        imsi_s;

}ET96MAP_V3_SND_RINFO_LCS_REQ_T;

typedef struct ET96MAP_ABSENT_SUBSCR_TAG
{
  BOOLEAN_T absentSubscriberReasonPresent;
  enum
  {
    ET96MAP_IMSI_DETACH = 0,
    ET96MAP_RESTRICTED_AREA,
    ET96MAP_NO_PAGE_RESPONSE
   }absentSubscriberReason;       /* OPTIONAL */

}ET96MAP_ABSENT_SUBSCR_T;

typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_UNKNOWN_SUBSCRIBER_T unknownSubscriber_s;
    ET96MAP_ABSENT_SUBSCR_T absentSubscriber_s;

  }u;

}ET96MAP_ERROR_ROUTING_INFO_FOR_LCS_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_ADDRESS_T     msisdn_s;
  ET96MAP_IMSI_T        imsi_s;
  ET96MAP_LMSI_T        lmsi_s;
  ET96MAP_ADDRESS_T     mscNum_s;
  BOOLEAN_T     gprsNodeIndicator;
  ET96MAP_ADDITIONAL_NUMBER_T  additionalNumber_s;
  ET96MAP_ERROR_ROUTING_INFO_FOR_LCS_T  errorSendRoutingInfoForLCS_s;
  ET96MAP_PROV_ERR_T    provErrCode; /* Cnf only  */

} ET96MAP_V3_SND_RINFO_LCS_CONF_T;

typedef enum
{
  ET96MAP_CURRENT_LOCATION = 0x00,
  ET96MAP_CURRENT_OR_LASTKNOWN_LOCATION,
  ET96MAP_INITIAL_LOCATION
} ET96MAP_LOCATION_TYPE_T;

typedef enum
{
  ET96MAP_EMERGENCY_SERVICES = 0x00,
  ET96MAP_VALUE_ADDED_SERVICES,
  ET96MAP_PLMN_OPERATOR_SERVICES,
  ET96MAP_LAWFUL_INTERCEPT_SERVICES
} ET96MAP_CLIENT_TYPE_T;

typedef struct
{
   UCHAR_T nameStringLength;
   UCHAR_T datacodingScheme;
   UCHAR_T nameString[ET96MAP_MAX_NAME_STRING_LEN];
} ET96MAP_LCS_CLIENT_NAME_T;

typedef struct
{
  UCHAR_T imeiLen;
  UCHAR_T imei[ET96MAP_MAX_IMEI_LEN];

} ET96MAP_IMEI_T;

typedef struct
{
  ET96MAP_HORIZONTAL_ACCURACY_T *horizontalAccuracy_p;        /* if NULL not used OPTIONAL */
  BOOLEAN_T verticalCoordinateReq;  /* if FALSE not used else TRUE OPTIONAL */
  ET96MAP_VERTICAL_ACCURACY_T *verticalAccuracy_p;          /* if NULL not used OPTIONAL */
  ET96MAP_RESP_TIME_CAT_T *respTimeCategory_p; /* if NULL not used OPTIONAL */

}ET96MAP_LCS_QOS_T; /* if NULL not used OPTIONAL */

typedef struct
{

  UCHAR_T lcsApnLength;
  UCHAR_T lcsApn[ET96MAP_MAX_LCS_APN_LEN];

}ET96MAP_LCS_APN_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_LOCATION_TYPE_T locationType;
  ET96MAP_ADDRESS_T     mlcNum_s;
  BOOLEAN_T     lcsClientTypePresent;
  ET96MAP_CLIENT_TYPE_T lcsClientType;
  ET96MAP_ADDRESS_T     lcsClientExternalId_s;
  ET96MAP_ADDRESS_T     lcsClientDialedByMS_s;
  BOOLEAN_T             lcsClientInternalIdPresent;
  ET96MAP_LCS_CLIENT_INTERNAL_ID_T lcsClientInternalId;
  ET96MAP_LCS_CLIENT_NAME_T lcsClientName_s;
  ET96MAP_PRIVACY_OVERRIDE_T  privacyOverride;
  ET96MAP_ADDRESS_T     msisdn_s;
  ET96MAP_IMSI_T        imsi_s;
  ET96MAP_LMSI_T        lmsi_s;
  ET96MAP_IMEI_T        imei_s;
  BOOLEAN_T              lcsPriorityPresent;
  ET96MAP_LCS_PRIORITY_T lcsPriority;
  BOOLEAN_T              lcsQualityOfServicePresent;
  BOOLEAN_T              horizontalAccuracyPresent;
  BOOLEAN_T              verticalAccuracyPresent;
  BOOLEAN_T              respTimeCategoryPresent;
  ET96MAP_LCS_QOS_T      lcsQualityOfService_s;
  ET96MAP_HORIZONTAL_ACCURACY_T horizontalAccuracy;
  BOOLEAN_T verticalCoordinateReq;
  ET96MAP_VERTICAL_ACCURACY_T verticalAccuracy;
  ET96MAP_RESP_TIME_CAT_T respTimeCategory;
  ET96MAP_LCS_APN_T      lcsApn_s;
  BOOLEAN_T      supportedGadShapesPresent;
  ET96MAP_SUPPORTED_GAD_SHAPES_T supportedGadShapes;
  BOOLEAN_T      deferredLocationEventTypePresent;
  ET96MAP_DEFERRED_LOC_EVENT_TYPE_T  deferredLocationEventType;

}ET96MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_REQ_T;

typedef struct
{
  BOOLEAN_T unauthorizedLCSClientDiagnosticPresent;
  enum
  {
    ET96MAP_NO_ADDITIONAL_INFO = 0,
    ET96MAP_CLIENT_NOT_IN_MS_PRIVACY_LIST,
    ET96MAP_CALL_TO_CLIENT_NOT_SETUP,
    ET96MAP_PRIV_OVERRIDE_NOT_APPLICABLE,
    ET96MAP_DISALLOWED_BY_LOCAL_REGULATORY_REQUIREMENT
   }unauthorizedLCSClientDiagnostic;       /* OPTIONAL */

}ET96MAP_UNAUTHORIZED_LCS_CLIENT_DIAGNOSTIC_T;

typedef struct
{
  BOOLEAN_T positionMethodFailureDiagnosticPresent;
  enum
  {
    ET96MAP_CONGESTION = 0,
    ET96MAP_INSUFFICIENT_RESOURCES,
    ET96MAP_INSUFFICIENT_MEASUREMENT_DATA,
    ET96MAP_INCONSISTENT_MEASUREMENT_DATA,
    ET96MAP_LOC_PROCEDURE_NOT_COMPLETED,
    ET96MAP_LOC_PROCEDURE_NOT_SUPPORTED_BY_TARGET_MS,
    ET96MAP_QUALITY_OF_SERVICE_NOT_ATTAINABLE,
    ET96MAP_POS_METHOD_NOT_AVAIL_IN_NETWORK,
    ET96MAP_POS_METHOD_NOT_AVAIL_IN_LOC_AREA
   }positionMethodFailureDiagnostic;       /* OPTIONAL */

}ET96MAP_POS_METHOD_FAILURE_DIAG_T;

typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_ABSENT_SUBSCR_T absentSubscriber_s;
    ET96MAP_UNAUTHORIZED_LCS_CLIENT_DIAGNOSTIC_T unauthorizedLCSClientDiagnostic_s;
    ET96MAP_POS_METHOD_FAILURE_DIAG_T positionMethodFailureDiagnostic_s;
  }u;

}ET96MAP_ERROR_PROVIDE_SUBSCRIBER_LCS_T;

typedef struct
{
   UCHAR_T geographicalInfoLen;
   UCHAR_T geographicalInfo[ET96MAP_MAX_GEOGRAPHICAL_INFO_LEN];

}ET96MAP_GEOGRAPHICAL_INFO_T;

typedef struct
{
   UCHAR_T addGeographicalInfoLen;
   UCHAR_T addGeographicalInfo[ET96MAP_MAX_ADD_GEOGRAPHICAL_INFO_LEN];

}ET96MAP_ADD_GEOGRAPHICAL_INFO_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T   localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T   invokeId;
  ET96MAP_GEOGRAPHICAL_INFO_T   locationEstimate_s;
  BOOLEAN_T ageOfLocationInfoPresent;
  ET96MAP_AGE_OF_LOCATION_INFO_T   ageOfLocationInfo;
  ET96MAP_ADD_GEOGRAPHICAL_INFO_T   addLocationEstimate_s;
  BOOLEAN_T  deferredmtLrResponseIndicator;
  ET96MAP_ERROR_PROVIDE_SUBSCRIBER_LCS_T  errorProvideSubscriberLCS_s;
  ET96MAP_PROV_ERR_T    provErrCode; /* Cnf only  */

} ET96MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_CONF_T;

typedef struct
{
  UCHAR_T errorCode;
  union
  {
    ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
    ET96MAP_UNKNOWN_SUBSCRIBER_T unknownSubscriber_s;
  }u;

}ET96MAP_ERROR_SUBSCRIBER_LCS_REPORT_T;

typedef struct
{

    ET96MAP_LOCAL_SSN_T localSsn;
    ET96MAP_DIALOGUE_ID_T dialogueId;
    ET96MAP_INVOKE_ID_T invokeId;
    ET96MAP_ERROR_SUBSCRIBER_LCS_REPORT_T errorSubscriberLCSReport_s;

}ET96MAP_V3_SUBSCRIBER_REPORT_LCS_RESP_T;

typedef enum
{
   ET96MAP_EMERGENCY_CALL_ORIG,
   ET96MAP_EMERGENCY_CALL_RELEASE,
   ET96MAP_MOB_ORIG_LOCATION_REQ,
   ET96MAP_DEFERRED_MT_LR_RESPONSE
}ET96MAP_LCS_EVENT_T;

typedef struct
{
   ET96MAP_LOCAL_SSN_T    localSsn;
   ET96MAP_DIALOGUE_ID_T  dialogueId;
   ET96MAP_INVOKE_ID_T    invokeId;
   ET96MAP_ADDRESS_T     mscNum_s;
   ET96MAP_LCS_EVENT_T   lcsEvent;
   BOOLEAN_T             lcsClientTypePresent;
   ET96MAP_CLIENT_TYPE_T lcsClientType;
   ET96MAP_ADDRESS_T     lcsClientExternalId_s;
   ET96MAP_ADDRESS_T     lcsClientDialedByMS_s;
   BOOLEAN_T                         lcsClientInternalIdPresent;
   ET96MAP_LCS_CLIENT_INTERNAL_ID_T  lcsClientInternalId;
   ET96MAP_LCS_CLIENT_NAME_T  lcsClientName_s;
   ET96MAP_LCS_APN_T      lcsApn_s;
   ET96MAP_IMSI_T        imsi_s;
   ET96MAP_ADDRESS_T     msisdn_s;
   ET96MAP_ADDRESS_T     naEsrd_s;
   ET96MAP_ADDRESS_T     naEsrk_s;
   ET96MAP_IMEI_T        imei_s;
   ET96MAP_GEOGRAPHICAL_INFO_T   locationEstimate_s;
   BOOLEAN_T                      ageOfLocationInfoPresent;
   ET96MAP_AGE_OF_LOCATION_INFO_T ageOfLocationInfo;
   ET96MAP_LMSI_T        lmsi_s;
   BOOLEAN_T                   gprsNodeIndicator;
   ET96MAP_ADDITIONAL_NUMBER_T additionalNodeNumber_s;
   ET96MAP_ADD_GEOGRAPHICAL_INFO_T   addLocationEstimate_s;
    BOOLEAN_T               deferredLocationEventTypePresent;
   ET96MAP_DEF_LOC_EVENT_TYPE_T      deferredLocationEventType;
    BOOLEAN_T        terminationCausePresent;
   ET96MAP_TERM_CAUSE_T          terminationCause;
    ET96MAP_ADDRESS_T     newNetworkNodeNumber_s;
    ET96MAP_LMSI_T        newLmsi_s;
    BOOLEAN_T             newGprsNodeIndicator;
    ET96MAP_ADDITIONAL_NUMBER_T newAdditionalNodeNumber_s;

}ET96MAP_V3_SUBSCRIBER_REPORT_LCS_IND_T;

typedef struct
{
  UCHAR_T            localSsn;
  ET96MAP_DIALOGUE_ID_T  dialogueId;

  UCHAR_T                 priorityOrder;
  UCHAR_T                 qualityOfService;
} ET96MAP_DELIMIT_REQ_IND_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T localSsn;
  UCHAR_T  userState;
  UCHAR_T  affectedSsn;
  ULONG_T affectedSpc;
  ULONG_T localSpc;

} ET96MAP_STATE_IND_T;

typedef struct ET96MAP_ADDR_LINK_T ET96MAP_ADDR_LINK_T;
typedef struct ET96MAP_AC_LINK_T ET96MAP_AC_LINK_T;

struct ET96MAP_AC_LINK_T
{
   ET96MAP_APP_CNTX_T   ac;
   UCHAR_T              version;
   ET96MAP_AC_LINK_T    *next;
};

struct ET96MAP_ADDR_LINK_T
{
   ET96MAP_SS7_ADDR_T   addr;
   ET96MAP_AC_LINK_T    *acHead;
   ET96MAP_ADDR_LINK_T  *next;
};

/* Added by DON  */

/*********************************************************************/
/*                                                                   */
/*                    MAP ATI API STRUCTURE DEFINITIONS              */
/*                                                                   */
/*********************************************************************/

typedef struct
{
  BOOLEAN_T locationInfo_s;
  BOOLEAN_T subscriberState_s;
}ET96MAP_ATI_REQUESTED_INFO_T;

typedef struct
{
  UCHAR_T typeOfAddress;
  UCHAR_T addrLen;
  UCHAR_T addr[ET96MAP_MAX_ADDR_LEN];
}ET96MAP_ATI_SUBSCRIBER_IDENTITY_T;

typedef struct
{
  ET96MAP_LOCAL_SSN_T localSsn;
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_INVOKE_ID_T invokeId;
  ET96MAP_IMSI_T imsi_s;
  ET96MAP_ADDRESS_T msisdn_s;
  BOOLEAN_T locationInfo;
  BOOLEAN_T subscriberState;
  BOOLEAN_T currentLocation;
  ET96MAP_ADDRESS_T gsmSCF_s;
}ET96MAP_ANY_TIME_INTERROGATION_REQ_IND_T;

typedef struct
{
  UCHAR_T element[ET96MAP_DEGREES_LATITUDE_LEN];
}ET96MAP_DEGREES_LATITUDE_T;

typedef struct
{
  UCHAR_T element[ET96MAP_DEGREES_LONGITUDE_LEN];
}ET96MAP_DEGREES_LONGITUDE_T;

typedef struct
{
   UCHAR_T geographicalInfoLen;
   UCHAR_T geographicalInfo[ET96MAP_GEOGRAPHICAL_INFO_LEN];
} ET96MAP_ATI_GEOGRAPHICAL_INFO_T;

typedef struct
{
   UCHAR_T geographicalInfoLen;
   UCHAR_T geographicalInfo[ET96MAP_GEOGRAPHICAL_INFO_LEN];
} ET96MAP_PROVIDE_GEOGRAPHICAL_INFO_T;

typedef struct
{
    UCHAR_T cellOrServiceLength;
    UCHAR_T cellOrServiceElement[ET96MAP_CELL_ID_LEN];
    UCHAR_T laiLength;
    UCHAR_T laiElement[ET96MAP_LAI_ID_LEN];
} ET96MAP_CELLID_OR_SERVICE_OR_LAI_T;

typedef struct
{
    UCHAR_T geodeticInfoLength;
    UCHAR_T geodeticInfo[ET96MAP_GEODETIC_INFO_LEN];
}ET96MAP_GEODETICINFO_T;

typedef struct
{
    UCHAR_T locationNumberLen;
    UCHAR_T locationNumber[ET96MAP_LOCATION_NUMBER_LEN];
}ET96MAP_LOCATIONNUMBER_T;

typedef struct
{
    UCHAR_T selectedLSAIdLen;
    UCHAR_T selectedLSAId[ET96MAP_SELECTED_LSA_ID_LEN];
}ET96MAP_SELECTED_LSA_ID_T;

typedef struct
{
  BOOLEAN_T ageOfLocationInformationPresent;
  USHORT_T ageOfLocationInformation;
  ET96MAP_ATI_GEOGRAPHICAL_INFO_T geographicalInformation_s;
  ET96MAP_ADDRESS_T vlrNumber_s;
  ET96MAP_LOCATIONNUMBER_T locationNumber_s;
  ET96MAP_CELLID_OR_SERVICE_OR_LAI_T cellIdOrServiceOrLai_s;
  ET96MAP_SELECTED_LSA_ID_T selectedLSAId_s;
  ET96MAP_ADDRESS_T mscNumber_s;
  ET96MAP_GEODETICINFO_T geodeticInfo_s;
  BOOLEAN_T currentLocationRetrived;
  BOOLEAN_T saiPresent;
}ET96MAP_ATI_LOCATION_INFO_T;

typedef struct
{
  BOOLEAN_T ageOfLocationInformationPresent;
  USHORT_T ageOfLocationInformation;
  ET96MAP_PROVIDE_GEOGRAPHICAL_INFO_T geographicalInformation_s;
  ET96MAP_ADDRESS_T vlrNumber_s;
  ET96MAP_LOCATIONNUMBER_T locationNumber_s;
  ET96MAP_CELLID_OR_SERVICE_OR_LAI_T cellIdOrServiceOrLai_s;
  ET96MAP_SELECTED_LSA_ID_T selectedLSAId_s;
  ET96MAP_ADDRESS_T mscNumber_s;
  ET96MAP_GEODETICINFO_T geodeticInfo_s;
  BOOLEAN_T currentLocationRetrived;
  BOOLEAN_T saiPresent;
}ET96MAP_PROVIDE_LOCATION_INFO_T;

typedef enum
{
  ATI_MSPURGED = 0,
  ATI_IMSIDETACHED = 1,
  ATI_RESTRICTEDAREA = 2,
  ATI_NOTREGISTERED =3
} ET96MAP_ATI_NOT_REACHABLE_REASON_T;

/* Constants relating to the Subscriber State Parameter.             */
/* Assumed Idle: any MS that in not CAMEL-busy or network determined */
/* not reachable.                                                    */
/* CAMEL-busy: the MS is engaged in a mobile-originated or mobile-   */
/* terminated circuit switched call                                  */
/* Network determined not reachable: the network can determine from  */
/* its internal data that the MS is not reachable.  This includes    */
/* detached and purged mobile stations                               */
/* Not Provided from the VLR: The VLR is unable to provide the       */
/* Subscriber State                                                  */

typedef struct
{
  enum SubscriberChoiceId
  {
    ET96MAP_ATI_ASSUMED_IDLE,
    ET96MAP_ATI_CAMEL_BUSY,
    ET96MAP_ATI_NET_DET_NOT_REACHABLE,
    ET96MAP_ATI_NOT_PROVIDED_FROM_VLR
  } choiceId;

  ET96MAP_ATI_NOT_REACHABLE_REASON_T notReachableReason;

} ET96MAP_ATI_SUBSCRIBER_STATE_T;

typedef struct
{
  UCHAR_T errorCode;
  ET96MAP_SYSTEM_FAILURE_NETWORK_RESOURCE_T systemFailureNetworkResource_s;
}ET96MAP_ERROR_ANY_TIME_INTERROGATION_T;

typedef struct
{
        ET96MAP_LOCAL_SSN_T localSsn;
        ET96MAP_DIALOGUE_ID_T dialogueId;
        ET96MAP_INVOKE_ID_T invokeId;
        ET96MAP_ATI_LOCATION_INFO_T atiLocationInfo_s;
        BOOLEAN_T subscriberStatePresent;
        ET96MAP_ATI_SUBSCRIBER_STATE_T subscriberState_s;
        ET96MAP_ERROR_ANY_TIME_INTERROGATION_T errorAnyTimeInterrogation_s;
        ET96MAP_PROV_ERR_T provErrCode;/* Conf Only */
}ET96MAP_ANY_TIME_INTERROGATION_CONF_RSP_T;

/* Added by DON */

typedef UCHAR_T ET96MAP_ERROR_PROVIDE_SUBSCRIBER_INFO_T;

typedef struct
{
        ET96MAP_LOCAL_SSN_T localSsn;
        ET96MAP_DIALOGUE_ID_T dialogueId;
        ET96MAP_INVOKE_ID_T invokeId;
        ET96MAP_PROVIDE_LOCATION_INFO_T provideLocationInfo_s;
        BOOLEAN_T subscriberStatePresent;
        ET96MAP_ATI_SUBSCRIBER_STATE_T subscriberState_s;
        ET96MAP_ERROR_PROVIDE_SUBSCRIBER_INFO_T errorProvideSubscriberInfo;
        ET96MAP_PROV_ERR_T provErrCode;/* Conf Only */
}ET96MAP_PROVIDE_SUBSCRIBER_INFO_CONF_T;

typedef struct
{
       ET96MAP_LOCAL_SSN_T localSsn;
       ET96MAP_DIALOGUE_ID_T dialogueId;
       ET96MAP_INVOKE_ID_T invokeId;
       ET96MAP_IMSI_T imsi_s;
       ET96MAP_LMSI_T lmsi_s;
       BOOLEAN_T locationInfo;
       BOOLEAN_T subscriberState;
       BOOLEAN_T currentLocation;
}ET96MAP_PROVIDE_SUBSCRIBER_INFO_REQ_T;

#ifdef VER_MAP_R9

#define  SAIPRESENT

#else

#define  SAIPRESENT BOOLEAN_T saiPresent,

#endif

extern ET96MAP_ADDR_LINK_T *versionListHead;

#endif /* __ET96MAP_API_TYPES_H__ */
