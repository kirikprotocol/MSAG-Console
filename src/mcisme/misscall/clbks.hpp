#include "portss7.h"
#include "ss7tmc.h"
#include "i97isup_api.h"
#include "mgmtapi.h"

/*********************************************************************/
/* ISUP                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_I97IsupIndError(USHORT_T errorCode,MSG_T *message);
USHORT_T EINSS7_I97IsupBindConf(USHORT_T isupUserID,UCHAR_T result);
USHORT_T EINSS7_I97IsupSetupInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,USHORT_T resourceGroup,EINSS7_I97_NATUREOFCONN_T *natureOfConn_sp,EINSS7_I97_FORWARD_T *forward_sp,UCHAR_T *callingPartyCat_p,UCHAR_T *transmissionReq_p,EINSS7_I97_CALLEDNUMB_T *calledNumb_sp,EINSS7_I97_CALLINGNUMB_T *callingNumb_sp,EINSS7_I97_OPTFORWARD_T *optForward_sp,EINSS7_I97_ACCESS_T *access_sp,EINSS7_I97_REDIRECTINGNUMB_T *redirectingNumb_sp,EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,EINSS7_I97_ORIGINALNUMB_T *originalNumb_sp,EINSS7_I97_USERINFORMATION_T *userInformation_sp,EINSS7_I97_USERSERVICE_T *userService_sp,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupReleaseConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupSetupConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,EINSS7_I97_BACKWARD_T *backward_sp,EINSS7_I97_OPTBACKWARD_T *optBackward_sp,EINSS7_I97_USERINFORMATION_T *userInformation_sp,EINSS7_I97_CONNECTEDNUMB_T *connectedNumb_sp,EINSS7_I97_ACCESS_T *access_sp,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupReleaseInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T responseInd,UCHAR_T sourceInd,EINSS7_I97_CAUSE_T *cause_sp,UCHAR_T *autoCongestLevel_p,EINSS7_I97_REDIRECTIONNUMB_T *redirectionNumb_sp,EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupProceedInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,EINSS7_I97_BACKWARD_T *backward_sp,EINSS7_I97_OPTBACKWARD_T *optBackward_sp,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupProgressInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,EINSS7_I97_EVENT_T *event_sp,EINSS7_I97_BACKWARD_T *backward_sp,EINSS7_I97_OPTBACKWARD_T *optBackward_sp,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupContinuityConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T continuityEvent);
USHORT_T EINSS7_I97IsupContinuityInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T responseInd,EINSS7_I97_CONREQSTATIND_T *conReqAndStatInd_sp);
USHORT_T EINSS7_I97IsupSuspendInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T *suspendResume_p,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupResumeInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T *suspendResume_p,EINSS7_I97_OPTPARAMS_T *extraOpts_sp);
USHORT_T EINSS7_I97IsupMiscInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T pamFlag,UCHAR_T miscMsgCode,EINSS7_I97_OPTPARAMS_T *miscParams_sp);
USHORT_T EINSS7_I97IsupResourceInd(USHORT_T resourceGroup,UCHAR_T state);
USHORT_T EINSS7_I97IsupReportInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,EINSS7_I97_REPORTINFO_T *reportInfo_sp);
USHORT_T EINSS7_I97IsupCongInd(USHORT_T resourceGroup,EINSS7_I97_SPC_T dpc,UCHAR_T congLevel);
USHORT_T EINSS7_I97IsupCircuitReservationConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T result);
USHORT_T EINSS7_I97IsupCircuitCancellationConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T result);
USHORT_T EINSS7_I97IsupCircuitCancellationInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,UCHAR_T cause);
}
/*********************************************************************/
/* MGMT                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,UCHAR_T result,UCHAR_T mmState,UCHAR_T xmRevision);
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,UCHAR_T orderId,UCHAR_T orderResult,UCHAR_T resultInfo,UCHAR_T lengthOfInfo,UCHAR_T *orderInfo_p);
USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,USHORT_T length,UCHAR_T *data_p);
USHORT_T EINSS7_MgmtApiHandleInitConf(USHORT_T moduleId,UCHAR_T result,ULONG_T offset);
USHORT_T EINSS7_MgmtApiHandleStartConf(USHORT_T moduleId,UCHAR_T result);
USHORT_T EINSS7_MgmtApiHandleStopConf(USHORT_T moduleId,UCHAR_T result);
USHORT_T EINSS7_MgmtApiHandleAlarmConf(USHORT_T moduleId,UCHAR_T noOfAlarms,ALARMS_T *alarms_sp);
USHORT_T EINSS7_MgmtApiHandleXStatConf(USHORT_T moduleId,USHORT_T xStatId,UCHAR_T statResult,UCHAR_T lastInd,USHORT_T totLength,XSTAT_T *xStat_sp);
USHORT_T EINSS7_MgmtApiHandleStatisticsConf(USHORT_T moduleId,UCHAR_T noOfStat,STAT_T *stat_sp);
USHORT_T EINSS7_MgmtApiHandleAlarmInd(USHORT_T moduleId,UCHAR_T alarmId,UCHAR_T alarmStatusLength,UCHAR_T *alarmStatus_p);
USHORT_T EINSS7_MgmtApiHandleErrorInd(UCHAR_T length,USHORT_T errorCode,UCHAR_T state,UCHAR_T event);
USHORT_T EINSS7_MgmtApiIndError(USHORT_T errorCode,MSG_T *msg_sp);
USHORT_T EINSS7_MgmtApiHandleSysInfoInd(USHORT_T moduleId,SHORT_T fileNameLength,CHAR_T* fileName_p,USHORT_T line,UCHAR_T lengthOfInfo,LONG_T curState,LONG_T curEvent,LONG_T p1,LONG_T p2,LONG_T errorCode);
USHORT_T EINSS7_MgmtApiSysInfoInd(USHORT_T errorCode,MSG_T *msg_sp);
}
