 #ifndef __A41MAPAPI_H__
#define __A41MAPAPI_H__
#if defined (__cplusplus) || (c_plusplus)
extern "C" {
#endif
/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* a41mapapi.h,v                                                     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
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
/* 12/190 55-CAA 201 16 Ux            C                             */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: a41mapapi.h,v 1.2 2000/05/24 13:55:11 Exp                */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/S Thomas Rosevall                                           */
/*                                                                   */
/* Purpose:                                                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*                                                                   */
/* Revision record:                                                  */
/*      95.02.03  First version   Peter Ivarsson                     */
/*                                                                   */
/*      95.02.14  EIN/S/AP Dan Liljemark                             */
/*                Minor changes after customer FS review.            */
/*                                                                   */
/*      95.03.16  EIN/S/AP Dan Liljemark                             */
/*                Updated access denied reason and cause Code        */
/*                according to API FS rev B.                         */
/*                Removed SMS delivery result and service denied     */
/*                reason.                                            */
/*                                                                   */
/*      95.11.03  EIN/N/P Dan Liljemark                              */
/*                Added support for ITU SCCP: Increased max SS7      */
/*                address length (IS41_MAX_ADDR_LENGTH) to 16.       */
/*                                                                   */
/*      96.09.03  EIN/N/P Hans Andersson                             */
/*                Adapted to IS-41-C.                                */
/*                                                                   */
/*      97.04.22  EIN/N/P Hans Andersson                             */
/*                Corrected TR 548. Changed faultyParameterUsed to   */
/*                faultyParameterLength and defined failure reason   */
/*                150 and 151. Added some failure reasons.           */
/*                                                                   */
/*      98.05.08  Anders Karlsson                                    */
/*                Added C++ compiler support(CR393).                 */
/*                                                                   */
/*      00.05.24  EIN/N/R Thomas Rosevall                            */
/*                Merged IS-41 R1 Japan, ANSI-41 OTASP R3 and        */
/*                ANSI-41 MPC R1 into this protocol layer.           */
/*                Changed names according to EIN naming convention.  */
/*                Added EINSS7_A41MapFeatreqInd,                     */
/*                EINSS7_A41MapFeatreqResp, EINSS7_A41MapReddirReq,  */
/*                EINSS7_A41MapReddiConf, EINSS7_A41MapRoutreqInd,   */
/*                EINSS7_A41MapRoutreqConf, EINSS7_A41MapRoutreqResp */
/*                EINSS7_A41MapRoutreqReq.                           */
/*                                                                   */
/*      00.11.10  EIN/N/S Thomas Rosevall                            */
/*                Added EINSS7_A41MapEsposReqInd,                    */
/*                EINSS7_A41MapEsposReqResp, EINSS7_A41MapLocReqConf,*/
/*                EINSS7_A41MapLocReqReq, EINSS7_A41MapQueryInd,     */
/*                EINSS7_A41MapQueryResp                             */
/*                                                                   */ 
/*      01.02.26  EIN/N/S Robert Gustavsson                          */
/*                Added EINSS7_A41MapPAbortInd and support for       */
/*                compiling with no api send with the                */
/*                EINSS7_NO_APISEND flag.                            */
/*                                                                   */
/* 11-20010321    Patrik Verme (einpawe)                             */
/*                Corrections of TR2322 and TR2323.                  */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*                       I N C L U D E S                             */
/*                                                                   */
/*********************************************************************/

#include        "ss7osdpn.h"
#include        "portss7.h"
#include        "ss7tmc.h"
#include        "ss7msg.h"

/*********************************************************************/
/*                                                                   */
/*         xxxxxxxxxxxxxxxxxxxxxx                                    */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*          P A R A M E T E R   C O N S T A N T S                    */
/*                                                                   */
/*********************************************************************/

/* AccessDeniedReason Used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    AccessDeniedReason  used          *//* Defined elsewere */

#define EINSS7_A41MAP_ACC_DENIED_REASON_LENGTH  1

/* Access Denied Reason */

#define EINSS7_A41MAP_ACDR_UNASS_DIRNUM  1
#define EINSS7_A41MAP_ACDR_INACTIVE  2
#define EINSS7_A41MAP_ACDR_BUSY  3
#define EINSS7_A41MAP_ACDR_TERM_DENIED  4
#define EINSS7_A41MAP_ACDR_NOPAGE_RESP 5           
#define EINSS7_A41MAP_ACDR_UNAVAILABLE 6
          
/* Action Code Used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    ActionCode  used                  *//* Defined elsewere */

#define EINSS7_A41MAP_ACTION_CODE_LENGTH  1

/* Action Code */


#define EINSS7_A41MAP_CONT_PROCESSING  1
#define EINSS7_A41MAP_DISCONNECT_CALL  2
#define EINSS7_A41MAP_DISCONNECT_CALL_LEG  3
#define EINSS7_A41MAP_CONF_CALL_DROP_LAST  4
#define EINSS7_A41MAP_BRIDGE_TO_CONF_CALL  5
#define EINSS7_A41MAP_DROP_ON_BUSY_OR_ROUT_FAIL  6
#define EINSS7_A41MAP_DISCONNECT_ALL_CALL_LEGS  7
#define EINSS7_A41MAP_ATT_MSC_OTAF  8
#define EINSS7_A41MAP_INITIATE_REGNOT  9
#define EINSS7_A41MAP_GENERATE_PUBLIC_ENCRYPTION  10
#define EINSS7_A41MAP_GENERATE_AKEY  11
#define EINSS7_A41MAP_PERFORM_SSD_UPDATE  12
#define EINSS7_A41MAP_PERFORM_REAUTHENTICATION  13
#define EINSS7_A41MAP_RELEASE_TRN  14
#define EINSS7_A41MAP_COMMIT_AKEY   15
#define EINSS7_A41MAP_RELEASE_RESOURCES  16
#define EINSS7_A41MAP_RECORD_NEWMSID  17
#define EINSS7_A41MAP_ALLOC_RESOURCES  18
#define EINSS7_A41MAP_GEN_AUTH_SIGN  19

/* Address digits                                                        */
/*                                                                       */
/* This is the address signals that comprise the address.                */
/* The Address is encoded in BCD and each octet shall contain one digit. */


#define EINSS7_A41MAP_BCD_CODE_11  11  /* Code 11     */
#define EINSS7_A41MAP_BCD_CODE_12  12  /* Code 12     */
#define EINSS7_A41MAP_BCD_STAR  13  /* *           */
#define EINSS7_A41MAP_BCD_HASH  14  /* #           */
#define EINSS7_A41MAP_BCD_ST  15  /* ST          */



/* A-Key Protocol Version Length */

#define EINSS7_A41MAP_NOT_USED  0
#define EINSS7_A41MAP_MAX_AKEY_LENGTH  2

/* A-Key Protocol Version */

#define EINSS7_A41MAP_AKEY_NOT_SUPPORTED        1
#define EINSS7_A41MAP_DIFFIE_HELLMAN_768        2
#define EINSS7_A41MAP_DIFFIE_HELLMAN_512        3
#define EINSS7_A41MAP_DIFFIE_HELLMAN_768_32BIT  4

/* Allowed Call Duration */

#define EINSS7_A41MAP_ALL_CALL_DUR_LENGTH               3



/* Announcement List Length */

#define EINSS7_A41MAP_MIN_ANNLIST_LENGTH 4
#define EINSS7_A41MAP_MAX_ANNLIST_LENGTH  35

/* Announcement List */

/* Announcement List: Announcement Code Tag  0x9F4C */

/* Announcement List: Announcement Code length  1-4 */

/* Announcement List: Announcement Code: Tone */

#define EINSS7_A41MAP_TONE_DIALTONE  0
#define EINSS7_A41MAP_TONE_RINGBACK  1
#define EINSS7_A41MAP_TONE_INTERCEPT_TONE  2
#define EINSS7_A41MAP_TONE_CONGESTION_TONE  3
#define EINSS7_A41MAP_TONE_BUSY_TONE  4
#define EINSS7_A41MAP_TONE_CONFIRM_TONE  5
#define EINSS7_A41MAP_TONE_ANSWER_TONE  6
#define EINSS7_A41MAP_TONE_CALL_WAITING_TONE  7
#define EINSS7_A41MAP_TONE_OFF_HOOK_TONE  8
#define EINSS7_A41MAP_TONE_RECALL_DIAL_TONE  17
#define EINSS7_A41MAP_TONE_BARGE_IN_TONE  18
#define EINSS7_A41MAP_TONE_TONES_OFF  63
#define EINSS7_A41MAP_TONE_PIP_TONE  192
#define EINSS7_A41MAP_TONE_ABBREV_INTERCEPT  193
#define EINSS7_A41MAP_TONE_ABBREV_CONGESTION  194
#define EINSS7_A41MAP_TONE_WARNING_TONE  195
#define EINSS7_A41MAP_TONE_DENIAL_TONE_BURST  196
#define EINSS7_A41MAP_TONE_DIAL_TONE_BURST  197
#define EINSS7_A41MAP_TONE_INCOMING_ADD_CALL_TONE  250
#define EINSS7_A41MAP_TONE_PRIORITY_ADD_CALL_TONE  251

/* Announcement List: Announcement Code: Class */

#define EINSS7_A41MAP_CLASS_CONCURRENT  0    
#define EINSS7_A41MAP_CLASS_SEQUENTIAL  1

/* Announcement List: Announcement Code: Standard Announcement */

#define EINSS7_A41MAP_SA_NONE  0
#define EINSS7_A41MAP_SA_UNAUTHORIZED_USER  1
#define EINSS7_A41MAP_SA_INVALID_ESN  2
#define EINSS7_A41MAP_SA_UNAUTHORIZED_MOBILE  3
#define EINSS7_A41MAP_SA_SUSPENDED_ORIGINATION  4
#define EINSS7_A41MAP_SA_ORIGINATION_DENIED  5
#define EINSS7_A41MAP_SA_SERVICE_AREA_DENIAL  6
#define EINSS7_A41MAP_SA_PARTIAL_DIAL  16
#define EINSS7_A41MAP_SA_REQUIRE_1PLUS  17
#define EINSS7_A41MAP_SA_REQUIRE_1PLUSNPA  18
#define EINSS7_A41MAP_SA_REQUIRE_0PLUS  19
#define EINSS7_A41MAP_SA_REQUIRE_0PLUSNPA  20
#define EINSS7_A41MAP_SA_DENY_1PLUS  21
#define EINSS7_A41MAP_SA_UNSUPPORTED_10PLUS  22
#define EINSS7_A41MAP_SA_DENY_10PLUS  23
#define EINSS7_A41MAP_SA_UNSUPPORTED_10XXX  24
#define EINSS7_A41MAP_SA_DENY_10XXX  25
#define EINSS7_A41MAP_SA_DENY_10XXX_LOCALLY  26
#define EINSS7_A41MAP_SA_REQUIRE_10PLUS  27
#define EINSS7_A41MAP_SA_REQUIRE_NPA  28
#define EINSS7_A41MAP_SA_DENY_TOLL_ORIGINATION  29
#define EINSS7_A41MAP_SA_DENY_INTERNAT_ORIG  30
#define EINSS7_A41MAP_SA_DENY_0MINUS  31
#define EINSS7_A41MAP_SA_DENY_NUMBER  48
#define EINSS7_A41MAP_SA_ALT_OPERATOR_SERVICES  49
#define EINSS7_A41MAP_SA_NO_CIRCUIT  64
#define EINSS7_A41MAP_SA_OVERLOAD  65
#define EINSS7_A41MAP_SA_INTERNAL_OFFICE_FAIL  66
#define EINSS7_A41MAP_SA_NO_WINK_RECEIVED  67
#define EINSS7_A41MAP_SA_INTEROFFICE_LINK_FAIL  68
#define EINSS7_A41MAP_SA_VACANT  69
#define EINSS7_A41MAP_SA_INVALI_PREFIX  70
#define EINSS7_A41MAP_SA_DIALING_IRREGULARITY  71
#define EINSS7_A41MAP_SA_VACANT_NUMBER  80
#define EINSS7_A41MAP_SA_DENY_TERMINATION  81
#define EINSS7_A41MAP_SA_SUSPENDED_TERMINATION  82
#define EINSS7_A41MAP_SA_CHANGED_NUMBER  83
#define EINSS7_A41MAP_SA_INACCESSIBLE_SUBSCRIBER  84
#define EINSS7_A41MAP_SA_DENY_INCOMING_TOLL  85
#define EINSS7_A41MAP_SA_ROAM_ACCESS_SCREENING  86
#define EINSS7_A41MAP_SA_REFUSE_CALL  87
#define EINSS7_A41MAP_SA_REDIRECT_CALL  88
#define EINSS7_A41MAP_SA_NO_PAGE_RESPONSE  89
#define EINSS7_A41MAP_SA_NO_ANSWER  90
#define EINSS7_A41MAP_SA_ROAM_INTERCEPT  96
#define EINSS7_A41MAP_SA_GENERAL_INFO  97
#define EINSS7_A41MAP_SA_UNRECOG_FEATURE_CODE  112
#define EINSS7_A41MAP_SA_UNAUTH_FEATURE_CODE 113
#define EINSS7_A41MAP_SA_RESTRICT_FEATURE_CODE  114
#define EINSS7_A41MAP_SA_INVALID_MOD_DIGITS  115
#define EINSS7_A41MAP_SA_SUCC_FEATURE_REG  116
#define EINSS7_A41MAP_SA_SUCC_FEATURE_DEREG 117
#define EINSS7_A41MAP_SA_SUCC_FEATURE_ACT  118
#define EINSS7_A41MAP_SA_SUCC_FEATURE_DEACT  119
#define EINSS7_A41MAP_SA_INVALID_FORWARD_TO_NO  120
#define EINSS7_A41MAP_SA_COURTESY_CALL_WARNING  121
#define EINSS7_A41MAP_SA_ENTER_PIN_SP  128
#define EINSS7_A41MAP_SA_ENTER_PIN_P  129
#define EINSS7_A41MAP_SA_REENTER_PIN_SP  130
#define EINSS7_A41MAP_SA_RENTER_PIN_P  131
#define EINSS7_A41MAP_SA_ENTER_OLD_PIN_SP  132
#define EINSS7_A41MAP_SA_ENTER_OLD_PIN_P  133
#define EINSS7_A41MAP_SA_ENTER_NEW_PIN_SP  134
#define EINSS7_A41MAP_SA_ENTER_NEW_PIN_P  135
#define EINSS7_A41MAP_SA_REENTER_NEW_PIN_SP  136
#define EINSS7_A41MAP_SA_REENTER_NEW_PIN_P  137
#define EINSS7_A41MAP_SA_ENTER_PASSWD_P  138
#define EINSS7_A41MAP_SA_DIRECT_NUM_P  139
#define EINSS7_A41MAP_SA_REENTER_DIRECT_NUM_P  140
#define EINSS7_A41MAP_SA_ENTER_FEATURE_CODE_P  141

/* P_ABORT Application response timedout*/
#define EINSS7_A41MAP_APPLICATION_RESPONSE_TO    1

/* Authentication Capability Used */

/*       FALSE   Not used                                  *//* Defined elsewere */
/*       TRUE    Authentication Capability   used          *//* Defined elsewere */

#define EINSS7_A41MAP_AUTH_CAP_LENGTH            1

/* Authentication Capability  */
                                 
#define EINSS7_A41MAP_NO_AUTHENTICATION          1  
#define EINSS7_A41MAP_AUTHENTICATION             2  

/* Authentication Data Used */

/*       FALSE   Not used                                  *//* Defined elsewere */
/*       TRUE    Authentication Data   used                *//* Defined elsewere */

#define EINSS7_A41MAP_AUTHDATA_LENGTH              3


/* Authentication Data  */


/* Authentication Response Used */

/*       FALSE   Not used                                  *//* Defined elsewere */
/*       TRUE    Authentication Response   used            *//* Defined elsewere */

#define EINSS7_A41MAP_AUTHRESP_LENGTH           3


/* Authentication Response  */


/* Authentication Response Base Station Used */

/*       FALSE   Not used                                         *//* Defined elsewere */
/*       TRUE    Authentication Response Base Station   used      *//* Defined elsewere */

#define EINSS7_A41MAP_AUTHRESP_BS_LENGTH            3


/* Authentication Response Base Station */


/* Authorization Denied used */

/*       FALSE   Not used                           *//* Defined elsewere */
/*       TRUE    Authorization Denied used          *//* Defined elsewere */

#define EINSS7_A41MAP_AUTH_DENIED_LENGTH          1

/* Authorization Denied */
                                  
#define EINSS7_A41MAP_AD_DELINQ_ACCOUNT             1  /* Delinquent account */ 
#define EINSS7_A41MAP_AD_INV_SERIAL_NUM            2 /* Invalid serial number  */
#define EINSS7_A41MAP_AD_STOLEN_UNIT                3  /* Stolen unit. */
#define EINSS7_A41MAP_AD_DUPLICATE_UNIT             4  /* Duplicate unit. */
#define EINSS7_A41MAP_AD_UNASS_DIR_NUM              5  /* Unassigned directory number.  */
#define EINSS7_A41MAP_AD_UNSPECIFIED                     6  /* Unspecified */
#define EINSS7_A41MAP_AD_MULTIPLE_ACC               7  /* Multiple access */
#define EINSS7_A41MAP_AD_NO_AUTH_FOR_MS             8  /* No Authorized for the MSC  */
#define EINSS7_A41MAP_AD_MISSING_AUTH_PARA         9  /* Missing authentication parameters. */
#define EINSS7_A41MAP_TERMTYPE_MISMATCH        10  /* TerminalType mismatch */


/* Authorization Period used  */

/*       FALSE   Not used                      *//* Defined elsewhere */ 
/*       TRUE    Authorization period used     *//* Defined elsewhere */

#define EINSS7_A41MAP_AUTHPER_LENGTH             2


/* Authorization Period  */

                            
#define EINSS7_A41MAP_PER_CALL                  1  
#define EINSS7_A41MAP_HOURS                     2   
#define EINSS7_A41MAP_DAYS                      3  
#define EINSS7_A41MAP_WEEKS                     4  
#define EINSS7_A41MAP_PER_AGREEMENT             5
#define EINSS7_A41MAP_INDEFINITE               6 
#define EINSS7_A41MAP_NUM_OF_CALLS              7  

 
/* Availability of number */

#define EINSS7_A41MAP_NUMBER_IS_AVAILABLE      0  /* Number is available     */
#define EINSS7_A41MAP_NUMBER_IS_NOT_AVAILABLE   1  /* Number is not available */


/* AvailabilityType  used  */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    AvailabilityType  used            *//* Defined elsewere */

/* AvailabilityType  */

#define EINSS7_A41MAP_UNSPEC_MS_INACT_TYPE       1   


/* Base Station Partial Key Length */

#define EINSS7_A41MAP_MAX_BSKEY_LENGTH              128


/* Base Station Partial Key */



/* Billing ID  used  */

/*       FALSE   Not used                    *//* Defined elsewere */
/*       TRUE    Billing ID  used            *//* Defined elsewere */

#define EINSS7_A41MAP_BILLING_ID_LENGTH            7  


/* Billing ID */

                            
/* Bind Status */

#define EINSS7_A41MAP_BS_SUCCESS                  0    /* Success                       */
#define EINSS7_A41MAP_BS_SSN_ALREADY_IN_USE       1    /* SSN already in use            */
#define EINSS7_A41MAP_BS_PROT_ERR                 2    /* Protocol error                */
#define EINSS7_A41MAP_BS_RES_UNAVAIL              3    /* Resources unavailable         */
#define EINSS7_A41MAP_BS_SSN_DISALLOWED           4    /* SSN not allowed               */
#define EINSS7_A41MAP_BS_SCCP_NOT_READY           5    /* SCCP not ready                */

#define EINSS7_A41MAP_BS_BIND_TIMEOUT                   16   /* Bind time-out                  */
#define EINSS7_A41MAP_BS_EINSS7_A41MAP_NOT_READY        17   /* A41MAP not ready               */
#define EINSS7_A41MAP_BS_EINSS7_A41MAP_RES_UNAVAIL_TMP  18   /* A41MAP:s resources unavailable */
#define EINSS7_A41MAP_BS_USER_ALREADY_IN_USE            19   /* User already in use            */
#define EINSS7_A41MAP_BS_TOO_MANY_SSN                   20   /* Too many A41MAP SSN            */
#define EINSS7_A41MAP_BS_EINSS7_A41MAP_TERMINATING      21   /* A41MAP about to terminate      */
#define EINSS7_A41MAP_BS_EINSS7_A41MAP_REINIT           22   /* A41MAP reinitializing          */
#define EINSS7_A41MAP_BS_TOO_MANY_USERS                 23   /* Too many A41MAP users          */
#define EINSS7_A41MAP_BS_FORMAT_ERROR                   24   
#define EINSS7_A41MAP_BS_UNSPEC_FAIL                    25

/* Bitwise defines */

#define EINSS7_A41MAP_MAX_2_BITS_VAL  3
#define EINSS7_A41MAP_MAX_4_BITS_VAL  15




/* Border Cell Access used */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    Border Cell Access  used            *//* Defined elsewere */


/* Border Cell Access */

#define EINSS7_A41MAP_BORDER_CELL_ACC             1

/* Callback Number */

#define EINSS7_A41MAP_MIN_CALLBACKNUM_ADDR_LENGTH               4
#define EINSS7_A41MAP_MAX_CALLBACKNUM_ADDR_LENGTH               20


/* Call History Count used */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    Call History Count  used            *//* Defined elsewere */

#define EINSS7_A41MAP_CALL_HIST_COUNT_LENGTH       1



/* Calling Party Name */

#define EINSS7_A41MAP_CPN_LENGTH        17

#define EINSS7_A41MAP_CPN_PS_PRESENTATION_ALLOWED       0
#define EINSS7_A41MAP_CPN_PS_PRESENTATION_RESTRICTED    1
#define EINSS7_A41MAP_CPN_PS_BLOCK_TOGGLE               2
#define EINSS7_A41MAP_CPN_PS_NO_INDICATION              3

#define EINSS7_A41MAP_CPN_AV_NAME_AVAIL                 0
#define EINSS7_A41MAP_CPN_AV_NAME_NOT_AVAIL             1

/* Calling Party Number Digits1 */

#define EINSS7_A41MAP_MIN_CALLPARTY_DGTS1_ADDR_LENGTH  4
#define EINSS7_A41MAP_MAX_CALLPARTY_DGTS1_ADDR_LENGTH  20

/* Cancellation Type  used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Cancellation Type used            *//* Defined elsewere */

/* Cancellation Type  */
                                 
#define EINSS7_A41MAP_CT_SERVING_SYS_OPT        1   /* Serving system option */
#define EINSS7_A41MAP_CT_REPORT_IN_CALL         2   /* Report in call   */
#define EINSS7_A41MAP_CT_DISCONTINUE            3   /* Discontinue  */


/*  Channel Data  used  */

/*       FALSE   Not used                   *//* Defined elsewere */
/*       TRUE    Channel Data used          *//* Defined elsewere */

#define EINSS7_A41MAP_CDATA_LENGTH               3

/* Company ID */

#define EINSS7_A41MAP_MIN_COMP_ID_LENGTH            1
#define EINSS7_A41MAP_MAX_COMP_ID_LENGTH            15



/* Control Channel Data  used */

/*       FALSE   Not used                           *//* Defined elsewere */
/*       TRUE    Control Channel Data used          *//* Defined elsewere */

#define EINSS7_A41MAP_CC_DATA_LENGTH              4  

/* Control Channel Data  */                                     


/* Denied Authorization Period used */

/*       FALSE   Not used                                  *//* Defined elsewere */
/*       TRUE    Denied Authorization Period used          *//* Defined elsewere */

#define EINSS7_A41MAP_DEN_AUTH_PER_LENGTH        2


/* Denied Authorization Period */

                                       
/* #define EINSS7_A41MAP_PER_CALL                  1 *//* Defined elsewere */
/* #define EINSS7_A41MAP_HOURS                     2 *//* Defined elsewere */ 
/* #define EINSS7_A41MAP_DAYS                      3 *//* Defined elsewere */
/* #define EINSS7_A41MAP_WEEKS                     4 *//* Defined elsewere */
/* #define EINSS7_A41MAP_PER_AGREEMENT             5 *//* Defined elsewere */
/* 6  reserved */  
/* #define EINSS7_A41MAP_NUM_OF_CALLS              7 *//* Defined elsewere */ 
 
#define EINSS7_A41MAP_MINUTES                      8  

/* Deny Access used */

/*       FALSE   Not used                                  *//* Defined elsewere */
/*       TRUE    Deny Access used                          *//* Defined elsewere */

#define EINSS7_A41MAP_DENY_ACC_LENGTH              1

/* Deny Access */

#define EINSS7_A41MAP_DA_UNSPECIFIED                   1
#define EINSS7_A41MAP_DA_SSD_UPD_FAIL                 2
#define EINSS7_A41MAP_DA_COUNT_UPD_FAIL               3
#define EINSS7_A41MAP_DA_UNIQ_CHALL_FAIL             4
#define EINSS7_A41MAP_DA_AUTHR_MISMATCH               5
#define EINSS7_A41MAP_DA_COUNT_MISMATCH               6
#define EINSS7_A41MAP_DA_PROCESS_COLLISION    7
#define EINSS7_A41MAP_DA_MISSING_AUTH_PARA    8
#define EINSS7_A41MAP_DA_TERMTYPE_MISMATCH        9
#define EINSS7_A41MAP_DA_MIN_ESN_AUTH_FAIL   10


/* Deregistration Type used */

/*       FALSE   Not used                                  *//* Defined elsewere */
/*       TRUE    Deregistration Type used                  *//* Defined elsewere */

/* Deregistration Type */

#define EINSS7_A41MAP_DEREG_UNSPEC                  1
#define EINSS7_A41MAP_DEREG_ADMIN                  2
#define EINSS7_A41MAP_DEREG_POWER_DOWN             3

/*Destination address*/

#define EINSS7_A41MAP_MIN_DEST_ADDR_LENGTH      4  
#define EINSS7_A41MAP_MAX_DEST_ADDR_LENGTH     12 

/* DMH Redirection Indicator */

#define EINSS7_A41MAP_DMH_REDIR_IND_LENGTH              1

#define EINSS7_A41MAP_DMH_NOT_SPECIFIED         0
#define EINSS7_A41MAP_DMH_CFU                   1
#define EINSS7_A41MAP_DMH_CFB                   2
#define EINSS7_A41MAP_DMH_CFNA                  3
#define EINSS7_A41MAP_DMH_CFO                   4
#define EINSS7_A41MAP_DMH_CD_UNSPEC             5
#define EINSS7_A41MAP_DMH_CD_PSTN               6
#define EINSS7_A41MAP_DMH_CD_PRIVATE            7
#define EINSS7_A41MAP_DMH_PSTN_TANDEM           8
#define EINSS7_A41MAP_DMH_PRIVATE_TANDEM        9
#define EINSS7_A41MAP_DMH_BUSY                  10
#define EINSS7_A41MAP_DMH_INACTIVE              11
#define EINSS7_A41MAP_DMH_UNASSIGNED            12
#define EINSS7_A41MAP_DMH_TERM_DENIED           13
#define EINSS7_A41MAP_DMH_CD_FAILURE            14
#define EINSS7_A41MAP_DMH_ECT                   15
#define EINSS7_A41MAP_DMH_MAH                   16
#define EINSS7_A41MAP_DMH_FA                    17
#define EINSS7_A41MAP_DMH_ABAND_CALL_LEG        18
#define EINSS7_A41MAP_DMH_PCA_CALL_REFUSED      19
#define EINSS7_A41MAP_DMH_SCA_CALL_REFUSED      20
#define EINSS7_A41MAP_DMH_DIALOGUE              21
#define EINSS7_A41MAP_DMH_CFD                   22
#define EINSS7_A41MAP_DMH_CD_LOCAL              23
#define EINSS7_A41MAP_DMH_VOICE_MAIL_RETR       24


/* DMH Service ID */

#define EINSS7_A41MAP_DMH_SERVID_LENGTH 10


/* Digits (dialed) */
#define EINSS7_A41MAP_MIN_DIGITS_ADDR_LENGTH      4  
#define EINSS7_A41MAP_MAX_DIGITS_ADDR_LENGTH     20  



/* Encoding */

#define EINSS7_A41MAP_BCD                1
#define EINSS7_A41MAP_IA5                2
#define EINSS7_A41MAP_OCTET_STRING       3


/* ESN used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    ESN used                          *//* Defined elsewere */

#define EINSS7_A41MAP_ESN_LENGTH                   4

/* ESRD Digits */

#define EINSS7_A41MAP_MIN_ESRD_DIGITS_ADDR_LENGTH               4
#define EINSS7_A41MAP_MAX_ESRD_DIGITS_ADDR_LENGTH               20




/* Error Code */

#define EINSS7_A41MAP_ESP_SYS_FAIL	 	1   /* System Failure (ESP), einpawe 20010410 */
#define EINSS7_A41MAP_ESP_UNAUTH_REQ		2   /* Unauthorized Request (ESP), einpawe 20010410 */
#define EINSS7_A41MAP_ESP_UNEXP_VALUE		3   /* Unexpected Data Value (ESP), einpawe 20010410 */
#define EINSS7_A41MAP_ESP_UNREC_KEY		4   /* Unrecognized Key (ESP), einpawe 20010410 */

#define EINSS7_A41MAP_UNREC_MIN                 129  /* Unrecognized MIN    */
#define EINSS7_A41MAP_UNREC_ESN                 130  /* Unrecognized ESN    */
#define EINSS7_A41MAP_MIN_HLR_MIS               131  /* MIN/HLR mismatch             */
#define EINSS7_A41MAP_OP_SEQ_PROBLEM            132  /* Operation sequence problem   */
#define EINSS7_A41MAP_RESOURCE_SHORT            133  /* Resource shortage            */
#define EINSS7_A41MAP_OP_NOT_SUPP               134  /* Operation not supported      */
#define EINSS7_A41MAP_TRUNK_UNAV                135  /* Trunk unavailable            */
#define EINSS7_A41MAP_PARAM_ERR                 136  /* Parameter error              */
#define EINSS7_A41MAP_SYS_FAIL                  137  /* System failure               */
#define EINSS7_A41MAP_UNREC_PARAM               138  /* Unrecognized parameter value */
#define EINSS7_A41MAP_FEAT_INACTIVE             139  /* Feature inactive             */
#define EINSS7_A41MAP_MISS_PARAM                140  /* Missing parameter            */          
#define EINSS7_A41MAP_UNREC_IMSI                141  /* Unrecognized IMSI */

/* ESME Identification */

#define EINSS7_A41MAP_MIN_ESME_ID_LENGTH            1
#define EINSS7_A41MAP_MAX_ESME_ID_LENGTH            15



/* ESRD length  */

#define EINSS7_A41MAP_MIN_ESRD_LENGTH    4  /* ESRD min length */
#define EINSS7_A41MAP_MAX_ESRD_LENGTH   12  /* ESRD max length */


/* Extended MSC ID used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Extended MSC ID used              *//* Defined elsewere */

#define EINSS7_A41MAP_EXT_MSCID_LENGTH             4

/* Extended MSC ID */

/* See PC SSN */

/* Extra Opt Parameter */

#define EINSS7_A41MAP_MAX_OPTPARAM_LENGTH     250


/* Failure reason */

#define EINSS7_A41MAP_UNRECOGNIZED_MIN         129  /* Unrecognized MIN    */
#define EINSS7_A41MAP_UNRECOGNIZED_ESN         130  /* Unrecognized ESN    */
#define EINSS7_A41MAP_MSID_HLR_MISMATCH        131  /* MSID/HLR mismatch             */
#define EINSS7_A41MAP_SEQUENCE_PROBLEM         132  /* Operation sequence problem   */
#define EINSS7_A41MAP_RESOURCE_SHORTAGE        133  /* Resource shortage            */
#define EINSS7_A41MAP_NOT_SUPPORTED            134  /* Operation not supported      */
#define EINSS7_A41MAP_TRUNK_UNAVAILABLE        135  /* Trunk unavailable            */
#define EINSS7_A41MAP_PARAMETER_ERROR          136  /* Parameter error              */
#define EINSS7_A41MAP_SYSTEM_ERROR             137  /* System failure               */
#define EINSS7_A41MAP_PARAMETER_VALUE          138  /* Unrecognized parameter value */
#define EINSS7_A41MAP_FEATURE_INACTIVE         139  /* Feature inactive             */
#define EINSS7_A41MAP_MISSING_PARAMETER        140  /* Missing parameter            */
#define EINSS7_A41MAP_TIMEOUT_FROM_REMOTE_NODE 141  /* Time-out from Remote Node    */
#define EINSS7_A41MAP_PRIMITIVE_FORMAT_ERROR   144  /* Format error in user primitive */
#define EINSS7_A41MAP_FAILURE_REASON_NOT_OBTAINABLE 145
#define EINSS7_A41MAP_INVALID_STATE            146  /* Invalid state                */
#define EINSS7_A41MAP_MM_INTERVENTION          149  /* MM intervention              */

#define EINSS7_A41MAP_INCORRECT_PARAMETER               151  /* An invalid parameter has been */
                                                     /* received at the remote end.   */
#define EINSS7_A41MAP_UNREC_COMPONENT                   152  /* Unrecognized component */
#define EINSS7_A41MAP_INCORR_COMPONENT_PORTION          153  /* Incorrect Component portion.   */
#define EINSS7_A41MAP_BAD_STRUCT_COMPONENT_PORTION      154  /* Badly structured component portion. */
#define EINSS7_A41MAP_MAX_CP_LENGTH_EXEEDED             155  /* Max CP length exeeded. */
#define EINSS7_A41MAP_DIALOGUE_MAX_EXEEDED              156  /* Dialogue max exeeded.  */
#define EINSS7_A41MAP_IS_BLOCKED                        157  /* Is blocked.            */
#define EINSS7_A41MAP_DUPLICATE_INVOKE_ID               158  /* Duplicate invoke ID.   */
#define EINSS7_A41MAP_UNREC_CORRELATION_ID              159  /* Unrecognized correlation ID. */
#define EINSS7_A41MAP_UNREC_TRANSACTION_TYPE            160  /* Unrecognized transaktion type. */
#define EINSS7_A41MAP_INCORRECT_TRANSACTION_PORTION     161  /* Incorrect transaction portion. */
#define EINSS7_A41MAP_BAD_STRUCT_TRANSACTION_PORTION    162  /* Badly structured transaction portion.*/
#define EINSS7_A41MAP_UNREC_TRANSACTION_ID              163  /* Unrecognized transaction ID. */
#define EINSS7_A41MAP_PERMISSION_TO_RELEASE             164  /* Permission to release. */
#define EINSS7_A41MAP_RESOURCE_UNAVAILABLE              165  /* Resource unavailable.  */
#define EINSS7_A41MAP_NOT_CONF_FOR_THIS_FUNC            168  /* Not configured for OTASP functionality. */
#define EINSS7_A41MAP_INCORR_PARAM_RECVD_IN_RET_RESULT  169  /* Incorr. param in result. */
#define EINSS7_A41MAP_UNRECOGNIZED_IMSI                 170  /* Unrecognized IMSI. */
#define EINSS7_A41MAP_COMPONENT_PROBLEM                 171  /* Component problem */




/* Faulty parameter used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Faulty parameter used             *//* Defined elsewere */

/* Faulty parameter */

#define EINSS7_A41MAP_MIN_PARA_CODE_LENGTH    1          
#define EINSS7_A41MAP_MAX_PARA_CODE_LENGTH    3

/* Feature Result */

#define EINSS7_A41MAP_UNSUCCESS          1
#define EINSS7_A41MAP_SUCCESS                    2

/* Generalized Time */

#define EINSS7_A41MAP_GENTIME_LENGTH            6


/* GeodeticLocation */

/* GeodeticLocation:Type of shape */
#define EINSS7_A41MAP_GEO_ELLIPSOID_POINT                0  /* Ellipsoid point */
#define EINSS7_A41MAP_GEO_UNCERTAIN_ELLIPSOID_POINT      1  /* Ellipsoid point with uncertainty */
#define EINSS7_A41MAP_GEO_UNCERTAIN_ALTITUDE_POINT       2  /* Point with altitude and uncertainty */
#define EINSS7_A41MAP_GEO_POLYGON                        5  /* Polygon */

/* GeodeticLocation:Location Presentation Restricted Indicator */
#define EINSS7_A41MAP_GEO_PRESENT_ALLOWED                0  /* Presentation allowed */
#define EINSS7_A41MAP_GEO_PRESENT_RESTRICTED             1  /* Presentation restricted */
#define EINSS7_A41MAP_GEO_LOCATION_NOT_AVAIL             2  /* Location not available */

/* GeodeticLocation:Screening Indicator */
#define EINSS7_A41MAP_GEO_NOT_VERIFIED                   0  /* User provided, not verified */
#define EINSS7_A41MAP_GEO_VERIFIED_PASSED                1  /* user provided, verified and passed */
#define EINSS7_A41MAP_GEO_VERIFIED_FAILED                2  /* User provided, verified and failed */
#define EINSS7_A41MAP_GEO_NETWORK_PROVIDED               3  /* Network provided */

/* Geographic Position */

#define EINSS7_A41MAP_MIN_GEOPOS_LENGTH         8
#define EINSS7_A41MAP_MAX_GEOPOS_LENGTH         10

/* Global Title */

#define EINSS7_A41MAP_GT_LENGTH         20



/* IMSI, New IMSI, MS IMSI length  */


#define EINSS7_A41MAP_MAX_IMSI_LENGTH   8  /* Maximal IMSI length */
#define EINSS7_A41MAP_MIN_IMSI_LENGTH   1  /* Minimal IMSI length   */
#define EINSS7_A41MAP_MAX_MSIMSI_LENGTH   8  /* Maximal MSIMSI length   */
#define EINSS7_A41MAP_MIN_MSIMSI_LENGTH   1  /* Minimal  MS IMSI length   */ 
#define EINSS7_A41MAP_MAX_NEWIMSI_LENGTH   8  /* Maximal  New IMSI length   */
#define EINSS7_A41MAP_MIN_NEWIMSI_LENGTH   1  /* Minimal  New IMSI length   */  

/* IMSI, New IMSI, MS IMSI */

#define EINSS7_A41MAP_BCD_ZERO      0   /* 0           */
#define EINSS7_A41MAP_BCD_ONE       1   /* 1           */
#define EINSS7_A41MAP_BCD_TWO       2   /* 2           */
#define EINSS7_A41MAP_BCD_THREE     3   /* 3           */
#define EINSS7_A41MAP_BCD_FOUR      4   /* 4           */
#define EINSS7_A41MAP_BCD_FIVE      5   /* 5           */
#define EINSS7_A41MAP_BCD_SIX       6   /* 6           */
#define EINSS7_A41MAP_BCD_SEVEN     7   /* 7           */
#define EINSS7_A41MAP_BCD_EIGHT     8   /* 8           */
#define EINSS7_A41MAP_BCD_NINE      9   /* 9           */
#define EINSS7_A41MAP_BCD_FILLER   15   /* Filler octet */



/* ISPOS Request Indicators used */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    Request indicators used             *//* Defined elsewere */

#define EINSS7_A41MAP_ISPOSREQ_IND_LENGTH                2

/* ISPOS Request Indicators */


#define EINSS7_A41MAP_POSREQ_INIT                        0  /* Initial Position Requested */
#define EINSS7_A41MAP_POSREQ_UPDATE                      1  /* Updated Position Requested */

#define EINSS7_A41MAP_POSREQ_NOT_REQ                     0  /* Position is not Requested */
#define EINSS7_A41MAP_POSREQ_REQ                         1  /* Position is Requested */

#define EINSS7_A41MAP_POSREQ_NOTIFICATION_NOT_REQ        0  /* Notification is not Requested */
#define EINSS7_A41MAP_POSREQ_NOTIFICATION_REQ            1  /* Notification is Requested */



/* Length of Address */

#define EINSS7_A41MAP_MIN_SMS_ADDR_LENGTH   4   /* Min SMS address length */
#define EINSS7_A41MAP_MAX_SMS_ADDR_LENGTH   12  /* Max SMS address length */

/* Length of address */

#define EINSS7_A41MAP_MAX_SUBADDR_LENGTH   20  /* Max length of subaddress */


/* Location Area ID used */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    Location Area Id used               *//* Defined elsewere */



/* Location Area ID */



/* Message Indication Flag */

/*       FALSE  No outstanding messages                  *//* Defined elsewere */
/*       TRUE   Outstanding messages exist               *//* Defined elsewere */


/* Message Waiting Notification Count */

#define EINSS7_A41MAP_MIN_MWNC_LENGTH           2  /* TR2322, einpawe 20010321 */ 

/* Message Waiting Notification Count used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    MWNC used                         *//* Defined elsewere */
 

/* MIN, New MIN, MS MIN Used */

/*       FALSE  Not used                                *//* Defined elsewere */
/*       TRUE   MIN, New MIN, MS MIN used               *//* Defined elsewere */


/* MIN, New MIN, MS MIN */

#define EINSS7_A41MAP_MIN_LENGTH   5  /* Mobile identification number - MIN length */
#define EINSS7_A41MAP_MSMIN_LENGTH   5  /* MS Mobile identification number - MSMIN length */
#define EINSS7_A41MAP_NEWMIN_LENGTH   5  /* New Mobile identification number - NEWMIN length */

/* Miscellaneous Information */

#define EINSS7_A41MAP_MISCINFO_LENGTH                   2

#define EINSS7_A41MAP_MISCINFO_NORMAL_CHARGING          0
#define EINSS7_A41MAP_MISCINFO_COLLECT_CALL_CHARGING    1

/* Mobile Call Status */

#define EINSS7_A41MAP_MOBCALL_STAT_LENGTH               1

/* Mobile Directory Number length  */


#define EINSS7_A41MAP_MIN_MDN_ADDR_LENGTH        4  /* Mobile directory number min address length */
#define EINSS7_A41MAP_MAX_MDN_ADDR_LENGTH        12  /* Mobile directory number max address length */



/* Mobile Station Partial Key Length */

#define EINSS7_A41MAP_MAX_MSKEY_LENGTH                128

/* Mobile Station Partial Key */


/* Modulus value length */

#define EINSS7_A41MAP_MAX_MODVAL_LENGTH            96

/* Modulus value */


/* MPAccurracy used */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    MPAccurracy used                    *//* Defined elsewere */

#define EINSS7_A41MAP_MPACC_LENGTH               1


/* MPAccurracy */


/* MSCID */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    MSCID used                        *//* Defined elsewere */

#define EINSS7_A41MAP_MSCID_LENGTH               3  

/*  MSCID NUMBER */

#define EINSS7_A41MAP_MIN_MSCID_NUM_ADDR_LENGTH         4
#define EINSS7_A41MAP_MAX_MSCID_NUM_ADDR_LENGTH         20                                     
                                  


/* Numbering plan */

#define EINSS7_A41MAP_UNKNOWN_OR_NOT_APPLICABLE     0
#define EINSS7_A41MAP_ISDN_NUMBERING                1
#define EINSS7_A41MAP_TELEPHONY_NUMBERING           2
#define EINSS7_A41MAP_DATA_NUMBERING                3
#define EINSS7_A41MAP_TELEX_NUMBERING               4
#define EINSS7_A41MAP_MARTIME_MOBILE_NUMBERING      5
#define EINSS7_A41MAP_LAND_MOBILE_NUMBERING         6
#define EINSS7_A41MAP_PRIVATE_NUMBERING_PLAN        7
#define EINSS7_A41MAP_ANSI_SS7_PC_AND_SSN           13 /* ANSI SS7 Point Code (PC) and */
                                               /* Subsystem Number (SSN)       */
#define EINSS7_A41MAP_IP_ADDRESS                    14


/* Odd / even indicator */

#define EINSS7_A41MAP_EVEN_NUMBER   0  /* Even number of address signals */
#define EINSS7_A41MAP_ODD_NUMBER    1  /* Odd number of address signals  */

/* One Time Feature Indicator used */

/*       FALSE   Not used                                             *//* Defined elsewere */
/*       TRUE    OneTimeFeatureIndicator used                         *//* Defined elsewere */

#define EINSS7_A41MAP_ONE_TIME_FEAT_IND_LENGTH                   2

/* One Time Feature Indicator */

/* origDestAddr length */

#define EINSS7_A41MAP_MIN_ORIG_DEST_ADDR_LENGTH      4  
#define EINSS7_A41MAP_MAX_ORIG_DEST_ADDR_LENGTH     12 


/* Origination/Destination Address length  */

#define EINSS7_A41MAP_MIN_ADDR_LENGTH    3   /* Min Orig or Dest address length */
#define EINSS7_A41MAP_MAX_ADDR_LENGTH   20   /* Max Orig or Dest address length */





/* Origination Indicator  used */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    Originating Indicator used          *//* Defined elsewere */

#define EINSS7_A41MAP_ORIG_IND_LENGTH                    1

 
/* Origination Indicator  */
                              
#define EINSS7_A41MAP_OI_PRIOR_AGREEMENT         1  /* Prior agreement */
#define EINSS7_A41MAP_OI_ORIG_DENIED             2  /* Origination denied  */ 
#define EINSS7_A41MAP_OI_LOCAL_ONLY              3  /* Local calls only */ 
#define EINSS7_A41MAP_OI_SEL_NPA                 4  /* Selected NPA-NXX  */
#define EINSS7_A41MAP_OI_SEL_NPA_LOCAL           5  /* Selected NPA-NXX and local calls only  */
#define EINSS7_A41MAP_OI_WORLD_ZONE_1            6  /* World Zone 1  */ 
#define EINSS7_A41MAP_OI_INTERNATIONAL_CALLS     7  /* International calls */
#define EINSS7_A41MAP_OI_SINGLE_NPA              8  /* Single NPA-NXX-XXXX  */

/* origOrigAddr length */

#define EINSS7_A41MAP_MIN_ORIG_ORIG_ADDR_LENGTH      4  
#define EINSS7_A41MAP_MAX_ORIG_ORIG_ADDR_LENGTH     12 


/* Origination Triggers  length  */

/*        0     EINSS7_A41MAP_NOT_USED                      *//* Defined elsewere */
/*        1-4   Origination Triggers length          *//* Defined elsewere */

/*   Origination Triggers  */

#define EINSS7_A41MAP_MIN_ORIG_TRIGG_LENGTH      1
#define EINSS7_A41MAP_MAX_ORIG_TRIGG_LENGTH      4


/* OTASP Result Code used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    OTASP Result Code used            *//* Defined elsewere */

/* OTASP Result Code */

#define EINSS7_A41MAP_OTASP_SUCCESS                 0
#define EINSS7_A41MAP_OTASP_REJECT                  1
#define EINSS7_A41MAP_OTASP_COMP_FAIL               2
#define EINSS7_A41MAP_OTASP_CSC_REJECT              3
#define EINSS7_A41MAP_OTASP_UNRECOG_CALL_ENTRY      4
#define EINSS7_A41MAP_OTASP_UNSUPPORTED_AKEY_VER    5
#define EINSS7_A41MAP_OTASP_UNABLE_TO_COMMIT        6


/* PC_SSN  used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    PC_SSN  used                      *//* Defined elsewere */

#define EINSS7_A41MAP_PC_SSN_LENGTH              5

/* PC_SSN  */

#define EINSS7_A41MAP_TYPE_SERVING_MSC          1
#define EINSS7_A41MAP_TYPE_HOME_MSC             2
#define EINSS7_A41MAP_TYPE_GATE_MSC             3
#define EINSS7_A41MAP_TYPE_HLR                  4
#define EINSS7_A41MAP_TYPE_VLR                  5
#define EINSS7_A41MAP_TYPE_EIR                  6
#define EINSS7_A41MAP_TYPE_AC                   7
#define EINSS7_A41MAP_TYPE_BORDER_MSC           8
#define EINSS7_A41MAP_TYPE_ORIG_MSC             9

                                   
 

/* Position cancellation reason */


#define EINSS7_A41MAP_PCR_UNSPEC                 1  /* Unspecified */
#define EINSS7_A41MAP_PCR_INTER                  2  /* Inter-system Handoff */
#define EINSS7_A41MAP_PCR_INTRA                  3  /* Intra-system Handoff */
#define EINSS7_A41MAP_PCR_DISCONNECT             4  /* Call Disconnection */



/* Position Cancellation Result */


#define EINSS7_A41MAP_PCRES_SUCCESFUL                    1  /* Succesful */
#define EINSS7_A41MAP_PCRES_UNKNOWN_SUBSCR               2  /* Not Succesful - Unknown Subscriber */
#define EINSS7_A41MAP_PCRES_GEOLOC_AVAIL                 3  /* Not Succesful - GeoLocation Available */


/* Position Determination Result */

/* Position Determination Result:PosDetResult */


#define EINSS7_A41MAP_PDR_SUCCESFUL              1  /* Succesful */
#define EINSS7_A41MAP_PDR_NOT_SUCCESFUL          2  /* Not Succesful */
#define EINSS7_A41MAP_PDR_ABORTED               3  /* Aborted */


/* PositionRequestReport:PositionRequestResult */

     
#define EINSS7_A41MAP_PRR_MOBINFO                1  /* Succesful MobInfo */
#define EINSS7_A41MAP_PRR_GEOLOC                 2  /* Succesful GeographicLocation */
#define EINSS7_A41MAP_PRR_NOT                    3  /* Not Succesful */

/* PositionRequestReport:PositionRequestFailureReason */

#define EINSS7_A41MAP_NOT_SPECIFIED                      0  /* Not Specified */
#define EINSS7_A41MAP_PRFR_AUTH_FAILURE          1  /* Authorisation Failure */
#define EINSS7_A41MAP_PRFR_NO_FUNC_SUPP          2  /* Function not supported */
#define EINSS7_A41MAP_PRFR_UNRECOG_MDN                   3  /* Unrecognized MDN */
#define EINSS7_A41MAP_PRFR_UNRECOG_MSID          4  /* Unrecognized MSID */
#define EINSS7_A41MAP_PRFR_NO_INTER_HANDOFF              5  /* Intersystem Handoff not supported */
#define EINSS7_A41MAP_PRFR_NO_REQTYPE_MATCH              6  /* MS state doesn't match RequestType */
#define EINSS7_A41MAP_PRFR_DISCONNECT                    7  /* Call Disconnected */
#define EINSS7_A41MAP_PRFR_ONGOING_POSPROC               8  /* Ongoing Positioning Process */
#define EINSS7_A41MAP_PRFR_RES_SHORTAGE          9  /* Resource Shortage */


/* PositionRequestType used */

/*       FALSE   Not used                                         *//* Defined elsewere */
/*       TRUE    Position Request Type  used                      *//* Defined elsewere */


/* PositionRequestType */

#define EINSS7_A41MAP_PRT_UNSPEC           1  /* Unspecified */
#define EINSS7_A41MAP_PRT_EMERG_SERVICE      2  /* Emergency service */
#define EINSS7_A41MAP_PRT_HOMEZONE_BILL      3  /* Home Zone Billing */

/* PositionRequestType ESPOS */

#define EINSS7_A41MAP_POSRT_INITIAL                     1  
#define EINSS7_A41MAP_POSRT_UPDATED                     2  
#define EINSS7_A41MAP_POSRT_UPDATED_OR_LAST_KNOWN       3 
#define EINSS7_A41MAP_POSRT_TEST                        4

/* Position Result */

#define EINSS7_A41MAP_PR_INIT_POS_RETURNED              1
#define EINSS7_A41MAP_PR_UPDATED_POS_RETURNED           2
#define EINSS7_A41MAP_PR_LAST_KNOWN_POS_RETURNED        3
#define EINSS7_A41MAP_PR_REQ_POS_NOT_AVAIL              4
#define EINSS7_A41MAP_PR_CALLER_DISCONNECTED            5
#define EINSS7_A41MAP_PR_CALLER_HANDED_OFF              6
#define EINSS7_A41MAP_PR_INACTIVE                       7
#define EINSS7_A41MAP_PR_UNRESPONSIVE                   8
#define EINSS7_A41MAP_PR_REFUSED                        9
#define EINSS7_A41MAP_PR_TEST                           10



/* Position Source */

#define EINSS7_A41MAP_POS_SOURCE_LENGTH         1

#define EINSS7_A41MAP_PS_UNKNOWN                        0
#define EINSS7_A41MAP_PS_NW_UNSPECIFIED                 1
#define EINSS7_A41MAP_PS_NW_AOA                         2
#define EINSS7_A41MAP_PS_NW_TOA                         3
#define EINSS7_A41MAP_PS_NW_TDOA                        4
#define EINSS7_A41MAP_PS_NW_RF_FINGERPRINTING           5
#define EINSS7_A41MAP_PS_NW_CELL_SECTOR                 6
#define EINSS7_A41MAP_PS_NW_CELL_SECTOR_WITH_TIMING     7
#define EINSS7_A41MAP_PS_HS_UNSPECIFIED                 16
#define EINSS7_A41MAP_PS_HS_GPS                         17
#define EINSS7_A41MAP_PS_HS_AGPS                        18
#define EINSS7_A41MAP_PS_HS_EOTD                        19
#define EINSS7_A41MAP_PS_HS_AFLT                        20



/* Presentation of number */

#define EINSS7_A41MAP_PRESENTATION_ALLOWED      0  /* Presentation allowed    */
#define EINSS7_A41MAP_PRESENTATION_RESTRICTED   1  /* Presentation restricted */


/* Primitive value length */

#define EINSS7_A41MAP_MAX_PRIMVAL_LENGTH           20

/* Primitive Value */


/* Priority */

#define EINSS7_A41MAP_MAX_PRIORITY                       3

#define EINSS7_A41MAP_PRI_HIGH_O                         0
#define EINSS7_A41MAP_PRI_HIGH_1                         1
#define EINSS7_A41MAP_PRI_HIGH_2                         2
#define EINSS7_A41MAP_PRI_HIGH_3                         3

/* Importants priority */
#define EINSS7_A41MAP_IMP_HIGH_4                         4
#define EINSS7_A41MAP_IMP_HIGH_5                         5
#define EINSS7_A41MAP_IMP_HIGH_6                         6
#define EINSS7_A41MAP_IMP_HIGH_7                         7

/* Provider of number */

#define EINSS7_A41MAP_USER_PROVIDED_NOT_SCREENED       0 
#define EINSS7_A41MAP_USER_PROVIDED_SCREENING_PASSED   1 
#define EINSS7_A41MAP_USER_PROVIDED_SCREENING_FAILED   2
#define EINSS7_A41MAP_NETWORK_PROVIDED                 3

/* Qualification Information Code */

#define EINSS7_A41MAP_QIC_NO_INFO                1
#define EINSS7_A41MAP_QIC_VALID          2
#define EINSS7_A41MAP_QIC_VALID_PROFILE  3
#define EINSS7_A41MAP_QIC_PROFILE                4


/* Quality of Service */

#define EINSS7_A41MAP_MAX_QUALITY_OF_SERVICE             3

#define EINSS7_A41MAP_QOS_NOT_USED                       0
#define EINSS7_A41MAP_QOS_SEQ_DEL_OPT                        1
#define EINSS7_A41MAP_QOS_MSG_RET_OPT                        2
#define EINSS7_A41MAP_QOS_ALL_SERVICE                        3

/* Query Type */

#define EINSS7_A41MAP_QT_PRECALL_ORIG_QUERY             1
#define EINSS7_A41MAP_QT_PRECALL_TERM_QUERY             2
#define EINSS7_A41MAP_QT_POSTCALL_REL_QUERY             3
#define EINSS7_A41MAP_QT_PRECALL_TERM_TRP               4
#define EINSS7_A41MAP_QT_PRECALL_TERM_CPP               5



/* Random Variable used */

/*       FALSE   Not used                                         *//* Defined elsewere */
/*       TRUE    Random Variable  used                            *//* Defined elsewere */

#define EINSS7_A41MAP_RAND_VAR_LENGTH                    4

/* Random Variable */


/* Random Variable Base Station used */

/*       FALSE   Not used                                         *//* Defined elsewere */
/*       TRUE    Random Variable Base Station  used               *//* Defined elsewere */

#define EINSS7_A41MAP_RAND_VARBS_LENGTH                  4

/* Random Variable */


/* Received Signal Quality used  */

/*       FALSE   Not used                              *//* Defined elsewere */
/*       TRUE    Received Signal Quality used          *//* Defined elsewere */

/* Received Signal Quality */    
                             
#define EINSS7_A41MAP_RSQ_NOT_USABLE             0    /* Not a usable signal */
                                              /* 9-245  Usable signal range */
#define EINSS7_A41MAP_RSQ_INTERFERENCE          255   /* Interference   */

/* Redirecting Number Digits */

#define EINSS7_A41MAP_MIN_REDIRNUM_DIGITS_ADDR_LENGTH        4  
#define EINSS7_A41MAP_MAX_REDIRNUM_DIGITS_ADDR_LENGTH        20 


/* Redirecting Party Name */

#define EINSS7_A41MAP_RPN_LENGTH        17

#define EINSS7_A41MAP_RPN_PS_PRESENTATION_ALLOWED       0
#define EINSS7_A41MAP_RPN_PS_PRESENTATION_RESTRICTED    1
#define EINSS7_A41MAP_RPN_PS_BLOCK_TOGGLE               2
#define EINSS7_A41MAP_RPN_PS_NO_INDICATION              3

#define EINSS7_A41MAP_RPN_AV_NAME_AVAIL                 0
#define EINSS7_A41MAP_RPN_AV_NAME_NOT_AVAIL             1

/* Release Reason */

#define EINSS7_A41MAP_RR_UNSPECIFIED                    0
#define EINSS7_A41MAP_RR_CALL_OVER_CLEAR_FORWARD        1
#define EINSS7_A41MAP_RR_CALL_OVER_CLEAR_BACKWARD       2
#define EINSS7_A41MAP_RR_HANDOFF_SUCCESSFUL             3
#define EINSS7_A41MAP_RR_HANDOFF_ABORT_CO               4
#define EINSS7_A41MAP_RR_HANDOFF_ABORT_NR               5
#define EINSS7_A41MAP_RR_ABNORMAL_MOB_TERM              6
#define EINSS7_A41MAP_RR_ABNORMAL_SWITCH_TERM           7
#define EINSS7_A41MAP_RR_SPEC_FEAT_REL                  8
#define EINSS7_A41MAP_RR_NORMAL_CALL_REL                224
#define EINSS7_A41MAP_RR_CALL_REL_DUE_TO_CALL_DUR       225
#define EINSS7_A41MAP_RR_CALL_REL_DUE_TO_DROP_CALL      226
#define EINSS7_A41MAP_RR_ABNORMAL_CALL_REL              227
#define EINSS7_A41MAP_RR_CONGESTION                     228
#define EINSS7_A41MAP_RR_BUSY                           229

/* Report cause used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Report cause used                 *//* Defined elsewere */

/* Report cause */

#define EINSS7_A41MAP_NO_TRANS_FOR_SUCH_NATURE   0   /* No translation for an address  */
                                             /* of such nature */
#define EINSS7_A41MAP_NO_TRANS_FOR_SPEC_ADDRESS  1   /* No translation for this   */
                                             /* specific address */
#define EINSS7_A41MAP_SUBSYSTEM_CONGESTION       2   /* Subsystem congestion   */
#define EINSS7_A41MAP_SUBSYSTEM_FAILURE          3   /* Subsystem failure   */
#define EINSS7_A41MAP_UNEQUIPPED_USER            4   /* Unequipped user   */
#define EINSS7_A41MAP_NETWORK_FAILURE            5   /* Network failure   */
#define EINSS7_A41MAP_NETWORK_CONGESTION         6   /* Network congestion   */
#define EINSS7_A41MAP_UNQUALIFIED                7   /* Unqualified   */
#define EINSS7_A41MAP_HOP_CNT_VIOLATION          8   /* SCCP hop counter violation */


/* ReportType  used  */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    ReportType  used            *//* Defined elsewere */

/* ReportType  */

#define EINSS7_A41MAP_RT_UNSPEC_SECURITY_PROBLEM    1
#define EINSS7_A41MAP_RT_MIN_ESN_MISMATCH           2
#define EINSS7_A41MAP_RT_RANDC_MISMATCH             3
#define EINSS7_A41MAP_RT_SSD_UPDATE_FAILED          5
#define EINSS7_A41MAP_RT_COUNT_MISM                 7
#define EINSS7_A41MAP_RT_UNIQUE_CHALL_FAILED        9
#define EINSS7_A41MAP_RT_UNSOL_BASE_STATION_CHALL  10
#define EINSS7_A41MAP_RT_SSD_UPDATE_NO_RESP        11
#define EINSS7_A41MAP_RT_COUNT_UPDATE_NO_RESP      12
#define EINSS7_A41MAP_RT_UNIQUE_CHALL_NO_RESP      13
#define EINSS7_A41MAP_RT_AUTHR_MISMATCH                14
#define EINSS7_A41MAP_RT_TERMTYP_MISMATCH          15
#define EINSS7_A41MAP_RT_MISSING_AUTH_PARAM        16


/*   Restriction Digits  */

#define EINSS7_A41MAP_REST_DIGITS_OFFICE_LENGTH  7  /*  6-digit NANP office code */
#define EINSS7_A41MAP_REST_DIGITS_DIR_LENGTH     9  /* 10-digit NANP directory number */


/* Routing Digits */
#define EINSS7_A41MAP_MIN_ROUT_DIGITS_LENGTH        4  
#define EINSS7_A41MAP_MAX_ROUT_DIGITS_LENGTH        12 

/* SendIdNo length */

#define EINSS7_A41MAP_MIN_SENDIDNO_ADDR_LENGTH      4  
#define EINSS7_A41MAP_MAX_SENDIDNO_ADDR_LENGTH     12 


/* Service Indicator used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Service Indicator used            *//* Defined elsewere */

#define EINSS7_A41MAP_SERVICE_IND_LENGTH         1

/* Service Indicator */

#define EINSS7_A41MAP_CDMA_OTASP_SERVICE         1
#define EINSS7_A41MAP_TDMA_OTASP_SERVICE         2
#define EINSS7_A41MAP_CDMA_OTAPA_SERVICE          3

/* Setup Code */

#define EINSS7_A41MAP_SETUP_CODE_LENGTH         1


/* Signaling Message Encrytion Report used */

/*       FALSE   Not used                                   *//* Defined elsewere */
/*       TRUE    Signaling Message Encrytion Report used    *//* Defined elsewere */

/* Signaling Message Encrytion Report */

#define EINSS7_A41MAP_SME_NOT_ATTEMPTED             1
#define EINSS7_A41MAP_SME_NO_RESPONSE               2
#define EINSS7_A41MAP_SME_ENABLED                   3
#define EINSS7_A41MAP_SME_FAILED                    4

/* SMS Access Denied Reason */
#define EINSS7_A41MAP_SMS_ACC_DEN_REAS_DENIED           1
#define EINSS7_A41MAP_SMS_ACC_DEN_REAS_POSTPONED        2
#define EINSS7_A41MAP_SMS_ACC_DEN_REAS_UNAVAIL          3
#define EINSS7_A41MAP_SMS_ACC_DEN_REAS_INVALID          4


/* SMS Bearer data length */

#define EINSS7_A41MAP_MIN_BEARER_DATA_LENGTH   0      /* Min bearer data length */
#define EINSS7_A41MAP_MAX_BEARER_DATA_LENGTH   253    /* Max bearer data length */

/* SMS Cause code used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    ESN used                          *//* Defined elsewere */


/* SMS Cause code */
#define EINSS7_A41MAP_CAUSE_CODE_LENGTH       1   /* Length of Cause Code */

#define EINSS7_A41MAP_VACANT                  0   /* Address vacant   */
#define EINSS7_A41MAP_TRANSLATION_FAILURE     1   /* Address translation failure */
#define EINSS7_A41MAP_NTW_RESOURCE_SHORTAGE   2   /* Network resource shortage */
#define EINSS7_A41MAP_NTW_FAILURE             3   /* Network failure */
#define EINSS7_A41MAP_INVALID_TELESERVICE_ID  4   /* Invalid Teleservice Id */
#define EINSS7_A41MAP_OTHER_NETWORK_PROBLEM   5   /* Other network problem */

#define EINSS7_A41MAP_NO_PAGE_RESPONSE        32  /* No page response */
#define EINSS7_A41MAP_DEST_BUSY               33  /* Destination busy */
#define EINSS7_A41MAP_NO_ACK                  34  /* No acknowledgement */
#define EINSS7_A41MAP_DEST_RESOURCE_SHORTAGE  35  /* Destination resource shortage */
#define EINSS7_A41MAP_SMS_DELIVERY_POSTPONED  36  /* SMS delivery postponed */
#define EINSS7_A41MAP_DEST_OUT_OF_SERVICE     37  /* Destination out of service */
#define EINSS7_A41MAP_NO_LONGER_AT_THIS_ADDRESS   38  /* Destination no longer at this */
                                            /* address */
#define EINSS7_A41MAP_OTHER_TERMINAL_PROBLEM  39  /* Other terminal problem */

#define EINSS7_A41MAP_RI_RESOURCE_SHORTAGE    64  /* Radio interface resource shortage */
#define EINSS7_A41MAP_RI_INCOMPATIBILITY      65  /* Radio interface incompatibility */
#define EINSS7_A41MAP_OTHER_RI_PROBLEM        66  /* Other radio interface problem */
#define EINSS7_A41MAP_UNSUPP_BS_COMPABILITY   67  /* Unsupported BS compability einpawe 990607 */

#define EINSS7_A41MAP_ENCODING_PROBLEM        96  /* Encoding problem */
#define EINSS7_A41MAP_SMS_ORIG_DENIED         97  /* SMS origination denied */
#define EINSS7_A41MAP_SMS_TERM_DENIED         98  /* SMS termination denied */
#define EINSS7_A41MAP_SS_NOT_SUPPORTED        99  /* Supplementary service not supported*/
#define EINSS7_A41MAP_SMS_NOT_SUPPORTED       100 /* SMS not supported */

#define EINSS7_A41MAP_MISSING_EXP_PARAMETER   102 /* Missing expected parameter */
#define EINSS7_A41MAP_MISSING_MAND_PARAMETER  103 /* Missing mandatory parameter */
#define EINSS7_A41MAP_UNREC_PARAMETER_VALUE   104 /* Unrecognized parameter value */
#define EINSS7_A41MAP_UNEXP_PARAMETER_VALUE   105 /* Unexpected parameter value */
#define EINSS7_A41MAP_USR_DATA_SIZE_ERROR     106 /* User data size error */
#define EINSS7_A41MAP_OTHER_GENERAL_PROBLEMS  107 /* Other general problems */
#define EINSS7_A41MAP_SESSION_NOT_ACTIVE      108 /* Session not active einpawe 990607 */ 


/* SMS charge indicator */

#define EINSS7_A41MAP_NO_CHARGE                     1    /* No charge */
#define EINSS7_A41MAP_CHARGE_ORIGINAL_ORIGINATOR    2    /* Charge original originator */
#define EINSS7_A41MAP_CHARGE_ORIGINAL_DESTINATION   3    /* Charge original destination */

#define EINSS7_A41MAP_CHARGE_IND_LENGTH             1


/* SMS Message Count */

#define EINSS7_A41MAP_MSG_COUNT_LENGTH              1


/* SMS notification indicator used */

/*       FALSE   Not used                                                 *//* Defined elsewere */
/*       TRUE    SMS Notification Indicator used                          *//* Defined elsewere */

#define EINSS7_A41MAP_NOT_IND_LENGTH          1

/* SMS notification indicator */

#define EINSS7_A41MAP_NOTIFY_WHEN_AVAILABLE   1  /* Notify when available */
#define EINSS7_A41MAP_DO_NOT_NOTIFY           2  /* Do not notifyvailable */
#define EINSS7_A41MAP_NOTIFY_ON_REG           3  /* Notify upon registration  */  

/* smsOrigAddr length */

#define EINSS7_A41MAP_MIN_SMS_ORIG_ADDR_LENGTH      4  
#define EINSS7_A41MAP_MAX_SMS_ORIG_ADDR_LENGTH     12 


/* SSD Update Report used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    SSD Update Report used            *//* Defined elsewere */

/* SSD Update Report */

#define EINSS7_A41MAP_SSD_NOT_ATTEMPTED             1
#define EINSS7_A41MAP_SSD_NO_RESPONSE               2
#define EINSS7_A41MAP_SSD_SUCCESSFUL                3
#define EINSS7_A41MAP_SSD_FAILED                    4


/* Start Time */

#define EINSS7_A41MAP_STARTTIME_LENGTH              3


/* Subsystem number */

#define EINSS7_A41MAP_MIN_SSN   2    /* Min subsystem number number */
#define EINSS7_A41MAP_MAX_SSN   254  /* Max subsystem number number */

/* System Access  Data  used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    System Access  Data used          *//* Defined elsewere */

#define EINSS7_A41MAP_SYS_ACC_DATA_LENGTH                 5  


/* System Access Data  */  

/* System Access Type used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    System Access Type used           *//* Defined elsewere */

/* System Access Type */

#define EINSS7_A41MAP_SAT_UNSPEC                 1
#define EINSS7_A41MAP_SAT_FLASH_REQ              2
#define EINSS7_A41MAP_SAT_AUTONOM_REG            3
#define EINSS7_A41MAP_SAT_CALL_ORIG              4
#define EINSS7_A41MAP_SAT_PAGE_RESP              5
#define EINSS7_A41MAP_SAT_NO_ACCESS              6
#define EINSS7_A41MAP_SAT_POWER_DOWN_REG         7
#define EINSS7_A41MAP_SAT_SMS_PAGE_RESP  8
#define EINSS7_A41MAP_SAT_OTASP          9

/* System Capability used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    System Capability  used           *//* Defined elsewere */

#define EINSS7_A41MAP_SYS_CAP_LENGTH             1

/* System Capability */


/* System My Type Code */
                                  
#define EINSS7_A41MAP_STC_EDS                    1 
#define EINSS7_A41MAP_STC_ASTRONET               2  
#define EINSS7_A41MAP_STC_AT_N_T                 3  /* AT & T Network Systems */
#define EINSS7_A41MAP_STC_ERICSSON               4  
#define EINSS7_A41MAP_STC_GTE                    5
#define EINSS7_A41MAP_STC_MOTOROLA               6
#define EINSS7_A41MAP_STC_NEC                    7
#define EINSS7_A41MAP_STC_NORTEL                 8 
#define EINSS7_A41MAP_STC_NOVATEL                9
#define EINSS7_A41MAP_STC_PLEXSYS               10
#define EINSS7_A41MAP_STC_DEC                   11  /* Digital equipment Corp. */
#define EINSS7_A41MAP_STC_INET                  12 
#define EINSS7_A41MAP_STC_BELLCORE              13 
#define EINSS7_A41MAP_STC_ALCATEL               14
#define EINSS7_A41MAP_STC_TANDEM                15
#define EINSS7_A41MAP_STC_QUALCOMM              16
#define EINSS7_A41MAP_STC_ALDISCON              17
#define EINSS7_A41MAP_STC_CELCORE               18
#define EINSS7_A41MAP_STC_TELOS                 19
#define EINSS7_A41MAP_STC_STANILITE             20
#define EINSS7_A41MAP_STC_CORAL                 21 /* Coral Systems */
#define EINSS7_A41MAP_STC_SYNACOM               22 /* Synacom Technology */




/*  TDMA Channel Data  used  */

/*       FALSE   Not used                        *//* Defined elsewere */
/*       TRUE    TDMA Channel Data used          *//* Defined elsewere */

#define EINSS7_A41MAP_TDMADATA_LENGTH   5  

/* TDMA Channel Data */   
                                  

#define EINSS7_A41MAP_TSR_ANALOG                 0  /* Analog(Not used if Channel Data is present) */ 
#define EINSS7_A41MAP_TSR_ONE_FULL               1  /* Assigned to timeslot 1, full rate */
#define EINSS7_A41MAP_TSR_TWO_FULL               2  /* Assigned to timeslot 2, full rate */
#define EINSS7_A41MAP_TSR_THREE_FULL             3  /* Assigned to timeslot 3, full rate */
#define EINSS7_A41MAP_TSR_ONE_HALF               9  /* Assigned to timeslot 1, half rate */
#define EINSS7_A41MAP_TSR_TWO_HALF               10  /* Assigned to timeslot 2, half rate */
#define EINSS7_A41MAP_TSR_THREE_HALF             11  /* Assigned to timeslot 3, half rate */
#define EINSS7_A41MAP_TSR_FOUR_HALF              12  /* Assigned to timeslot 4, half rate */
#define EINSS7_A41MAP_TSR_FIVE_HALF              13  /* Assigned to timeslot 5, half rate */
#define EINSS7_A41MAP_TSR_SIX_HALF               14  /* Assigned to timeslot 6, half rate */

/* TDMA Service Code */

#define EINSS7_A41MAP_TDMA_SERVICE_CODE_LENGTH          1

#define EINSS7_A41MAP_TSC_ANALOG_SPEECH                 0
#define EINSS7_A41MAP_TSC_DIGTAL_SPEECH                 1
#define EINSS7_A41MAP_TSC_AOD_ANALOG_PREFERRED          2
#define EINSS7_A41MAP_TSC_AOD_DIGITAL_PREFERRED         3
#define EINSS7_A41MAP_TSC_ASYNCH_DATA                   4
#define EINSS7_A41MAP_TSC_G3_FAX                        5
#define EINSS7_A41MAP_TSC_NOT_USED                      6
#define EINSS7_A41MAP_TSC_STU_III                       7



/* TeleserviceId Used */

/*    FALSE        Not used                             *//* Defined elsewere */
/*    TRUE         TeleserviceId used                   *//* Defined elsewere */

#define EINSS7_A41MAP_TELE_SERVICE_ID_LENGTH             2

/* Teleservice identifier */

#define EINSS7_A41MAP_AMPS                       4096  /* AMPS Extended Protocol Enhanced Services */
#define EINSS7_A41MAP_CDMA_PAGING                4097  /* CDMA Cellular Paging Teleservice */
#define EINSS7_A41MAP_CDMA_MESSAGING             4098  /* CDMA Cellular Messaging Teleservice */
#define EINSS7_A41MAP_CDMA_VOICE                 4099  /* CDMA Voice Mail Notification */
#define EINSS7_A41MAP_TDMA                       32513 /* TDMA Cellular Messaging Teleservice */
#define EINSS7_A41MAP_TDMA_PAGING                32514 /* TDMA Cellular Paging  Teleservicee  */
#define EINSS7_A41MAP_TDMA_OATS                  32515 /* TDMA Over-the-Air Activation Teleservice  */
#define EINSS7_A41MAP_TDMA_OPTS                  32516 /* TDMA Over-the-Air Programming Teleservice  */
#define EINSS7_A41MAP_TDMA_GUTS                  32517 /* TDMA General UDP Transport Service  */
#define EINSS7_A41MAP_TDMA_SEG_MESSAGING         32577 /* TDMA Segmented Cellular Messaging Teleservice  */
#define EINSS7_A41MAP_TDMA_SEG_PAGING            32578 /* TDMA Segmented Cellular Paging Teleservice  */
#define EINSS7_A41MAP_TDMA_SEG_OATS              32579 /* TDMA Segmented OATS  */
#define EINSS7_A41MAP_TDMA_SEG_OPTS              32580 /* TDMA Segmented OPTS  */
#define EINSS7_A41MAP_TDMA_SEG_GUTS              32581 /* TDMA Segmented GUTS  */ 
#define EINSS7_A41MAP_TDMA_OAA                   32641 /* TDMA MS-based SME for Core Interim OAA  */


/* Temporary Reference Number length */
#define EINSS7_A41MAP_MIN_TEMP_REF_LENGTH                 1
#define EINSS7_A41MAP_MAX_TEMP_REF_LENGTH                20

/* TerminalType  used  */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    TerminalType  used                *//* Defined elsewere */

#define EINSS7_A41MAP_TERM_TYPE_LENGTH           1

/* TerminalType  */

#define EINSS7_A41MAP_TERMT_NOT_DIST             1
#define EINSS7_A41MAP_TERMT_IS54B                2
#define EINSS7_A41MAP_TERMT_IS136                3
#define EINSS7_A41MAP_TERMT_IS95                32
#define EINSS7_A41MAP_TERMT_IS95A               33
#define EINSS7_A41MAP_TERMT_IS88                64
#define EINSS7_A41MAP_TERMT_IS94                65
#define EINSS7_A41MAP_TERMT_IS91                66

/* Termination Access Type */

#define EINSS7_A41MAP_TAT_LENGTH        1

#define EINSS7_A41MAP_TAT_MOB_TO_MDN_ACCESS             252
#define EINSS7_A41MAP_TAT_LAND_TO_MDN_ACCESS            253
#define EINSS7_A41MAP_TAT_REMOTE_FEAT_CP_ACCESS         254
#define EINSS7_A41MAP_TAT_RP_ACCESS                     255



/* Termination List length */

#define EINSS7_A41MAP_MIN_TERMLIST_LENGTH           8
#define EINSS7_A41MAP_MAX_TERMLIST_LENGTH           80

/* Termination List */


/* Termination Restriction Code  used */

/*       FALSE   Not used                                     *//* Defined elsewere */
/*       TRUE     Termination Restriction Code  used          *//* Defined elsewere */

#define EINSS7_A41MAP_TERM_REST_CODE_LENGTH               1

/*  Termination Restriction Code  */

                                   
#define EINSS7_A41MAP_TRC_TERM_DENIED            1 /* Termination Denied  */ 
#define EINSS7_A41MAP_TRC_UNREST                 2 /* Unrestricted  */ 


/* TransactionCapability  used  */

/*       FALSE   Not used                            *//* Defined elsewere */
/*       TRUE    TransactionCapability used          *//* Defined elsewere */

#define EINSS7_A41MAP_TRANSACT_CAPA_LENGTH           2


/*  TransactionCapability   */                                     



/* Transaction reference */

#define EINSS7_A41MAP_MIN_TRANS_REF   1      /* Min transaction reference number */
#define EINSS7_A41MAP_MAX_TRANS_REF   65535  /* Max transaction reference number */

/* Trigger Capability */

#define EINSS7_A41MAP_TRIGG_CAPA_LENGTH         3

/* Trigger List */

#define EINSS7_A41MAP_MAX_TRIGGLIST             5

/* Trigger Type */

#define EINSS7_A41MAP_TRIGG_TYPE_LENGTH         1

#define EINSS7_A41MAP_TT_UNSPECIFIED                    0
#define EINSS7_A41MAP_TT_ALL_CALLS                      1
#define EINSS7_A41MAP_TT_DOUBLE_INTRO_STAR              2
#define EINSS7_A41MAP_TT_SINGLE_INTRO_STAR              3
#define EINSS7_A41MAP_TT_DOUBLE_INTRO_POUND             5
#define EINSS7_A41MAP_TT_SINGLE_INTRO_POUND             6
#define EINSS7_A41MAP_TT_RETR_CALL                      7
#define EINSS7_A41MAP_TT_0_DIGIT                        8
#define EINSS7_A41MAP_TT_1_DIGIT                        9
#define EINSS7_A41MAP_TT_2_DIGIT                        10
#define EINSS7_A41MAP_TT_3_DIGIT                        11
#define EINSS7_A41MAP_TT_4_DIGIT                        12
#define EINSS7_A41MAP_TT_5_DIGIT                        13
#define EINSS7_A41MAP_TT_6_DIGIT                        14
#define EINSS7_A41MAP_TT_7_DIGIT                        15
#define EINSS7_A41MAP_TT_8_DIGIT                        16
#define EINSS7_A41MAP_TT_9_DIGIT                        17
#define EINSS7_A41MAP_TT_10_DIGIT                       18
#define EINSS7_A41MAP_TT_11_DIGIT                       19
#define EINSS7_A41MAP_TT_12_DIGIT                       20
#define EINSS7_A41MAP_TT_13_DIGIT                       21
#define EINSS7_A41MAP_TT_14_DIGIT                       22
#define EINSS7_A41MAP_TT_15_DIGIT                       23
#define EINSS7_A41MAP_TT_LOCAL_CALL                     24
#define EINSS7_A41MAP_TT_INTRA_TATA                     25
#define EINSS7_A41MAP_TT_INTER_TATA                     26
#define EINSS7_A41MAP_TT_WORLD_ZONE                     27
#define EINSS7_A41MAP_TT_INTERNATIONAL                  28
#define EINSS7_A41MAP_TT_UNREC_NUMBER                   29
#define EINSS7_A41MAP_TT_PRIOR_AGREEMENT                30
#define EINSS7_A41MAP_TT_SPEC_CALLED_PARTY_DIG_STR      31
#define EINSS7_A41MAP_TT_MOB_TERM                       32
#define EINSS7_A41MAP_TT_ADVANCED_TERM                  33
#define EINSS7_A41MAP_TT_LOCATION                       34
#define EINSS7_A41MAP_TT_LOC_ALLOW_SPEC_DIG_STR         35
#define EINSS7_A41MAP_TT_ORIG_ATT_AUTH                  36
#define EINSS7_A41MAP_TT_CALLING_ROUT_ADDR_AVAIL        37
#define EINSS7_A41MAP_TT_INIT_TERM                      38
#define EINSS7_A41MAP_TT_CALLED_ROUT_ADDR_AVAIL         39
#define EINSS7_A41MAP_TT_0_ANSWER                       40
#define EINSS7_A41MAP_TT_0_DISCONNECT                   41
#define EINSS7_A41MAP_TT_TERM_RESOURCE_AVAIL            64
#define EINSS7_A41MAP_TT_T_BUSY                         65
#define EINSS7_A41MAP_TT_T_NO_ANSWER                    66
#define EINSS7_A41MAP_TT_T_NO_PAGE_RESP                 67
#define EINSS7_A41MAP_TT_T_UNROUTABLE                   68
#define EINSS7_A41MAP_TT_T_ANSWER                       69
#define EINSS7_A41MAP_TT_T_DISCONNECT                   70



/* Type of digits */

#define EINSS7_A41MAP_DIALLED_OR_CALLED_PARTY    1  /* Dialled number or called party  */
/* number.                         */
#define EINSS7_A41MAP_CALLING_PARTY_NUMBER       2  /* Called party number             */
#define EINSS7_A41MAP_CALLER_INTERACTION         3  /* Caller interaction. These are   */
/* the digits dialled by a user in */
/* response to a prompt.           */
#define EINSS7_A41MAP_ROUTING_NUMBER             4  /* Routing Number. This number is  */
/* used to steer a call towards    */
/* its ultimate destination.       */
#define EINSS7_A41MAP_BILLING_NUMBER             5  /* Billing Number. This is the     */
/* number to use for ANI,          */
/* ChargeNumber or other recording */
/* purposes.                       */
#define EINSS7_A41MAP_DESTINATION_NUMBER         6  /* Destination Number. This is the */
/* network address of the called   */
/* party.                          */
#define EINSS7_A41MAP_LATA                       7  /* LATA                            */
#define EINSS7_A41MAP_CARRIER                    8  /* Carrier. In North America the   */
/* three or four digits represent  */
/* an interexchange or             */
/* international carrier.          */
#define EINSS7_A41MAP_ESRD			13 /* ESRD, einpawe 20010410 */


/* Type of number */

#define EINSS7_A41MAP_NATIONAL             0  /* National number      */
#define EINSS7_A41MAP_INTERNATIONAL        1  /* International number */

/* Type of subaddress */

#define EINSS7_A41MAP_TOS_CCITT_NSAP       0  /* NSAP (CCITT Rec. X213   */
/*       or ISO 8348 AD2). */
#define EINSS7_A41MAP_TOS_USER_SPECIFIED   2 /* User specified,  TR2323 1->2, einpawe 20010321  */



/* Unique Challange Report used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Unique Challenge Report used      *//* Defined elsewere */

/* Unique Challenge Report */

#define EINSS7_A41MAP_UC_NOT_ATTEMPTED              1
#define EINSS7_A41MAP_UC_NO_RESPONSE                2
#define EINSS7_A41MAP_UC_SUCCESSFUL                 3
#define EINSS7_A41MAP_UC_FAILED                     4

/* User Status */

#define EINSS7_A41MAP_STATE_UIS     0 /* User in Service */
#define EINSS7_A41MAP_STATE_UOS     1  /* User out of  Service */
#define EINSS7_A41MAP_STATE_CON1    2 /* User in Service with congestion Level1*/
#define EINSS7_A41MAP_STATE_CON2    3 /* User in Service with congestion Level2*/
#define EINSS7_A41MAP_STATE_CON3    4 /* User in Service with congestion Level3*/

/* Voice Privacy Report used */

/*       FALSE   Not used                          *//* Defined elsewere */
/*       TRUE    Voice Privacy Report used         *//* Defined elsewere */

/* Voice Privacy Report */

#define EINSS7_A41MAP_VP_NOT_ATTEMPTED              1
#define EINSS7_A41MAP_VP_NO_RESPONSE                2
#define EINSS7_A41MAP_VP_SUCCESSFUL                 3
#define EINSS7_A41MAP_VP_FAILED                     4

/* WIN Operations Capability */

#define EINSS7_A41MAP_WINOP_CAPA_LENGTH         1


/* WIN Capability */

#define EINSS7_A41MAP_WC_CONN_SENDER_NOT_CAP            0
#define EINSS7_A41MAP_WC_CONN_SENDER_CAP                1

#define EINSS7_A41MAP_WC_CCDIR_SENDER_NOT_CAP           0
#define EINSS7_A41MAP_WC_CCDIR_SENDER_CAP               1






/*********************************************************************/
/*                                                                   */
/*    R E Q U E S T  /  R E S P O N S E   R E T U R N   C O D E S    */
/*                                                                   */
/*********************************************************************/

#define EINSS7_A41MAP_REQUEST_OK                              0  
#define EINSS7_A41MAP_INVALID_SUBSYSTEM_NUMBER                1  
#define EINSS7_A41MAP_INVALID_TRANSACTION_REF                 2  
#define EINSS7_A41MAP_INVALID_OR_ADDR_LENGTH                  3 
#define EINSS7_A41MAP_INVALID_DE_ADDR_LENGTH                  4  

#define EINSS7_A41MAP_INVALID_MIN                             6   

#define EINSS7_A41MAP_INVALID_SMS_NOT_IND                     9 
#define EINSS7_A41MAP_INVALID_ORIG_PRESENTATION               10 
#define EINSS7_A41MAP_INVALID_SMS_DEST_TYPE_OF_DIGITS         11 
#define EINSS7_A41MAP_INVALID_SMS_DEST_TYPE_OF_NUMBER         12 
#define EINSS7_A41MAP_INVALID_SMS_DEST_PRESENT_OF_NO          13 
#define EINSS7_A41MAP_INVALID_SMS_DEST_AVAIL_OF_NO            14 
#define EINSS7_A41MAP_INVALID_SMS_DEST_ADDR_LENGTH            15 
#define EINSS7_A41MAP_INVALID_SMS_ORIG_TYPE_OF_DIGITS         16  
#define EINSS7_A41MAP_INVALID_SMS_ORIG_TYPE_OF_NUMBER         17  
#define EINSS7_A41MAP_INVALID_SMS_ORIG_PRESENT_OF_NO          18 
#define EINSS7_A41MAP_INVALID_SMS_ORIG_AVAIL_OF_NO            19 
#define EINSS7_A41MAP_INVALID_SMS_ORIG_ADDR_LENGTH            20 
#define EINSS7_A41MAP_INVALID_BCD_DIGIT                       21 
#define EINSS7_A41MAP_INVALID_TELESERVICE_IND                 22 
#define EINSS7_A41MAP_INVALID_CHARGE_IND                      23 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_SUB_ODD_EVEN_IND      24 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_SUB_ADDR_TYPE         25 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_SUB_ADDR_LENGTH       26 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_SUB_ODD_EVEN_IND      27 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_SUB_ADDR_TYPE         28 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_SUB_ADDR_LENGTH       29 
#define EINSS7_A41MAP_INVALID_BEARER_DATA_LENGTH              30 
#define EINSS7_A41MAP_INVALID_DEST_NUM_PLAN                   31
#define EINSS7_A41MAP_INVALID_DEST_ENCODE                     32 
#define EINSS7_A41MAP_INVALID_DEST_PROV_OF_NO                 33 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_ADDR_LENGTH           34 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_TYPE_OF_DIGITS        35 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_TYPE_OF_NUMBER        36 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_PRESENT_OF_NO         37 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_AVAIL_OF_NO           38 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_PROV_OF_NO            39 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_NUM_PLAN              40 
#define EINSS7_A41MAP_INVALID_ORIG_DEST_ENCODE                41 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_ADDR_LENGTH           42 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_TYPE_OF_DIGITS        43 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_TYPE_OF_NUMBER        44 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_PRESENT_OF_NO         45 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_AVAIL_OF_NO           46 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_PROV_OF_NO            47 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_NUM_PLAN              48 
#define EINSS7_A41MAP_INVALID_ORIG_ORIG_ENCODE                49 
#define EINSS7_A41MAP_INVALID_ORIG_PROV_OF_NO                 50 
#define EINSS7_A41MAP_INVALID_ORIG_ENCODE                     51 
#define EINSS7_A41MAP_INVALID_ORIG_NUM_PLAN                   52 
#define EINSS7_A41MAP_MSID_MISSING                            53  /* Replaced MIN with MSID einpawe 990607 */
#define EINSS7_A41MAP_BEARER_DATA_MISSING                     54          
#define EINSS7_A41MAP_BOTH_BEARER_DATA_AND_SMS_CAUSE_CODE     55
#define EINSS7_A41MAP_INVALID_DIGITS_ADDR_LENGTH              56 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_TYPE_OF_DIGITS           57 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_TYPE_OF_NUMBER           58 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_PRESENT_OF_NO            59 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_AVAIL_OF_NO              60 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_PROV_OF_NO               61 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_NUM_PLAN                 62 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_DIGITS_ENCODE                   63 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_ADDR_LENGTH         64 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_TYPE_OF_REST_DIGITS 65 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_TYPE_OF_NUMBER      66 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_PRESENT_OF_NO       67 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_AVAIL_OF_NO         68 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_PROV_OF_NO          69 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_NUM_PLAN            70 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_REST_DIGITS_ENCODE              71 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_ERROR_CODE                      72 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_FAULTY_PARAM_LENGTH             73 /* Added by einkarl 980227 */

#define EINSS7_A41MAP_INVALID_ORIG_TRIGG_LENGTH               75 /* Added by einkarl 980227 */
#define EINSS7_A41MAP_INVALID_ANNOUNCE_LIST_LENGTH            76 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_ADDR_LENGTH         77 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_TYPE_OF_NUMBER      78 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_PRESENT_OF_NO       79 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_AVAIL_OF_NO         80 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_PROV_OF_NO          81 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_NUM_PLAN            82 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_ROUT_DIGITS_ENCODE              83 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_TERM_LIST_LENGTH                84 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_AKEY_PROT_VER_LENGTH            85 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_MS_PART_KEY_LENGTH              86 /* Added by einpelt 980707 */
#define EINSS7_A41MAP_INVALID_IMSI                            87 /* Added by einpawe 990607 */
#define EINSS7_A41MAP_BOTH_MIN_AND_IMSI                       88 /* Added by einpawe 990607 */
#define EINSS7_A41MAP_INVALID_MDN_ADDR_LENGTH                 89 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_TYPE_OF_DIGITS              90 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_TYPE_OF_NUMBER              91 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_PRESENT_OF_NO               92 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_AVAIL_OF_NO                 93 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_PROV_OF_NO                  94 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_ENCODE                      95 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_MDN_NUM_PLAN                    96 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_NEITHER_MIN_OR_IMSI_OR_BOTH             97 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_SCREENING_IND                   98 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_LPRI                            99 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_TYPE_OF_SHAPE                   100 /* Added by qinxrot 990114 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_ADDR_LENGTH            101 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_TYPE_OF_DIGITS         102 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_TYPE_OF_NUMBER         103 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_PRESENT_OF_NO          104 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_AVAIL_OF_NO            105 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_PROV_OF_NO             106 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_ENCODE                 107 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SENDIDNO_NUM_PLAN               108 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_DEST_ADDR_LENGTH                109 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_DEST_TYPE_OF_DIGITS             110 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_DEST_TYPE_OF_NUMBER             111 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_DEST_PRESENT_OF_NO              112 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_DEST_AVAIL_OF_NO                113 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_PRIORITY                        114 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_QUALITY_OF_SERVICE              115 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_TIMER_VALUE                     116 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_TRN                             117 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_OPTPARAM_LENGTH                 118 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SMS_ORIG_PROV_OF_NO             119 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SMS_ORIG_ENCODE                 120 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_SMS_ORIG_NUM_PLAN               121 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_STRUCTURE_POINTER               122 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_MSIMSI                          123 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_BOTH_MSMIN_AND_MSIMSI                   124 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_INVALID_NEWIMSI                         125 /* Added by qinxrot 000225 */
#define EINSS7_A41MAP_BOTH_NEWMIN_AND_NEWIMSI                 126 /* Added by qinxrot 000225 */      
#define EINSS7_A41MAP_DIGITS_MISSING                          127 /* Added by einpelt 000225 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_ADDR_LENGTH     128 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_TYPE_OF_DIGITS  129 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_TYPE_OF_NUMBER  130 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_PRESENT_OF_NO   131 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_AVAIL_OF_NO     132 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_PROV_OF_NO      133 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_ENCODE          134 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLPARTY_DGTS1_NUM_PLAN        135 /* Added by qinxrot 001025 */ 
#define EINSS7_A41MAP_INVALID_MSCID_NUM_ADDR_LENGTH           136 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_TYPE_OF_DIGITS        137 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_TYPE_OF_NUMBER        138 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_PRESENT_OF_NO         139 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_AVAIL_OF_NO           140 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_PROV_OF_NO            141 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_ENCODE                142 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_MSCID_NUM_NUM_PLAN              143 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_ADDR_LENGTH         144 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_TYPE_OF_DIGITS      145 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_TYPE_OF_NUMBER      146 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_PRESENT_OF_NO       147 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_AVAIL_OF_NO         148 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_PROV_OF_NO          149 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_ENCODE              150 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_ESRD_DIGITS_NUM_PLAN            151 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_TRIGG_ADDR_LIST                 152 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_ADDR_LENGTH         153 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_TYPE_OF_DIGITS      154 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_TYPE_OF_NUMBER      155 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_PRESENT_OF_NO       156 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_AVAIL_OF_NO         157 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_PROV_OF_NO          158 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_ENCODE              159 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_CALLBACKNUM_NUM_PLAN            160 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_ADDR_LENGTH     161 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_TYPE_OF_DIGITS  162 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_TYPE_OF_NUMBER  163 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_PRESENT_OF_NO   164 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_AVAIL_OF_NO     165 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_PROV_OF_NO      166 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_ENCODE          167 /* Added by qinxrot 001025 */
#define EINSS7_A41MAP_INVALID_REDIRNUM_DIGITS_NUM_PLAN        168 /* Added by qinxrot 001025 */ 
#define EINSS7_A41MAP_ENCODING_OK                             169 /* Added by einpawe 20010305 */     
#define EINSS7_A41MAP_INVALID_OPERATION                       170 /* Added by einpawe 20010305 */ 
#define EINSS7_A41MAP_INVALID_MWNC                            171 /* TR2322, einpawe 20010321 */ 


/*********************************************************************/
/*                                                                   */
/*    I N D I C A T I O N   R E T U R N   C O D E S                  */
/*                                                                   */
/*********************************************************************/
#define EINSS7_A41MAP_IND_UNKNOWN_CODE                      0xFE
#define EINSS7_A41MAP_IND_LENGTH_ERROR                      0xFF


/*********************************************************************/
/*                                                                   */
/*          A D D R E S S I N F O                                    */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ADDRINFO_TAG
{
    UCHAR_T  typeOfDigits;          /* Type of digits                    */
    UCHAR_T  typeOfNumber;          /* Type of number                    */
    UCHAR_T  presentationOfNumber;  /* Presentation of number            */
    UCHAR_T  availabilityOfNumber;  /* Availability of number            */
    UCHAR_T  providerOfNumber;      /* Provider of number                */
    UCHAR_T  encoding;              /* Encoding                          */
    UCHAR_T  numberingPlan;         /* Numbering plan                    */
    UCHAR_T  lengthOfAddress;       /* Length of address exept the 3     */
                                    /* first octets. (TypeOfDigits + ..  */
    UCHAR_T  address[EINSS7_A41MAP_MAX_DIGITS_ADDR_LENGTH];  
} EINSS7_A41MAP_ADDRINFO_T; 

/*********************************************************************/
/*                                                                   */
/*          A D D R E S S  C H O I C E                               */
/*          Added by qinxrot 001024                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ADDRCHOICE_TAG
{
    EINSS7_A41MAP_ADDRINFO_T *esprKey_sp;
    EINSS7_A41MAP_ADDRINFO_T *callbackNum_sp;
    EINSS7_A41MAP_ADDRINFO_T *esrdDigits_sp;
} EINSS7_A41MAP_ADDRCHOICE_T; 


/*********************************************************************/
/*                                                                   */
/*          S U B A D D R E S S I N F O                              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SUBADDRINFO_TAG
{
    UCHAR_T  oddEvenInd;        /* Odd / even indicator */
    UCHAR_T  typeOfSubaddress;  /* Type of subaddress   */
    UCHAR_T  lengthOfAddress;   /* Length of address    */
    UCHAR_T  address[EINSS7_A41MAP_MAX_SUBADDR_LENGTH];
} EINSS7_A41MAP_SUBADDRINFO_T; 


