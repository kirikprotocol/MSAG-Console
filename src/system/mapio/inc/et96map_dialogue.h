/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_dialogue.h,v                                              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_dialogue.h                                                */
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
/* 6/190 55-CAA 201 45 Ux                                            */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.3 1998/05/26 12:05:08 Exp                     */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Lambkin                                                      */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Contiains the prototypes for the ET96 MAP Phase 2 dialogue        */ 
/* primitives                                                        */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 971105       Ulf Melin       Changed return code from             */
/*                              MAP_API_RESULT_T to USHORT_T         */
/*                                                                   */
/* Revision record:                                                  */
/* 980406       Ulf Hellsten                                         */
/* 981015       Anders Wallden   Changed parameter in Et96MapOpenReq,*/
/*                               ValEt96MapOpenReq,                  */
/*                               Et96MapUnpackOpenInd,               */
/*                               and ValEt96MapUnpackOpenInd         */
/*                               from MAP_ADDRESS_T                  */
/*                               to MAP_SS7_ADDR_T                   */
/* 990104  Ulf hellsten                                              */
/* 010122  Magnus Malmgren       Added                               */
/*								     */
/* 010606  Patrik Verme	    	Added V3 to ATI prototypes.	     */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
    
    
 
#ifndef __ET96MAP_DIALOGUE_H__
#define __ET96MAP_DIALOGUE_H__



/* maximum length of the error message buffer */
#define ET96MAP_MAX_ERR_STR 100

#ifdef EINSS7_FUNC_POINTER

typedef USHORT_T (*ET96MAPGETACVERSIONCONF)(ET96MAP_LOCAL_SSN_T, UCHAR_T,
                                          ET96MAP_SS7_ADDR_T*,
                                          ET96MAP_APP_CONTEXT_T);
                                          

typedef USHORT_T  (*ET96MAPBINDCONF)(ET96MAP_LOCAL_SSN_T,
                                     ET96MAP_BIND_STAT_T);
                                     
typedef USHORT_T  (*ET96MAPOPENIND)(ET96MAP_LOCAL_SSN_T,
                                    ET96MAP_DIALOGUE_ID_T, 
                                    ET96MAP_APP_CNTX_T*,
                                    ET96MAP_SS7_ADDR_T*,
                                    ET96MAP_SS7_ADDR_T*,
                                    ET96MAP_IMSI_T*,
                                    ET96MAP_ADDRESS_T*, 
                                    ET96MAP_USERDATA_T * );


typedef USHORT_T (*ET96MAPCLOSEIND)(ET96MAP_LOCAL_SSN_T,
                                    ET96MAP_DIALOGUE_ID_T,
                                    ET96MAP_USERDATA_T *,
                                    UCHAR_T priorityOrder);




typedef USHORT_T (*ET96MAPUABORTIND)(ET96MAP_LOCAL_SSN_T,
                                     ET96MAP_DIALOGUE_ID_T,
                                     ET96MAP_USER_REASON_T *,
                                     ET96MAP_DIAGNOSTIC_INFO_T*,
                                     ET96MAP_USERDATA_T *,
                                     UCHAR_T priorityOrder);  

typedef USHORT_T (*ET96MAPDELIMITERIND)(ET96MAP_LOCAL_SSN_T,
                                        ET96MAP_DIALOGUE_ID_T,
                                        UCHAR_T priorityOrder);


typedef USHORT_T (*ET96MAPPABORTIND)(ET96MAP_LOCAL_SSN_T,
                                     ET96MAP_DIALOGUE_ID_T,
                                     ET96MAP_PROV_REASON_T,
                                     ET96MAP_SOURCE_T,
                                     UCHAR_T priorityOrder);





typedef USHORT_T (*ET96MAPNOTICEIND)(ET96MAP_LOCAL_SSN_T,
                                     ET96MAP_DIALOGUE_ID_T,
                                     ET96MAP_DIALOGUE_ID_T, 
                                     ET96MAP_DIAGNOSTIC_PROBLEM_T,
                                     ET96MAP_RETURNED_OPERATION_T*);
                                     

typedef USHORT_T (*ET96MAPSTATEIND)(ET96MAP_LOCAL_SSN_T,
                                   UCHAR_T,
                                   UCHAR_T,
                                   ULONG_T,
                                   ULONG_T);


