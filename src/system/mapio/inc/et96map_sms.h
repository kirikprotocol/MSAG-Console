/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_sms.h,v                                                   */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_sms.h                                                     */
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
/* 12/190 55-CAA 201 45 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.18 2000/04/07 09:05:20 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Lambkin                                                      */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Prototypes for the EMAP API Short Msg Services.                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 971105       Ulf Melin       Changed MAP_ERR_CODE_T to            */
/*                              MAP_ERROR_CODE_T                     */
/*                              Changed return code from             */
/*                              MAP_API_RESULT_T to USHORT_T         */
/* Revision record:                                                  */
/* 980406       Ulf Hellsten                                         */
/*                                                                   */
/*                                                                   */
/* Revision record:                                                  */
/* 010122       Magnus Malmgren Added func. prototypes               */
/*                               Et96MapV3SendRInfoForSmReq          */
/*                               Et96MapV3ForwardMOSmResp            */
/*                               Et96MapV3ReportSmDelStatReq         */
/*                               Et96MapV3ForwardMTSmReq             */
/*                                                                   */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/



#ifndef __ET96MAP_SMS_H__
#define __ET96MAP_SMS_H__








#define defineMAPV1FORWARDSMCONF  Et96MapV1ForwardSmMT_MOConf
#define defineMAPV2FORWARDSMCONF  Et96MapV2ForwardSmMTConf
#define defineMAPV1FORWARDSMIND   Et96MapV1ForwardSmMOInd
#define defineMAPV2FORWARDSMIND   Et96MapV2ForwardSmMOInd


#ifndef EINSS7_FUNC_POINTER





                
#ifdef VER_MAP_R9

USHORT_T Et96MapV3SubscriberLCSReportInd(
                              ET96MAP_LOCAL_SSN_T,
                              ET96MAP_DIALOGUE_ID_T,
                              ET96MAP_INVOKE_ID_T,
                              ET96MAP_ADDRESS_T *,                              
                              ET96MAP_LCS_EVENT_T,
                              ET96MAP_CLIENT_TYPE_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_LCS_CLIENT_INTERNAL_ID_T *,
                              ET96MAP_LCS_CLIENT_NAME_T *,           
                              ET96MAP_IMSI_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_IMEI_T *,
                              ET96MAP_GEOGRAPHICAL_INFO_T *,
                              ET96MAP_AGE_OF_LOCATION_INFO_T *,
                              ET96MAP_LMSI_T *);

#else

USHORT_T Et96MapV3SubscriberLCSReportInd(
                              ET96MAP_LOCAL_SSN_T,
                              ET96MAP_DIALOGUE_ID_T,
                              ET96MAP_INVOKE_ID_T,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_GPRS_NODE_IND_T,
                              ET96MAP_LCS_EVENT_T,
                              ET96MAP_CLIENT_TYPE_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_LCS_CLIENT_INTERNAL_ID_T *,
                              ET96MAP_LCS_CLIENT_NAME_T *,           
                              ET96MAP_LCS_APN_T *,
                              ET96MAP_IMSI_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_ADDRESS_T *,
                              ET96MAP_IMEI_T *,
                              ET96MAP_GEOGRAPHICAL_INFO_T *,
                              ET96MAP_AGE_OF_LOCATION_INFO_T *,
                              ET96MAP_LMSI_T *,
                              ET96MAP_ADDITIONAL_NUMBER_T*,
                              ET96MAP_ADD_GEOGRAPHICAL_INFO_T*,
                              ET96MAP_DEF_LOC_EVENT_TYPE_T*,
                              ET96MAP_TERM_CAUSE_T*,
                              ET96MAP_ADDRESS_T*,
                              ET96MAP_GPRS_NODE_IND_T,
                              ET96MAP_ADDITIONAL_NUMBER_T*,
                              ET96MAP_LMSI_T * );




#endif


#ifdef VER_MAP_R9

USHORT_T Et96MapV3ProvideSubscriberLCSConf( ET96MAP_LOCAL_SSN_T,
                                            ET96MAP_DIALOGUE_ID_T,
                                            ET96MAP_INVOKE_ID_T,
                                            ET96MAP_GEOGRAPHICAL_INFO_T *,
                                            ET96MAP_AGE_OF_LOCATION_INFO_T *,  
                                            ET96MAP_ERROR_PROVIDE_SUBSCRIBER_LCS_T *, 
                                            ET96MAP_PROV_ERR_T *);

#else


USHORT_T Et96MapV3ProvideSubscriberLCSConf( ET96MAP_LOCAL_SSN_T,
                                            ET96MAP_DIALOGUE_ID_T,
                                            ET96MAP_INVOKE_ID_T,
                                            ET96MAP_GEOGRAPHICAL_INFO_T *,
                                            ET96MAP_AGE_OF_LOCATION_INFO_T *,
                                            ET96MAP_ADD_GEOGRAPHICAL_INFO_T *, 
                                            BOOLEAN_T,
                                            ET96MAP_ERROR_PROVIDE_SUBSCRIBER_LCS_T *, 
                                            ET96MAP_PROV_ERR_T *);

