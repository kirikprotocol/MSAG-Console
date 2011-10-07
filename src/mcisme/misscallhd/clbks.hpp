#include "ss7cp.h"
#include "portss7.h"
#include "ss7tmc.h"
#include "i97isup_api.h"
#include "oamapi.h"
#include "oamapi_mem.h"
//#include "mgmtapi.h"

/*********************************************************************/
/* ISUP                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_I97IsupIndError (USHORT_T errorCode,MSG_T*msg_sp,EINSS7INSTANCE_T isupInstanceId);
USHORT_T EINSS7_I97IsupBindConf(USHORT_T isupUserID,EINSS7INSTANCE_T isupInstanceId,UCHAR_T result,UCHAR_T isupStd);
USHORT_T EINSS7_I97IsupSetupInd(EINSS7_I97_ISUPHEAD_T*isupHead_sp,EINSS7INSTANCE_T isupInstanceId,USHORT_T resourceGroup,EINSS7_I97_NATUREOFCONN_T*natureOfConn_sp,EINSS7_I97_FORWARD_T*forward_sp,UCHAR_T*callingPartyCat_p,UCHAR_T*transmissionReq_p,EINSS7_I97_CALLEDNUMB_T*calledNumb_sp,EINSS7_I97_CALLINGNUMB_T*callingNumb_sp,EINSS7_I97_OPTFORWARD_T*optForward_sp,EINSS7_I97_ACCESS_T*access_sp,EINSS7_I97_REDIRECTINGNUMB_T*redirectingNumb_sp,EINSS7_I97_REDIRECTIONINFO_T*redirectionInfo_sp,EINSS7_I97_ORIGINALNUMB_T*originalNumb_sp,EINSS7_I97_USERINFORMATION_T*userInformation_sp,EINSS7_I97_USERSERVICE_T*userService_sp,EINSS7_I97_OPTPARAMS_T*extraOpts_sp);
USHORT_T EINSS7_I97IsupSetupConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,EINSS7_I97_BACKWARD_T* backward_sp,EINSS7_I97_OPTBACKWARD_T* optBackward_sp,EINSS7_I97_USERINFORMATION_T* userInformation_sp,EINSS7_I97_CONNECTEDNUMB_T* connectedNumb_sp,EINSS7_I97_ACCESS_T* access_sp,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupReleaseInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T responseInd,UCHAR_T sourceInd,EINSS7_I97_CAUSE_T* cause_sp,UCHAR_T* autoCongestLevel_p,EINSS7_I97_REDIRECTIONNUMB_T* redirectionNumb_sp,EINSS7_I97_REDIRECTIONINFO_T* redirectionInfo_sp,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupReleaseConf(EINSS7_I97_ISUPHEAD_T*isupHead_sp,EINSS7INSTANCE_T isupInstanceId,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupProceedInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,EINSS7_I97_BACKWARD_T* backward_sp,EINSS7_I97_OPTBACKWARD_T* optBackward_sp,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupProgressInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,EINSS7_I97_EVENT_T* event_sp,EINSS7_I97_BACKWARD_T* backward_sp,EINSS7_I97_OPTBACKWARD_T* optBackward_sp,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupContinuityConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T continuityEvent);
USHORT_T EINSS7_I97IsupContinuityInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T responseInd,EINSS7_I97_CONREQSTATIND_T* conReqAndStatInd_sp);
USHORT_T EINSS7_I97IsupSuspendInd(EINSS7_I97_ISUPHEAD_T*isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T* suspendResume_p,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupResumeInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T* suspendResume_p,EINSS7_I97_OPTPARAMS_T* extraOpts_sp);
USHORT_T EINSS7_I97IsupMiscInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T pamFlag,UCHAR_T miscMsgCode,EINSS7_I97_OPTPARAMS_T* miscParams_sp);
USHORT_T EINSS7_I97IsupResourceInd(USHORT_T resourceGroup,EINSS7INSTANCE_T isupInstanceId,UCHAR_T state);
USHORT_T EINSS7_I97IsupReportInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,EINSS7_I97_REPORTINFO_T* reportInfo_sp);
USHORT_T EINSS7_I97IsupCongInd(USHORT_T resourceGroup,EINSS7INSTANCE_T isupInstanceId,EINSS7_I97_SPC_T dpc,UCHAR_T congLevel);
USHORT_T EINSS7_I97IsupCircuitCancellationConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T result);
USHORT_T EINSS7_I97IsupCircuitReservationConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T result);
USHORT_T EINSS7_I97IsupCircuitCancellationInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,EINSS7INSTANCE_T isupInstanceId,UCHAR_T cause);
USHORT_T brokenConnCallback(USHORT_T fromID,USHORT_T toID,EINSS7INSTANCE_T toInstanceID);
}
/*********************************************************************/
/* MGMT                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_OamApiHandleBindConf(EINSS7OAMAPI_BINDCONF_T *info_sp);
USHORT_T EINSS7_OamApiHandleErrorInd(EINSS7OAMAPI_ERRORIND_T*  msg_sp);
USHORT_T EINSS7_OamApiHandleModuleInfoInd(EINSS7OAMAPI_MODULEINFOIND_T*  msg_sp);
USHORT_T EINSS7_OamApiHandleSwitchStartInd(EINSS7OAMAPI_SWITCHSTARTIND_T*  msg_sp);
USHORT_T EINSS7_OamApiHandleSwitchEndInd(EINSS7OAMAPI_SWITCHENDIND_T*  msg_sp);
USHORT_T EINSS7_OamApiHandleProcessInfoInd(EINSS7OAMAPI_PROCESSINFOIND_T*  msg_sp);
USHORT_T EINSS7_OamApiHandleOrderConf(EINSS7OAMAPI_ORDERCONF_T*  msg_sp);
USHORT_T EINSS7_OamApiHandleStatusConf(EINSS7OAMAPI_STATUSCONF_T*  msg_sp);
}
