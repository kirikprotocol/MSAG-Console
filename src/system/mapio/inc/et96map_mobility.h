
/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_mobility.h,v                                              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_mobility.h                                                */
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
/* @EINVER: $RCfile$ 1.17 1999/04/26 14:33:59 Exp                    */
/*                                                                   */
/* Programmer:                                                       */
/* Damian ONeill                                                     */
/* AePONA Ltd.                                                       */
/*                                                                   */
/* Purpose:                                                          */
/* Prototypes for the ETSIMAP Mobility Services.                     */
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-2001      Damian ONeill                                        */
/*              Initial version.                                     */
/*                                                                   */
/* 01-20010606  Patrik Verme (einpawe)                               */
/*              Changed the argumentlist in all user interface       */
/*              prototypes.                                          */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#ifndef __ET96MAP_MOBILITY_H__
#define __ET96MAP_MOBILITY_H__

USHORT_T Et96MapV3ProvideSubscriberInfoReq (ET96MAP_LOCAL_SSN_T localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T invokeId,
                                           ET96MAP_IMSI_T *imsi_sp,
                                           ET96MAP_LMSI_T *lmsi_sp,
                                           BOOLEAN_T locationInfo,
                                           BOOLEAN_T subscriberState,
                                           BOOLEAN_T currentLocation);

/* Added by DON */

USHORT_T Et96MapV3AnyTimeInterrogationReq (ET96MAP_LOCAL_SSN_T localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T invokeId,
                                           ET96MAP_ADDRESS_T * gsmSCF_sp,
                                           ET96MAP_IMSI_T *imsi_sp,
                                           ET96MAP_ADDRESS_T *msisdn_sp,
                                           BOOLEAN_T locationInfo,
                                           BOOLEAN_T subscriberState,
                                           BOOLEAN_T currentLocation);

USHORT_T  Et96MapV3AnyTimeInterrogationResp(ET96MAP_LOCAL_SSN_T localSsn,
                                            ET96MAP_DIALOGUE_ID_T dialogueId,
                                            ET96MAP_INVOKE_ID_T invokeId,
                                            USHORT_T * ageOfLocationInformation_p,
                                            ET96MAP_ATI_GEOGRAPHICAL_INFO_T * geographicalInformation_sp,
                                            ET96MAP_ADDRESS_T * vlrNumber_sp,
                                            ET96MAP_LOCATIONNUMBER_T * locationNumber_sp,
                                            ET96MAP_CELLID_OR_SERVICE_OR_LAI_T * cellIdOrServiceOrLai_sp,
                                            SAIPRESENT
                                            ET96MAP_SELECTED_LSA_ID_T * selectedLSAId_sp,
                                            ET96MAP_ADDRESS_T * mscNumber_sp,
                                            ET96MAP_GEODETICINFO_T * geodeticInfo_sp,
                                            BOOLEAN_T currentLocationRetrived,
                                            ET96MAP_ATI_SUBSCRIBER_STATE_T * subscriberState_sp,
                                            ET96MAP_ERROR_ANY_TIME_INTERROGATION_T * errorAnyTimeInterrogation_sp);

USHORT_T ValEt96MapV3ProvideSubscriberInfoReq (ET96MAP_LOCAL_SSN_T localSsn,
                                              ET96MAP_DIALOGUE_ID_T dialogueId,
                                              ET96MAP_INVOKE_ID_T invokeId,
                                              ET96MAP_IMSI_T * imsi_sp,
                                              ET96MAP_LMSI_T *lmsi_sp,
                                              BOOLEAN_T locationInfo,
                                              BOOLEAN_T subscriberState,
                                              BOOLEAN_T currentLocation);