#endif

#ifdef VER_MAP_R9

USHORT_T Et96MapV3SendRInfoForLCSConf(ET96MAP_LOCAL_SSN_T,
                                      ET96MAP_DIALOGUE_ID_T,
                                      ET96MAP_INVOKE_ID_T,
                                      ET96MAP_ADDRESS_T *,
                                      ET96MAP_IMSI_T *,                                       
                                      ET96MAP_LMSI_T *,
                                      ET96MAP_ADDRESS_T *,
                                      ET96MAP_ERROR_ROUTING_INFO_FOR_LCS_T *, 
                                      ET96MAP_PROV_ERR_T *);

#else
/* NEW RELEASE 10 */
USHORT_T Et96MapV3SendRInfoForLCSConf(ET96MAP_LOCAL_SSN_T,
                                      ET96MAP_DIALOGUE_ID_T,
                                      ET96MAP_INVOKE_ID_T,
                                      ET96MAP_ADDRESS_T *,
                                      ET96MAP_IMSI_T *,                                       
                                      ET96MAP_LMSI_T *,
                                      ET96MAP_ADDRESS_T *,
                                      ET96MAP_GPRS_NODE_IND_T ,
                                      ET96MAP_ADDITIONAL_NUMBER_T *,
                                      ET96MAP_ERROR_ROUTING_INFO_FOR_LCS_T *, 
                                      ET96MAP_PROV_ERR_T *);
#endif


USHORT_T Et96MapV3ForwardSmMOInd(ET96MAP_LOCAL_SSN_T, 
                                  ET96MAP_DIALOGUE_ID_T,
                                  ET96MAP_INVOKE_ID_T, 
                                  ET96MAP_SM_RP_DA_T*,
                                  ET96MAP_SM_RP_OA_T*,
                                  ET96MAP_SM_RP_UI_T*,
                                  ET96MAP_IMSI_T*);





USHORT_T Et96MapV3InformSCInd(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                 ET96MAP_ADDRESS_T*,
                                ET96MAP_MWD_STATUS_T*);


USHORT_T Et96MapV3ForwardSmMTConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_SM_RP_UI_T*,
                                ET96MAP_ERROR_FORW_SM_MT_T*, 
                                ET96MAP_PROV_ERR_T*);


USHORT_T Et96MapV3ReportSmDelStatConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ADDRESS_T* ,
                                ET96MAP_ERROR_REPORT_SM_DEL_STAT_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*); 


USHORT_T Et96MapV3SendRInfoForSmConf(ET96MAP_LOCAL_SSN_T   ,
                                     ET96MAP_DIALOGUE_ID_T ,
                                     ET96MAP_INVOKE_ID_T   ,
                                     ET96MAP_IMSI_T*,
                                     ET96MAP_ADDRESS_T*,
                                     ET96MAP_LMSI_T*,
#ifdef VER_MAP_R9
                                     ET96MAP_GPRS_NODE_IND_T* ,
#else
                                     ET96MAP_GPRS_NODE_IND_T ,
#endif
                                     ET96MAP_ADDITIONAL_NUMBER_T*,
                                     ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T*, /* anwa TR1351 */
                                     ET96MAP_PROV_ERR_T*);


USHORT_T defineMAPV1FORWARDSMCONF(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ERROR_FORW_SM_MT_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*);


USHORT_T defineMAPV1FORWARDSMIND(ET96MAP_LOCAL_SSN_T, 
                                 ET96MAP_DIALOGUE_ID_T,
                                 ET96MAP_INVOKE_ID_T, 
                                 ET96MAP_SM_RP_DA_T*,
                                 ET96MAP_SM_RP_OA_T*,
                                 ET96MAP_SM_RP_UI_T*);


USHORT_T defineMAPV2FORWARDSMIND( ET96MAP_LOCAL_SSN_T, 
                                  ET96MAP_DIALOGUE_ID_T,
                                  ET96MAP_INVOKE_ID_T, 
                                  ET96MAP_SM_RP_DA_T*,
                                  ET96MAP_SM_RP_OA_T*,
                                  ET96MAP_SM_RP_UI_T*);



 


USHORT_T Et96MapV1AlertSCInd(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ADDRESS_T*,
                                ET96MAP_ADDRESS_T*);

USHORT_T Et96MapV2AlertSCInd(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ADDRESS_T*,
                                ET96MAP_ADDRESS_T*);



                                
USHORT_T Et96MapV2InformSCInd(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                 ET96MAP_ADDRESS_T*,
                                ET96MAP_MWD_STATUS_T*);





USHORT_T defineMAPV2FORWARDSMCONF(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ERROR_FORW_SM_MT_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*);






USHORT_T Et96MapV2ForwardSmMOConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ERROR_FORW_SM_MO_T*, 
                                ET96MAP_PROV_ERR_T*);

                                








