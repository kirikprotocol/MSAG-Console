/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_types.h,v                                                         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* Et96map_types.h                                                   */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998          		     */
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
/* 34/190 55-CAA 201 45 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.18 2000/04/07 09:05:28 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Concannon                                                    */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Provides common types and structures used by the ET96 MAP Module  */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* <date>       <Author>         <Revision text>                     */
/*                                                                   */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __ET96MAP_TYPES_H__
#define __ET96MAP_TYPES_H__




/* timer type */
typedef UCHAR_T  ET96MAP_TIMER_T;

/* Transaction id */
typedef USHORT_T  ET96MAP_TRANS_ID_T;

/* ASN1 operation code */
typedef UCHAR_T  ET96MAP_MAP_OPCODE_T;

typedef SHORT_T ET96MAP_PRIMITIVE_T;

/* type of value returned by transaction state machine */
typedef LONG_T ET96MAP_TRANS_RETCODE_T;

/* union of protocol machine messages */
typedef union 
{
  ET96MAP_TBIND_T        tBind_s;
  ET96MAP_TBIND_T        tBindConf_s;
  ET96MAP_TBEGIN_T       tBegin_s;
  ET96MAP_TCONTINUE_T    tContinue_s;
  ET96MAP_TEND_T         tEnd_s;
  ET96MAP_TINV_RES_ERR_T tInvOrResOrErr_s;
  ET96MAP_T_CANCEL_T     tCancel_s;    
  ET96MAP_TREJECT_T      tReject_s;
  ET96MAP_TSTATE_T       tState_s;
  ET96MAP_TU_ABORT_T     tUAbort_s;
  ET96MAP_TP_ABORT_T     tPAbort_s;
  ET96MAP_T_NOTICE_T     tNotice_s;
  ET96MAP_BEGIN_T             mBegin_s;
  ET96MAP_BIND_T              mBind_s;
  ET96MAP_END_REQ_T           mEnd_s;
  ET96MAP_ERR_T               mErr_s;
  ET96MAP_UNBIND_T            mUnbind_s;
  ET96MAP_ALERT_IND_T         mAlertInd_s;
  ET96MAP_OPEN_REQ_IND_T      mOpenReqInd_s;
  ET96MAP_OPEN_CONF_RSP_T     mOpenConfRsp_s;
  ET96MAP_CLOSE_REQ_IND_T     mCloseReqInd_s;
  ET96MAP_UABORT_REQ_IND_T    mUAbortReqInd_s;
  ET96MAP_PABORT_IND_T        mPAbortInd_s;
  ET96MAP_NOTICE_IND_T        mNoticeInd_s;
  ET96MAP_USSD_T              mUssd_s;
  ET96MAP_USSD_NOTIFY_CNF_T   mUssdNotifyCnf_s; /* anwa added TR1351 */
  ET96MAP_USSD_REQ_CNF_T      mUssdReqCnf_s; /* anwa added TR1351 */
  ET96MAP_PROC_USSD_REQ_CNF_T mProcUssdReqCnf_s; /* anwa added TR1351 */
  ET96MAP_DEL_SM_REQ_T        mDelSmReq_s;
  ET96MAP_RPT_SM_DEL_REQ_IND_T mRptSmDelReqInd_s;
  ET96MAP_RPT_SM_DEL_CONF_RSP_T  mRptSmDelConfRsp_s; 
  ET96MAP_SUBMIT_SM_CONF_T     mSubmitSmConf_s;
  ET96MAP_SUBMIT_SM_IND_T      mSubmitSmInd_s;
  ET96MAP_SND_RINFO_SM_REQ_IND_T  mSndRinfoSmReqInd_s;
  ET96MAP_SND_RINFO_SM_CONF_RSP_T mSndRinfoSmConfRsp_s; 
  ET96MAP_FWD_SM_REQ_IND_T     mFwdSmReqInd_s;
  ET96MAP_FWD_SM_CONF_RSP_T    mFwdSmConfRsp_s;
  ET96MAP_FWD_SM_MO_V2_CONF    mFwdSmMoV2Conf_s;
  ET96MAP_RDY_SM_REQ_IND_T     mRdySmReqInd_s;
  ET96MAP_RDY_SM_CONF_RSP_T    mRdySmConfRsp_s; 
  ET96MAP_ALERT_SC_REQ_IND_T   mAlertSCReqInd_s;
  ET96MAP_ALERT_SC_CONF_RSP_T  mAlertSCConfRsp_s; 
  ET96MAP_INFORM_SC_REQ_IND_T  mInformSCReqInd_s;
  ET96MAP_REPORT_SM_IND_T      mReportSmInd_s;
  ET96MAP_DELIMIT_REQ_IND_T    mDelimitReqInd_s;
  ET96MAP_PABORT_IND_T         mPAbortReqInd_s;
  ET96MAP_STATE_IND_T          mStateInd_s;
  ET96MAP_GET_VERSION_REQ_CONF_T mGetVersionReqConf_s;

  /* New for LCS Op. */


  ET96MAP_V3_SND_RINFO_LCS_CONF_T mSndRinfoLCSConf_s;
  ET96MAP_V3_SND_RINFO_LCS_REQ_T mSndRinfoLCSReq_s;
  ET96MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_REQ_T  mProvideSubscriberLCSReq_s;
  ET96MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_CONF_T mProvideSubscriberLCSConf_s;
  ET96MAP_V3_SUBSCRIBER_REPORT_LCS_IND_T mSubscriberLCSReportInd_s;
  ET96MAP_V3_SUBSCRIBER_REPORT_LCS_RESP_T mSubscriberLCSReportResp_s;

   /* AePONA */
  ET96MAP_BEGIN_SUBSCRIBER_ACTIVITY_IND_T mBeginSubscriberActivityInd_s;
  ET96MAP_PROCESS_UNSTRUCTURED_SS_DATA_IND_T    mProcessUnstructuredSSDataInd_s;
  ET96MAP_PROCESS_UNSTRUCTURED_SS_DATA_RSP_T    mProcessUnstructuredSSDataRsp_s;
  ET96MAP_PROCESS_UNSTRUCTURED_SS_REQUEST_IND_T    mProcessUnstructuredSSRequestInd_s;    
  ET96MAP_PROCESS_UNSTRUCTURED_SS_REQUEST_RSP_T    mProcessUnstructuredSSRequestRsp_s; 
  ET96MAP_UNSTRUCTURED_SS_REQUEST_REQ_T mUnstructuredSSRequestReq_s;  
  ET96MAP_UNSTRUCTURED_SS_REQUEST_CNF_T mUnstructuredSSRequestCnf_s; 
  ET96MAP_UNSTRUCTURED_SS_NOTIFY_REQ_T mUnstructuredSSNotifyReq_s;    
  /* AEPONA */
  
  ET96MAP_ANY_TIME_INTERROGATION_CONF_RSP_T mAtiConfRsp_s;  /* Added by DON  */
  ET96MAP_ANY_TIME_INTERROGATION_REQ_IND_T mAtiReqInd_s;    /* Added by DON */
  ET96MAP_PROVIDE_SUBSCRIBER_INFO_REQ_T   mProvideSubscriberInfoReq_s;
  ET96MAP_PROVIDE_SUBSCRIBER_INFO_CONF_T   mProvideSubscriberInfoConf_s;

} ET96MAP_MSG_T;

/*********************************************************************/
/*                                                                   */
/*                     FORWARD  DECLARATIONS                         */
/*                                                                   */
/*********************************************************************/
/* tcap request prototypes - cant be added in the tcap.h file because */
/* of dependencies on the MAP_MSG_T union. */



USHORT_T Et96MapTBeginReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTUnBindReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTBindReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTUCancelReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTContinueReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTEndReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTInvokeReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTResultReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTUAbortReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTUErrorReq(ET96MAP_MSG_T *tReq_sp);
USHORT_T Et96MapTURejectReq(ET96MAP_MSG_T *tReq_sp);




#endif /* __ET96MAP_TYPES_H__ */