typedef USHORT_T (*ET96MAPOPENCONF)(ET96MAP_LOCAL_SSN_T,
                                    ET96MAP_DIALOGUE_ID_T,
                                    ET96MAP_OPEN_RESULT_T,
                                    ET96MAP_REFUSE_REASON_T*,
                                    ET96MAP_SS7_ADDR_T*,
                                    ET96MAP_APP_CNTX_T*,
                                    ET96MAP_USERDATA_T*,
                                    ET96MAP_PROV_ERR_T*);

typedef void (*ET96MAPINDICATIONERROR)(USHORT_T error, UCHAR_T *errString);



typedef USHORT_T (*ET96MAPV1FORWARDSMIND)(ET96MAP_LOCAL_SSN_T, 
                                          ET96MAP_DIALOGUE_ID_T,
                                          ET96MAP_INVOKE_ID_T, 
                                          ET96MAP_SM_RP_DA_T*,
                                          ET96MAP_SM_RP_OA_T*,
                                          ET96MAP_SM_RP_UI_T*);
                                          
typedef USHORT_T (*ET96MAPV2FORWARDSMIND)(ET96MAP_LOCAL_SSN_T, 
                                          ET96MAP_DIALOGUE_ID_T,
                                          ET96MAP_INVOKE_ID_T, 
                                          ET96MAP_SM_RP_DA_T*,
                                          ET96MAP_SM_RP_OA_T*,
                                          ET96MAP_SM_RP_UI_T*);                                   

typedef USHORT_T (*ET96MAPV1ALERTSCIND)(ET96MAP_LOCAL_SSN_T, 
                                        ET96MAP_DIALOGUE_ID_T,
                                        ET96MAP_INVOKE_ID_T,
                                        ET96MAP_ADDRESS_T*,
                                        ET96MAP_ADDRESS_T*);


typedef USHORT_T (*ET96MAPV2ALERTSCIND)(ET96MAP_LOCAL_SSN_T, 
                                        ET96MAP_DIALOGUE_ID_T,
                                        ET96MAP_INVOKE_ID_T,
                                        ET96MAP_ADDRESS_T*,
                                        ET96MAP_ADDRESS_T*);

typedef USHORT_T (*ET96MAPV2INFORMSCIND)(ET96MAP_LOCAL_SSN_T, 
                                         ET96MAP_DIALOGUE_ID_T,
                                         ET96MAP_INVOKE_ID_T,
                                         ET96MAP_ADDRESS_T*,
                                         ET96MAP_MWD_STATUS_T*);                                          


typedef USHORT_T (*ET96MAPV1FORWARDSMCONF)(ET96MAP_LOCAL_SSN_T, 
                                           ET96MAP_DIALOGUE_ID_T,
                                           ET96MAP_INVOKE_ID_T,
                                           ET96MAP_ERROR_FORW_SM_MT_T*, /* anwa TR1351 */
                                           ET96MAP_PROV_ERR_T*);


typedef USHORT_T (*ET96MAPV2FORWARDSMCONF)(ET96MAP_LOCAL_SSN_T, 
                                           ET96MAP_DIALOGUE_ID_T,
                                           ET96MAP_INVOKE_ID_T,
                                           ET96MAP_ERROR_FORW_SM_MT_T*, /* anwa TR1351 */
                                           ET96MAP_PROV_ERR_T*);        

typedef USHORT_T (*ET96MAPV2FORWARDSMMOCONF)(ET96MAP_LOCAL_SSN_T, 
                                           ET96MAP_DIALOGUE_ID_T,
                                           ET96MAP_INVOKE_ID_T,
                                           ET96MAP_ERROR_FORW_SM_MO_T*, 
                                           ET96MAP_PROV_ERR_T*);        


typedef USHORT_T (*ET96MAPV1SENDRINFOFORSMCONF)(ET96MAP_LOCAL_SSN_T, 
                                                ET96MAP_DIALOGUE_ID_T,
                                                ET96MAP_INVOKE_ID_T,
                                                ET96MAP_IMSI_T*,                                                
                                                ET96MAP_LOCATION_INFO_T*,
                                                ET96MAP_LMSI_T*,
                                                ET96MAP_MWD_SET_T*,
                                                ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T*, /* anwa TR1351 */
                                                ET96MAP_PROV_ERR_T*);