USHORT_T Et96MapV1SendRInfoForSmConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_IMSI_T*,
                                ET96MAP_LOCATION_INFO_T*,
                                ET96MAP_LMSI_T*,
                                ET96MAP_MWD_SET_T*,
                                ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*);

USHORT_T Et96MapV2SendRInfoForSmConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_IMSI_T*,
                                ET96MAP_ADDRESS_T*,
                                ET96MAP_LMSI_T*,
                                ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*);


USHORT_T Et96MapV1ReportSmDelStatConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ERROR_REPORT_SM_DEL_STAT_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*);

USHORT_T Et96MapV2ReportSmDelStatConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ADDRESS_T* ,
                                ET96MAP_ERROR_REPORT_SM_DEL_STAT_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T*);                       

#endif


USHORT_T  Et96MapV3SubscriberLCSReportResp(ET96MAP_LOCAL_SSN_T ,
                                    ET96MAP_DIALOGUE_ID_T ,
                                    ET96MAP_INVOKE_ID_T ,
                                    ET96MAP_ERROR_SUBSCRIBER_LCS_REPORT_T *);




USHORT_T  Et96MapV3SendRInfoForLCSReq(ET96MAP_LOCAL_SSN_T   localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T   invokeId,
                                           ET96MAP_ADDRESS_T     *mlcNumber_sp,
                                           ET96MAP_ADDRESS_T     *msisdn_sp,
                                           ET96MAP_IMSI_T        *imsi_sp);                          













USHORT_T  Et96MapV1SendRInfoForSmReq(ET96MAP_LOCAL_SSN_T,
                                           ET96MAP_DIALOGUE_ID_T,
                                           ET96MAP_INVOKE_ID_T,
                                           ET96MAP_ADDRESS_T*,
                                           ET96MAP_SM_RP_PRI_T,
                                           ET96MAP_ADDRESS_T*,
                                           ET96MAP_CUG_CODE_T*,
                                           ET96MAP_TELE_CODE_T*);

USHORT_T  Et96MapV2SendRInfoForSmReq(ET96MAP_LOCAL_SSN_T    localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T   invokeId,
                                           ET96MAP_ADDRESS_T      *smMsisdn_sp,
                                           ET96MAP_SM_RP_PRI_T   smRpPri,
                                           ET96MAP_ADDRESS_T     *smSca_sp);



USHORT_T  Et96MapV2ForwardSmMOReq(ET96MAP_LOCAL_SSN_T   localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T  invokeId,
                                      ET96MAP_SM_RP_DA_T    *smRpDa_sp,
                                      ET96MAP_SM_RP_OA_T    *smRpOa_sp,
                                      ET96MAP_SM_RP_UI_T          *smRpUi_sp);
                                      


                                     
USHORT_T  Et96MapV1ReportSmDelStatReq(ET96MAP_LOCAL_SSN_T       localSsn,
                                            ET96MAP_DIALOGUE_ID_T       dialogueId,
                                            ET96MAP_INVOKE_ID_T     invokeId,
                                            ET96MAP_ADDRESS_T       *msisdn_sp,
                                            ET96MAP_ADDRESS_T   *sCa_sp);
                                            
USHORT_T  Et96MapV2ReportSmDelStatReq(ET96MAP_LOCAL_SSN_T       localSsn,
                                            ET96MAP_DIALOGUE_ID_T       dialogueId,
                                            ET96MAP_INVOKE_ID_T     invokeId,
                                            ET96MAP_ADDRESS_T       *msisdn_sp,
                                            ET96MAP_ADDRESS_T   *sCa_sp,
                                            ET96MAP_DEL_OUTCOME_T smDeliveryOutcome);


USHORT_T  Et96MapReadyForSmReq(ET96MAP_LOCAL_SSN_T      localSsn,
                                       ET96MAP_DIALOGUE_ID_T dialogueId,
                                       ET96MAP_INVOKE_ID_T      invokeId,
                                       ET96MAP_IMSI_T           *smImsi_sp,
                                       ET96MAP_ALERT_REASON_T alertReason);


USHORT_T  Et96MapAlertSCReq(ET96MAP_LOCAL_SSN_T         localSsn,
                                    ET96MAP_DIALOGUE_ID_T       dialogueId,
                                    ET96MAP_INVOKE_ID_T         invokeId,
                                    ET96MAP_ADDRESS_T           *msisdn_sp,
                                    ET96MAP_ADDRESS_T           *sCA_sp);


USHORT_T  Et96MapInformSCReq(ET96MAP_LOCAL_SSN_T localSsn,
                                     ET96MAP_DIALOGUE_ID_T dialogueId,
                                     ET96MAP_INVOKE_ID_T invokeId,
                                     ET96MAP_ADDRESS_T *smMsisdn_sp,
                                     ET96MAP_MWD_STATUS_T *mwdStatus_sp);
#ifdef VER_MAP_R9