/*********************************************************************/
/*                                                                   */
/*             E R R O R   P A R A M E T E R S                       */
/*             Added by einkarl 980227                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ERROR_TAG
{
    UCHAR_T     errorCode;              /* errorCode           */
    UCHAR_T     faultyParamLen;         /* Faulty parameter length   */
    UCHAR_T     faultyParam[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH];/* Faulty parameter */
} EINSS7_A41MAP_ERROR_T;

/*********************************************************************/
/*                                                                   */
/*          E X T R A   O P T I O N A L   P A R A M E T E R S        */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_EXTRAOPTPARAM_TAG
{
    UCHAR_T     optParamLen;              /* extra optional parameters length             */
    UCHAR_T     *optParam_p;              /* extra optional parameters pointer        */
} EINSS7_A41MAP_EXTRAOPTPARAM_T;

/*********************************************************************/
/*                                                                   */
/*       P O S I T I O N  I N F O R M A T I O M                      */
/*             Added by qinxrot 001018                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_POSINFO_TAG
{
    UCHAR_T     genTime[EINSS7_A41MAP_GENTIME_LENGTH];          /* Generalized Time */
    UCHAR_T     geoPosLen;                                      /* Geographic Position length */
    UCHAR_T     geoPos[EINSS7_A41MAP_MAX_GEOPOS_LENGTH];        /* Geographic Position */
    BOOLEAN_T   posSourceUsed;                                  /* Position Source used */
    UCHAR_T     posSource;                                      /* Position Source */
} EINSS7_A41MAP_POSINFO_T;

