#ifndef __SMSC_MTSMS_CLBKS_HPP__
#define __SMSC_MTSMS_CLBKS_HPP__
#include "ss7cp.h"
extern "C" {
#include "i96sccpapi.h"
}
#include "mgmtapi.h"

/*********************************************************************/
/* TCAP                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_I96SccpNoticeInd(UCHAR_T, UCHAR_T, UCHAR_T,UCHAR_T, UCHAR_T, UCHAR_T *,UCHAR_T, UCHAR_T *, USHORT_T,UCHAR_T *);
USHORT_T EINSS7_I96SccpConnectInd(UCHAR_T, ULONG_T, UCHAR_T,UCHAR_T, UCHAR_T *, UCHAR_T,UCHAR_T *, USHORT_T,UCHAR_T *);
USHORT_T EINSS7_I96SccpConnectConf(UCHAR_T,ULONG_T,UCHAR_T,UCHAR_T,UCHAR_T*,USHORT_T,UCHAR_T*);
USHORT_T EINSS7_I96SccpDataInd(UCHAR_T,ULONG_T,UCHAR_T,USHORT_T,UCHAR_T *);
USHORT_T EINSS7_I96SccpDiscInd(UCHAR_T,ULONG_T,UCHAR_T,UCHAR_T,UCHAR_T,UCHAR_T*,USHORT_T,UCHAR_T*);
}
/*********************************************************************/
/* MGMT                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,UCHAR_T result,UCHAR_T mmState,UCHAR_T xmRevision);
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,UCHAR_T orderId,UCHAR_T orderResult,UCHAR_T resultInfo,UCHAR_T lengthOfInfo,UCHAR_T *orderInfo_p);
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
USHORT_T EINSS7_I97IsupResourceInd(USHORT_T resourceGroup,UCHAR_T rgstate);
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,UCHAR_T orderId,UCHAR_T orderResult,UCHAR_T resultInfo,UCHAR_T lengthOfInfo,UCHAR_T *orderInfo_p);
}
#endif