USHORT_T  Et96MapV3ProvideSubscriberLCSReq(
                     ET96MAP_LOCAL_SSN_T localSsn,
                     ET96MAP_DIALOGUE_ID_T dialogueId,
                     ET96MAP_INVOKE_ID_T   invokeId,
                     ET96MAP_LOCATION_TYPE_T locationType,                       
                     ET96MAP_ADDRESS_T     *mlcNum_sp,                     
                     ET96MAP_CLIENT_TYPE_T *lcsClientType_p,
                     ET96MAP_ADDRESS_T     *lcsClientExternalId_sp,
                     ET96MAP_ADDRESS_T     *lcsClientDialedByMS_sp,
                     ET96MAP_LCS_CLIENT_INTERNAL_ID_T *lcsClientInternalId_p,
                     ET96MAP_LCS_CLIENT_NAME_T *lcsClientName_sp,
                     ET96MAP_PRIVACY_OVERRIDE_T  privacyOverride,
                     ET96MAP_ADDRESS_T     *msisdn_sp,
                     ET96MAP_IMSI_T        *imsi_sp,
                     ET96MAP_LMSI_T        *lmsi_sp,
                     ET96MAP_IMEI_T        *imei_sp,
                     ET96MAP_LCS_PRIORITY_T *lcsPriority_p,
                     ET96MAP_LCS_QOS_T     *lcsQualityOfService_sp);
#else


USHORT_T  Et96MapV3ProvideSubscriberLCSReq(
                     ET96MAP_LOCAL_SSN_T localSsn,
                     ET96MAP_DIALOGUE_ID_T dialogueId,
                     ET96MAP_INVOKE_ID_T   invokeId,
                     ET96MAP_LOCATION_TYPE_T locationEstimateType,                                           
                     ET96MAP_ADDRESS_T     *mlcNum_sp, 
                     ET96MAP_DEFERRED_LOC_EVENT_TYPE_T *deferredLocationEventType_p,                     
                     ET96MAP_CLIENT_TYPE_T *lcsClientType_p,
                     ET96MAP_ADDRESS_T     *lcsClientExternalId_sp,
                     ET96MAP_ADDRESS_T     *lcsClientDialedByMS_sp,
                     ET96MAP_LCS_CLIENT_INTERNAL_ID_T *lcsClientInternalId_p,
                     ET96MAP_LCS_CLIENT_NAME_T *lcsClientName_sp,
                     ET96MAP_LCS_APN_T      *lcsApn_sp,
                     ET96MAP_PRIVACY_OVERRIDE_T  privacyOverride,
                     ET96MAP_ADDRESS_T     *msisdn_sp,
                     ET96MAP_IMSI_T        *imsi_sp,
                     ET96MAP_LMSI_T        *lmsi_sp,
                     ET96MAP_IMEI_T        *imei_sp,
                     ET96MAP_LCS_PRIORITY_T *lcsPriority_p,
                     ET96MAP_LCS_QOS_T     *lcsQualityOfService_sp,
                     ET96MAP_SUPPORTED_GAD_SHAPES_T *supportedGadShapes_p);
#endif


USHORT_T   ValEt96MapV3SubscriberLCSReportInd( 
                           ET96MAP_LOCAL_SSN_T    localSsn,
                           ET96MAP_DIALOGUE_ID_T  dialogueId,
                           ET96MAP_INVOKE_ID_T    invokeId,   
                           ET96MAP_LCS_EVENT_T   lcsEvent,
                           ET96MAP_CLIENT_TYPE_T *lcsClientType_p);

USHORT_T  ValEt96MapV3SubscriberLCSReportResp(ET96MAP_LOCAL_SSN_T localSsn,
                                       ET96MAP_DIALOGUE_ID_T dialogueId,
                                       ET96MAP_INVOKE_ID_T invokeId,
                                       ET96MAP_ERROR_SUBSCRIBER_LCS_REPORT_T *);


BOOLEAN_T ValEt96MapV3SubscriberLCSReporErrorCode(UCHAR_T errorCode);




USHORT_T  ValEt96MapV3ProvideSubscriberLCSReq(
                     ET96MAP_LOCAL_SSN_T localSsn,
                     ET96MAP_DIALOGUE_ID_T dialogueId,
                     ET96MAP_INVOKE_ID_T   invokeId,
                     ET96MAP_LOCATION_TYPE_T locationType,
                     ET96MAP_CLIENT_TYPE_T *lcsClientType_p,
                     ET96MAP_ADDRESS_T     *mlcNum_sp,                     
                     ET96MAP_ADDRESS_T     *lcsClientExternalId_sp,
                     ET96MAP_ADDRESS_T     *lcsClientDialedByMS_sp,
                     ET96MAP_LCS_CLIENT_INTERNAL_ID_T *lcsClientInternalId_p,
                     ET96MAP_LCS_CLIENT_NAME_T *lcsClientName_sp,                     
                     ET96MAP_ADDRESS_T     *msisdn_sp,
                     ET96MAP_IMSI_T        *imsi_sp,
                     ET96MAP_LMSI_T        *lmsi_sp,
                     ET96MAP_IMEI_T        *imei_sp,
                     ET96MAP_LCS_PRIORITY_T *lcsPriority_p,
                     ET96MAP_DEFERRED_LOC_EVENT_TYPE_T *deferredLocationEventType_p,
                     ET96MAP_SUPPORTED_GAD_SHAPES_T *supportedGadShapes_p,
                     ET96MAP_LCS_APN_T      *lcsApn_sp,
                     ET96MAP_LCS_QOS_T     *lcsQualityOfService_sp);