/*********************************************************************/
/*                                                                   */
/*             T R I G G E R   L I S T                               */
/*             Added by qinxrot 001024                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_TRIGGERLIST_TAG
{
  UCHAR_T       globalTitleLen;                                  /* Global Title length */
  UCHAR_T       globalTitle[EINSS7_A41MAP_GT_LENGTH];            /* Global Title */
  BOOLEAN_T     pcSsnUsed;                                       /* PC SSN used */
  UCHAR_T       pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];              /* PC SSN */
  UCHAR_T       winTriggerListLen;                               /* WIN Trigger List length */
  UCHAR_T      *winTriggerList_p;                                /* WIN Trigger List pointer */
} EINSS7_A41MAP_TRIGGERLIST_T;


/*********************************************************************/
/*                                                                   */
/*          T R I G G E R  A D D R  L I S T                          */
/*             Added by qinxrot 001024                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_TRIGGADDRLIST_TAG
{
  UCHAR_T       numOfTriggList;
  EINSS7_A41MAP_TRIGGERLIST_T *triggList_sp[EINSS7_A41MAP_MAX_TRIGGLIST];
} EINSS7_A41MAP_TRIGGADDRLIST_T;


/*********************************************************************/
/*                                                                   */
/*             E S P O S R E Q  I N D   P A R A M E T E R S          */
/*             Added by qinxrot 001022                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ESPOSREQ_IND_TAG
{
    UCHAR_T     esmeIdLen;                                      /* ESME Identification length */
    UCHAR_T     esmeId[EINSS7_A41MAP_MAX_ESME_ID_LENGTH];       /* EMSE Identification */
    UCHAR_T     posReqType;                                     /* Position Request Type */
    EINSS7_A41MAP_ADDRCHOICE_T *esposAddr_sp;                   /* Address choice structurpointer */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;            /* Extra Optional Parameter */
} EINSS7_A41MAP_ESPOSREQ_IND_T;

