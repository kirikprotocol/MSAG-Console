//#include "util.hpp"
#include "clbks.hpp"
#include "logger/Logger.h"


using smsc::logger::Logger;
namespace smsc{namespace mtsmsme{namespace processor{
extern Logger* MtSmsProcessorLogger;
}}}

using smsc::mtsmsme::processor::MtSmsProcessorLogger;

USHORT_T EINSS7_I96SccpNoticeInd(UCHAR_T subSystemNumber,
                                 UCHAR_T segmentationInd,
                                 UCHAR_T errorLocation,
                                 UCHAR_T reasonForReturn,
                                 UCHAR_T calledAddressLength,
                                 UCHAR_T *calledAddress_p,
                                 UCHAR_T callingAddressLength,
                                 UCHAR_T *callingAddress_p,
                                 USHORT_T userDataLength,
                                 UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpNoticeInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}

USHORT_T EINSS7_I96SccpConnectInd(UCHAR_T subSystemNumber,
                                  ULONG_T connectionId,
                                  UCHAR_T messPriImportance,
                                  UCHAR_T calledAddressLength,
                                  UCHAR_T *calledAddress_p,
                                  UCHAR_T callingAddressLength,
                                  UCHAR_T *callingAddress_p,
                                  USHORT_T userDataLength,
                                  UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpConnectInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}

USHORT_T EINSS7_I96SccpConnectConf(UCHAR_T subSystemNumber,
                                   ULONG_T connectionId,
                                   UCHAR_T messPriImportance,
                                   UCHAR_T respondAddressLength,
                                   UCHAR_T *respondAddress_p,
                                   USHORT_T userDataLength,
                                   UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpConnectConf");
  return EINSS7_I96SCCP_REQUEST_OK;
}
USHORT_T EINSS7_I96SccpDataInd(UCHAR_T subSystemNumber,
                               ULONG_T connectionId,
                               UCHAR_T messPriImportance,
                               USHORT_T userDataLength,
                               UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpDataInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}
USHORT_T EINSS7_I96SccpDiscInd(UCHAR_T subSystemNumber,
                               ULONG_T connectionId,
                               UCHAR_T reason,
                               UCHAR_T originator,
                               UCHAR_T respondAddressLength,
                               UCHAR_T *respondAddress_p,
                               USHORT_T userDataLength,
                               UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpDiscInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}


/*********************************************************************/
/* MGMT EMPTY CALLBACKS                                              */
/*-------------------------------------------------------------------*/
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
USHORT_T EINSS7_MgmtApiHandleInitConf(USHORT_T moduleId,
                                      UCHAR_T result,
                                      ULONG_T offset)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleInitConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleStartConf(USHORT_T moduleId,
                                       UCHAR_T result)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleStartConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleStopConf(USHORT_T moduleId,
                                      UCHAR_T result)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleStopConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleAlarmConf(USHORT_T moduleId,
                                       UCHAR_T noOfAlarms,
                                       ALARMS_T *alarms_sp)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleAlarmConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleXStatConf(USHORT_T moduleId,
                                       USHORT_T xStatId,
                                       UCHAR_T statResult,
                                       UCHAR_T lastInd,
                                       USHORT_T totLength,
                                       XSTAT_T *xStat_sp)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleXStatConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleStatisticsConf(USHORT_T moduleId,
                                            UCHAR_T noOfStat,
                                            STAT_T *stat_sp)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleStatisticsConf");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleAlarmInd(USHORT_T moduleId,
                                      UCHAR_T alarmId,
                                      UCHAR_T alarmStatusLength,
                                      UCHAR_T *alarmStatus_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleAlarmInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleErrorInd(UCHAR_T length,
                                      USHORT_T errorCode,
                                      UCHAR_T state,
                                      UCHAR_T event)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleErrorInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiIndError(USHORT_T errorCode,
                                MSG_T *msg_sp)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiIndError");
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
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiHandleSysInfoInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiSysInfoInd(USHORT_T errorCode,MSG_T *msg_sp)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_MgmtApiSysInfoInd");
  return EINSS7_MGMTAPI_RETURN_OK;
}