USHORT_T  ValEt96MapV3SendRInfoForLCSReq(ET96MAP_LOCAL_SSN_T ,
                                              ET96MAP_DIALOGUE_ID_T ,
                                              ET96MAP_INVOKE_ID_T ,
                                              ET96MAP_ADDRESS_T     *,
                                              ET96MAP_ADDRESS_T     *,                                     
                                              ET96MAP_IMSI_T        *);









USHORT_T  ValEt96MapSendRInfoForSmReq(ET96MAP_LOCAL_SSN_T    localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T   invokeId,
                                      ET96MAP_ADDRESS_T     *smMsisdn_sp,
                                      ET96MAP_SM_RP_PRI_T   smRpPri,
                                      ET96MAP_ADDRESS_T     *smSca_sp); /* TR2545 HeUl Remove parameters*/

                                         


USHORT_T  ValEt96MapForwardSmReq(ET96MAP_LOCAL_SSN_T   localSsn,
                                         ET96MAP_DIALOGUE_ID_T dialogueId,
                                         ET96MAP_INVOKE_ID_T  invokeId,
                                         ET96MAP_SM_RP_DA_T    *smRpDa_sp,
                                         ET96MAP_SM_RP_OA_T    *smRpOa_sp,
                                         ET96MAP_SM_RP_UI_T       *smRpUi_sp);


USHORT_T  ValEt96MapRepSmDelStatReq(ET96MAP_LOCAL_SSN_T         localSsn,
                                            ET96MAP_DIALOGUE_ID_T       dialogueId,
                                            ET96MAP_INVOKE_ID_T     invokeId,
                                            ET96MAP_ADDRESS_T       *msisdn_sp,
                                            ET96MAP_ADDRESS_T   *sCa_sp);
                                            



USHORT_T  ValEt96MapReadyForSmReq(ET96MAP_LOCAL_SSN_T      localSsn,
                                          ET96MAP_DIALOGUE_ID_T dialogueId,
                                          ET96MAP_INVOKE_ID_T      invokeId,
                                          ET96MAP_IMSI_T           *smImsi_sp,
                                         ET96MAP_ALERT_REASON_T alertReason);


USHORT_T  ValEt96MapAlertSCReq(ET96MAP_LOCAL_SSN_T        localSsn,
                                       ET96MAP_DIALOGUE_ID_T dialogueId,
                                       ET96MAP_INVOKE_ID_T        invokeId,
                                       ET96MAP_ADDRESS_T          *msisdn_sp,
                                       ET96MAP_ADDRESS_T     *sCA_sp);


USHORT_T  ValEt96MapInformSCReq(ET96MAP_LOCAL_SSN_T localSsn,
                                        ET96MAP_DIALOGUE_ID_T dialogueId,
                                        ET96MAP_INVOKE_ID_T invokeId,
                                        ET96MAP_ADDRESS_T *smMsisdn_sp,
                                        ET96MAP_MWD_STATUS_T *mwdStatus_sp);


USHORT_T  Et96MapSendRInfoForSmResp(ET96MAP_LOCAL_SSN_T    localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T   invokeId,
                                           ET96MAP_IMSI_T        *smImsi_sp,
                                           ET96MAP_ADDRESS_T      *smMsisdn_sp,
                                           ET96MAP_LMSI_T        *smLmsi_sp,
                                           ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T  *errorSendRoutingInfoForSm_sp); /* xsro TR1351 */





USHORT_T  Et96MapV1ForwardSmMT_MOReq(ET96MAP_LOCAL_SSN_T   localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T  invokeId,
                                      ET96MAP_SM_RP_DA_T    *smRpDa_sp,
                                      ET96MAP_SM_RP_OA_T    *smRpOa_sp,
                                      ET96MAP_SM_RP_UI_T          *smRpUi_sp);



USHORT_T  Et96MapV1ForwardSmMOResp(ET96MAP_LOCAL_SSN_T   localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T  invokeId,
                               ET96MAP_ERROR_FORW_SM_MO_T  *errorForwardSMmo_sp);
                               




USHORT_T  Et96MapV2ForwardSmMOResp(ET96MAP_LOCAL_SSN_T   localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T  invokeId,
                               ET96MAP_ERROR_FORW_SM_MO_T  *errorForwardSMmo_sp);




USHORT_T  Et96MapV2ForwardSmMTReq(ET96MAP_LOCAL_SSN_T   localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T  invokeId,
                                      ET96MAP_SM_RP_DA_T    *smRpDa_sp,
                                      ET96MAP_SM_RP_OA_T    *smRpOa_sp,
                                      ET96MAP_SM_RP_UI_T          *smRpUi_sp,
                                      ET96MAP_MMS_T  moreMessagesToSend);