/*********************************************************************/
/*                                                                   */
/*             E S P O S R E Q  R E S P   P A R A M E T E R S        */
/*             Added by qinxrot 001022                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ESPOSREQ_RESP_TAG
{
    UCHAR_T     posResult;                                              /* Position Result */
    EINSS7_A41MAP_POSINFO_T *posInfo_sp;                                /* Position Information */
    EINSS7_A41MAP_ADDRINFO_T *callbackNum_sp;                           /* Callback Number */
    EINSS7_A41MAP_ADDRINFO_T *esrdDigits_sp;                            /* Emergency Services Routing Digits */
    BOOLEAN_T   esrkTimeUsed;                                           /* Generalized Time used */
    UCHAR_T     esrkTime[EINSS7_A41MAP_GENTIME_LENGTH];                 /* Generalized Time */
    BOOLEAN_T   minUsed;                                                /* MIN used */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                   /* MIN */
    UCHAR_T     imsiLen;                                                /* IMSI length */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];                    /* IMSI */
    BOOLEAN_T   mobCallStatusUsed;                                      /* Mobile Call Status */
    UCHAR_T     mobCallStatus;                                          /* Mobile Call Status */
    UCHAR_T     companyIdLen;                                           /* Company ID length */
    UCHAR_T     companyId[EINSS7_A41MAP_MAX_COMP_ID_LENGTH];            /* Company ID */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;                    /* Extra Optional Parameter */
} EINSS7_A41MAP_ESPOSREQ_RESP_T;