typedef USHORT_T (*ET96MAPV2SENDRINFOFORSMCONF)(ET96MAP_LOCAL_SSN_T, 
                                                ET96MAP_DIALOGUE_ID_T,
                                                ET96MAP_INVOKE_ID_T,
                                                ET96MAP_IMSI_T*,
                                                ET96MAP_ADDRESS_T*,
                                                ET96MAP_LMSI_T*,
                                                ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T*, /* anwa TR1351 */
                                                ET96MAP_PROV_ERR_T*);


typedef USHORT_T (*ET96MAPV1REPORTSMDELSTATCONF)(ET96MAP_LOCAL_SSN_T, 
                                                 ET96MAP_DIALOGUE_ID_T,
                                                 ET96MAP_INVOKE_ID_T,
                                                 ET96MAP_ERROR_REPORT_SM_DEL_STAT_T*, /* anwa TR1351 */
                                                 ET96MAP_PROV_ERR_T*);

typedef USHORT_T (*ET96MAPV2REPORTSMDELSTATCONF)(ET96MAP_LOCAL_SSN_T, 
                                                 ET96MAP_DIALOGUE_ID_T,
                                                 ET96MAP_INVOKE_ID_T,
                                                 ET96MAP_ADDRESS_T* ,
                                                 ET96MAP_ERROR_REPORT_SM_DEL_STAT_T*, /* anwa TR1351 */
                                                 ET96MAP_PROV_ERR_T*);           


/* Added by DON */
typedef USHORT_T (*ET96MAPV3ANYTIMEINTERROGATIONCONF)(ET96MAP_LOCAL_SSN_T,
						      ET96MAP_DIALOGUE_ID_T,
						      ET96MAP_INVOKE_ID_T,
						      ET96MAP_AGE_OF_LOCATION_INFO_T*,
						      ET96MAP_ATI_GEOGRAPHICAL_INFO_T*,
						      ET96MAP_ADDRESS_T*,
						      ET96MAP_LOCATIONNUMBER_T*,
						      ET96MAP_CELLID_OR_SERVICE_OR_LAI_T*,
#ifndef VER_MAP_R9
                                                      BOOLEAN_T,
#endif
						      ET96MAP_SELECTED_LSA_ID_T*,
						      ET96MAP_ADDRESS_T*,
						      ET96MAP_GEODETICINFO_T*,
						      BOOLEAN_T,
						      ET96MAP_ATI_SUBSCRIBER_STATE_T*,
						      ET96MAP_ERROR_ANY_TIME_INTERROGATION_T*,
						      ET96MAP_PROV_ERR_T*);

typedef USHORT_T (*ET96MAPV3ANYTIMEINTERROGATIONIND)(ET96MAP_LOCAL_SSN_T,
						     ET96MAP_DIALOGUE_ID_T,
						     ET96MAP_INVOKE_ID_T,
						     ET96MAP_ADDRESS_T*,
						     ET96MAP_IMSI_T*,
						     ET96MAP_ADDRESS_T*,
						     BOOLEAN_T,
						     BOOLEAN_T,
						     BOOLEAN_T);


typedef USHORT_T (*ET96MAPV1BEGINSUBSCRIBERACTIVITYIND)(ET96MAP_LOCAL_SSN_T,
                                                        ET96MAP_DIALOGUE_ID_T,
                                                        ET96MAP_INVOKE_ID_T,
                                                        ET96MAP_IMSI_T,
                                                        ET96MAP_ADDRESS_T);

typedef USHORT_T (*ET96MAPV1PROCESSUNSTRUCTUREDSSDATAIND)(ET96MAP_LOCAL_SSN_T,
                                                          ET96MAP_DIALOGUE_ID_T,
                                                          ET96MAP_INVOKE_ID_T,
                                                          ET96MAP_SS_USER_DATA_T);

typedef USHORT_T (*ET96MAPV2PROCESSUNSTRUCTUREDSSREQUESTIND)(ET96MAP_LOCAL_SSN_T,
                                                             ET96MAP_DIALOGUE_ID_T,
                                                             ET96MAP_INVOKE_ID_T,
                                                             ET96MAP_USSD_DATA_CODING_SCHEME_T,
                                                             ET96MAP_USSD_STRING_T,
                                                             ET96MAP_ADDRESS_T*);