USHORT_T  Et96MapReportSmDelStatResp(ET96MAP_LOCAL_SSN_T        localSsn,
                                            ET96MAP_DIALOGUE_ID_T       dialogueId,
                                            ET96MAP_INVOKE_ID_T     invokeId,
                                            ET96MAP_ADDRESS_T       *msisdn_sp,
                                            ET96MAP_ERROR_REPORT_SM_DEL_STAT_T   *errorReportSmDelStat_sp); /* xsro TR1351 */


USHORT_T  Et96MapReadyForSmResp(ET96MAP_LOCAL_SSN_T      localSsn,
                                       ET96MAP_DIALOGUE_ID_T dialogueId,
                                       ET96MAP_INVOKE_ID_T      invokeId,
                                       ET96MAP_ERROR_READY_FOR_SM_T *errorReadyForSm_sp); /* anwa TR1351 */


USHORT_T  Et96MapV2AlertSCResp(ET96MAP_LOCAL_SSN_T              localSsn,
                                    ET96MAP_DIALOGUE_ID_T       dialogueId,
                                    ET96MAP_INVOKE_ID_T         invokeId,
                                    ET96MAP_ERROR_ALERT_SERVICE_T *errorAlertService_sp);

USHORT_T  ValEt96MapSendRInfoForSmRsp(ET96MAP_LOCAL_SSN_T    localSsn,
                                              ET96MAP_DIALOGUE_ID_T dialogueId,
                                              ET96MAP_INVOKE_ID_T   invokeId,
                                              ET96MAP_IMSI_T        *smImsi_sp,
                                              ET96MAP_ADDRESS_T     *smMsisdn_sp,
                                              ET96MAP_LMSI_T        *smLmsi_sp,
                                              ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T  *errorSendRoutingInfoForSm_sp); /* xsro TR1351 */


USHORT_T  ValEt96MapForwardSmRsp(ET96MAP_LOCAL_SSN_T   localSsn,
                                 ET96MAP_DIALOGUE_ID_T dialogueId,
                                 ET96MAP_INVOKE_ID_T  invokeId,
                                 ET96MAP_SM_RP_UI_T *smRpUi_sp,
                                 ET96MAP_ERROR_FORW_SM_MO_T  *errorForwardSMmo_sp,
                                 UCHAR_T version);

USHORT_T  ValEt96MapRepSmDelStatRsp(ET96MAP_LOCAL_SSN_T         localSsn,
                                    ET96MAP_DIALOGUE_ID_T       dialogueId,
                                    ET96MAP_INVOKE_ID_T     invokeId,
                                    ET96MAP_ADDRESS_T       *msisdn_sp,
                                    ET96MAP_ERROR_REPORT_SM_DEL_STAT_T  *errorReportSmDelStat_sp); /* xsro TR1351*/


USHORT_T  ValEt96MapReadyForSmRsp(ET96MAP_LOCAL_SSN_T      localSsn,
                                  ET96MAP_DIALOGUE_ID_T dialogueId,
                                  ET96MAP_INVOKE_ID_T      invokeId,
                                  ET96MAP_ERROR_READY_FOR_SM_T *errorReadyForSm_sp); /* anwa TR1351 */


USHORT_T  ValEt96MapAlertSCRsp(ET96MAP_LOCAL_SSN_T              localSsn,
                               ET96MAP_DIALOGUE_ID_T    dialogueId,
                               ET96MAP_INVOKE_ID_T              invokeId,
                               ET96MAP_ERROR_ALERT_SERVICE_T *errorAlertService_sp);

USHORT_T  Et96MapV3SendRInfoForSmReq(ET96MAP_LOCAL_SSN_T   ,
                                     ET96MAP_DIALOGUE_ID_T ,
                                     ET96MAP_INVOKE_ID_T   ,
                                     ET96MAP_ADDRESS_T*     ,
                                     ET96MAP_SM_RP_PRI_T   ,
                                     ET96MAP_ADDRESS_T*     ,
                                     ET96MAP_SM_RP_MTI_T*   , /* Cond */
                                     ET96MAP_SM_RP_SMEA_T*  ,/* Cond */
#ifdef VER_MAP_R9
                                     ET96MAP_GPRS_SUPP_IND_T   gprsSuppInd  /* Cond */
#else
                                     ET96MAP_GPRS_SUPP_IND_T   *gprsSuppInd_p
#endif
                                                                             );




 


USHORT_T  Et96MapV3ReportSmDelStatReq(ET96MAP_LOCAL_SSN_T ,
                                      ET96MAP_DIALOGUE_ID_T ,
                                      ET96MAP_INVOKE_ID_T ,
                                      ET96MAP_ADDRESS_T*,
                                      ET96MAP_ADDRESS_T*,
                                      ET96MAP_DEL_OUTCOME_T ,
                                      ET96MAP_ABSENT_DIAG_T*,                       
                                      ET96MAP_GPRS_SUPP_IND_T* ,                                 
                                      ET96MAP_DEL_OUTCOME_IND_T*,                
                                      ET96MAP_DEL_OUTCOME_T* ,                     
                                      ET96MAP_ADD_DIAG_T* );
    