/*********************************************************************/
/*                                                                   */
/*          F E A T R E Q  I N D   P A R A M E T E R S               */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_FEATREQ_IND_TAG
{
        EINSS7_A41MAP_ADDRINFO_T *digits_sp;            /* Digits (Dialed or Dest.) */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];/* ESN */
        BOOLEAN_T minUsed;                              /* MIN used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];           /* MIN */
        UCHAR_T imsiLen;                                /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
        BOOLEAN_T billingIdUsed;                        /* Billing ID used */
        UCHAR_T billingId[EINSS7_A41MAP_BILLING_ID_LENGTH];     /* Billing ID */
        BOOLEAN_T mscIdUsed;                            /* MSC ID used*/
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSC ID */
        BOOLEAN_T pcSsnUsed;                            /* PC SSN used */
        UCHAR_T pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];             /*PC SSN */
        BOOLEAN_T transactCapUsed;                      /*Transaction Capability used*/
        UCHAR_T transactCap[EINSS7_A41MAP_TRANSACT_CAPA_LENGTH]; /* Transaction capability*/
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_FEATREQ_IND_T;

/*********************************************************************/
/*                                                                   */
/*          F E A T R E Q  R E S P   P A R A M E T E R S             */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_FEATREQ_RESP_TAG
{
        UCHAR_T featureResult;                          /* Feature Result */
        BOOLEAN_T accessDeniedReasonUsed;               /*AccessDeniedReason used */
        UCHAR_T accessDeniedReason;                     /*accessDeniedReason */
        EINSS7_A41MAP_ADDRINFO_T *digits_sp;            /*Digits Dialed or Destination) */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_FEATREQ_RESP_T;

/*********************************************************************/
/*                                                                   */
/* G E O D E T I C L O C A T I O N     P A R A M E T E R S           */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_GEOLOCATION_TAG
{
  UCHAR_T     screening;         /* screening */
  UCHAR_T     lpri;              /* LPRI      */
  UCHAR_T     typeOfShape;       /* type of shapeot1234
   */
  UCHAR_T     shapeDescLen;      /* Shape Description length  */
  UCHAR_T     *shapeDesc_p;      /* Shape Description pointer */
} EINSS7_A41MAP_GEOLOCATION_T;


/*********************************************************************/
/*                                                                   */
/*     I S P O S C A N C   I N D  P A R A M E T E R S                */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ISPOSCANC_IND_TAG
{
    UCHAR_T     posCancReason;                          /* PositionCancellationReason */
    EINSS7_A41MAP_ADDRINFO_T    *mobileDirNum_sp;               /* Mobile directory number */
    BOOLEAN_T   minUsed;                                /* Mobile identification number used */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];   /* Mobile identification number */
    UCHAR_T     imsiLen;                                /* IMSI length */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ISPOSCANC_IND_T;

/*********************************************************************/
/*                                                                   */
/*     I S P O S C A N C  R E S P  P A R A M E T E R S               */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ISPOSCANC_RESP_TAG
{
    UCHAR_T     posCancResult;                          /* Position cancellation result */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ISPOSCANC_RESP_T;

/*********************************************************************/
/*                                                                   */
/*     I S P O S D I R   C O N F  P A R A M E T E R S                */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ISPOSDIR_CONF_TAG
{
   
    UCHAR_T     failureReason;                          /* Failure Reason               */
    EINSS7_A41MAP_ADDRINFO_T    *esrd_sp;                       /* EmergencyServicesRoutingDigits */
    BOOLEAN_T   minUsed;                                /* Mobile identification number used */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];   /* Mobile identification number */
    UCHAR_T     imsiLen;                                /* IMSI length */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
    BOOLEAN_T   posReqReportUsed;                       /* PositionRequestReport used */
    UCHAR_T     posReqReport;                           /* PositionRequestReport */
    BOOLEAN_T   channelDataUsed;                        /* Channel data used */
    UCHAR_T     channelData[EINSS7_A41MAP_CDATA_LENGTH];       /* Channel data */
    BOOLEAN_T   tdmaChannelDataUsed;                    /* TDMA Channel data used */
    UCHAR_T     tdmaChannelData[EINSS7_A41MAP_TDMADATA_LENGTH];/* TDMA Channel data */
    EINSS7_A41MAP_GEOLOCATION_T *geoLocation_sp;        /* GeographicLocation */
    UCHAR_T     faultyParameterLength;                  /* Faulty parameter length      */
    UCHAR_T     faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH]; /* Faulty parameter */
    BOOLEAN_T   reportCauseUsed;                        /* Report cause used            */
    UCHAR_T     reportCause;                            /* Report cause                 */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ISPOSDIR_CONF_T;

/*********************************************************************/
/*                                                                   */
/*               I S P O S D I R  R E Q  P A R A M E T E R S         */
/*                    Added by qinxrot 000222                        */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ISPOSDIR_REQ_TAG
{
    
    UCHAR_T     systemMyTypeCode;                       /* SystemMyTypeCode */
    UCHAR_T     posReqType;                             /* PositionRequestType */
    EINSS7_A41MAP_ADDRINFO_T    *mobileDirNum_sp;               /* Mobile directory number */
    BOOLEAN_T   minUsed;                                /* Mobile identification number used */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];   /* Mobile identification number */
    UCHAR_T     imsiLen;                                /* IMSI used */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
    BOOLEAN_T   esnUsed;                                /* ElectronicSerialNumber used */
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];   /* ElectronicSerialNumber */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ISPOSDIR_REQ_T;

/*********************************************************************/
/*                                                                   */
/*     I S P O S R E Q   I N D  P A R A M E T E R S                  */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ISPOSREQ_IND_TAG
{
    
    UCHAR_T     systemMyTypeCode;                       /* SystemMyTypeCode */
    UCHAR_T     billingId[EINSS7_A41MAP_BILLING_ID_LENGTH];     /* Billing ID */
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];   /* ElectronicSerialNumber */
    EINSS7_A41MAP_ADDRINFO_T    *esrd_sp;                       /* EmergencyServicesRoutingDigits */
    BOOLEAN_T   minUsed;                                /* Mobile identification number used */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];   /* Mobile identification number */
    UCHAR_T     imsiLen;                                /* IMSI used */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
    EINSS7_A41MAP_ADDRINFO_T    *mobileDirNum_sp;               /* Mobile directory number */
    BOOLEAN_T   channelDataUsed;                        /* Channel data used */
    UCHAR_T     channelData[EINSS7_A41MAP_CDATA_LENGTH];        /* Channel data */
    BOOLEAN_T   tdmaChannelDataUsed;                    /* TDMA Channel data used */
    UCHAR_T     tdmaChannelData[EINSS7_A41MAP_TDMADATA_LENGTH];/* TDMA Channel data */
    BOOLEAN_T   posReqTypeUsed;                         /* Position req type used */
    UCHAR_T     posReqType;                             /* Position req type */
    BOOLEAN_T   pcSsnUsed;                              /* PC SSN used */
    UCHAR_T     pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];     /* PC SSN */
    BOOLEAN_T   mscIdUsed;                              /* MSCID Used */
    UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];      /* MSCID */
    BOOLEAN_T   mpAccurracyUsed;                        /* MPAccurracy used */
    UCHAR_T     mpAccurracy[EINSS7_A41MAP_MPACC_LENGTH];        /* MPAccurracy */
    BOOLEAN_T   isposReqIndUsed;                        /* ISPOS request indicators used */
    UCHAR_T     isposReqInd[EINSS7_A41MAP_ISPOSREQ_IND_LENGTH];/* ISPOS request indicators */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ISPOSREQ_IND_T;

