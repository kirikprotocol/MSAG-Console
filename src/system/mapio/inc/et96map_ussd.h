/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_ussd.h,v                                                  */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_ussd.h                                                    */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
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
/* 8/190 55-CAA 201 45 Ux                                            */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.6 2000/04/07 09:05:41 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Peter Ryan (QINXRYP)                                              */
/* AePONA Ltd                                                   */
/*                                                                   */
/* Purpose:                                                          */
/* Contains the prototypes for the MAP API USSD functions            */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 971105       Ulf Melin       Changed MAP_USSD_MSG_T to            */
/*                              MAP_USSD_MESS_T                      */
/*                              Changed MAP_ERR_CODE_T to            */
/*                              ET96MAP_ERROR_CODE_T                 */
/*                              Changed return codes from            */
/*                              MAP_API_RESULT_T to USHORT_T         */
/* Revision record:                                                  */
/* 980406       Ulf Hellsten                                         */
/*                                                                   */
/* 010213       QINXRYP         Added prototypes for V1 USSD         */
/*                              primitives and updated prototypes    */
/*                              for V2 USSD primitives according to  */
/*                              ETSI/3GPP specs.                     */
/*  010419      Ulf Hellsten    Added Notify conf                    */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __ET96MAP_USSD_H__

#define __ET96MAP_USSD_H__


/* Values for Alerting Pattern */
#define ET96MAP_ALERTING_PATTERN_LEVEL0 0
#define ET96MAP_ALERTING_PATTERN_LEVEL1 1
#define ET96MAP_ALERTING_PATTERN_LEVEL2 2
#define ET96MAP_ALERTING_PATTERN_CATEGORY1 4
#define ET96MAP_ALERTING_PATTERN_CATEGORY2 5
#define ET96MAP_ALERTING_PATTERN_CATEGORY3 6
#define ET96MAP_ALERTING_PATTERN_CATEGORY4 7
#define ET96MAP_ALERTING_PATTERN_CATEGORY5 8




#ifndef EINSS7_FUNC_POINTER


USHORT_T Et96MapV1BeginSubscriberActivityInd(ET96MAP_LOCAL_SSN_T,
                                 ET96MAP_DIALOGUE_ID_T,
                                 ET96MAP_INVOKE_ID_T,
                                 ET96MAP_IMSI_T,
                                 ET96MAP_ADDRESS_T);

USHORT_T Et96MapV1ProcessUnstructuredSSDataInd(ET96MAP_LOCAL_SSN_T,
                                 ET96MAP_DIALOGUE_ID_T,
                                 ET96MAP_INVOKE_ID_T,
                                 ET96MAP_SS_USER_DATA_T);

USHORT_T Et96MapV2ProcessUnstructuredSSRequestInd(ET96MAP_LOCAL_SSN_T,
                                 ET96MAP_DIALOGUE_ID_T,
                                 ET96MAP_INVOKE_ID_T,
                                 ET96MAP_USSD_DATA_CODING_SCHEME_T,
                                 ET96MAP_USSD_STRING_T,
                                 ET96MAP_ADDRESS_T*);

USHORT_T Et96MapV2UnstructuredSSRequestConf(ET96MAP_LOCAL_SSN_T,
                                 ET96MAP_DIALOGUE_ID_T,
                                 ET96MAP_INVOKE_ID_T,
                                 ET96MAP_USSD_DATA_CODING_SCHEME_T*,
                                 ET96MAP_USSD_STRING_T*,
                                 ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T*,
                                 ET96MAP_PROV_ERR_T*);

USHORT_T     Et96MapV2UnstructuredSSNotifyConf(ET96MAP_LOCAL_SSN_T localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_INVOKE_ID_T       invokeId,
                ET96MAP_ERROR_USSD_NOTIFY_T *,
                ET96MAP_PROV_ERR_T*);

#endif


USHORT_T  Et96MapV2UnstructuredSSNotifyReq(ET96MAP_LOCAL_SSN_T     localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_INVOKE_ID_T       invokeId,
                ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDcs,
                ET96MAP_USSD_STRING_T ussdString_s,
                ET96MAP_ALERTING_PATTERN_T *alertingPattern_p);


USHORT_T  Et96MapV2UnstructuredSSRequestReq(ET96MAP_LOCAL_SSN_T      localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_INVOKE_ID_T invokeId,
                ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDcs,
                ET96MAP_USSD_STRING_T ussdString_s,
                ET96MAP_ALERTING_PATTERN_T *alertingPattern_p);


USHORT_T  ValEt96MapV2UnstructuredSSNotifyReq(ET96MAP_LOCAL_SSN_T     localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_INVOKE_ID_T       invokeId,
                ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDcs,
                ET96MAP_USSD_STRING_T ussdString_s,
                ET96MAP_ALERTING_PATTERN_T *alertingPattern_p);


USHORT_T  ValEt96MapV2UnstructuredSSRequestReq(ET96MAP_LOCAL_SSN_T      localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_INVOKE_ID_T invokeId,
                ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDcs,
                ET96MAP_USSD_STRING_T ussdString_s,
                ET96MAP_ALERTING_PATTERN_T *alertingPattern_p);


USHORT_T Et96MapV1ProcessUnstructuredSSDataResp(ET96MAP_LOCAL_SSN_T localSsn,
                                 ET96MAP_DIALOGUE_ID_T dialogueId,
                                 ET96MAP_INVOKE_ID_T invokeId,
                                 ET96MAP_SS_USER_DATA_T *ssUserData_sp,
                                 ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_DATA_T
                                        *errorProcessUnstructuredSSData_sp);

