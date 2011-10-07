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
/* ProceedInd                                                        */
/* ProgressInd                                                       */
/* ContinuityConf                                                    */
/* ContinuityInd                                                     */
/* SuspendInd                                                        */
/* ResumeInd                                                         */
/* MiscInd                                                           */
/* ReportInd                                                         */
/* CongInd                                                           */
/* CircuitReservationConf                                            */
/* CircuitCancellationConf                                           */
/* CircuitCancellationInd                                            */
/*********************************************************************/
USHORT_T EINSS7_I97IsupSetupConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                 EINSS7INSTANCE_T isupInstanceId,
                                 EINSS7_I97_BACKWARD_T* backward_sp,
                                 EINSS7_I97_OPTBACKWARD_T* optBackward_sp,
                                 EINSS7_I97_USERINFORMATION_T* userInformation_sp,
                                 EINSS7_I97_CONNECTEDNUMB_T* connectedNumb_sp,
                                 EINSS7_I97_ACCESS_T* access_sp,
                                 EINSS7_I97_OPTPARAMS_T* extraOpts_sp)
{
  smsc_log_warn( missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupSetupConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupProceedInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                  EINSS7INSTANCE_T isupInstanceId,
                                  EINSS7_I97_BACKWARD_T* backward_sp,
                                  EINSS7_I97_OPTBACKWARD_T* optBackward_sp,
                                  EINSS7_I97_OPTPARAMS_T* extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupProceedInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupProgressInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                   EINSS7INSTANCE_T isupInstanceId,
                                   EINSS7_I97_EVENT_T* event_sp,
                                   EINSS7_I97_BACKWARD_T* backward_sp,
                                   EINSS7_I97_OPTBACKWARD_T* optBackward_sp,
                                   EINSS7_I97_OPTPARAMS_T* extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupProgressInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupContinuityConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                      EINSS7INSTANCE_T isupInstanceId,
                                      UCHAR_T continuityEvent)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupContinuityConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupContinuityInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                     EINSS7INSTANCE_T isupInstanceId,
                                     UCHAR_T responseInd,
                                     EINSS7_I97_CONREQSTATIND_T* conReqAndStatInd_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupContinuityInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupSuspendInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                  EINSS7INSTANCE_T isupInstanceId,
                                  UCHAR_T* suspendResume_p,
                                  EINSS7_I97_OPTPARAMS_T* extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupSuspendInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupResumeInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                 EINSS7INSTANCE_T isupInstanceId,
                                 UCHAR_T* suspendResume_p,
                                 EINSS7_I97_OPTPARAMS_T* extraOpts_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupResumeInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupMiscInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                               EINSS7INSTANCE_T isupInstanceId,
                               UCHAR_T pamFlag,
                               UCHAR_T miscMsgCode,
                               EINSS7_I97_OPTPARAMS_T* miscParams_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupMiscInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupReportInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                 EINSS7INSTANCE_T isupInstanceId,
                                 EINSS7_I97_REPORTINFO_T* reportInfo_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupReportInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCongInd(USHORT_T resourceGroup,
                               EINSS7INSTANCE_T isupInstanceId,
                               EINSS7_I97_SPC_T dpc,
                               UCHAR_T congLevel)
{
  smsc_log_warn(missedCallProcessorLogger, 
                "IsupCongInd in RG=%d to DPC=%d new CongLevel=%s",
		resourceGroup,
		dpc,
		getCongLevelDescription(congLevel));
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCircuitReservationConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                              EINSS7INSTANCE_T isupInstanceId,
                                              UCHAR_T result)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupCircuitReservationConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCircuitCancellationConf(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                               EINSS7INSTANCE_T isupInstanceId,
                                               UCHAR_T result)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupCircuitCancellationConf");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupCircuitCancellationInd(EINSS7_I97_ISUPHEAD_T* isupHead_sp,
                                              EINSS7INSTANCE_T isupInstanceId,
                                              UCHAR_T cause)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_I97IsupCircuitCancellationInd");
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupIndError(USHORT_T errorCode,
                                MSG_T*msg_sp,
                                EINSS7INSTANCE_T isupInstanceId)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiIndError");
  return EINSS7OAMAPI_RESULT_OK;
}
/*********************************************************************/
/* OAM EMPTY CALLBACKS                                              */
/*-------------------------------------------------------------------*/
/* ErrorInd                                                          */
/* ProcessInfoInd                                                    */
/* SwitchEndInd                                                      */
/* SwitchStartInd                                                    */
/* ModuleInfoInd                                                     */
/*********************************************************************/
USHORT_T EINSS7_OamApiHandleErrorInd(EINSS7OAMAPI_ERRORIND_T*  msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_OamApiHandleErrorInd");
  return EINSS7OAMAPI_RESULT_OK;
}
USHORT_T EINSS7_OamApiHandleProcessInfoInd(EINSS7OAMAPI_PROCESSINFOIND_T*  msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_OamApiHandleProcessInfoInd");
  return EINSS7OAMAPI_RESULT_OK;
}
USHORT_T EINSS7_OamApiHandleSwitchEndInd(EINSS7OAMAPI_SWITCHENDIND_T*  msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_OamApiHandleSwitchEndInd");
  return EINSS7OAMAPI_RESULT_OK;
}
USHORT_T EINSS7_OamApiHandleSwitchStartInd(EINSS7OAMAPI_SWITCHSTARTIND_T*  msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_OamApiHandleSwitchStartInd");
  return EINSS7OAMAPI_RESULT_OK;
}
USHORT_T EINSS7_OamApiHandleModuleInfoInd(EINSS7OAMAPI_MODULEINFOIND_T*  msg_sp)
{
  smsc_log_warn(missedCallProcessorLogger, "EMPTY CALLBACK: EINSS7_OamApiHandleModuleInfoInd");
  return EINSS7OAMAPI_RESULT_OK;
}