/*********************************************************************/
/*                                                                   */
/*     I S P O S R E Q  R E S P  P A R A M E T E R S                 */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ISPOSREQ_RESP_TAG
{
    UCHAR_T     posDetResult;                           /* Position determination result */
    EINSS7_A41MAP_GEOLOCATION_T  *geoLocation_sp;       /* GeodeticLOcation */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ISPOSREQ_RESP_T;

/*********************************************************************/
/*                                                                   */
/*         L O C R E Q   C O N F  P A R A M E T E R S                */
/*             Added by qinxrot 001018                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_LOCREQ_CONF_TAG
{
    UCHAR_T     failureReason;                                  /* Failure Reason */
    BOOLEAN_T   esnUsed;                                        /* ESN used */
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];   /* ESN */
    BOOLEAN_T   minUsed;                                        /* MIN used */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];           /* MIN */
    BOOLEAN_T   mscIdUsed;                                      /* MSCID used */       
    UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSCID */
    BOOLEAN_T   accessDeniedReasonUsed;                         /* AccessDeniedReason used */
    UCHAR_T     accessDeniedReason;                             /* AccessDeniedReason */
    UCHAR_T     annListLen;                                     /* Announcement List length */
    UCHAR_T     annList[EINSS7_A41MAP_MAX_ANNLIST_LENGTH];      /* Announcement List */
    EINSS7_A41MAP_ADDRINFO_T    *callPartyNumStr1_sp;           /* CallingPartyNumberString1 */
    EINSS7_A41MAP_ADDRINFO_T    *digitsCarrier_sp;              /* Digits (Carrier) */
    EINSS7_A41MAP_ADDRINFO_T    *digitsDestination_sp;          /* Digits (Destination) */
    BOOLEAN_T   dmhRedirIndUsed;                                /* DMH Redirection Indicator used */
    UCHAR_T     dmhRedirInd;                                    /* DMH Redirection Indicator */
    BOOLEAN_T   pcSsnUsed;                                      /* PC SSN used */
    UCHAR_T     pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];             /* PC SSN */
    UCHAR_T     termListLen;                                    /* Termination List length      */
    UCHAR_T     termList[EINSS7_A41MAP_MAX_TERMLIST_LENGTH];    /* Termination List       */
    EINSS7_A41MAP_ADDRINFO_T    *digitsDialed_sp;               /* Digits (Dialed) */
    BOOLEAN_T   tdmaServCodeUsed;                               /* TDMA Service Code used */
    UCHAR_T     tdmaServCode;                                   /* TDMA Service Code */
    EINSS7_A41MAP_TRIGGADDRLIST_T *triggAddrList_sp;            /* Trigger Address List */
    UCHAR_T     imsiLen;                                        /* IMSI length */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
    BOOLEAN_T   dmhServIdUsed;                                  /* DMH Service ID used */
    UCHAR_T     dmhServId[EINSS7_A41MAP_DMH_SERVID_LENGTH];     /* DMH Service ID */
    UCHAR_T     faultyParameterLength;                  /* Faulty parameter length      */
    UCHAR_T     faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH]; /* Faulty parameter */
    BOOLEAN_T   reportCauseUsed;                        /* Report cause used            */
    UCHAR_T     reportCause;                            /* Report cause                 */                        
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_LOCREQ_CONF_T;


/*********************************************************************/
/*                                                                   */
/*         L O C R E Q   R E Q  P A R A M E T E R S                  */
/*             Added by qinxrot 001018                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_LOCREQ_REQ_TAG
{
    UCHAR_T     billingId[EINSS7_A41MAP_BILLING_ID_LENGTH];     /* Billing ID */
    EINSS7_A41MAP_ADDRINFO_T    *digits_sp;                     /* Digits */
    UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSC ID */
    UCHAR_T     systemMyTypeCode;                               /* SystemMyTypeCode */
    EINSS7_A41MAP_ADDRINFO_T    *callPartyNumDgts1_sp;          /* CallingPartyNumberDigits1 */
    EINSS7_A41MAP_ADDRINFO_T    *mscIdNum_sp;                   /* MSCIdentificationNumber      */
    BOOLEAN_T   pcSsnUsed;                                      /* PC SSN used */
    UCHAR_T     pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];             /* PC SSN */
    EINSS7_A41MAP_ADDRINFO_T    *redirNumDigits_sp;             /* RedirectingNumberDigits */
    BOOLEAN_T   termAccessTypeUsed;                             /* Termination Access Type used */
    UCHAR_T     termAccessType;                                 /* Termination Access Type */
    BOOLEAN_T   transactCapUsed;                                /* Transaction Capability used   */    
    UCHAR_T     transactCap[EINSS7_A41MAP_TRANSACT_CAPA_LENGTH]; /* Transaction Capability */ 
    BOOLEAN_T   tdmaServCodeUsed;                               /* TDMA Service Code used */
    UCHAR_T     tdmaServCode;                                   /* TDMA Service Code */
    BOOLEAN_T   triggCapUsed;                                   /* Trigger Capability used */
    UCHAR_T     triggCap[EINSS7_A41MAP_TRIGG_CAPA_LENGTH];      /* Trigger Capability */
    BOOLEAN_T   winOpCapUsed;                                   /* WIN Operations Capability */
    UCHAR_T     winOpCap;                                       /* WIN Operatins Capability */
    BOOLEAN_T   triggTypeUsed;                                  /* Trigger Type used */
    UCHAR_T     triggType;                                      /* Trigger Type */
    BOOLEAN_T   callPartyNameUsed;                              /* Calling Party Name used */
    UCHAR_T     callPartyName[EINSS7_A41MAP_CPN_LENGTH];        /* Calling Party Name */
    BOOLEAN_T   redirPartyNameUsed;                             /* Redirecting Party Name used */
    UCHAR_T     redirPartyName[EINSS7_A41MAP_RPN_LENGTH];       /* Redirecting Party Name */                             
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_LOCREQ_REQ_T;


/*********************************************************************/
/*                                                                   */
/*     M S I N A C T I V E  I N D  P A R A M E T E R S               */
/*             Added by einkarl 980227                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_MSINACTIVE_IND_TAG
{
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN                   */
    BOOLEAN_T   minUsed;                      /* MIN used, einpawe 990607     */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];  /* MIN                          */
    UCHAR_T     imsiLen;                      /* Length of IMSI, einpawe 990607 */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* IMSI, einpawe 990607 */
    BOOLEAN_T   callHistCountUsed;            /* Call History Count  used     */
    UCHAR_T     callHistCount;                /* Call History Count           */
    BOOLEAN_T   deregTypeUsed;                /* Deregistration Type Used     */
    UCHAR_T     deregType;                    /* Deregistration Type          */ 
    BOOLEAN_T   locAreaIdUsed;                /* Location Area Id Used        */
    USHORT_T    locAreaId;                    /* Location Area Id             */ 
    EINSS7_A41MAP_ADDRINFO_T  *sendIdNo_sp;            /* Sender Identification Number */
    BOOLEAN_T   smsMWI;                       /* SMS Message Waiting Indicato */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_MSINACTIVE_IND_T;

/*********************************************************************/
/*                                                                   */
/*     M S I N A C T I V E  R E S P  P A R A M E T E R S             */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_MSINACTIVE_RESP_TAG
{
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_MSINACTIVE_RESP_T;

/*********************************************************************/
/*                                                                   */
/*             M W N C  P A R A M E T E R S                          */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/ 

typedef struct EINSS7_A41MAP_MWNC_TAG
{
    UCHAR_T mwncLength;           /* Length *//* TR2322, einpawe 20010321 */  
    UCHAR_T typeOfMessages;         /* Type of messages   */
    UCHAR_T noOfMessagesWaiting;    /* Number of messages */ 
} EINSS7_A41MAP_MWNC_T; 

/*********************************************************************/
/*                                                                   */
/*        O R I G I N A T I O N  I N D  P A R A M E T E R S          */
/*                    Added by einpelt 980707                        */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ORREQ_IND_TAG
{
    UCHAR_T     billingId[EINSS7_A41MAP_BILLING_ID_LENGTH]; /* Billing ID              */
    EINSS7_A41MAP_ADDRINFO_T  *digits_sp;                /* Digits                       */
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN                   */
    BOOLEAN_T   minUsed;                      /* MIN used, einpawe 990607     */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];  /* MIN                          */
    UCHAR_T     imsiLen;                      /* Length of IMSI, einpawe 990607 */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* IMSI, einpawe 990607         */
    UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];     /* MSC ID                       */
    UCHAR_T     origTriggLen;                 /* Origination Triggers length  */
    UCHAR_T     origTrigg[EINSS7_A41MAP_MAX_ORIG_TRIGG_LENGTH];/* Origination Triggers */
    UCHAR_T     transactCap[EINSS7_A41MAP_TRANSACT_CAPA_LENGTH]; /* Transact. Capabilities */
    EINSS7_A41MAP_ADDRINFO_T *callPartyNumDgts1_sp;      /* CallingPartyNumberDigits1    */
    EINSS7_A41MAP_ADDRINFO_T *callPartyNumDgts2_sp;      /* CallingPartyNumberDigits2    */
    EINSS7_A41MAP_SUBADDRINFO_T *callPartySubaddr_sp;    /* CallingPartySubaddress       */
    EINSS7_A41MAP_ADDRINFO_T *mobileDirNum_sp;           /* MobileDirectoryNumber        */
    EINSS7_A41MAP_ADDRINFO_T *mscIdNum_sp;               /* MSCIdentificationNumber      */
    BOOLEAN_T   oneTimeFeatIndUsed;           /* OneTimeFeatureIndicator used */
    UCHAR_T     oneTimeFeatInd[EINSS7_A41MAP_ONE_TIME_FEAT_IND_LENGTH];
    BOOLEAN_T   pcSsnUsed;                    /* PC SSN used                  */
    UCHAR_T     pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];    /* PC SSN                       */
    EINSS7_A41MAP_ADDRINFO_T *sendIdNo_sp;               /* SenderIdentificationNumber   */
    EINSS7_A41MAP_ADDRINFO_T *smsAddress_sp;             /* SMS Address                  */
    BOOLEAN_T   sysCapUsed;               /* System Capabilities used     */
    UCHAR_T     sysCap;                   /* System Capabilities          */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ORREQ_IND_T;

/*********************************************************************/
/*                                                                   */
/*        O R I G I N A T I O N  R E S P  P A R A M E T E R S        */
/*                    Added by einpelt 980707                        */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ORREQ_RESP_TAG
{
    BOOLEAN_T   accessDeniedReasonUsed;       /* AccessDeniedReason used      */
    UCHAR_T     accessDeniedReason;           /* AccessDeniedReason           */
    BOOLEAN_T   actionCodeUsed;               /* Action Code used             */
    UCHAR_T     actionCode;                   /* Action Code                  */
    UCHAR_T     annListLen;                   /* Announcement List length     */
    UCHAR_T     annList[EINSS7_A41MAP_MAX_ANNLIST_LENGTH]; /* Announcement List        */
    EINSS7_A41MAP_ADDRINFO_T  *digits_sp;                /* Digits                       */
    EINSS7_A41MAP_ADDRINFO_T  *routDigits_sp;            /* Routing Digits               */
    UCHAR_T     termListLen;                  /* Termination List length      */
    UCHAR_T     termList[EINSS7_A41MAP_MAX_TERMLIST_LENGTH]; /* Termination List       */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_ORREQ_RESP_T;

/*********************************************************************/
/*                                                                   */
/*               O T A S P  C O N F  P A R A M E T E R S             */
/*                    Added by einpelt 980707                        */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_OTASP_CONF_TAG
{
    UCHAR_T     failureReason;                /* Failure Reason               */
    UCHAR_T     aKeyProtVerLen;               /* A-Key Protocol Version length*/
    UCHAR_T     aKeyProtVer[EINSS7_A41MAP_MAX_AKEY_LENGTH]; /* A-Key Protocol Version  */
    BOOLEAN_T   authRespBsUsed;               /*Authentic.resp BaseStation used */
    UCHAR_T     authRespBs[EINSS7_A41MAP_AUTHRESP_BS_LENGTH];/* Authentic. resp BaseStation */
    UCHAR_T     bsKeyLen;                     /* BS Partial Key length        */
    UCHAR_T     bsKey[EINSS7_A41MAP_MAX_BSKEY_LENGTH]; /* Base Station Partial Key     */
    BOOLEAN_T   denyAccessUsed;                /* Deny Access used */
    UCHAR_T     denyAccess;                    /* Deny Access */
    UCHAR_T     modulusValLen;                /* Modulus value length         */
    UCHAR_T     modulusVal[EINSS7_A41MAP_MAX_MODVAL_LENGTH]; /* Modulus Value          */
    BOOLEAN_T   otaspRCUsed;                  /* OTASP Result Code used       */
    UCHAR_T     otaspRC;                      /* OTASP Result Code            */
    UCHAR_T     primValLen;                   /* Primitive Value length       */
    UCHAR_T     primVal[EINSS7_A41MAP_MAX_PRIMVAL_LENGTH]; /* Primitive Value          */
    BOOLEAN_T   sigMsgEncRepUsed; /* Signaling Message Encryption Report Used */
    UCHAR_T     sigMsgEncRep;          /* Signaling Message Encryption Report */
    BOOLEAN_T   ssdUpdateReportUsed;          /* SSD Update Report used       */
    UCHAR_T     ssdUpdateReport;              /* SSD Update Report            */
    BOOLEAN_T   uniqueChalReportUsed;         /* Unique Challenge Report used */
    UCHAR_T     uniqueChalReport;             /* Unique Challenger Report     */
    BOOLEAN_T   voicePrivReportUsed;          /* Voice Privacy Report Used    */
    UCHAR_T     voicePrivReport;              /* Voice Privacy Report         */
    UCHAR_T     faultyParameterLength;        /* Faulty parameter length      */
    UCHAR_T     faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH];
    BOOLEAN_T   reportCauseUsed;              /* Report cause used            */
    UCHAR_T     reportCause;                  /* Report cause                 */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_OTASP_CONF_T;


/*********************************************************************/
/*                                                                   */
/*               O T A S P  R E Q  P A R A M E T E R S               */
/*                    Added by einpelt 980707                        */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_OTASP_REQ_TAG
{
    BOOLEAN_T   actionCodeUsed;               /* Action Code used             */
    UCHAR_T     actionCode;                   /* Action Code                  */
    UCHAR_T     aKeyProtVerLen;               /* A-Key Protocol Version length*/
    UCHAR_T     aKeyProtVer[EINSS7_A41MAP_MAX_AKEY_LENGTH]; /* A-Key Protocol Version  */
    BOOLEAN_T   authDataUsed;                 /* Authentication Data used */
    UCHAR_T     authData[EINSS7_A41MAP_AUTHDATA_LENGTH];/* Authentication Data */
    BOOLEAN_T   authRespUsed;                 /* Authentication Response used */
    UCHAR_T     authResp[EINSS7_A41MAP_AUTHRESP_LENGTH];/* Authentication Response */
    BOOLEAN_T   callHistCountUsed;            /* CallHistoryCount used */
    UCHAR_T     callHistCount;                /* CallHistoryCount */
    BOOLEAN_T   esnUsed;                      /* ESN used                     */
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN                   */
    BOOLEAN_T   minUsed;                         /* MIN used                     */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];   /* MIN                          */
    UCHAR_T     imsiLen;                         /* Length of IMSI, einpawe 990607 */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];    /* IMSI, einpawe 990607         */
    UCHAR_T     msKeyLen;                     /* MS Partial Key length        */
    UCHAR_T     msKey[EINSS7_A41MAP_MAX_MSKEY_LENGTH]; /* Mobile Station Partial Key   */
    BOOLEAN_T   msMinUsed;                       /* MS MIN used */
    UCHAR_T     msMobileIdNo[EINSS7_A41MAP_MIN_LENGTH]; /* MS MIN */
    UCHAR_T     msImsiLen;                       /* MS IMSI length */
    UCHAR_T     msImsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* MS IMSI */
    BOOLEAN_T   mscIdUsed;                    /* MSC ID used                  */
    UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];     /* MSC ID                       */
    BOOLEAN_T   newMinUsed;                   /* New MIN used                 */
    UCHAR_T     newMobileIdNo[EINSS7_A41MAP_MIN_LENGTH]; /* New MIN                    */
    UCHAR_T     newImsiLen;                   /* New IMSI length, einpawe 990607 */
    UCHAR_T     newImsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];/* New IMSI, einpawe 990607     */
    BOOLEAN_T   pcSsnUsed;                    /* PC SSN used                  */
    UCHAR_T     pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];    /* PC SSN                       */
    BOOLEAN_T   randVarUsed;                   /* Random variable used */
    UCHAR_T     randVar[EINSS7_A41MAP_RAND_VAR_LENGTH];  /* Random variable */
    BOOLEAN_T   randVarBsUsed;                 /* Random variable BS used */
    UCHAR_T     randVarBs[EINSS7_A41MAP_RAND_VARBS_LENGTH];/* Random variable BS */
    BOOLEAN_T   serviceIndUsed;               /* Service Indicator used       */
    UCHAR_T     serviceInd;                   /* Service Indicator            */
    BOOLEAN_T   sysCapUsed;                   /* System capabilities used */
    UCHAR_T     sysCap;                       /* System capabilities */
    BOOLEAN_T   termTypeUsed;                 /* Terminal type used */
    UCHAR_T     termType;                     /* Terminal type */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_OTASP_REQ_T;

/*********************************************************************/
/*                                                                   */
/*             P _ A B O R T   P A R A M E T E R S                   */
/*             Added by qinxert 010226                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_P_ABORT_IND_TAG
{
  UCHAR_T     ssn;                                  /* Subsystem number             */
  USHORT_T    transRef;                             /* Transaction reference        */
  UCHAR_T     abortCause;                           /* Abort Cause */ 
} EINSS7_A41MAP_P_ABORT_IND_T;

/*********************************************************************/
/*                                                                   */
/*           Q U E R Y   I N D  P A R A M E T E R S                  */
/*             Added by qinxrot 001018                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_QUERY_IND_TAG
{
    UCHAR_T     queryType;                                      /* Query Type */
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];   /* ElectronicSerialNumber */
    EINSS7_A41MAP_ADDRINFO_T    *mobileDirNum_sp;               /* Mobile directory number */
    UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSCID */
    BOOLEAN_T   startTimeUsed;                                  /* Start Time used */
    UCHAR_T     startTime[EINSS7_A41MAP_STARTTIME_LENGTH];      /* Start Time */
    BOOLEAN_T   relReasonUsed;                                  /* Release Reason used */
    UCHAR_T     relReason;                                      /* Release Reason */
    EINSS7_A41MAP_ADDRINFO_T    *routDigits_sp;                 /* Routing Digits */
    BOOLEAN_T   locAreaIdUsed;                                  /* Location Area Id Used */
    USHORT_T    locAreaId;                                      /* Location Area Id */
    BOOLEAN_T   actDurationUsed;                                /* Actual Duration Used */
    USHORT_T    actDuration;                                    /* Actual Duration */
    EINSS7_A41MAP_ADDRINFO_T    *carrierDigits_sp;              /* Carrier Digits */
    EINSS7_A41MAP_ADDRINFO_T    *callPartyNumDgts1_sp;          /* CallingPartyNumberDigits1 */
    BOOLEAN_T   miscInfoUsed;                                   /* Miscellaneous Information used */
    UCHAR_T     miscInfo[EINSS7_A41MAP_MISCINFO_LENGTH];        /* Miscellaneous Information */
    BOOLEAN_T   billingIdUsed;                                  /* Billing ID used */
    UCHAR_T     billingId[EINSS7_A41MAP_BILLING_ID_LENGTH];     /* Billing ID */
    BOOLEAN_T   servCellIdUsed;                                 /* Serving Cell ID Used */
    USHORT_T    servCellId;                                     /* Serving Cell ID */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;            /* Extra optional parameters */
} EINSS7_A41MAP_QUERY_IND_T;

/*********************************************************************/
/*                                                                   */
/*           Q U E R Y   R E S P  P A R A M E T E R S                */
/*             Added by qinxrot 001018                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_QUERY_RESP_TAG
{
    UCHAR_T     queryType;                                          /* Query Type */
    BOOLEAN_T   allowedCallDurUsed;                                 /* Allowed Call Duration used */
    UCHAR_T     allowedCallDur[EINSS7_A41MAP_ALL_CALL_DUR_LENGTH];  /* Allowed Call Duration */
    BOOLEAN_T   setupCodeUsed;                                      /* Setup Code used */
    UCHAR_T     setupCode;                                          /* Setup Code */
    EINSS7_A41MAP_ADDRINFO_T    *routDigits_sp;                     /* Routing Digits */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;                /* Extra optional parameters */
} EINSS7_A41MAP_QUERY_RESP_T;


/*********************************************************************/
/*                                                                   */
/*     R E D D I R  C O N F  P A R A M E T E R S                     */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_REDDIR_CONF_TAG
{
        UCHAR_T failureReason;                                  /* Failure Reason */
        UCHAR_T faultyParameterLength;                          /* Faulty parameter length*/
        UCHAR_T faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH];    /* Faulty parameter */
        BOOLEAN_T reportCauseUsed;                              /* Report cause used */
        UCHAR_T reportCause;                                    /* Report cause */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;        /* Extra Optional Parameter */
} EINSS7_A41MAP_REDDIR_CONF_T;

/*********************************************************************/
/*                                                                   */
/*     R E D D I R  R E Q  P A R A M E T E R S                       */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_REDDIR_REQ_TAG
{
        UCHAR_T billingId[EINSS7_A41MAP_BILLING_ID_LENGTH]; /* Billing ID */
        EINSS7_A41MAP_ADDRINFO_T *digits_sp;            /* Digits (Destination) */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];   /* ESN */
        BOOLEAN_T minUsed;                              /* MIN used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];           /* MIN */
        UCHAR_T imsiLen;                                /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
        UCHAR_T systemMyTypeCode;                       /* SystemMyTypeCode */
        EINSS7_A41MAP_ADDRINFO_T *sendIdNo_sp;          /* Sender Id Number*/
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_REDDIR_REQ_T;