USHORT_T ValEt96MapV3AnyTimeInterrogationReq (ET96MAP_LOCAL_SSN_T localSsn,
                                              ET96MAP_DIALOGUE_ID_T dialogueId,
                                              ET96MAP_INVOKE_ID_T invokeId,
                                              ET96MAP_IMSI_T * imsi_sp,
                                              ET96MAP_ADDRESS_T * msisdn_sp,
                                              BOOLEAN_T locationInfo,
                                              BOOLEAN_T subscriberState,
                                              BOOLEAN_T currentLocation,
                                              ET96MAP_ADDRESS_T * gsmSCF_sp);

USHORT_T  ValEt96MapV3AnyTimeInterrogationRsp(ET96MAP_LOCAL_SSN_T   localSsn,
                                              ET96MAP_DIALOGUE_ID_T dialogueId,
                                              ET96MAP_INVOKE_ID_T invokeId,
                                              USHORT_T * ageOfLocationInformation_p,
                                              ET96MAP_ATI_GEOGRAPHICAL_INFO_T * geographicalInformation_sp,
                                              ET96MAP_ADDRESS_T * vlrNumber_sp,
                                              ET96MAP_LOCATIONNUMBER_T * locationNumber_sp,
                                              ET96MAP_CELLID_OR_SERVICE_OR_LAI_T * cellIdOrServiceOrLai_sp,
                                              ET96MAP_SELECTED_LSA_ID_T * selectedLSAId_sp,
                                              ET96MAP_ADDRESS_T * mscNumber_sp,
                                              ET96MAP_GEODETICINFO_T * geodeticInfo_sp,
                                              BOOLEAN_T currentLocationRetrived,
                                              ET96MAP_ATI_SUBSCRIBER_STATE_T * subscriberState_sp,
                                              ET96MAP_ERROR_ANY_TIME_INTERROGATION_T *errorAnyTimeInterrogation_sp);

USHORT_T Et96MapV3UnpackAnyTimeInterrogationInd(MSG_T *ipcMessage_sp,
                                                ET96MAP_ANY_TIME_INTERROGATION_REQ_IND_T *mAtiReqInd_sp);

USHORT_T  Et96MapV3UnpackAnyTimeInterrogationCnf(MSG_T *ipcMessage_sp,
                                                 ET96MAP_ANY_TIME_INTERROGATION_CONF_RSP_T *mAtiCnfRsp_sp);

USHORT_T  ValEt96MapV3AnyTimeInterrogationCnf(ET96MAP_LOCAL_SSN_T localSsn,
                                              ET96MAP_DIALOGUE_ID_T dialogueId,
                                              ET96MAP_INVOKE_ID_T invokeId,
                                              ET96MAP_ATI_LOCATION_INFO_T *atiLocationInfo_sp,
                                              ET96MAP_ATI_SUBSCRIBER_STATE_T *subscriberState_sp,
                                              ET96MAP_ERROR_ANY_TIME_INTERROGATION_T *errorAnyTimeInterrogation_sp,
                                              ET96MAP_PROV_ERR_T  provErrCode_p); /* einpawe 20010606 */

USHORT_T ValEt96MapV3AnyTimeInterrogationInd(ET96MAP_LOCAL_SSN_T   localSsn,
                                             ET96MAP_DIALOGUE_ID_T  dialogueId,
                                             ET96MAP_INVOKE_ID_T  invokeId,
                                             ET96MAP_IMSI_T * imsi_sp,
                                             ET96MAP_ADDRESS_T * msisdn_sp,
                                             BOOLEAN_T locationInfo,
                                             BOOLEAN_T subscriberState,
                                             BOOLEAN_T currentLocation,
                                             ET96MAP_ADDRESS_T  *gsmSCF_sp);

USHORT_T ValEt96MapV3ProvideSubscriberInfoCnf (ET96MAP_LOCAL_SSN_T localSsn,
                                              ET96MAP_DIALOGUE_ID_T dialogueId,
                                              ET96MAP_INVOKE_ID_T invokeId,
                                              ET96MAP_PROVIDE_LOCATION_INFO_T *provideLocationInfo_sp,
                                              ET96MAP_ATI_SUBSCRIBER_STATE_T *subscriberState_sp,
                                              ET96MAP_ERROR_PROVIDE_SUBSCRIBER_INFO_T errorProvideSubscriberInfo,
                                              ET96MAP_PROV_ERR_T provErrCode);

