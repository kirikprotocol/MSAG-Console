/*********************************************************************/
/* Don't remove these two lines, contain depends on them!            */
/* Document Number: %Container% Revision: %Revision% */
/*                                                                   */
/* et94inapapidef.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infocom AB 1996                                */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infocom AB. The program may be used and/or               */
/* copied only with the written permission from Ericsson Infocom AB  */
/* or in the accordance with the terms and conditions                */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                  Rev:                            */
/* 19/190 55-CAA 201 15 Ux            A                              */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: et94inapapidef.h,v 1.12 2000/05/26 11:53:28 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/P Andreas Wilde                                             */
/*                                                                   */
/* Purpose: Definitions for et94inapapi.c                            */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-96XXXX  EIN/N/B Andreas Wilde                                  */
/*                    Initial issue                                  */
/*                                                                   */
/* 01-970922 EIN/N/B Mikael Blom Ericsson Infotech AB.               */
/*      Added compile switch EINSS7_FUNC_POINTER for use of local    */
/*      naming of the functions(primitives) in the application.      */
/*                                                                   */
/* 02-980130 EIN/N/B    Mikael Blom Ericsson Infotech AB             */
/*      Added define CALLBACK_FUNC_NOT_SET                           */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*            P A R A M E T E R   C O N S T A N T S                  */
/*                                                                   */
/*********************************************************************/
#ifndef __INAPAPI_H__
#define __INAPAPI_H__
#if defined __cplusplus || defined c_plusplus
extern "C" {
#endif

/*********************************************************************/
/*                                                                   */
/*              V A R I O U S   C O N S T A N T S                    */
/*                                                                   */
/*********************************************************************/


#define MIN_SUBSYSTNBR          0x02
#define MAX_SUBSYSTNBR          0xfe
#define MIN_DIALOGUEID          0x01
#define MAX_DIALOGUEID          0xffff
#define MIN_QSRVC               0x00
#define MAX_QSRVC               0x03
#define EIN_MIN_PRIORITY        0x00
#define EIN_MAX_PRIORITY        0x03
#define MIN_DEST_ADDR_LEN       0x02
#define MIN_ORIG_ADDR_LEN       0x01

#ifdef ITU_SCCP              /* ITU ADDRESSING */                      
#define MAX_DEST_ADDR_LEN       0x12
#define MAX_ORIG_ADDR_LEN       0x12
#elif EINSS7_TWENTYFOURBITPC /* CHINESE ADDRESSING */
#define MAX_DEST_ADDR_LEN       0x14
#define MAX_ORIG_ADDR_LEN       0x14
#else                        /* ANSI ADDRESSING */
#define MAX_DEST_ADDR_LEN       0x13
#define MAX_ORIG_ADDR_LEN       0x13
#endif /* ITU_SCCP */

#define MIN_U_INFO_LEN          0x00
#define MAX_U_INFO_LEN          0x12c
#define MIN_AB_INFO_LEN         0x00
#define MAX_AB_INFO_LEN         0x7f
#define MAX_COMP_ARR_ELEMENTS   100

#define BIND_OK                 0
#define SSN_IN_USE              1
#define PROTOCOL_ERROR          2
#define RESOURCES_UNAVAIL       3
#define TCAP_NOT_READY          4
#define INAP_NOT_READY          5

#define TRANS_OK                0
#define USER_CONGESTED          2
#define PARAM_ERROR             3
#define EXECUTION_ERROR         4
#define ILLEGAL_PARAM_COMB      7
#define INFO_NOT_AVAILABLE      8
#define NO_AUTHORIZATION        12
#define PARAMETER_MISSING       13
#define INVALID_IDENTIFIER      16
#define OTHER_ERROR             100

#define BER_FALSE               0
#define BER_TRUE                1

#define INAP_OK                 0
#define NOT_END_SEQ             1

#define UNKNOWN_AC_SHORT        255

/* component offset values */

#define COMP_OFFS_INVOKE        7

/* bind result values */

#define BIND_RES_SUCCESS                        0x00
#define BIND_RES_SSN_ALREADY_IN_USE             0x01
#define BIND_RES_PRTCL_ERROR                    0x02
#define BIND_RES_RSRCS_UNVLBL                   0x03
#define BIND_RES_SSN_NT_ALLWD                   0x04
#define BIND_RES_SCCP_NT_RDY                    0x05
#define BIND_RES_USER_IN_USE                    0x06
#define BIND_RES_USERID_ERROR                   0x07 /* TR1826 HeUl 991015 */
#define BIND_RES_BIND_T_O                       0x10
#define BIND_RES_INAP_NT_RDY                    0x11
#define BIND_RES_INAP_RSRCS_TMP_UNVLBL          0x12
#define BIND_RES_INAP_RSRCS_PRMNTLY_UNVLBL      0x13
#define BIND_RES_TOO_MNY_INAP_USRS              0x14
#define BIND_RES_INAP_ABT_TO_TRMNT              0x15
#define BIND_RES_INAP_RNTLZNG                   0x16
#define BIND_RES_DND_BY_MM                      0x17

/* disconectIndicator values */

#define DSCI_DIALOGUE_RUNNING                   0x00
#define DSCI_DIALOGUE_NOT_RUNNING               0x01

/* einReason values */

#define E_RSN_INAP_NOT_USED                     0
#define E_RSN_IDLE_DIAL_TO                      1
#define E_RSN_INVALID_STATE                     2
#define E_RSN_OP_AC_INCOMP                      3
#define E_RSN_AC_NOT_SUPP                       4
#define E_RSN_DIAL_START_WRONG_OP               5
#define E_RSN_MM_INTERVENTION                   6
#define E_RSN_BAD_FORM_COMP_REC                 7
#define E_RSN_MAND_PARAM_MISSING                8
#define E_RSN_WRONG_RETURN_RESULT               9
#define E_RSN_UNKNOWN_OP                        10
#define E_RSN_RESOURCE_LIMIT                    11
#define E_RSN_REASON_NOT_OBTAIN                 12
#define E_RSN_GP_UNREC_COMP                     13
#define E_RSN_GP_MISTYPED_COMP                  14
#define E_RSN_GP_BAD_STRUCT_COMP                15
#define E_RSN_IP_DUP_INV_ID                     16
#define E_RSN_IP_UNREC_OP                       17
#define E_RSN_IP_MISTYPED_PARAM                 18
#define E_RSN_IP_RESOURCE_LIMIT                 19
#define E_RSN_IP_INIT_REL                       20
#define E_RSN_IP_UNREC_LINKID                   21
#define E_RSN_IP_LINK_RESP_UNEXP                22
#define E_RSN_IP_UNEXP_LINK_OP                  23
#define E_RSN_RRP_UNREC_INVID                   24
#define E_RSN_RRP_RETRES_UNEXP                  25
#define E_RSN_RRP_MISTYPED_PARAM                26
#define E_RSN_REP_UNREC_INVID                   27
#define E_RSN_REP_RETERR_UNEXP                  28
#define E_RSN_REP_UNREC_ERR                     29
#define E_RSN_REP_UNEXP_ERR                     30
#define E_RSN_REP_MISTYPED_PARAM                31

/* qSrvc values */

#define IN_QSRVC_NO_SEQ                         0x00
#define IN_QSRVC_IN_SEQ                         0x01
#define IN_QSRVC_NO_SEQ_R_MSG_ON_ERR            0x02
#define IN_QSRVC_IN_SEQ_R_MSG_ON_ERR            0x03


/*********************************************************************/
/*                                                                   */
/*              C O M P O N E N T   N U M B E R S                    */
/*                                                                   */
/*********************************************************************/
#define INAP_INVOKE_IND                 0x00
#define INAP_INVOKE_REQ                 0x01
#define INAP_L_CANCEL_IND               0x02
#define INAP_L_REJECT_IND               0x03
#define INAP_R_REJECT_IND               0x04
#define INAP_RESULT_L_IND               0x05
#define INAP_RESULT_L_REQ               0x06
#define INAP_U_ERROR_IND                0x07
#define INAP_U_ERROR_REQ                0x08
#define INAP_U_REJECT_REQ               0x09
#define INAP_U_REJECT_IND               0x0a

/*********************************************************************/
/*                                                                   */
/*              D E F A U L T   V A L U E S                          */
/*                                                                   */
/*********************************************************************/
#define DEFAULT_MISCCALLINFO                    0
#define DEFAULT_MONITORMODE                     1
#define DEFAULT_DISCONNECTFROMIPFORBIDDEN       TRUE
#define DEFAULT_MINIMUMNBOFDIGITS               1
#define DEFAULT_ERRORTREATMENT                  0
#define DEFAULT_INTERRUPTABLEANNIND             TRUE
#define DEFAULT_VOICEINFORMATION                FALSE
#define DEFAULT_VOICEBACK                       FALSE
#define DEFAULT_CRITICALITY                     0
#define DEFAULT_SENDCALCULATIONTOSCPINDICATION  FALSE
#define DEFAULT_DISCONNECTFROMIPFORBIDDEN       TRUE
#define DEFAULT_REQUESTANNOUNCEMENTCOMPLETE     TRUE

/*********************************************************************/
/*                                                                   */
/*              C O M P O N E N T   T Y P E S                        */
/*                                                                   */
/*********************************************************************/
#define INVOKE          0
#define LCANCEL         1
#define L_CANCEL        1
#define LREJECT         2
#define L_REJECT        2
#define RESULTL         3
#define RESULT_L        3
#define RREJECT         5
#define R_REJECT        5
#define UCANCEL         6
#define U_CANCEL        6
#define UERROR          7
#define U_ERROR         7
#define UREJECT         8
#define U_REJECT        8

/*********************************************************************/
/*                                                                   */
/*              E R R O R   C O D E S                                */
/*                                                                   */
/*********************************************************************/
#define INVALID_CHOOSER         0

/*********************************************************************/
/*                                                                   */
/*           R E Q U E S T   R E T U R N   C O D E S                 */
/*                                                                   */
/*********************************************************************/

#define REQUEST_OK              0
#define INAP_INVALID_SSN        1
#define INV_SUBSYSTNBR          1
#define INV_DIALOGUE_ID         2
#define INV_ERROR_CODE          3
#define INV_DATA_INFO_LENGTH    4
#define INV_KNOWN_AC            5
#define INV_QSRVC               6
#define INV_PRIORITY            7
#define INV_DEST_ADDR_LEN       8
#define INV_ORIG_ADDR_LEN       9
#define INV_UINFO_LEN           10
#define INV_ABORT_INFO_LEN      11
#define INV_ACSHORT             12
#define INV_COMP_PTR            13
#define INV_UINFO_PTR           14
#define IND_LENGTH_ERROR        255


/*********************************************************************/
/*                                                                   */
/*           I N D I C A T I O N   R E T U R N   C O D E S           */
/*                                                                   */
/*********************************************************************/

#define IND_UNKNOWN_CODE      254
#define IND_LENGTH_ERROR      255

/*********************************************************************/
/*                                                                   */
/*                       E R R O R   C O D E S                       */
/*                                                                   */
/*********************************************************************/
#define INAP_INVALID_SSN                        1
#define INAP_INVALID_USERID                     2
#define INAP_INVALID_DIALOGUE_ID                3
#define INAP_INVALID_ADDRESS_LEN                4


/*********************************************************************/
/*                                                                   */
/*    T A G S   F O R   B E R   E N C O D I N G / D E C O D I N G    */
/*                                                                   */
/*********************************************************************/

#define BER_BOOLEAN             0x01
#define BER_OCTET_STRING        0x04
#define BER_INTEGER             0x02
#define BER_SEQUENCE            0x30
#define BER_SET                 0x31
#define BER_ENUMERATED          0x0a
#define BER_NULL                0x05
#define BER_OBJECT_IDENT        0x06

#define CS_C_TAG                0xa0
#define CS_C_TAG_00             0xa0
#define CS_C_TAG_01             0xa1
#define CS_C_TAG_02             0xa2
#define CS_C_TAG_03             0xa3
#define CS_C_TAG_04             0xa4
#define CS_C_TAG_05             0xa5
#define CS_C_TAG_06             0xa6
#define CS_C_TAG_07             0xa7
#define CS_C_TAG_08             0xa8
#define CS_C_TAG_09             0xa9
#define CS_C_TAG_10             0xaa
#define CS_C_TAG_11             0xab
#define CS_C_TAG_12             0xac
#define CS_C_TAG_13             0xad
#define CS_C_TAG_14             0xae
#define CS_C_TAG_15             0xaf
#define CS_C_TAG_16             0xb0
#define CS_C_TAG_17             0xb1
#define CS_C_TAG_18             0xb2
#define CS_C_TAG_19             0xb3
#define CS_C_TAG_20             0xb4
#define CS_C_TAG_21             0xb5
#define CS_C_TAG_22             0xb6
#define CS_C_TAG_23             0xb7
#define CS_C_TAG_24             0xb8
#define CS_C_TAG_25             0xb9
#define CS_C_TAG_26             0xba
#define CS_C_TAG_27             0xbb
#define CS_C_TAG_28             0xbc
#define CS_C_TAG_29             0xbd
#define CS_C_TAG_30             0xbe

#define CS_P_TAG                0x80
#define CS_P_TAG_00             0x80
#define CS_P_TAG_01             0x81
#define CS_P_TAG_02             0x82
#define CS_P_TAG_03             0x83
#define CS_P_TAG_04             0x84
#define CS_P_TAG_05             0x85
#define CS_P_TAG_06             0x86
#define CS_P_TAG_07             0x87
#define CS_P_TAG_08             0x88
#define CS_P_TAG_09             0x89
#define CS_P_TAG_10             0x8a
#define CS_P_TAG_11             0x8b
#define CS_P_TAG_12             0x8c
#define CS_P_TAG_13             0x8d
#define CS_P_TAG_14             0x8e
#define CS_P_TAG_15             0x8f
#define CS_P_TAG_16             0x90
#define CS_P_TAG_17             0x91
#define CS_P_TAG_18             0x92
#define CS_P_TAG_19             0x93
#define CS_P_TAG_20             0x94
#define CS_P_TAG_21             0x95
#define CS_P_TAG_22             0x96
#define CS_P_TAG_23             0x97
#define CS_P_TAG_24             0x98
#define CS_P_TAG_25             0x99
#define CS_P_TAG_26             0x9a
#define CS_P_TAG_27             0x9b
#define CS_P_TAG_28             0x9c
#define CS_P_TAG_29             0x9d
#define CS_P_TAG_30             0x9e

#define PR_C_TAG_00             oxe0
#define PR_C_TAG_01             0xe1
#define PR_C_TAG_02             0xe2
#define PR_C_TAG_03             0xe3
#define PR_C_TAG_04             0xe4
#define PR_C_TAG_05             0xe5
#define PR_C_TAG_06             0xe6
#define PR_C_TAG_07             0xe7
#define PR_C_TAG_08             0xe8
#define PR_C_TAG_09             0xe9
#define PR_C_TAG_10             0xea
#define PR_C_TAG_11             0xeb
#define PR_C_TAG_12             oxec
#define PR_C_TAG_13             0xed
#define PR_C_TAG_14             0xee
#define PR_C_TAG_15             0xef

#define PR_P_TAG_01             0xc1
#define PR_P_TAG_02             0xc2
#define PR_P_TAG_03             0xc3
#define PR_P_TAG_04             0xc4
#define PR_P_TAG_05             0xc5
#define PR_P_TAG_06             0xc6
#define PR_P_TAG_07             0xc7
#define PR_P_TAG_08             0xc8
#define PR_P_TAG_09             0xc9
#define PR_P_TAG_10             0xca
#define PR_P_TAG_11             0xcb
#define PR_P_TAG_12             0xcc
#define PR_P_TAG_13             0xcd
#define PR_P_TAG_14             0xce
#define PR_P_TAG_15             0xcf

#ifdef EINSS7_FUNC_POINTER
/*********************************************************************/
/*                                                                   */
/*           EINSS7_FUNC_POINTER   R E T U R N   C O D E S           */
/*                                                                   */
/*********************************************************************/

#define CALLBACK_FUNC_NOT_SET   250

#endif
/*********************************************************************/
/*                                                                   */
/*          R E Q U E S T / R E S P O N S E   P R O T O T Y P E S    */
/*                                                                   */
/*********************************************************************/
 

USHORT_T E94InapAbortReq(       UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                ABORT_T         *abort_sp );

USHORT_T E94InapBeginReq(       UCHAR_T         ssn,
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

USHORT_T E94InapBindReq(        UCHAR_T         ssn, 
                                USHORT_T        senderId,
                                BOOLEAN_T       apiDoesBer );

USHORT_T E94InapCancelReq(      UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         invokeId );

USHORT_T E94InapDataReq(        UCHAR_T         ssn, 
                                USHORT_T        dialogueId, 
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                SS7_ADDRESS_T   *origAddress_sp, 
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents, 
                                COMP_T          *comp_sp);

USHORT_T E94InapEndReq(         UCHAR_T         ssn, 
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority, 
                                UCHAR_T         acShort,
                                UCHAR_T         termination, 
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents, 
                                COMP_T          *comp_sp);

USHORT_T E94InapUnBindReq(      UCHAR_T         ssn );


/*********************************************************************/
/*                                                                   */
/*        C O N F I R M A T I O N   A N D  I N D I C A T I O N       */
/*                     P R O T O T Y P E S                           */
/*                                                                   */
/*********************************************************************/

#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */
typedef USHORT_T (*E94INAPABORTIND_T)(UCHAR_T   ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                ABORT_T         *abort_sp );

typedef USHORT_T (*E94INAPPABORTIND_T)(UCHAR_T  ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         pAbortCause );

typedef USHORT_T (*E94INAPBEGININD_T)(UCHAR_T   ssn,
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

typedef USHORT_T (*E94INAPBINDCONF_T)(UCHAR_T   ssn, 
                                UCHAR_T         bindResult );

typedef USHORT_T (*E94INAPDATAIND_T)(UCHAR_T    ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

typedef USHORT_T (*E94INAPENDIND_T)(    UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         qSrvc,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                USHORT_T        uInfoLen,
                                UCHAR_T         *uInfo_p,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

typedef USHORT_T (*E94INAPERRORIND_T)(  UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         einReason,
                                BOOLEAN_T       invokeIdUsed,
                                UCHAR_T         invokeId,
                                UCHAR_T         disconnectIndicator );

typedef USHORT_T (*E94INAPNOTICEIND_T)( UCHAR_T         ssn,
                                USHORT_T        dialogueId,
                                UCHAR_T         priority,
                                UCHAR_T         acShort,
                                UCHAR_T         reportCause,
                                UCHAR_T         returnIndicator,
                                USHORT_T        relatedDialogueId,
                                USHORT_T        noOfComponents,
                                COMP_T          *comp_p );

typedef USHORT_T (*E94INAPSTATEIND_T)(  UCHAR_T         ssn,
                                UCHAR_T         userStatus,
                                UCHAR_T         affectedSsn,
                                ULONG_T         affectedSpc,
                                ULONG_T         localSpc );

typedef USHORT_T (*E94INAPINDERROR_T)(  USHORT_T        indErrorCode,
                                MSG_T           *msg_sp );

#else

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

#endif /*EINSS7_FUNC_POINTER */


USHORT_T E94InapHandleInd(      MSG_T           *msg_sp );


#ifdef EINSS7_FUNC_POINTER
  typedef struct E94INAPINIT
  {
        E94INAPABORTIND_T       E94InapAbortInd;        /* Pointer to a function */
        E94INAPPABORTIND_T      E94InapPAbortInd;
        E94INAPBEGININD_T       E94InapBeginInd;
        E94INAPBINDCONF_T       E94InapBindConf;
        E94INAPDATAIND_T        E94InapDataInd;
        E94INAPENDIND_T         E94InapEndInd;
        E94INAPERRORIND_T       E94InapErrorInd;
        E94INAPNOTICEIND_T      E94InapNoticeInd;
        E94INAPSTATEIND_T       E94InapStateInd;
        E94INAPINDERROR_T       E94InapIndError;
  }E94INAPINIT_T;

/* The application must call this function to register the call back funtions. */ 
        USHORT_T E94InapRegFunc(E94INAPINIT_T *);
#endif /* EINSS7_FUNC_POINTER */

#if defined __cplusplus || defined c_plusplus
}
#endif
#endif /* __INAPAPI_H__ */