USHORT_T  Et96MapV3ForwardSmMTReq(ET96MAP_LOCAL_SSN_T ,
                                  ET96MAP_DIALOGUE_ID_T ,
                                  ET96MAP_INVOKE_ID_T ,
                                  ET96MAP_SM_RP_DA_T* ,
                                  ET96MAP_SM_RP_OA_T* ,
                                  ET96MAP_SM_RP_UI_T* ,
                                  ET96MAP_MMS_T*  );
    
USHORT_T Et96MapV3UnpackSubscriberLCSReportInd(MSG_T *ipcMessage_sp,
                     ET96MAP_V3_SUBSCRIBER_REPORT_LCS_IND_T *mSubscriberLCSReportInd_sp);



USHORT_T  Et96MapUnpackProvideSubscriberLCSCnf(MSG_T *ipcMessage_sp,
                           ET96MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_CONF_T  *mProvideSubscriberLCSConf_sp);

USHORT_T  Et96MapUnpackSendRoutingInfoForLCSCnf(MSG_T *ipcMessage_sp,
                           ET96MAP_V3_SND_RINFO_LCS_CONF_T *mSndRInfoLCSCnf_sp);



USHORT_T Et96MapUnpackForwardSmInd(MSG_T *ipcMessage_sp,
                                     ET96MAP_FWD_SM_REQ_IND_T *mFwdSmInd_sp);


USHORT_T Et96MapUnpackRepSmDelStatInd(MSG_T *ipcMessage_sp,
                                        ET96MAP_RPT_SM_DEL_REQ_IND_T *mRptSmDelInd_sp);




USHORT_T Et96MapUnpackAlertSCInd(MSG_T *ipcMessage_sp,
                                   ET96MAP_ALERT_SC_REQ_IND_T *mAlertSCInd_sp);


USHORT_T Et96MapUnpackInformSCInd(MSG_T *ipcMessage_sp,
                                     ET96MAP_INFORM_SC_REQ_IND_T *mInformSCInd_sp);

USHORT_T  Et96MapUnpackSendRoutingInfoForSmCnf(MSG_T *ipcMessage_sp,
                                                  ET96MAP_SND_RINFO_SM_CONF_RSP_T *mSndRInfoCnf_sp);



USHORT_T  Et96MapUnpackForwardSmCnf(MSG_T *ipcMessage_sp,
                                       ET96MAP_FWD_SM_CONF_RSP_T *mFwdSmCnf_sp );

USHORT_T Et96MapUnpackForwardSmMOV2Cnf(MSG_T *ipcMessage_sp,ET96MAP_FWD_SM_MO_V2_CONF *mFwdSmMoV2Conf_sp);


USHORT_T  Et96MapUnpackRepSmDelStatCnf(MSG_T *ipcMessage_sp,
                                       ET96MAP_RPT_SM_DEL_CONF_RSP_T *mRptSm_sp);

USHORT_T  Et96MapV3ForwardSmMOResp(ET96MAP_LOCAL_SSN_T   ,
                                   ET96MAP_DIALOGUE_ID_T ,
                                   ET96MAP_INVOKE_ID_T  ,
                                   ET96MAP_SM_RP_UI_T* ,
                                   ET96MAP_ERROR_FORW_SM_MO_T*  );

    
USHORT_T ValEt96MapSendRInfoForSmInd(ET96MAP_LOCAL_SSN_T    localSsn,
                                     ET96MAP_DIALOGUE_ID_T  dialogueId,
                                     ET96MAP_INVOKE_ID_T    invokeId,
                                     ET96MAP_ADDRESS_T      *smMsisdn_sp,
                                     ET96MAP_SM_RP_PRI_T    smRpPri,
                                     ET96MAP_ADDRESS_T      *smSCA_sp );


USHORT_T ValEt96MapForwardSmInd(ET96MAP_LOCAL_SSN_T   localSsn,
                                ET96MAP_DIALOGUE_ID_T  dialogueId,
                                ET96MAP_INVOKE_ID_T  invokeId,
                                ET96MAP_SM_RP_DA_T  *smRpDa_sp,
                                ET96MAP_SM_RP_OA_T  *smRpOa_sp,
                                ET96MAP_SM_RP_UI_T  *smRpUi_sp);


USHORT_T ValEt96MapAlertSCInd(ET96MAP_LOCAL_SSN_T  localSsn,
                                      ET96MAP_DIALOGUE_ID_T dialogueId,
                                      ET96MAP_INVOKE_ID_T   invokeId,
                                      ET96MAP_ADDRESS_T     *msisdn_sp,
                                      ET96MAP_ADDRESS_T     *sCA_sp);

