/*********************************************************************/
/* Don't remove these two lines, contain depends on them!            */
/* Document Number: %Container% Revision: %Revision% */
/*                                                                   */
/* et94inapcs1plusapi.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infocom AB 1996                                */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infocom Consultants AB. The program may be used and/or   */
/* copied only with the written permission from Ericsson Infocom     */
/* Consultants AB or in the accordance with the terms and conditions */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 Rev:                             */
/* <n>/190 55-<product number> Ux   A     */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: et94inapcs1plusapi.h,v 1.4 2000/05/26 11:53:50 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/B  Andreas Wilde                                            */
/*                                                                   */
/* Purpose:                                                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-97XXXX  EIN/N/B  Andreas Wilde                                 */
/*              Original version.                                    */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

#ifndef __ET94INAPCS1PLUSCOMMONAPI_H__
#define __ET94INAPCS1PLUSCOMMONAPI_H__
#if defined __cplusplus || defined c_plusplus
extern "C" {
#endif

#include        "et94inapcs1defapi.h"
#include        "et94inapcs1plusdefapi.h"

/*********************************************************************/
/*                                                                   */
/*            DATATYPES FOR REQUEST AND INDICATION FUNCTIONS         */
/*                                                                   */
/*********************************************************************/
typedef RESULTL_T RESULTNL_T;
#define RESULTNL                4
#define IN_RESULTNL             4
#define INAP_RESULT_NL_REQ      12
#define INAP_RESULT_NL_IND      11

typedef struct
{
        UCHAR_T  abInfoLen;
        UCHAR_T  abInfo[MAX_ABINFO_LEN];
        UCHAR_T  acNameLen;
        UCHAR_T  acName[MAX_ACNAME_LEN];
        USHORT_T uInfoLen;
        UCHAR_T  uInfo[MAX_UINFO_LEN];
} ABORT_T;

typedef enum
{
        IN_E_AC_CS1_SSP_2_SCP = 0,
        IN_E_AC_CS1_ASSIST_HOFF_SSP_2_SCP,
        IN_E_AC_CS1_IP_2_SCP,
        IN_E_AC_CS1_SCP_2_SSP,
        IN_E_AC_CS1_SCP_2_SSP_TFCMGMT,
        IN_E_AC_CS1_SCP_2_SSP_SRVCMGMT,
        IN_E_AC_CS1_SSP_2_SCP_SRVCMGMT,
        IN_CPL_AC_SSP_TO_SCP = 20,
        IN_CPL_AC_ASSIST_HOFF_SSP_TO_SCP,
        IN_CPL_AC_IP_TO_SCP,
        IN_CPL_AC_SCP_TO_SSP,
        IN_CPL_AC_SCP_TO_SSP_TFCMGMT,
        IN_CPL_AC_SCP_TO_SSP_SRVCMGMT,
        IN_CPL_AC_SSP_TO_SCP_SRVCMGMT,
        IN_CPL_AC_DATA_MGMT,
        IN_CPL_AC_SCP_SSP_TFCLIM,
        IN_CPL_AC_SSP_TO_SCP_LOCAL,
        IN_CPL_AC_ASSIST_HOFF_SSP_TO_SCP_LOCAL,
        IN_CPL_AC_IP_TO_SCP_LOCAL,
        IN_CPL_AC_SCP_TO_SSP_LOCAL,
        IN_CPL_AC_SCP_TO_SSP_TFCMGMT_LOCAL,
        IN_CPL_AC_SCP_TO_SSP_SRVCMGMT_LOCAL,
        IN_CPL_AC_SSP_TO_SCP_SRVCMGMT_LOCAL,
        IN_CPL_AC_DATA_MGMT_LOCAL,
        IN_CPL_AC_SCP_SSP_TFCLIM_LOCAL,
        IN_E_AC_CS2_SCP_2_SDP_EXTDIRACC
} WELLKNOWNAC_T;

#define MIN_CS1PLUS_ACSHORT     20

typedef struct
{
  BOOLEAN_T             knownAc;        /* TRUE = The recived AC is known
                                        FALSE = The recived AC is NOT known */
  union
  {
        WELLKNOWNAC_T wellKnownAc;
        struct
        {
                UCHAR_T         acNameLen;
                UCHAR_T         acName[MAX_ACNAME_LEN];
        }       unknownAc;
  } u;
} AC_NAMEREQ_T;

/*********************************************************************/
/*                                                                   */
/*            DATATYPES FOR ASN.1/BER ENCODING AND DECODING          */
/*                                                                   */
/*********************************************************************/
typedef struct
{
        UCHAR_T         errorCode;      /* Values: See ch. 2.5.3 */
        union
        {
        CANCELFAILED_T                          cancelFailed;
        CPLCANCELFAILED_T                       cplCancelFailed;
        CONGESTION_T                            congestion;
        IMPROPERCALLERRESPONSE_T                improperCallerResponse;
        ERRORINPARAMETERVALUE_T                 errorInParameterValue;
        EXECUTIONERROR_T                        executionError;
        ILLEGALCOMBINATIONOFPARAMETERS_T        illegalCombinationOfParameters;
        INFONOTAVAILABLE_T                      infoNotAvailable;
        INVALIDDATAITEMID_T                     invalidDataItemID;
        NOTAUTHORIZED_T                         notAuthorized;
        OTHERERROR_T                            otherError;
        PARAMETERMISSING_T                      parameterMissing;
        REQUESTEDINFOERROR_T                    requestedInfoError;
        SYSTEMFAILURE_T                         systemFailure;
        TASKREFUSED_T                           taskRefused;
        ATTRIBUTEERROR_T                        attributeError;
        NAMEERROR_T                             nameError;
        EXECERROR_T                             execError;
        } eprm;
} ERR_T;

typedef struct
{
        UCHAR_T         operation;
        union
        {
        RECEIVEDINFORMATIONARG_T      receivedInformationArg;
        EXECUTERESULT_T               executeResult;
        CPLRECEIVEDINFORMATIONARG_T             cplReceivedInformationArg;
        RETRIEVERESULTARG_T                     retrieveResultArg;
        UPDATERESULTARG_T                       updateResultArg;
        } prm_u;
} RES_T;

typedef struct
{
        UCHAR_T         operation;      /* Values: see ch. 3.8.4 */
        union
        {
        ACTIVATESERVICEFILTERINGARG_T   activateServiceFilteringArg;
        CPLACTIVATESERVICEFILTERINGARG_T cplActivateServiceFilteringArg;
        ACTIVITYTESTARG_T               activityTestArg;
        APPLYCHARGINGARG_T              applyChargingArg;
        CPLAPPLYCHARGINGARG_T           cplApplyChargingArg;
        APPLYCHARGINGREPORTARG_T        applyChargingReportArg;
        CPLAPPLYCHARGINGREPORTARG_T     cplApplyChargingReportArg;
        ASSISTREQUESTINSTRUCTIONSARG_T assistRequestInstructionsArg;
        CPLASSISTREQUESTINSTRUCTIONSARG_T cplAssistRequestInstructionsArg;
        CALLGAPARG_T                    callGapArg;
        CPLCALLGAPARG_T                 cplCallGapArg;
        CALLINFORMATIONREPORTARG_T      callInformationReportArg;
        CPLCALLINFORMATIONREPORTARG_T   cplCallInformationReportArg;
        CALLINFORMATIONREQUESTARG_T     callInformationRequestArg;
        CPLCALLINFORMATIONREQUESTARG_T  cplCallInformationRequestArg;
        CALLLIMITARG_T                  callLimitArg;
        CANCELARG_T                     cancelArg;
        COLLECTINFORMATIONARG_T         collectInformationArg;
        CONNECTARG_T                    connectArg;
        CPLCONNECTARG_T                 cplConnectArg;
        CONNECTTORESOURCEARG_T          connectToResourceArg;
        CPLCONNECTTORESOURCEARG_T       cplConnectToResourceArg;
        CONTINUEARG_T                   continueArg;
        CPLCONTINUEARG_T                cplContinueArg;
        DIALOGUEUSERINFORMATIONARG_T    dialogueUserInformationArg;
        DISCONNECTFORWARDCONNECTIONARG_T disconnectForwardConnectionArg;
        CPLDISCONNECTFORWARDCONNECTIONARG_T cplDisconnectForwardConnectionArg;
        ESTABLISHTEMPORARYCONNECTIONARG_T establishTemporaryConnectionArg;
        CPLESTABLISHTEMPORARYCONNECTIONARG_T cplEstablishTemporaryConnectionArg;
        EVENTNOTIFICATIONCHARGINGARG_T  eventNotificationChargingArg;
        CPLEVENTNOTIFICATIONCHARGINGARG_T cplEventNotificationChargingArg;
        EVENTREPORTBCSMARG_T            eventReportBCSMArg;
        CPLEVENTREPORTBCSMARG_T         cplEventReportBCSMArg;
        FURNISHCHARGINGINFORMATIONARG_T furnishChargingInformationArg;
        CPLFURNISHCHARGINGINFORMATIONARG_T cplFurnishChargingInformationArg;
        CPLINITIALDPARG_T               handOverArg;
        HOLDCALLPARTYCONNECTIONARG_T    holdCallPartyConnectionArg;
        INITIALDPARG_T                  initialDPArg;
        CPLINITIALDPARG_T               cplInitialDPArg;
        INITIATECALLATTEMPTARG_T        initiateCallAttemptArg;
        CPLINITIATECALLATTEMPTARG_T     cplInitiateCallAttemptArg;
        PLAYANNOUNCEMENTARG_T           playAnnouncementArg;
        CPLPLAYANNOUNCEMENTARG_T        cplPlayAnnouncementArg;
        PROMPTANDCOLLECTUSERINFORMATIONARG_T promptAndCollectUserInformationArg;
        CPLPROMPTANDCOLLECTUSERINFORMATIONARG_T
                                        cplPromptAndCollectUserInformationArg;
        RELEASECALLARG_T                releaseCallArg;
        RECONNECTARG_T                  reconnectArg;
        RELEASECALLPARTYCONNECTIONARG_T releaseCallPartyConnectionArg;
        REQUESTNOTIFICATIONCHARGINGEVENTARG_T 
                                        requestNotificationChargingEventArg;
        CPLREQUESTNOTIFICATIONCHARGINGEVENTARG_T
                                        cplRequestNotificationChargingEventArg;
        REQUESTREPORTBCSMEVENTARG_T     requestReportBCSMEventArg;
        CPLREQUESTREPORTBCSMEVENTARG_T  cplRequestReportBCSMEventArg;
        RESETTIMERARG_T                 resetTimerArg;
        RETRIEVEARG_T                   retrieveArg;
        SENDCHARGINGINFORMATIONARG_T    sendChargingInformationArg;
        CPLSENDCHARGINGINFORMATIONARG_T cplSendChargingInformationArg;
        SERVICEFILTERINGRESPONSEARG_T   serviceFilteringResponseArg;
        CPLSERVICEFILTERINGRESPONSEARG_T cplServiceFilteringResponseArg;
        SIGNALLINGINFORMATIONARG_T      signallingInformationArg;
        SPECIALIZEDRESOURCEREPORTARG_T  specializedResourceReportArg;
        CPLSPECIALIZEDRESOURCEREPORTARG_T cplSpecializedResourceReportArg;
        UPDATEARG_T                     updateArg;
        EXECUTEARG_T                    executeArg;
        } prm_u;
} OPER_T;

typedef struct
{
        UCHAR_T                 compType;
        union
        {
                INVOKE_T                        invoke;
                LCANCEL_T                       lcancel;
                LREJECT_T                       lreject;
                RESULTL_T                       resultl;
                RREJECT_T                       rreject;
                UCANCEL_T                       ucancel;
                UERROR_T                        uerror;
                UREJECT_T                       ureject;
        } u;
        union
        {
                struct
                {
                        USHORT_T                paramLen;
                        UCHAR_T                 param[MAX_PARAM_LEN];
                }               buff;
                OPER_T          op;
                ERR_T           err;
                RES_T           res;
        } pm;
} COMP_T;

#include        "et94inapapidef.h"

#if defined __cplusplus || defined c_plusplus
}
#endif
#endif /* __ET94INAPCS1PLUSCOMMONAPI_H__ */