#ifndef EINSS7_FUNC_POINTER

USHORT_T Et96MapV3AnyTimeInterrogationConf(ET96MAP_LOCAL_SSN_T localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T invokeId,
                                           ET96MAP_AGE_OF_LOCATION_INFO_T *ageOfLocationInfo_p,
                                           ET96MAP_ATI_GEOGRAPHICAL_INFO_T *geographicalInfo_sp,
                                           ET96MAP_ADDRESS_T *vlrNumber_sp,
                                           ET96MAP_LOCATIONNUMBER_T * locationNumber_sp,
                                           ET96MAP_CELLID_OR_SERVICE_OR_LAI_T * cellIdOrServiceOrLai_sp,
#ifndef VER_MAP_R9
                                           BOOLEAN_T saiPresent,
#endif
                                           ET96MAP_SELECTED_LSA_ID_T * selectedLSAId_sp,
                                           ET96MAP_ADDRESS_T *mscNumber_sp,
                                           ET96MAP_GEODETICINFO_T *geodeticInfo_sp,
                                           BOOLEAN_T currentLocationRetrived,
                                           ET96MAP_ATI_SUBSCRIBER_STATE_T *subscriberState_sp,
                                           ET96MAP_ERROR_ANY_TIME_INTERROGATION_T * errorAnyTimeInterrogation_sp,
                                           ET96MAP_PROV_ERR_T *provError_p);

#ifndef VER_MAP_R9

USHORT_T Et96MapV3ProvideSubscriberInfoConf(ET96MAP_LOCAL_SSN_T localSsn,
                                           ET96MAP_DIALOGUE_ID_T dialogueId,
                                           ET96MAP_INVOKE_ID_T invokeId,
                                           ET96MAP_AGE_OF_LOCATION_INFO_T *ageOfLocationInfo_p,
                                           ET96MAP_PROVIDE_GEOGRAPHICAL_INFO_T *geographicalInfo_sp,
                                           ET96MAP_ADDRESS_T *vlrNumber_sp,
                                           ET96MAP_LOCATIONNUMBER_T * locationNumber_sp,
                                           ET96MAP_CELLID_OR_SERVICE_OR_LAI_T * cellIdOrServiceOrLai_sp,
                                           BOOLEAN_T saiPresent,
                                           ET96MAP_SELECTED_LSA_ID_T * selectedLSAId_sp,
                                           ET96MAP_ADDRESS_T *mscNumber_sp,
                                           ET96MAP_GEODETICINFO_T *geodeticInfo_sp,
                                           BOOLEAN_T currentLocationRetrived,
                                           ET96MAP_ATI_SUBSCRIBER_STATE_T *subscriberState_sp,
                                           ET96MAP_ERROR_PROVIDE_SUBSCRIBER_INFO_T *errorProvideSubscriberInfo_p,
                                           ET96MAP_PROV_ERR_T *provError_p);

#endif

USHORT_T Et96MapV3AnyTimeInterrogationInd(ET96MAP_LOCAL_SSN_T localSsn,
                                          ET96MAP_DIALOGUE_ID_T dialogueId,
                                          ET96MAP_INVOKE_ID_T invokeId,
                                          ET96MAP_ADDRESS_T *gsmSCF_sp,
                                          ET96MAP_IMSI_T * imsi_sp,
                                          ET96MAP_ADDRESS_T * msisdn_sp,
                                          BOOLEAN_T locationInfo,
                                          BOOLEAN_T subscriberState,
                                          BOOLEAN_T currentLocation);

#endif

#endif /* __ET96MAP_MOBILITY_H__ */