typedef USHORT_T (*ET96MAPV2UNSTRUCTUREDSSREQUESTCONF)(ET96MAP_LOCAL_SSN_T,
                                                       ET96MAP_DIALOGUE_ID_T,
                                                       ET96MAP_INVOKE_ID_T,
                                                       ET96MAP_USSD_DATA_CODING_SCHEME_T*,
                                                       ET96MAP_USSD_STRING_T*,
                                                       ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T*,
                                                       ET96MAP_PROV_ERR_T*); 


typedef USHORT_T (*ET96MAPV2UNSTRUCTEDNOTIFYCONF)(ET96MAP_LOCAL_SSN_T     localSsn,
                                                  ET96MAP_DIALOGUE_ID_T dialogueId,
                                                  ET96MAP_INVOKE_ID_T       invokeId,
                                                  ET96MAP_ERROR_USSD_NOTIFY_T*,
                                                  ET96MAP_PROV_ERR_T*);
#ifdef VER_MAP_R9

typedef USHORT_T (*ET96MAPV3PROVSUBSCRLCSCONF)( ET96MAP_LOCAL_SSN_T,
                                                ET96MAP_DIALOGUE_ID_T,
                                                ET96MAP_INVOKE_ID_T,
                                                ET96MAP_GEOGRAPHICAL_INFO_T *,
                                                ET96MAP_AGE_OF_LOCATION_INFO_T *,  
                                                ET96MAP_ERROR_PROVIDE_SUBSCRIBER_LCS_T *, 
                                                ET96MAP_PROV_ERR_T *); 


#else


typedef USHORT_T (*ET96MAPV3PROVSUBSCRLCSCONF)( ET96MAP_LOCAL_SSN_T,
                                                ET96MAP_DIALOGUE_ID_T,
                                                ET96MAP_INVOKE_ID_T,
                                                ET96MAP_GEOGRAPHICAL_INFO_T *,
                                                ET96MAP_AGE_OF_LOCATION_INFO_T *,  
                                                ET96MAP_ERROR_PROVIDE_SUBSCRIBER_LCS_T *, 
                                                ET96MAP_ADD_GEOGRAPHICAL_INFO_T *,
                                                BOOLEAN_T,
                                                ET96MAP_PROV_ERR_T *); 



#endif

 
#ifdef VER_MAP_R9