USHORT_T  ValEt96MapSendRInfoForSmCnf( ET96MAP_LOCAL_SSN_T localSsn,
                                                ET96MAP_DIALOGUE_ID_T dialogueId,
                                                ET96MAP_INVOKE_ID_T invokeId,
                                                ET96MAP_IMSI_T   *smImsi_sp,
                                                ET96MAP_ADDRESS_T  *smMscNumber_sp,
                                                ET96MAP_LMSI_T    *smLmsi_sp,
                                       ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp, /* anwa TR1351 */
                                                ET96MAP_PROV_ERR_T  provErrCode,
                                                UCHAR_T version );

USHORT_T  ValEt96MapForwardSmCnf(ET96MAP_LOCAL_SSN_T localSsn,
                                         ET96MAP_DIALOGUE_ID_T dialogueId,
                                         ET96MAP_INVOKE_ID_T invokeId,
                                         ET96MAP_PROV_ERR_T  provErrCode);


USHORT_T  ValEt96MapRepSmDelStatCnf( ET96MAP_LOCAL_SSN_T localSsn,
                                             ET96MAP_DIALOGUE_ID_T dialogueId,
                                             ET96MAP_INVOKE_ID_T invokeId,
                                             ET96MAP_ADDRESS_T    *msisdn_sp,
                                     ET96MAP_ERROR_REPORT_SM_DEL_STAT_T *errorReportSmDelStat_sp, /* anwa TR1351 */
                                             ET96MAP_PROV_ERR_T  provErrCode,
                                             UCHAR_T );






USHORT_T  ValEt96MapTeleServiceCode(ET96MAP_TELE_CODE_T*);

/* These underlying functions are not used in a GWMSC */
#ifdef USE_THIS_PART
USHORT_T Et96MapReportSmDelStatInd(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T, 
                              ET96MAP_ADDRESS_T*,
                              ET96MAP_ADDRESS_T*,
                             ET96MAP_DEL_OUTCOME_T);
USHORT_T ValEt96MapRepSmDelStatInd(ET96MAP_LOCAL_SSN_T  localSsn,
                                           ET96MAP_DIALOGUE_ID_T  dialogueId,
                                           ET96MAP_INVOKE_ID_T    invokeId,
                                           ET96MAP_ADDRESS_T      *msisdn_sp,
                                           ET96MAP_ADDRESS_T      *sCA_sp);
                              

USHORT_T Et96MapUnpackRepSmDelStatInd(MSG_T *ipcMessage_sp,
                                        ET96MAP_RPT_SM_DEL_REQ_IND_T *mRptSmDelInd_sp);

USHORT_T Et96MapUnpackReadyForSmInd(MSG_T *ipcMessage_sp,
                                      ET96MAP_RDY_SM_REQ_IND_T *mRdySmInd_sp);

USHORT_T Et96MapReadyForSmInd(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T, 
                            ET96MAP_IMSI_T*, 
                            ET96MAP_ALERT_REASON_T);
                            
USHORT_T ValEt96MapReadyForSmInd(ET96MAP_LOCAL_SSN_T  localSsn,
                                         ET96MAP_DIALOGUE_ID_T  dialogueId,
                                         ET96MAP_INVOKE_ID_T  invokeId,
                                         ET96MAP_IMSI_T   *smImsi_sp,
                                         ET96MAP_ALERT_REASON_T alertReason);                       

USHORT_T Et96MapReadyForSmConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ERROR_READY_FOR_SM_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T);
USHORT_T  ValEt96MapReadyForSmCnf( ET96MAP_LOCAL_SSN_T localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T invokeId,
                                   ET96MAP_ERROR_READY_FOR_SM_T *errorReadyForSm_sp, /* anwa TR1351 */
                                           ET96MAP_PROV_ERR_T  provErrCode );

USHORT_T  Et96MapUnpackReadyForSmCnf(MSG_T *ipcMessage_sp,
                                        ET96MAP_RDY_SM_CONF_RSP_T *mRdySm_sp);

USHORT_T  ValEt96MapAlertSCCnf( ET96MAP_LOCAL_SSN_T localSsn,
                                        ET96MAP_DIALOGUE_ID_T dialogueId,
                                        ET96MAP_INVOKE_ID_T invokeId,
                                ET96MAP_ERROR_ALERT_SERVICE_T *, /* anwa TR1351 */
                                        ET96MAP_PROV_ERR_T  provErrCode );


USHORT_T Et96MapAlertSCConf(ET96MAP_LOCAL_SSN_T, 
                                ET96MAP_DIALOGUE_ID_T,
                                ET96MAP_INVOKE_ID_T,
                                ET96MAP_ERROR_ALERT_SERVICE_T*, /* anwa TR1351 */
                                ET96MAP_PROV_ERR_T);

USHORT_T  Et96MapUnpackAlertSCCnf(MSG_T *ipcMessage_sp,
                                    ET96MAP_ALERT_SC_CONF_RSP_T *mAlertSC_sp);
                                    
USHORT_T Et96MapUnpackSendRInfoForSmInd(MSG_T *ipcMessage_sp,
                                          ET96MAP_SND_RINFO_SM_REQ_IND_T *mSndRInfoInd_sp);
                                    
#endif

#endif /* __ET96MAP_SM<S_H__ */









