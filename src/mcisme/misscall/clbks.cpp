#include "util.hpp"
#include "clbks.hpp"
#include "logger/Logger.h"


using smsc::misscall::util::getCongLevelDescription;
using smsc::logger::Logger;
namespace smsc{
namespace misscall {
extern Logger* missedCallProcessorLogger;
}//namespace misscall
}//namespace smsc
using smsc::misscall::missedCallProcessorLogger;
/*********************************************************************/
/* ISUP EMPTY CALLBACKS                                              */
/*-------------------------------------------------------------------*/
/* SetupConf                                                         */
/* ReleaseInd                                                        */
/* ProceedInd                                                        */
/* ProgressInd                                                       */
/* ContinuityConf                                                    */
/* ContinuityInd                                                     */
/* SuspendInd                                                        */
/* ResumeInd                                                         */
/* MiscInd                                                           */
/* ResourceInd                                                       */
/* ReportInd                                                         */
/* CongInd                                                           */
/* CircuitReservationConf                                            */
/* CircuitCancellationConf                                           */
/* CircuitCancellationInd                                            */
/*********************************************************************/
USHORT_T EINSS7_I97IsupSetupConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                 EINSS7_I97_BACKWARD_T *backward_sp,
                                 EINSS7_I97_OPTBACKWARD_T *optBackward_sp,
                                 EINSS7_I97_USERINFORMATION_T *userInformation_sp,
                                 EINSS7_I97_CONNECTEDNUMB_T *connectedNumb_sp,
                                 EINSS7_I97_ACCESS_T *access_sp,
                                 EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_warn( missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupSetupConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupReleaseInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                  UCHAR_T responseInd,
                                  UCHAR_T sourceInd,
                                  EINSS7_I97_CAUSE_T *cause_sp,
                                  UCHAR_T *autoCongestLevel_p,
                                  EINSS7_I97_REDIRECTIONNUMB_T *redirectionNumb_sp,
                                  EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
                                  EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_warn(Logger::getInstance("smsc.misscall"), "EMPTY CALLBACK: EINSS7_I97IsupReleaseInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupProceedInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                  EINSS7_I97_BACKWARD_T *backward_sp,
                                  EINSS7_I97_OPTBACKWARD_T *optBackward_sp,
                                  EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupProceedInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupProgressInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                   EINSS7_I97_EVENT_T *event_sp,
                                   EINSS7_I97_BACKWARD_T *backward_sp,
                                   EINSS7_I97_OPTBACKWARD_T *optBackward_sp,
                                   EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupProgressInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupContinuityConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                      UCHAR_T continuityEvent)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupContinuityConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupContinuityInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                     UCHAR_T responseInd,
                                     EINSS7_I97_CONREQSTATIND_T *conReqAndStatInd_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupContinuityInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupSuspendInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                  UCHAR_T *suspendResume_p,
                                  EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupSuspendInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupResumeInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                 UCHAR_T *suspendResume_p,
                                 EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupResumeInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupMiscInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                               UCHAR_T pamFlag,
                               UCHAR_T miscMsgCode,
                               EINSS7_I97_OPTPARAMS_T *miscParams_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupMiscInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupResourceInd(USHORT_T resourceGroup,
                                   UCHAR_T state)
{
  smsc_log_warn(missedCallProcessorLogger, "ResourceInd RG=%d is %s",resourceGroup,state?"available":"unavailable");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupReportInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                 EINSS7_I97_REPORTINFO_T *reportInfo_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupReportInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCongInd(USHORT_T resourceGroup,
                               EINSS7_I97_SPC_T dpc,
                               UCHAR_T congLevel)
{
  smsc_log_warn(missedCallProcessorLogger, 
                "EMPTY CALLBACK: EINSS7_I97IsupCongInd in RG=%d to DPC=%ld new CongLevel=%s",
		resourceGroup,
		dpc,
		getCongLevelDescription(congLevel));
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCircuitReservationConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                              UCHAR_T result)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupCircuitReservationConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCircuitCancellationConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                               UCHAR_T result)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupCircuitCancellationConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCircuitCancellationInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                              UCHAR_T cause)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupCircuitCancellationInd");
  return EINSS7_I97_REQUEST_OK;
}
/*********************************************************************/
/* MGMT EMPTY CALLBACKS                                              */
/*-------------------------------------------------------------------*/
/* MgmtConf                                                          */
/* InitConf                                                          */
/* StartConf                                                         */
/* StopConf                                                          */
/* AlarmConf                                                         */
/* XStatConf                                                         */
/* StatisticsConf                                                    */
/* AlarmInd                                                          */
/* ErrorInd                                                          */
/* IndError                                                          */
/* SysInfoInd                                                        */
/* SysInfoInd                                                        */
/*********************************************************************/
USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,
                                      USHORT_T length,
                                      UCHAR_T *data_p)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleMgmtConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleInitConf(USHORT_T moduleId,
                                      UCHAR_T result,
                                      ULONG_T offset)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleInitConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleStartConf(USHORT_T moduleId,
                                       UCHAR_T result)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleStartConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleStopConf(USHORT_T moduleId,
                                      UCHAR_T result)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleStopConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleAlarmConf(USHORT_T moduleId,
                                       UCHAR_T noOfAlarms,
                                       ALARMS_T *alarms_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleAlarmConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleXStatConf(USHORT_T moduleId,
                                       USHORT_T xStatId,
                                       UCHAR_T statResult,
                                       UCHAR_T lastInd,
                                       USHORT_T totLength,
                                       XSTAT_T *xStat_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleXStatConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleStatisticsConf(USHORT_T moduleId,
                                            UCHAR_T noOfStat,
                                            STAT_T *stat_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleStatisticsConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleAlarmInd(USHORT_T moduleId,
                                      UCHAR_T alarmId,
                                      UCHAR_T alarmStatusLength,
                                      UCHAR_T *alarmStatus_p)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleAlarmInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleErrorInd(UCHAR_T length,
                                      USHORT_T errorCode,
                                      UCHAR_T state,
                                      UCHAR_T event)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleErrorInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiIndError(USHORT_T errorCode,
                                MSG_T *msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiIndError");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleSysInfoInd(USHORT_T moduleId,
                                        SHORT_T fileNameLength,
                                        CHAR_T* fileName_p,
                                        USHORT_T line,
                                        UCHAR_T lengthOfInfo,
                                        LONG_T curState,
                                        LONG_T curEvent,
                                        LONG_T p1,
                                        LONG_T p2,
                                        LONG_T errorCode)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleSysInfoInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiSysInfoInd(USHORT_T errorCode,MSG_T *msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiSysInfoInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