/*********************************************************************/
/*                                                                   */
/*     R E G C A N C E L  I N D  P A R A M E T E R S                 */
/*             Added by einkarl 980227                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_REGCANCEL_IND_TAG
{
    UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN                   */
    BOOLEAN_T   minUsed;                      /* MIN used, einpawe 990607     */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];  /* MIN                          */
    UCHAR_T     imsiLen;                      /* Length of IMSI, einpawe 990607 */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* IMSI, einpawe 990607         */
    BOOLEAN_T   cancelTypeUsed;               /* Cancellation Type  used      */
    UCHAR_T     cancelType;                   /* Cancellation Type            */
    BOOLEAN_T   ccDataUsed;                   /* Control Channel Data Used    */
    UCHAR_T     ccData[EINSS7_A41MAP_CC_DATA_LENGTH];  /* Control Channel Data         */ 
    BOOLEAN_T   recvSigQualUsed;              /* Received Signal Quality Used */
    UCHAR_T     recvSigQual;                  /* Received Signal Quality      */ 
    EINSS7_A41MAP_ADDRINFO_T  *sendIdNo_sp;              /* Sender Identification Numbe  */
    BOOLEAN_T   sysAccDataUsed;               /* System Access Data Used      */
    UCHAR_T     sysAccData[EINSS7_A41MAP_SYS_ACC_DATA_LENGTH]; /* System Access Data           */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */ 
} EINSS7_A41MAP_REGCANCEL_IND_T;

/*********************************************************************/
/*                                                                   */
/*     R E G C A N C E L  R E S P  P A R A M E T E R S               */
/*             Added by qinxrot 000222                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_REGCANCEL_RESP_TAG
{
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_REGCANCEL_RESP_T; 

/*********************************************************************/
/*                                                                   */
/*     R E G I S T R A T I O N  I N D  P A R A M E T E R S           */
/*             Added by einkarl 980227                               */
/*        Added optional parameters (CR444) by einpelt 980811        */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_REGNOT_IND_TAG
{
  UCHAR_T     electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN                   */
  BOOLEAN_T   minUsed;                      /* MIN used, einpawe 990607     */
  UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];  /* MIN                          */
  UCHAR_T     imsiLen;                      /* Length of IMSI, einpawe 990607 */
  UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* IMSI, einpawe 990607       */
  UCHAR_T     mscId[EINSS7_A41MAP_MSCID_LENGTH];     /* MSCID                        */
  UCHAR_T     qualInfoCode;               /* Qualification Information Code */
  UCHAR_T     systemMyTypeCode;             /* System My Type Code         */ 
  BOOLEAN_T   availabilityTypeUsed;         /* Availability Type Used   *//*CR444*/
  UCHAR_T     availabilityType;             /* Availability Type        *//*CR444*/
  BOOLEAN_T   borderCellAccUsed;            /* Border Cell Access Used  *//*CR444*/
  UCHAR_T     borderCellAcc;                /* Border Cell Access       *//*CR444*/
  BOOLEAN_T   ccDataUsed;                   /* Control Channel Data Used*//*CR444*/
  UCHAR_T     ccData[EINSS7_A41MAP_CC_DATA_LENGTH];  /* Control Channel Data     *//*CR444*/
  BOOLEAN_T   extMscIdUsed;                 /* Extended MSCID Used      *//*CR444*/
  UCHAR_T     extMscId[EINSS7_A41MAP_EXT_MSCID_LENGTH]; /* Extended MSCID        *//*CR444*/
  BOOLEAN_T   locAreaIdUsed;                /* Location Area Id Used    *//*CR444*/
  USHORT_T    locAreaId;                    /* Location Area Id         *//*CR444*/
  BOOLEAN_T   pcSsnUsed;                    /* PC SSN Used */
  UCHAR_T     pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];    /* PC  */ 
  BOOLEAN_T   recvSigQualUsed;              /* Received Signal Quality Used*//*CR444*/
  UCHAR_T     recvSigQual;                  /* Received Signal Quality     *//*CR444*/
  BOOLEAN_T   reportTypeUsed;               /* Report Type Used   */      /*CR444*/
  UCHAR_T     reportType;                   /* Report Type        */      /*CR444*/
  EINSS7_A41MAP_ADDRINFO_T  *sendIdNo_sp;   /* Sender Identification Number*//*CR444*/
  EINSS7_A41MAP_ADDRINFO_T  *smsAddress_sp; /* SMS address  */
  BOOLEAN_T   smsMWI;                       /* SMS Message Waiting Indicator*//*CR444*/
  BOOLEAN_T   sysAccDataUsed;               /* System Access Data Used   *//*CR444*/
  UCHAR_T     sysAccData[EINSS7_A41MAP_SYS_ACC_DATA_LENGTH]; /* System Access Data*//*CR444*/
  BOOLEAN_T   sysAccTypeUsed;               /* System Access Type  Used  */
  UCHAR_T     sysAccType;                   /* System Access Type   */ 
  BOOLEAN_T   sysCapUsed;                   /* System Capabilities  Used */
  UCHAR_T     sysCap;                       /* System Capabilities   */ 
  BOOLEAN_T   terminalTypeUsed;             /* Terminal Type Used   */    /*CR444*/
  UCHAR_T     terminalType;                 /* Terminal Type        */    /*CR444*/
  BOOLEAN_T   transactCapUsed;             /* Transact Capa Used   */    /*CR444*/
  UCHAR_T     transactCap[EINSS7_A41MAP_TRANSACT_CAPA_LENGTH];                    /*CR444*/
  EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;      /* Extra optional parameters */
} EINSS7_A41MAP_REGNOT_IND_T;

/*********************************************************************/
/*                                                                   */
/*     R E G I S T R A T I O N  R E S P  P A R A M E T E R S         */
/*             Added by einkarl 980227                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_REGNOT_RESP_TAG
{
    UCHAR_T     systemMyTypeCode;             /* System My Type Code         */ 
    BOOLEAN_T   authDeniedUsed;               /* Authorization Denied  Used  */
    UCHAR_T     authDenied;                   /* Authorization Denied        */ 
    BOOLEAN_T   denAuthPeriodUsed;            /* Denied Authorization Period Used */
    UCHAR_T     denAuthPeriod[EINSS7_A41MAP_DEN_AUTH_PER_LENGTH];/* Denied Authorization Period  */ 
    EINSS7_A41MAP_ADDRINFO_T   *digits_sp;               /* Digits   */
    BOOLEAN_T   authCapUsed;                  /* Authentication Capability Used */
    UCHAR_T     authCap;                      /* Authentication Capability   */ 
    BOOLEAN_T   origIndUsed;                  /* Origination Indicator Used  */
    UCHAR_T     origInd;                      /* Origination Indicator       */ 
    UCHAR_T     origTriggLen;                 /* Origination Triggers Length */
    UCHAR_T     origTrigg[EINSS7_A41MAP_MAX_ORIG_TRIGG_LENGTH]; /* Origination Triggers */
    EINSS7_A41MAP_ADDRINFO_T   *restDigits_sp;           /* Restriction Digits   */
    BOOLEAN_T   termRestCodeUsed;              /* Termination Restriction Code Used*/
    UCHAR_T     termRestCode;                 /* Termination Restriction Code  */
    BOOLEAN_T   minUsed;                      /* MIN used, einpawe 990607    */
    UCHAR_T     mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];  /* MIN, einpawe 990607         */
    UCHAR_T     imsiLen;                      /* Length of IMSI, einpawe 990607 */
    UCHAR_T     imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* IMSI, einpawe 990607        */
    BOOLEAN_T   authPerUsed;                  /* Authorization Period used, einpawe 990607 */
    UCHAR_T     authPer[EINSS7_A41MAP_AUTHPER_LENGTH];  /* Authorization Period, einpawe 990607 */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */
} EINSS7_A41MAP_REGNOT_RESP_T;

/*********************************************************************/
/*                                                                   */
/*          R O U T R E Q   C O N F   P A R A M E T E R S            */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ROUTREQ_CONF_TAG
{
        UCHAR_T failureReason;                                  /* Failure Reason */
        BOOLEAN_T mscIdUsed;                                    /* MSC ID used */
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSC ID */
        BOOLEAN_T accessDeniedReasonUsed;                       /* AccessDeniedReason  used */
        UCHAR_T accessDeniedReason;                             /* AccessDeniedReason */
        EINSS7_A41MAP_ADDRINFO_T *digits_sp;                    /* Digits (destination)*/
        UCHAR_T faultyParameterLength;                          /* Faulty parameter length*/
        UCHAR_T faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH];    /* Faulty Parameter */
        BOOLEAN_T reportCauseUsed;                              /* Report cause used */
        UCHAR_T reportCause;                                    /* Report cause */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;        /* Extra Optional Parameter */
} EINSS7_A41MAP_ROUTREQ_CONF_T;

/*********************************************************************/
/*                                                                   */
/*          R O U T R E Q   I N D   P A R A M E T E R S              */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ROUTREQ_IND_TAG
{
        UCHAR_T billingId[EINSS7_A41MAP_BILLING_ID_LENGTH];     /* Billing ID */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN */
        BOOLEAN_T minUsed;                              /* MIN used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];           /* MIN */
        UCHAR_T imsiLen;                              /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSC ID */
        UCHAR_T systemMyTypeCode;                       /* System My Type Code */
        BOOLEAN_T pcSsnUsed;                            /* PC SSN used */
        UCHAR_T pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];             /* PC SSN */
        EINSS7_A41MAP_ADDRINFO_T *sendIdNo_sp;          /* Sender Id Number */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_ROUTREQ_IND_T;

/*********************************************************************/
/*                                                                   */
/*          R O U T R E Q   R E Q    P A R A M E T E R S             */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ROUTREQ_REQ_TAG
{
        UCHAR_T billingId[EINSS7_A41MAP_BILLING_ID_LENGTH];     /* Billing ID */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];   /* ESN */
        BOOLEAN_T minUsed;                              /* MIN used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];           /* MIN */
        UCHAR_T imsiLen;                              /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];            /* IMSI */
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];              /* MSC ID */
        UCHAR_T systemMyTypeCode;                       /* System My Type Code */
        BOOLEAN_T pcSsnUsed;                            /* PC SSN used */
        UCHAR_T pcSsn[EINSS7_A41MAP_PC_SSN_LENGTH];             /* PC SSN */
        EINSS7_A41MAP_ADDRINFO_T *sendIdNo_sp;          /* Sender Id Number */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_ROUTREQ_REQ_T;

/*********************************************************************/
/*                                                                   */
/*          R O U T R E Q   R E S P   P A R A M E T E R S            */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_ROUTREQ_RESP_TAG
{
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];      /* MSC ID */
        BOOLEAN_T accessDeniedReasonUsed;               /* AccessDeniedReason used */
        UCHAR_T accessDeniedReason;                     /* AccessDeniedReason */
        EINSS7_A41MAP_ADDRINFO_T *digits_sp;            /* Digits (destination)*/
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp; /* Extra Optional Parameter */
} EINSS7_A41MAP_ROUTREQ_RESP_T;

/*********************************************************************/
/*                                                                   */
/*          S M D P P  C O N F   P A R A M E T E R S                 */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMDPP_CONF_TAG
{
        UCHAR_T failureReason;                                  /* Failure reason */
        BOOLEAN_T authDeniedUsed;                               /* Authorization Denied used */
        UCHAR_T authDenied;                                     /* Authorization Denied */
        BOOLEAN_T denyAccessUsed;                               /* Deny Access used */
        UCHAR_T denyAccess;                                     /* Deny Access */
        BOOLEAN_T esnUsed;                                      /* ESN used */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];           /* ESN */
        BOOLEAN_T msMinUsed;                                    /* MS MIN used */
        UCHAR_T msMobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                        /* MS MIN */
        UCHAR_T msImsiLen;                                      /* MS IMSI length*/
        UCHAR_T msImsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                 /* MS IMSI */
        BOOLEAN_T mscIdUsed;                                    /* MSC ID used*/
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];                      /* MSC ID */
        BOOLEAN_T bearerDataUsed;                               /* Bearer data used */
        UCHAR_T bearerDataLength;                               /* Bearer data length */
        UCHAR_T bearerData[EINSS7_A41MAP_MAX_BEARER_DATA_LENGTH];       /* Bearer data */ 
        BOOLEAN_T causeCodeUsed;                                /* SMS Cause code used*/
        UCHAR_T causeCode;                                      /* SMS Cause code */
        BOOLEAN_T sysCapUsed;                                   /* System capabilities used */
        UCHAR_T sysCap;                                         /* System capabilities */
        BOOLEAN_T faultyParameterLength;                        /* Faulty parameter length*/
        UCHAR_T faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH];    /* Faulty Parameter */
        BOOLEAN_T reportCauseUsed;                              /* Report cause used */
        UCHAR_T reportCause;                                    /* Report cause */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;        /* Extra Optional Parameter */
} EINSS7_A41MAP_SMDPP_CONF_T; 

/*********************************************************************/
/*                                                                   */
/*          S M D P P  I N D   P A R A M E T E R S                   */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMDPP_IND_TAG
{
        UCHAR_T bearerDataLength;                               /* Bearer data length */
        UCHAR_T bearerData[EINSS7_A41MAP_MAX_BEARER_DATA_LENGTH];       /* Bearer data */
        BOOLEAN_T teleserviceIdUsed;
        USHORT_T teleserviceId;                                 /* SMS Teleservice Identifier */
        BOOLEAN_T actionCodeUsed;                               /* Action Code used */
        UCHAR_T actionCode;                                     /* Action Code */
        BOOLEAN_T esnUsed;                                      /* ESN used */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];           /* ESN */
        BOOLEAN_T minUsed;                                      /* MIN used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                   /* MIN */
        UCHAR_T imsiLen;                                        /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                   /* IMSI */
        BOOLEAN_T newMinUsed;                                   /* New MIN used */
        UCHAR_T newMobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                /* New MIN */
        UCHAR_T newImsiLen;                                     /* New IMSI length */
        UCHAR_T newImsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                /* New IMSI */
        UCHAR_T serviceIndUsed;                                 /* Service Indicator used*/
        UCHAR_T serviceInd;                                     /* Service Indicator */
        BOOLEAN_T smsChargeIndUsed;                             /* SMS Charge Indication used*/
        UCHAR_T smsChargeInd;                                   /* SMS Charge Indicator */
        EINSS7_A41MAP_ADDRINFO_T *destAddr_sp;                  /* Destination addr */
        BOOLEAN_T msgCountUsed;                                 /* Message count used */
        UCHAR_T msgCount;                                       /* Message count */
        BOOLEAN_T smsNotIndUsed;                                /* Notification indicator used */
        UCHAR_T smsNotInd;                                      /* SMS Notification indicator */
        EINSS7_A41MAP_ADDRINFO_T *origDestAddr_sp;              /* Orig.destination addr */
        EINSS7_A41MAP_SUBADDRINFO_T *origDestSubaddr_sp;                /* Orig. dest. subaddr. */
        EINSS7_A41MAP_ADDRINFO_T *origOrigAddr_sp;              /* Orig.originating addr */
        EINSS7_A41MAP_SUBADDRINFO_T *origOrigSubaddr_sp;                /* Orig. orig. subaddr. */
        EINSS7_A41MAP_ADDRINFO_T *origAddr_sp;                  /* Originating addr */
        UCHAR_T tempRefNoLen;                                 /* Temporary Referense No length */
        UCHAR_T tempRefNo[EINSS7_A41MAP_MAX_TEMP_REF_LENGTH];           /* Temporary Reference No */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;        /* Extra Optional Parameter */
} EINSS7_A41MAP_SMDPP_IND_T; 

/*********************************************************************/
/*                                                                   */
/*          S M D P P  R E Q   P A R A M E T E R S                   */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMDPP_REQ_TAG
{
        UCHAR_T bearerDataLength;                               /* Bearer data length */
        UCHAR_T bearerData[EINSS7_A41MAP_MAX_BEARER_DATA_LENGTH];       /* Bearer data */
        BOOLEAN_T teleserviceIdUsed;
        USHORT_T teleserviceId;                                 /* SMS Teleservice Identifier */
        BOOLEAN_T actionCodeUsed;                               /* Action Code used */
        UCHAR_T actionCode;                                     /* Action Code */
        BOOLEAN_T esnUsed;                                      /* ESN used */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];           /* ESN */
        BOOLEAN_T minUsed;                                      /* MIN used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                   /* MIN */
        UCHAR_T imsiLen;                                        /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                   /* IMSI */
        BOOLEAN_T newMinUsed;                                   /* New MIN used */
        UCHAR_T newMobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                /* New MIN */
        UCHAR_T newImsiLen;                                     /* New IMSI length */
        UCHAR_T newImsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                /* New IMSI */
        BOOLEAN_T serviceIndUsed;                                 /* Service Indicator used*/
        UCHAR_T serviceInd;                                     /* Service Indicator */
        BOOLEAN_T smsChargeIndUsed;                             /* SMS Charge Indication used*/
        UCHAR_T smsChargeInd;                                   /* SMS Charge Indicator */
        EINSS7_A41MAP_ADDRINFO_T *destAddr_sp;                  /* Destination addr */
        BOOLEAN_T msgCountUsed;                                 /* Message count used */
        UCHAR_T msgCount;                                       /* Message count */
        BOOLEAN_T smsNotIndUsed;                                /* SMS Notification indicator used */
        UCHAR_T smsNotInd;                                      /* SMS Notification indicator */
        EINSS7_A41MAP_ADDRINFO_T *origDestAddr_sp;              /* Orig. destination addr */
        EINSS7_A41MAP_SUBADDRINFO_T *origDestSubaddr_sp;                /* Orig. dest. subaddr. */
        EINSS7_A41MAP_ADDRINFO_T *origOrigAddr_sp;              /* Orig.originating addr */
        EINSS7_A41MAP_SUBADDRINFO_T *origOrigSubaddr_sp;                /* Orig. orig. subaddr. */
        EINSS7_A41MAP_ADDRINFO_T *origAddr_sp;                  /* Originating addr */
        UCHAR_T tempRefNoLen;                                 /* Temporary Referense No length */
        UCHAR_T tempRefNo[EINSS7_A41MAP_MAX_TEMP_REF_LENGTH];           /* Temporary Reference No */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;        /* Extra Optional Parameter */
} EINSS7_A41MAP_SMDPP_REQ_T; 

/*********************************************************************/
/*                                                                   */
/*          S M D P P  R E S P   P A R A M E T E R S                 */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMDPP_RESP_TAG
{
        BOOLEAN_T authDeniedUsed;                               /* Authorization Denied used */
        UCHAR_T authDenied;                                     /* Authorization Denied */
        BOOLEAN_T denyAccessUsed;                               /* Deny Access used */
        UCHAR_T denyAccess;                                     /* Deny Access */
        BOOLEAN_T esnUsed;                                      /* ESN used */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];           /* ESN */
        BOOLEAN_T msMinUsed;                                    /* MS MIN used */
        UCHAR_T msMobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                 /* MS MIN */
        UCHAR_T msImsiLen;                                      /* MS IMSI length */
        UCHAR_T msImsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                 /* MS IMSI */
        BOOLEAN_T mscIdUsed;                                    /* MSC ID used*/
        UCHAR_T mscId[EINSS7_A41MAP_MSCID_LENGTH];                      /* MSC ID */
        BOOLEAN_T bearerDataUsed;                               /* Bearer data used */
        UCHAR_T bearerDataLength;                               /* Bearer data length */
        UCHAR_T bearerData[EINSS7_A41MAP_MAX_BEARER_DATA_LENGTH];       /* Bearer data */
        BOOLEAN_T causeCodeUsed;                                /* SMS Cause code used*/
        UCHAR_T causeCode;                                      /* SMS Cause code */
        BOOLEAN_T sysCapUsed;                                   /* System capabilities used */
        UCHAR_T sysCap;                                         /* System capabilities */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_A41MAP_SMDPP_RESP_T; 


/*********************************************************************/
/*                                                                   */
/*             S M S N O T  I N D   P A R A M E T E R S              */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMSNOT_IND_TAG
{
    UCHAR_T      electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH]; /* ESN */
    BOOLEAN_T    minUsed;                               /* MIN used, einpawe 990607 */
    UCHAR_T      mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];  /* MIN                       */
    UCHAR_T      imsiLen;                               /* Length of IMSI, einpawe 990607 */
    UCHAR_T      imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH];   /* IMSI, einpawe 990607 */
    BOOLEAN_T    teleserviceIdUsed;                 /* SMS Teleservice id used, einpawe 990607 */
    USHORT_T     teleserviceId;                     /* SMS Teleservice id, einpawe 990607 */
    EINSS7_A41MAP_ADDRINFO_T   *smsAddress_sp;      /* SMS address, einkarl 980227  */
    BOOLEAN_T    smsAccDeniedReasonUsed;            /* SMS Access Denied Reason used*/
    UCHAR_T      smsAccDeniedReason;                /* SMS Access Denied Reason     */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */    
} EINSS7_A41MAP_SMSNOT_IND_T;

/*********************************************************************/
/*                                                                   */
/*          S M S N O T   R E S P   P A R A M E T E R S              */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_A41MAP_SMSNOT_RESP_TAG
{
    BOOLEAN_T           causeCodeUsed;          /* SMS CauseCode used */
    UCHAR_T             causeCode;              /* SMS CauseCode */
    BOOLEAN_T           msgCountUsed;           /* SMS Message Count used */
    UCHAR_T             msgCount;               /* SMS Message Count */
    EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;    /* Extra optional parameters */    
} EINSS7_A41MAP_SMSNOT_RESP_T; 

/*********************************************************************/
/*                                                                   */
/*          S M S R E Q   C O N F   P A R A M E T E R S              */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMSREQ_CONF_TAG
{
        EINSS7_A41MAP_ADDRINFO_T smsAddress_s;                  /* SMS Address */
        UCHAR_T failureReason;                                  /* Failure reason */
        BOOLEAN_T esnUsed;                                      /* ESN used */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];           /* ESN */
        EINSS7_A41MAP_ADDRINFO_T *smsAddress_sp;                /* SMS Address */
        BOOLEAN_T smsAccDeniedReasonUsed;                       /* SMS AccessDeniedReason used */
        UCHAR_T smsAccDeniedReason;                             /* SMS Access Denied reason*/
        BOOLEAN_T causeCodeUsed;                                /* SMS Cause code used*/
        UCHAR_T causeCode;                                      /* SMS Cause code */
        UCHAR_T faultyParameterLength;                        /* Faulty parameter length*/
        UCHAR_T faultyParameter[EINSS7_A41MAP_MAX_PARA_CODE_LENGTH];    /* Faulty Parameter */
        BOOLEAN_T reportCauseUsed;                              /* Report cause used */
        UCHAR_T reportCause;                                    /* Report cause */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;        /* Extra Optional Parameter */
} EINSS7_A41MAP_SMSREQ_CONF_T; 