USHORT_T Et96MapV2ProcessUnstructuredSSRequestResp(ET96MAP_LOCAL_SSN_T,
                                 ET96MAP_DIALOGUE_ID_T,
                                 ET96MAP_INVOKE_ID_T,
                                 ET96MAP_USSD_DATA_CODING_SCHEME_T* ussdDcs_p,
                                 ET96MAP_USSD_STRING_T* ussdString_sp,
                                 ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_REQUEST_T 
                                 *errorProcessUnstructuredSSReq_sp);


USHORT_T ValEt96MapV1ProcessUnstructuredSSDataResp(ET96MAP_LOCAL_SSN_T localSsn,
                                 ET96MAP_DIALOGUE_ID_T dialogueId,
                                 ET96MAP_INVOKE_ID_T invokeId,
                                 ET96MAP_SS_USER_DATA_T *ssUserData_sp,
                                 ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_DATA_T
                                         *errorProcessUnstructuredSSData_sp);


USHORT_T ValEt96MapV2ProcessUnstructuredSSRequestResp(ET96MAP_LOCAL_SSN_T localSsn,
                                 ET96MAP_DIALOGUE_ID_T dialogueId,
                                 ET96MAP_INVOKE_ID_T invokeId,
                                 ET96MAP_USSD_DATA_CODING_SCHEME_T* ussdDcs_p,
                                 ET96MAP_USSD_STRING_T* ussdString_sp,
                                 ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_REQUEST_T
                                  *errorProcessUnstructuredSSReq_sp);
                         
USHORT_T  Et96MapV1UnpackBeginSubscriberActivityInd(MSG_T *ipcMessage_sp,
                                                ET96MAP_BEGIN_SUBSCRIBER_ACTIVITY_IND_T 
                                                *mBeginSubscriberActivityInd_sp);

                                                 
USHORT_T  Et96MapV1UnpackProcessUnstructuredSSDataInd(MSG_T *ipcMessage_sp,
                                                ET96MAP_PROCESS_UNSTRUCTURED_SS_DATA_IND_T 
                                                *mProcessUnstructuredSSDataInd_sp);

USHORT_T  Et96MapV2UnpackProcessUnstructuredSSRequestInd(MSG_T *ipcMessage_sp,
                                                ET96MAP_PROCESS_UNSTRUCTURED_SS_REQUEST_IND_T 
                                                *mProcessUnstructuredSSRequestInd_sp);

USHORT_T  ValEt96MapV1BeginSubscriberActivityInd(ET96MAP_LOCAL_SSN_T localSsn,
                                                  ET96MAP_DIALOGUE_ID_T dialogueId,
                                                  ET96MAP_INVOKE_ID_T invokeId,
                                                  ET96MAP_IMSI_T imsi_s,
                                                  ET96MAP_ADDRESS_T originatingEntityNumber_s);
                         
USHORT_T  ValEt96MapV1ProcessUnstructuredSSDataInd(ET96MAP_LOCAL_SSN_T localSsn,
                                                  ET96MAP_DIALOGUE_ID_T dialogueId,
                                                  ET96MAP_INVOKE_ID_T invokeId,
                                                  ET96MAP_SS_USER_DATA_T ssUserData_s);

USHORT_T  ValEt96MapV2ProcessUnstructuredSSRequestInd(ET96MAP_LOCAL_SSN_T localSsn,
                                                  ET96MAP_DIALOGUE_ID_T dialogueId,
                                                  ET96MAP_INVOKE_ID_T invokeId,
                                                  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDcs,
                                                  ET96MAP_USSD_STRING_T ussdString_s,
                                                  ET96MAP_ADDRESS_T* msisdn_sp);
                                                  

USHORT_T  Et96MapV2UnpackUnstructuredSSRequestConf(MSG_T *ipcMessage_sp,
                                        ET96MAP_UNSTRUCTURED_SS_REQUEST_CNF_T 
                                        *mUnstructuredSSRequestCnf_sp); 


USHORT_T  ValEt96MapV2UnstructuredSSRequestConf(ET96MAP_LOCAL_SSN_T      localSsn,
                                             ET96MAP_DIALOGUE_ID_T dialogueId,
                                             ET96MAP_INVOKE_ID_T invokeId,
                                             ET96MAP_USSD_DATA_CODING_SCHEME_T* ussdDcs_p,
                                             ET96MAP_USSD_STRING_T* ussdString_sp,
                                             ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T *errorUnstructuredSSRequest_sp,
                                             ET96MAP_PROV_ERR_T *provErrCode_p);

USHORT_T ValMapAlertingPattern(ET96MAP_ALERTING_PATTERN_T* alertingPattern_p);

USHORT_T ET96MapUnpackUssdNotifyCnf(MSG_T *ipcMessage_sp,
                                        ET96MAP_USSD_NOTIFY_CNF_T *mUssdNotifyCnf_sp);


USHORT_T  ValEt96MapUnpackUssdNotifyCnf(ET96MAP_LOCAL_SSN_T localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_INVOKE_ID_T       invokeId,
                ET96MAP_ERROR_USSD_NOTIFY_T  *,
                ET96MAP_PROV_ERR_T  provErrCode);






#endif /* __ET96MAP_USSD_H__ */