typedef USHORT_T (*ET96MAPV3SENDRINFOFORLCSCONF)(ET96MAP_LOCAL_SSN_T,
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
typedef USHORT_T (*ET96MAPV3SENDRINFOFORLCSCONF)(ET96MAP_LOCAL_SSN_T,
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


#ifdef VER_MAP_R9

typedef USHORT_T (*ET96MAPV3SUBSCRLCSREPORTIND)(ET96MAP_LOCAL_SSN_T,
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

typedef USHORT_T (*ET96MAPV3SUBSCRLCSREPORTIND)
                             (ET96MAP_LOCAL_SSN_T,
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


typedef USHORT_T (*ET96MAPV3FORWARDSMMTCONF)(ET96MAP_LOCAL_SSN_T,
                                             ET96MAP_DIALOGUE_ID_T,
                                             ET96MAP_INVOKE_ID_T,
                                             ET96MAP_SM_RP_UI_T*,
                                             ET96MAP_ERROR_FORW_SM_MT_T*,
                                             ET96MAP_PROV_ERR_T*);

typedef USHORT_T (*ET96MAPV3SENDRINFOFORSMCONF)(ET96MAP_LOCAL_SSN_T,
                                                ET96MAP_DIALOGUE_ID_T,
                                                ET96MAP_INVOKE_ID_T,
                                                ET96MAP_IMSI_T*,
                                                ET96MAP_ADDRESS_T*,
                                                ET96MAP_LMSI_T*,
#ifdef VER_MAP_R9
                                                ET96MAP_GPRS_NODE_IND_T* ,
#else
                                                ET96MAP_GPRS_NODE_IND_T ,
#endif
                                                ET96MAP_ADDITIONAL_NUMBER_T*,
                                                ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T*,
                                                ET96MAP_PROV_ERR_T*);

typedef USHORT_T (*ET96MAPV3FORWARDSMMOIND)(ET96MAP_LOCAL_SSN_T,
                                            ET96MAP_DIALOGUE_ID_T,
                                            ET96MAP_INVOKE_ID_T,
                                            ET96MAP_SM_RP_DA_T*,
                                            ET96MAP_SM_RP_OA_T*,
                                            ET96MAP_SM_RP_UI_T*,
                                            ET96MAP_IMSI_T*);

typedef USHORT_T (*ET96MAPV3REPORTSMDELSTATCONF)(ET96MAP_LOCAL_SSN_T,
                                                 ET96MAP_DIALOGUE_ID_T,
                                                 ET96MAP_INVOKE_ID_T,
                                                 ET96MAP_ADDRESS_T*,
                                                 ET96MAP_ERROR_REPORT_SM_DEL_STAT_T*,
                                                 ET96MAP_PROV_ERR_T*);



typedef USHORT_T (*ET96MAPV3INFORMSCIND)(ET96MAP_LOCAL_SSN_T,
                                         ET96MAP_DIALOGUE_ID_T,
                                         ET96MAP_INVOKE_ID_T,
                                         ET96MAP_ADDRESS_T*,
                                         ET96MAP_MWD_STATUS_T*);




#ifndef MAP_VER_R9
       
typedef USHORT_T (*ET96MAPV3PROVIDESUBSCRIBERINFOCONF)(ET96MAP_LOCAL_SSN_T localSsn,
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


typedef struct ET96MAPINIT_T
{
  ET96MAPGETACVERSIONCONF                   Et96MapGetACVersionConf;
  ET96MAPBINDCONF                           Et96MapBindConf;
  ET96MAPV1FORWARDSMCONF                    Et96MapV1ForwardSmMT_MOConf;
  ET96MAPV2FORWARDSMCONF                    Et96MapV2ForwardSmMTConf;
  ET96MAPV2FORWARDSMMOCONF                  Et96MapV2ForwardSmMOConf;
  ET96MAPOPENCONF                           Et96MapOpenConf;
  ET96MAPV1REPORTSMDELSTATCONF              Et96MapV1ReportSmDelStatConf;
  ET96MAPV2REPORTSMDELSTATCONF              Et96MapV2ReportSmDelStatConf;
  ET96MAPV1SENDRINFOFORSMCONF               Et96MapV1SendRInfoForSmConf;
  ET96MAPV2SENDRINFOFORSMCONF               Et96MapV2SendRInfoForSmConf;   
  ET96MAPV1ALERTSCIND                       Et96MapV1AlertSCInd;
  ET96MAPV2ALERTSCIND                       Et96MapV2AlertSCInd;
  ET96MAPCLOSEIND                           Et96MapCloseInd;
  ET96MAPDELIMITERIND                       Et96MapDelimiterInd;
  ET96MAPV1FORWARDSMIND                     Et96MapV1ForwardSmMOInd;
  ET96MAPV2FORWARDSMIND                     Et96MapV2ForwardSmMOInd; 
  ET96MAPV2INFORMSCIND                      Et96MapV2InformSCInd;
  ET96MAPNOTICEIND                          Et96MapNoticeInd;
  ET96MAPOPENIND                            Et96MapOpenInd;
  ET96MAPPABORTIND                          Et96MapPAbortInd;
  ET96MAPSTATEIND                           Et96MapStateInd;
  ET96MAPUABORTIND                          Et96MapUAbortInd; 
  ET96MAPINDICATIONERROR                    Et96MapIndicationError;
  ET96MAPV3PROVSUBSCRLCSCONF                Et96MapV3ProvideSubscriberLCSConf; 
  ET96MAPV3SENDRINFOFORLCSCONF              Et96MapV3SendRInfoForLCSConf;
  ET96MAPV3SUBSCRLCSREPORTIND               Et96MapV3SubscriberLCSReportInd;
  ET96MAPV3FORWARDSMMTCONF                  Et96MapV3ForwardSmMTConf;    
  ET96MAPV3SENDRINFOFORSMCONF               Et96MapV3SendRInfoForSmConf;
  ET96MAPV3FORWARDSMMOIND                   Et96MapV3ForwardSmMOInd;
  ET96MAPV3REPORTSMDELSTATCONF              Et96MapV3ReportSmDelStatConf;   
  ET96MAPV3INFORMSCIND                      Et96MapV3InformSCInd;
  ET96MAPV1BEGINSUBSCRIBERACTIVITYIND       Et96MapV1BeginSubscriberActivityInd;
  ET96MAPV1PROCESSUNSTRUCTUREDSSDATAIND     Et96MapV1ProcessUnstructuredSSDataInd;
  ET96MAPV2PROCESSUNSTRUCTUREDSSREQUESTIND  Et96MapV2ProcessUnstructuredSSRequestInd;
  ET96MAPV2UNSTRUCTUREDSSREQUESTCONF        Et96MapV2UnstructuredSSRequestConf;
  ET96MAPV2UNSTRUCTEDNOTIFYCONF             Et96MapV2UnstructuredSSNotifyConf;
  ET96MAPV3ANYTIMEINTERROGATIONCONF           Et96MapV3AnyTimeInterrogationConf;
  ET96MAPV3ANYTIMEINTERROGATIONIND            Et96MapV3AnyTimeInterrogationInd;
#ifndef VER_MAP_R9
  ET96MAPV3PROVIDESUBSCRIBERINFOCONF         Et96MapV3ProvideSubscriberInfoConf;
#endif


}ET96MAPINIT_T;




extern ET96MAPINIT_T et96MapApplFunc_s;

USHORT_T  Et96MapRegFunc(ET96MAPINIT_T  *newFuncs);

#else

void Et96MapIndicationError(USHORT_T error, UCHAR_T *errString);



                              

USHORT_T Et96MapOpenInd(ET96MAP_LOCAL_SSN_T,
                        ET96MAP_DIALOGUE_ID_T, 
                        ET96MAP_APP_CNTX_T*,
                        ET96MAP_SS7_ADDR_T*,
                        ET96MAP_SS7_ADDR_T*,
                        ET96MAP_IMSI_T*,
                        ET96MAP_ADDRESS_T*,
                        ET96MAP_USERDATA_T *);

USHORT_T Et96MapCloseInd(ET96MAP_LOCAL_SSN_T,
                         ET96MAP_DIALOGUE_ID_T,
                         ET96MAP_USERDATA_T *,
                         UCHAR_T priorityOrder);

USHORT_T Et96MapUAbortInd(ET96MAP_LOCAL_SSN_T,
                          ET96MAP_DIALOGUE_ID_T,
                          ET96MAP_USER_REASON_T *,
                          ET96MAP_DIAGNOSTIC_INFO_T*,
                          ET96MAP_USERDATA_T *,UCHAR_T priorityOrder);



USHORT_T Et96MapDelimiterInd(ET96MAP_LOCAL_SSN_T,
                             ET96MAP_DIALOGUE_ID_T ,UCHAR_T priorityOrder);


USHORT_T Et96MapNoticeInd(ET96MAP_LOCAL_SSN_T,
                          ET96MAP_DIALOGUE_ID_T,
                          ET96MAP_DIALOGUE_ID_T,
                          ET96MAP_DIAGNOSTIC_PROBLEM_T,
                          ET96MAP_RETURNED_OPERATION_T *);
                          
USHORT_T Et96MapPAbortInd(ET96MAP_LOCAL_SSN_T,
                          ET96MAP_DIALOGUE_ID_T,
                          ET96MAP_PROV_REASON_T,
                          ET96MAP_SOURCE_T ,UCHAR_T priorityOrder);
                          
USHORT_T Et96MapStateInd(ET96MAP_LOCAL_SSN_T,
                         UCHAR_T,
                         UCHAR_T,
                         ULONG_T,
                         ULONG_T);




USHORT_T Et96MapOpenConf(ET96MAP_LOCAL_SSN_T,
                         ET96MAP_DIALOGUE_ID_T,
                         ET96MAP_OPEN_RESULT_T,
                         ET96MAP_REFUSE_REASON_T*,
                         ET96MAP_SS7_ADDR_T *,
                         ET96MAP_APP_CNTX_T*,
                         ET96MAP_USERDATA_T*,
                         ET96MAP_PROV_ERR_T*);
                          
USHORT_T  Et96MapBindConf(ET96MAP_LOCAL_SSN_T localSsn,
                          ET96MAP_BIND_STAT_T);

USHORT_T Et96MapGetACVersionConf(ET96MAP_LOCAL_SSN_T, UCHAR_T,
                               ET96MAP_SS7_ADDR_T*,
                               ET96MAP_APP_CONTEXT_T);

#endif

USHORT_T  Et96MapGetACVersionReq(ET96MAP_LOCAL_SSN_T, 
                                 ET96MAP_SS7_ADDR_T *ss7Address_sp,
                                 ET96MAP_APP_CONTEXT_T ac);

USHORT_T  Et96MapOpenReq(ET96MAP_LOCAL_SSN_T localSsn,
                         ET96MAP_DIALOGUE_ID_T dialogueId,
                         ET96MAP_APP_CNTX_T *appContext_sp,
                         ET96MAP_SS7_ADDR_T *ss7DestAddr_sp, /* anwa */
                         ET96MAP_SS7_ADDR_T *ss7OrigAddr_sp,  /* anwa */
                         ET96MAP_IMSI_T *destRef_sp,
                         ET96MAP_ADDRESS_T *origRef_sp,
                         ET96MAP_USERDATA_T *specificInfo_sp );

USHORT_T  Et96MapUnpackPAbortInd(MSG_T *ipcMessage_sp,
                                 ET96MAP_PABORT_IND_T *pAbortInd_sp);

USHORT_T  Et96MapBindReq(USHORT_T  mUserId,
                         ET96MAP_LOCAL_SSN_T localSsn);


USHORT_T  Et96MapUnbindReq(ET96MAP_LOCAL_SSN_T localSsn);


USHORT_T Et96MapUnpackBindConf(MSG_T *ipcMessage_sp,
                               ET96MAP_BIND_T *mBind_sp);

USHORT_T  Et96MapCloseReq(ET96MAP_LOCAL_SSN_T      localSsn,
                          ET96MAP_DIALOGUE_ID_T dialogueId,
                          ET96MAP_RELEASE_METHOD_T releaseMethod,
                          ET96MAP_USERDATA_T  *specificInfo_sp ,
                          UCHAR_T priorityOrder ,
                          UCHAR_T qualityOfService);


USHORT_T  Et96MapUAbortReq(ET96MAP_LOCAL_SSN_T      localSsn,
                           ET96MAP_DIALOGUE_ID_T dialogueId,
                           ET96MAP_USER_REASON_T    *userReason_p,
                           ET96MAP_DIAGNOSTIC_INFO_T *diagInfo_p,
                           ET96MAP_USERDATA_T  *specificInfo_sp ,UCHAR_T priorityOrder);


USHORT_T  Et96MapDelimiterReq(ET96MAP_LOCAL_SSN_T      localSsn,
                              ET96MAP_DIALOGUE_ID_T dialogueId ,
                              UCHAR_T priorityOrder, 
                              UCHAR_T qualityOfService);

USHORT_T  Et96MapOpenResp(ET96MAP_LOCAL_SSN_T   localSsn,
                          ET96MAP_DIALOGUE_ID_T dialogueId,
                          ET96MAP_OPEN_RESULT_T openRes,
                          ET96MAP_REFUSE_REASON_T *refuseReason_p,
                          ET96MAP_SS7_ADDR_T   *ss7RespAddress_sp,
                          ET96MAP_APP_CNTX_T *appContext_sp,
                          ET96MAP_USERDATA_T *specificInfo_sp );


USHORT_T  Et96MapUnpackGetACVersionConf(MSG_T *ipcMessage_sp,
                                        ET96MAP_GET_VERSION_REQ_CONF_T *);

USHORT_T  Et96MapUnpackOpenConf(MSG_T *ipcMessage_sp,
                                ET96MAP_OPEN_CONF_RSP_T *mOpenCnf_sp);

USHORT_T Et96MapUnpackOpenInd(MSG_T *ipcMessage_sp,
                                      ET96MAP_OPEN_REQ_IND_T *mOpenInd_sp);


USHORT_T Et96MapUnpackCloseInd(MSG_T *ipcMessage_sp,
                                       ET96MAP_CLOSE_REQ_IND_T *mCloseInd_sp);


USHORT_T Et96MapUnpackUAbortInd(MSG_T *ipcMessage_sp,
                                        ET96MAP_UABORT_REQ_IND_T *mUAbortInd_sp );


USHORT_T Et96MapUnpackDelimiterInd(MSG_T *ipcMessage_sp,
                                           ET96MAP_DELIMIT_REQ_IND_T *mDelimInd_sp);


USHORT_T Et96MapUnpackNoticeInd(MSG_T *ipcMessage_sp,
                                        ET96MAP_NOTICE_IND_T *mNoticeInd_sp);

USHORT_T  ValEt96MapOpenReq(
        ET96MAP_LOCAL_SSN_T      localSsn,
        ET96MAP_DIALOGUE_ID_T    dialogueId,
        ET96MAP_APP_CNTX_T       *appContext_sp,
        ET96MAP_SS7_ADDR_T       *ss7DestAddr_sp, /* anwa */
        ET96MAP_IMSI_T           *destRef_sp,
        ET96MAP_SS7_ADDR_T       *ss7OrigAddr_sp, /* anwa */
        ET96MAP_ADDRESS_T        *origRef_sp,
        ET96MAP_USERDATA_T       *specificInfo_sp);

USHORT_T  ValEt96MapUAbortReq(ET96MAP_LOCAL_SSN_T      localSsn,
                ET96MAP_DIALOGUE_ID_T dialogueId,
                ET96MAP_USER_REASON_T    *userReason_p,
                ET96MAP_DIAGNOSTIC_INFO_T *diagInfo_p,
                ET96MAP_USERDATA_T *specificInfo_sp);

BOOLEAN_T  ValEt96MapProvReason(ET96MAP_PROV_REASON_T provReason);

USHORT_T ValEt96MapUnpackOpenInd( ET96MAP_LOCAL_SSN_T localSsn,
                                  ET96MAP_DIALOGUE_ID_T dialogueId,
                                  ET96MAP_APP_CNTX_T *appContext_sp,
                                  ET96MAP_SS7_ADDR_T *ss7DestAddr_sp, /* anwa */
                                  ET96MAP_IMSI_T     *destRef_sp,
                                  ET96MAP_SS7_ADDR_T *ss7OrigAddr_sp, /* anwa */
                                  ET96MAP_ADDRESS_T *origRef_sp,
                                  ET96MAP_USERDATA_T *specificInfo_sp );


USHORT_T Et96MapUnpackStateInd(MSG_T *ipcMessage_sp,
                                       ET96MAP_STATE_IND_T *mStateInd_sp);


USHORT_T  ValEt96MapStateInd(ET96MAP_LOCAL_SSN_T localSsn,
                                     UCHAR_T userState);



void Et96MapIndicationErr(USHORT_T error, UCHAR_T *errString);

USHORT_T Et96MapHandleIndication(MSG_T *msg_sp);
BOOLEAN_T ValEt96MapForwardSmErrorCode(UCHAR_T errorCode,UCHAR_T version); /* anwa TR1351 */
BOOLEAN_T ValEt96MapAlertServiceErrorCode(UCHAR_T errorCode); /* anwa TR1351 */
BOOLEAN_T ValEt96MapProcessUssdReqErrorCode(UCHAR_T errorCode); /* anwa TR1351 */
BOOLEAN_T ValEt96MapReadyForSmErrorCode(UCHAR_T errorCode); /* anwa TR1351 */
BOOLEAN_T ValEt96MapUssdNotifyErrorCode(UCHAR_T errorCode); /* anwa TR1351 */
BOOLEAN_T ValEt96MapUssdReqErrorCode(UCHAR_T errorCode); /* anwa TR1351 */
BOOLEAN_T ValEt96MapProviderError(ET96MAP_PROV_ERR_T provError);
BOOLEAN_T ValEt96MapAppContext(ET96MAP_APP_CNTX_T *appContext_sp);
BOOLEAN_T ValEt96MapAppContextReq(ET96MAP_APP_CNTX_T *appContext_sp);
BOOLEAN_T ValEt96MapAC(ET96MAP_APP_CNTX_T *appContext_sp);

BOOLEAN_T ValEt96MapAppContextInd(ET96MAP_APP_CNTX_T *appContext_sp);
BOOLEAN_T ValEt96MapRepSmDelStatErrorCode(UCHAR_T errorCode,UCHAR_T version);
BOOLEAN_T ValEt96MapSendRoutingInfoErrorCode(UCHAR_T errorCode,UCHAR_T version);

/* Added by DON */
BOOLEAN_T ValAnyTimeInterrogationErrorCode(UCHAR_T errorCode);
/* Added by DON */

/* AePONA */
BOOLEAN_T ValEt96MapProcessUnstructuredSSDataErrorCode(UCHAR_T errorCode);
BOOLEAN_T ValEt96MapProcessUnstructuredSSRequestErrorCode(UCHAR_T errorCode); 
BOOLEAN_T ValEt96MapUnstructuredSSRequestErrorCode(UCHAR_T errorCode); 
/* AePONA */


/*********************************************************************/
/*                                                                   */
/*                         EXTERN VARIABLES                          */
/*                                                                   */
/*********************************************************************/



extern UCHAR_T et96mapOrigId;

#endif /* __ET96MAP_DIALOGUE_H__ */