/*********************************************************************/
/*                                                                   */
/*          S M S R E Q   R E Q   P A R A M E T E R S                */
/*          Added by qinxrot 000222                                  */
/*********************************************************************/
/*                                                                   */
/*          T Y P E   D E F I T I O N S                              */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_SMSREQ_REQ_TAG
{
        BOOLEAN_T minUsed;                                        /* Mobile Id Number used */
        UCHAR_T mobileIdNo[EINSS7_A41MAP_MIN_LENGTH];                     /* MIN */
        UCHAR_T imsiLen;                                          /* IMSI length */
        UCHAR_T imsi[EINSS7_A41MAP_MAX_IMSI_LENGTH] ;                     /* IMSI */
        BOOLEAN_T esnUsed;                                        /* ESN used */
        UCHAR_T electronicSerialNo[EINSS7_A41MAP_ESN_LENGTH];     /* ESN */
        BOOLEAN_T serviceIndUsed;                                   /* Service Indicator used*/
        UCHAR_T serviceInd;                                       /* Service Indicator */
        EINSS7_A41MAP_MWNC_T *mwnc_sp;                            /* Message Waiting Notification Count */
        BOOLEAN_T smsNotIndUsed;                                  /* SMS Notification indicator used */
        UCHAR_T smsNotInd;                                        /* SMS Notification indicator */
        BOOLEAN_T teleserviceIdUsed;                              /* SMS TeleserviceId used */
        USHORT_T teleserviceId;                                   /* SMS Teleservice Identifier */
        EINSS7_A41MAP_EXTRAOPTPARAM_T *extraOptParam_sp;          /* Extra Optional Parameter */
} EINSS7_A41MAP_SMSREQ_REQ_T; 


/*********************************************************************/
/*                                                                   */
/*          S T A T E  I N D   P A R A M E T E R S                   */
/*             Added by einkarl 980227                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_STATE_TAG
{
    UCHAR_T     ssn;                     /* Subsystem Number         */ 
    UCHAR_T     userStatus;              /* User Status */
    UCHAR_T     affSsn;                  /* Affected Subsystem number  */ 
    ULONG_T     affSpc;                  /* Affected Signalling Point Code  */ 
    ULONG_T     locSpc;                  /* Local SPC */
    UCHAR_T     smi;                     /* Subsystem Multiplicity Indicator */
} EINSS7_A41MAP_STATE_T;


/*********************************************************************/
/*                                                                   */
/*          T R A N S A C T I O N   P A R A M E T E R S              */
/*             Added by einkarl 980227                               */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_A41MAP_TRANSACT_TAG
{
    UCHAR_T     ssn;                                  /* Subsystem number             */
    USHORT_T    transRef;                             /* Transaction reference        */
    UCHAR_T     origAddressLen;                       /* Origination Address length*/ 
    UCHAR_T     origAddress[EINSS7_A41MAP_MAX_ADDR_LENGTH];  /* Origination Address */
    UCHAR_T     destAddressLen;                       /* Destination Address length*/
    UCHAR_T     destAddress[EINSS7_A41MAP_MAX_ADDR_LENGTH];  /* Destination Address */
    UCHAR_T     priority;                             /* Priority */
    UCHAR_T     qualityOfService;                     /* Quality of Service */
    USHORT_T    timerValue;                           /* Timer Value */
} EINSS7_A41MAP_TRANSACT_T;



#ifdef EINSS7_A41MAP_FUNC_POINTER
/*********************************************************************/
/*                                                                   */
/*           EINSS7_A41MAP_FUNC_POINTER   R E T U R N    C O D E S   */
/*                                                                   */
/*********************************************************************/

#define EINSS7_A41MAP_CALLBACK_FUNC_NOT_SET   250

#endif

/*********************************************************************/
/*                                                                   */
/*               R E Q U E S T    P R O T O T Y P E S                */
/*                                                                   */
/*********************************************************************/

#ifndef EINSS7_NO_APISEND      /* qinxert 010226 */

extern USHORT_T EINSS7_A41MapBindReq( UCHAR_T, USHORT_T, USHORT_T );

extern USHORT_T EINSS7_A41MapUnbindReq( UCHAR_T );

extern USHORT_T EINSS7_A41MapEsposReqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                          EINSS7_A41MAP_ERROR_T *error_sp,
                                          EINSS7_A41MAP_ESPOSREQ_RESP_T *esposReqResp_sp );

extern USHORT_T EINSS7_A41MapFeatreqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                         EINSS7_A41MAP_ERROR_T *error_sp,
                                         EINSS7_A41MAP_FEATREQ_RESP_T *featreqResp_sp );
                                  
extern USHORT_T EINSS7_A41MapIsposCancResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                           EINSS7_A41MAP_ERROR_T *error_sp,
                                           EINSS7_A41MAP_ISPOSCANC_RESP_T *isposCancResp_sp);

extern USHORT_T EINSS7_A41MapIsposDirReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                         EINSS7_A41MAP_ISPOSDIR_REQ_T *isposDirReq_sp);

extern USHORT_T EINSS7_A41MapIsposReqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                          EINSS7_A41MAP_ERROR_T *error_sp,
                                          EINSS7_A41MAP_ISPOSREQ_RESP_T *isposReqResp_sp);

extern USHORT_T EINSS7_A41MapLocReqReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                          EINSS7_A41MAP_LOCREQ_REQ_T *locReqReq_sp);

extern USHORT_T EINSS7_A41MapMsInactiveResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                             EINSS7_A41MAP_ERROR_T *error_sp,
                                             EINSS7_A41MAP_MSINACTIVE_RESP_T *msinactiveResp_sp);

extern USHORT_T EINSS7_A41MapOriginationResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                              EINSS7_A41MAP_ERROR_T *error_sp,
                                              EINSS7_A41MAP_ORREQ_RESP_T *origResp_sp);

extern USHORT_T EINSS7_A41MapOtaspReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                      EINSS7_A41MAP_OTASP_REQ_T *otaspReq_sp);

extern USHORT_T EINSS7_A41MapQueryResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                        EINSS7_A41MAP_ERROR_T *error_sp,
                                        EINSS7_A41MAP_QUERY_RESP_T *queryResp_sp);

extern USHORT_T EINSS7_A41MapReddirReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_REDDIR_REQ_T *reddirReq_sp);

extern USHORT_T EINSS7_A41MapRegCancelResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                            EINSS7_A41MAP_ERROR_T *error_sp,
                                            EINSS7_A41MAP_REGCANCEL_RESP_T *regCancelResp_sp);

extern USHORT_T EINSS7_A41MapRegistrationResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                               EINSS7_A41MAP_ERROR_T *error_sp,
                                               EINSS7_A41MAP_REGNOT_RESP_T *regnotResp_sp);
                                     
extern USHORT_T EINSS7_A41MapRoutreqReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                        EINSS7_A41MAP_ROUTREQ_REQ_T *routreqReq_sp);

extern USHORT_T EINSS7_A41MapRoutreqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                         EINSS7_A41MAP_ERROR_T *error_sp,
                                         EINSS7_A41MAP_ROUTREQ_RESP_T *routreqResp_sp);

extern USHORT_T EINSS7_A41MapSmdppReq( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_SMDPP_REQ_T *smdppReq_sp);

extern USHORT_T EINSS7_A41MapSmdppResp( EINSS7_A41MAP_TRANSACT_T *transact_sp, 
                                        EINSS7_A41MAP_SMDPP_RESP_T *smdppResp_sp);

extern USHORT_T EINSS7_A41MapSmsnotResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                        EINSS7_A41MAP_ERROR_T *error_sp,
                                        EINSS7_A41MAP_SMSNOT_RESP_T *smsnotResp_sp);

extern USHORT_T EINSS7_A41MapSmsreqReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_SMSREQ_REQ_T *smsreqReq_sp);

#else  /* NO_APISEND */

extern USHORT_T EINSS7_A41MapBindReq( UCHAR_T, USHORT_T, USHORT_T, MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapUnbindReq( UCHAR_T, MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapEsposReqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                          EINSS7_A41MAP_ERROR_T *error_sp,
                                          EINSS7_A41MAP_ESPOSREQ_RESP_T *esposReqResp_sp,
                                          MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapFeatreqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                         EINSS7_A41MAP_ERROR_T *error_sp,
                                         EINSS7_A41MAP_FEATREQ_RESP_T *featreqResp_sp,
                                         MSG_T *msg_sp );
                                  
extern USHORT_T EINSS7_A41MapIsposCancResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                           EINSS7_A41MAP_ERROR_T *error_sp,
                                           EINSS7_A41MAP_ISPOSCANC_RESP_T *isposCancResp_sp,
                                           MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapIsposDirReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                         EINSS7_A41MAP_ISPOSDIR_REQ_T *isposDirReq_sp,
                                         MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapIsposReqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                          EINSS7_A41MAP_ERROR_T *error_sp,
                                          EINSS7_A41MAP_ISPOSREQ_RESP_T *isposReqResp_sp,
                                          MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapLocReqReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_LOCREQ_REQ_T *locReqReq_sp,
                                       MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapMsInactiveResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                             EINSS7_A41MAP_ERROR_T *error_sp,
                                             EINSS7_A41MAP_MSINACTIVE_RESP_T *msinactiveResp_sp,
                                             MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapOriginationResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                              EINSS7_A41MAP_ERROR_T *error_sp,
                                              EINSS7_A41MAP_ORREQ_RESP_T *origResp_sp,
                                              MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapOtaspReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                      EINSS7_A41MAP_OTASP_REQ_T *otaspReq_sp,
                                      MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapQueryResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                        EINSS7_A41MAP_ERROR_T *error_sp,
                                        EINSS7_A41MAP_QUERY_RESP_T *queryResp_sp,
                                        MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapReddirReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_REDDIR_REQ_T *reddirReq_sp,
                                       MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapRegCancelResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                            EINSS7_A41MAP_ERROR_T *error_sp,
                                            EINSS7_A41MAP_REGCANCEL_RESP_T *regCancelResp_sp,
                                            MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapRegistrationResp( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                               EINSS7_A41MAP_ERROR_T *error_sp,
                                               EINSS7_A41MAP_REGNOT_RESP_T *regnotResp_sp,
                                               MSG_T *msg_sp );
                                     
extern USHORT_T EINSS7_A41MapRoutreqReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                        EINSS7_A41MAP_ROUTREQ_REQ_T *routreqReq_sp,
                                        MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapRoutreqResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                         EINSS7_A41MAP_ERROR_T *error_sp,
                                         EINSS7_A41MAP_ROUTREQ_RESP_T *routreqResp_sp,
                                         MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapSmdppReq( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_SMDPP_REQ_T *smdppReq_sp,
                                       MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapSmdppResp( EINSS7_A41MAP_TRANSACT_T *transact_sp, 
                                        EINSS7_A41MAP_SMDPP_RESP_T *smdppResp_sp,
                                        MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapSmsnotResp(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                        EINSS7_A41MAP_ERROR_T *error_sp,
                                        EINSS7_A41MAP_SMSNOT_RESP_T *smsnotResp_sp,
                                        MSG_T *msg_sp );

extern USHORT_T EINSS7_A41MapSmsreqReq(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                       EINSS7_A41MAP_SMSREQ_REQ_T *smsreqReq_sp,
                                       MSG_T *msg_sp );

#endif



/*********************************************************************/
/*                                                                   */
/*                    H E L P   F U N C T I O N                      */
/*                       P R O T O T Y P E S                         */
/*                                                                   */
/*********************************************************************/
extern USHORT_T EINSS7_A41MapHandleInd(MSG_T *msg_sp);
extern USHORT_T EINSS7_A41MapIndError(USHORT_T indErrorCode, MSG_T *msg_sp);
#ifdef EINSS7_A41MAP_FUNC_POINTER
/* creates the datatypes with "pointer to function" */

typedef USHORT_T (*EINSS7A41MAPBINDCONF_T)( UCHAR_T ssn, UCHAR_T bindStatus);

typedef USHORT_T (*EINSS7A41MAPESPOSREQIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ESPOSREQ_IND_T *esposReqInd_sp);

typedef USHORT_T (*EINSS7A41MAPFEATREQIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_FEATREQ_IND_T *featreqInd_sp);

typedef USHORT_T (*EINSS7A41MAPISPOSCANCIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ISPOSCANC_IND_T *isposCancInd_sp);

typedef USHORT_T (*EINSS7A41MAPISPOSDIRCONF_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ISPOSDIR_CONF_T *isposDirConf_sp);

typedef USHORT_T (*EINSS7A41MAPISPOSREQIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ISPOSREQ_IND_T *isposReqInd_sp);

typedef USHORT_T (*EINSS7A41MAPLOCREQCONF_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_LOCREQ_CONF_T *locReqConf_sp);

typedef USHORT_T (*EINSS7A41MAPMSINACTIVEIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                     EINSS7_A41MAP_MSINACTIVE_IND_T *msinactiveInd_sp);
                                     
typedef USHORT_T (*EINSS7A41MAPORIGINATIONIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp, 
                                      EINSS7_A41MAP_ORREQ_IND_T *origInd_sp);
                                      
typedef USHORT_T (*EINSS7A41MAPOTASPCONF_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                 EINSS7_A41MAP_OTASP_CONF_T *otaspConf_sp);

typedef USHORT_T (*EINSS7A41MAPPABORTIND_T)( EINSS7_A41MAP_P_ABORT_IND_T *abortInd_sp);
                                 
typedef USHORT_T (*EINSS7A41MAPQUERYIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                 EINSS7_A41MAP_QUERY_IND_T *queryInd_sp);

typedef USHORT_T (*EINSS7A41MAPREDDIRCONF_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                 EINSS7_A41MAP_REDDIR_CONF_T *reddirConf_sp);

typedef USHORT_T (*EINSS7A41MAPREGCANCELIND_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp, 
                                    EINSS7_A41MAP_REGCANCEL_IND_T *regCancelInd_sp);

typedef USHORT_T (*EINSS7A41MAPREGISTRATIONIND_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                      EINSS7_A41MAP_REGNOT_IND_T *regnotInd_sp);

typedef USHORT_T (*EINSS7A41MAPROUTREQCONF_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ROUTREQ_CONF_T *routreqConf_sp);

typedef USHORT_T (*EINSS7A41MAPROUTREQIND_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                      EINSS7_A41MAP_ROUTREQ_IND_T *routreqInd_sp);

typedef USHORT_T (*EINSS7A41MAPSMDPPCONF_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                EINSS7_A41MAP_SMDPP_CONF_T *smdppConf_sp);

typedef USHORT_T (*EINSS7A41MAPSMDPPIND_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                               EINSS7_A41MAP_SMDPP_IND_T *smdppInd_sp);

typedef USHORT_T (*EINSS7A41MAPSMSNOTIND_T)(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                EINSS7_A41MAP_SMSNOT_IND_T *smsnotInd_sp);
                                
typedef USHORT_T (*EINSS7A41MAPSMSREQCONF_T)( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_SMSREQ_CONF_T *smsreqConf_sp);

typedef USHORT_T (*EINSS7A41MAPSTATE_T)( EINSS7_A41MAP_STATE_T *state_sp);

typedef USHORT_T (*EINSS7A41MAPINDERROR_T)( USHORT_T indErrorCode, 
                                  MSG_T *msg_sp);

typedef struct EINSS7_A41MAP_INIT_TAG
{
    EINSS7A41MAPBINDCONF_T EINSS7_A41MapBindConf;

    EINSS7A41MAPESPOSREQIND_T EINSS7_A41MapEsposReqInd;
    
    EINSS7A41MAPFEATREQIND_T EINSS7_A41MapFeatreqInd; 

    EINSS7A41MAPINDERROR_T EINSS7_A41MapIndError;   

    EINSS7A41MAPLOCREQCONF_T EINSS7_A41MapLocReqConf;

    EINSS7A41MAPMSINACTIVEIND_T EINSS7_A41MapMsInactiveInd;
    
    EINSS7A41MAPORIGINATIONIND_T EINSS7_A41MapOriginationInd;
    
    EINSS7A41MAPOTASPCONF_T EINSS7_A41MapOtaspConf;

    EINSS7A41MAPPABORTIND_T EINSS7_A41MapPAbortInd;        

    EINSS7A41MAPREDDIRCONF_T EINSS7_A41MapReddirConf;
    
    EINSS7A41MAPREGCANCELIND_T EINSS7_A41MapRegCancelInd;
    
    EINSS7A41MAPREGISTRATIONIND_T EINSS7_A41MapRegistrationInd;
    
    EINSS7A41MAPROUTREQCONF_T EINSS7_A41MapRoutreqConf;
    
    EINSS7A41MAPROUTREQIND_T EINSS7_A41MapRoutreqInd;
    
    EINSS7A41MAPSMDPPCONF_T EINSS7_A41MapSmdppConf;
    
    EINSS7A41MAPSMDPPIND_T EINSS7_A41MapSmdppInd;
    
    EINSS7A41MAPSMSNOTIND_T EINSS7_A41MapSmsnotInd;
    
    EINSS7A41MAPSMSREQCONF_T EINSS7_A41MapSmsreqConf;
    
    EINSS7A41MAPSTATE_T EINSS7_A41MapStateInd; 

#ifdef EINSS7_ERICSSON_PROPRIETARY   

    EINSS7A41MAPISPOSCANCIND_T EINSS7_A41MapIsposCancInd;

    EINSS7A41MAPISPOSDIRCONF_T EINSS7_A41MapIsposDirConf;

    EINSS7A41MAPISPOSREQIND_T EINSS7_A41MapIsposReqInd;

    EINSS7A41MAPQUERYIND_T EINSS7_A41MapQueryInd;

#endif

}EINSS7_A41MAP_INIT_T;
#else
/*********************************************************************/
/*                                                                   */
/*        C O N F I R M A T I O N   A N D  I N D I C A T I O N       */
/*                     P R O T O T Y P E S                           */
/*                                                                   */
/*********************************************************************/

extern USHORT_T EINSS7_A41MapBindConf( UCHAR_T ssn, UCHAR_T bindStatus);

extern USHORT_T EINSS7_A41MapEsposReqInd( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ESPOSREQ_IND_T *esposReqInd_sp);

extern USHORT_T EINSS7_A41MapFeatreqInd( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_FEATREQ_IND_T *featreqInd_sp);

extern USHORT_T EINSS7_A41MapIsposCancInd( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ISPOSCANC_IND_T *isposCancInd_sp);

extern USHORT_T EINSS7_A41MapIsposDirConf( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ISPOSDIR_CONF_T *isposDirConf_sp);

extern USHORT_T EINSS7_A41MapIsposReqInd( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ISPOSREQ_IND_T *isposReqInd_sp);

extern USHORT_T EINSS7_A41MapLocReqConf( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_LOCREQ_CONF_T *locReqConf_sp);

extern USHORT_T EINSS7_A41MapMsInactiveInd( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                     EINSS7_A41MAP_MSINACTIVE_IND_T *msinactiveInd_sp);

extern USHORT_T EINSS7_A41MapOriginationInd( EINSS7_A41MAP_TRANSACT_T *transact_sp, 
                                      EINSS7_A41MAP_ORREQ_IND_T *origInd_sp);
                                      
extern USHORT_T EINSS7_A41MapOtaspConf( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                 EINSS7_A41MAP_OTASP_CONF_T *otaspConf_sp);

extern USHORT_T EINSS7_A41MapPAbortInd( EINSS7_A41MAP_P_ABORT_IND_T *abortInd_sp);

extern USHORT_T EINSS7_A41MapQueryInd( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                 EINSS7_A41MAP_QUERY_IND_T *queryInd_sp);

extern USHORT_T EINSS7_A41MapReddirConf(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                 EINSS7_A41MAP_REDDIR_CONF_T *reddirConf_sp);

extern USHORT_T EINSS7_A41MapRegCancelInd( EINSS7_A41MAP_TRANSACT_T *transact_sp, 
                                    EINSS7_A41MAP_REGCANCEL_IND_T *regCancelInd_sp);

extern USHORT_T EINSS7_A41MapRegistrationInd(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                      EINSS7_A41MAP_REGNOT_IND_T *regnotInd_sp);

extern USHORT_T EINSS7_A41MapRoutreqConf(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_ROUTREQ_CONF_T *routreqConf_sp);

extern USHORT_T EINSS7_A41MapRoutreqInd(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                      EINSS7_A41MAP_ROUTREQ_IND_T *routreqInd_sp);

extern USHORT_T EINSS7_A41MapSmdppConf(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                EINSS7_A41MAP_SMDPP_CONF_T *smdppConf_sp);

extern USHORT_T EINSS7_A41MapSmdppInd(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                               EINSS7_A41MAP_SMDPP_IND_T *smdppInd_sp);

extern USHORT_T EINSS7_A41MapSmsnotInd(EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                EINSS7_A41MAP_SMSNOT_IND_T *smsnotInd_sp);
                                
extern USHORT_T EINSS7_A41MapSmsreqConf( EINSS7_A41MAP_TRANSACT_T *transact_sp,
                                  EINSS7_A41MAP_SMSREQ_CONF_T *smsreqConf_sp);

extern USHORT_T EINSS7_A41MapStateInd( EINSS7_A41MAP_STATE_T *state_sp);

#endif /* EINSS7_A41MAP_FUNC_POINTER */ 

#ifdef EINSS7_A41MAP_FUNC_POINTER
/* The application must call this function to register the callback functions. */
    USHORT_T EINSS7_A41MapRegFunc(EINSS7_A41MAP_INIT_T *);
#endif /* EINSS7_A41MAP_FUNC_POINTER */ 

#if defined (__cplusplus) || (c_plusplus)
}
#endif
#endif
