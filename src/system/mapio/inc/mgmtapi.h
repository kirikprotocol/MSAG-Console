/************************************************************************/
/*                                                                      */
/* Don't remove these two lines, contain depends on them!               */
/* Document Number: %Container% Revision: %Revision%                    */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* mgmtapi.h,v                                                          */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* COPYRIGHT Ericsson Infotech                                          */
/*                                                                      */
/* All rights reserved. The copyright of the computer program herein    */
/* is the property of Ericsson Infotech AB, Sweden. The                 */
/* program may be used and/or copied only with the permission of        */
/* Ericsson Infotech AB or in accordance with the                       */
/* terms and conditions stipulated in the agreement/contract under      */
/* which the program has been supplied.                                 */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* Document Number: 5/190 55-CAA 201 30 Ux         Rev: A               */
/*                                                                      */
/*                                                                      */
/* Revision:                                                            */
/*  @EINVER: $RCSfile $ $Revision $ $Date $ $State $                    */
/*                                                                      */
/* Programmer: Êsa Nilsson                                              */
/*                                                                      */
/* Purpose:                                                             */
/* Management Application Program Interface.                            */
/* The purpose of this API is to give the user a C interface to the     */
/* SS7 stack. It is specified in Functional Specification for           */
/* Management API R4A, 155 19-CAA 201 30 Uen.                           */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* Revision record: Initial version.                                    */
/*                                                                      */
/* Notes:                                                               */
/* A  1998-07-03   Updated after inspection.                            */
/*                 6/170 17-CAA 201 30 Ux                               */
/*                                                                      */
/* B  1999-11-30   EINTOAS Updates due to TR1782, include of            */
/*                 ss7log.h                                             */
/*             Comment for                                              */
/*             "#endif of EINSS7_FUNC_POINTER"                          */
/*             was corrected                                            */
/*                                                                      */
/* C   2000-01-18   EINTOAS Changed document number from,               */
/*                4/190 55-CAA 201 30  to                               */
/*                12/190 55-CAA 201 30                                  */
/*                                                                      */
/* D   2000-09-26   EINASGU Making API thread safe. Removed parameter   */
/*                  msg_sp from functions sending a request. Adding     */
/*                  functionality for a new primitive XM_SYSINFO_ind.   */
/*                                                                      */
/* E   2000-11-29   EINASGU added typedef first for enum                */
/*                  SYSLOG_LENGTH_TAG.                                  */
/*                                                                      */
/* F   2001-01-25   EINASGU Added a new result code,                    */
/*                  EINSS7_MGMTAPI_MESSAGE_EMPTY                        */
/*                                                                      */
/* G   2001-03-05   EINHEMO                                             */
/*                                                                      */
/************************************************************************/

/* C++ compability definitions */
#ifndef __MGMTAPI_H__
#define __MGMTAPI_H__
#if defined (__cplusplus) || (c_plusplus)
extern "C" {
#endif 

/*********************************************************************/
/*                                                                   */
/*                       I N C L U D E S                             */
/*                                                                   */
/*********************************************************************/

#include "ss7osdpn.h"
#include "ss7tmc.h"
#include "portss7.h"
#include "ss7msg.h"
#include "ss7log.h"

/*********************************************************************/
/*                                                                   */
/*                       C O N S T A N T S                           */
/*                                                                   */
/*********************************************************************/

#define MAX_PK_BUF_SIZE 300
#define NO_OF_ALARMS 100
#define TIMEOUT 2000 
#define MAX_NUMBER_OF_ALARMS 50

/*********************************************************************/
/*                                                                   */
/*                       T Y P E D E F S                             */
/*                                                                   */
/*********************************************************************/
typedef enum SYNC_T{NO_WAIT = 0, WAIT } SYNC_T;

typedef struct ALARM
{
  UCHAR_T alarmId;
  UCHAR_T alarmInfoLength;
  UCHAR_T *alarmInfo_p;
}ALARM_T;

typedef struct ALARMS
{
  UCHAR_T alarmId;
  UCHAR_T alarmResult;
}ALARMS_T;

typedef struct XSTAT
{
  UCHAR_T lengthOfInfo;
  UCHAR_T *xStatInfo_p;
  USHORT_T statParamLength;
  UCHAR_T *statParam_p;
}XSTAT_T;

typedef struct STAT
{
  UCHAR_T statId;
  UCHAR_T statResult;
  UCHAR_T lengthOfInfo;
  UCHAR_T *statInfo_p;
} STAT_T;

typedef enum SYSLOG_LENGTH_TAG
{
    SYSLOG = 12,
    EINSS7CP_SYSLOG = 20
}   SYSLOG_LENGTH_T;

/*********************************************************************/
/*                                                                   */
/*           R E Q U E S T   R E T U R N   C O D E S                 */
/*                                                                   */
/*********************************************************************/

#define EINSS7_MGMTAPI_RETURN_OK 0
#define EINSS7_MGMTAPI_REQUEST_OK 0

#define EINSS7_MGMTAPI_BINDREQ_FAILED 10
#define EINSS7_MGMTAPI_MGMTREQ_FAILED 11
#define EINSS7_MGMTAPI_INITREQ_FAILED 12
#define EINSS7_MGMTAPI_STARTREQ_FAILED 13
#define EINSS7_MGMTAPI_STOPREQ_FAILED 14
#define EINSS7_MGMTAPI_ALARMREQ_FAILED 15
#define EINSS7_MGMTAPI_ORDERREQ_FAILED 16
#define EINSS7_MGMTAPI_XSTATREQ_FAILED 17
#define EINSS7_MGMTAPI_STATISTICSREQ_FAILED 18

#define EINSS7_MGMTAPI_MESSAGE_EMPTY 19

/*********************************************************************/
/*                                                                   */
/* EINSS7_MgmtApiReceivedXMMsg  R E T U R N  C O D E S               */
/*                                                                   */
/*********************************************************************/

#define EINSS7_MGMTAPI_UNKNOWN_PRIMITIVE 30

/*********************************************************************/
/*                                                                   */
/*           I N D I C A T I O N   R E T U R N   C O D E S           */
/*                                                                   */
/*********************************************************************/

#define EINSS7_MGMTAPI_IND_LENGTH_ERROR 100
#define EINSS7_MGMTAPI_MEMORY_ERROR 101
#define EINSS7_MGMTAPI_SYSLOG_ERROR 102

#ifdef EINSS7_FUNC_POINTER

/*********************************************************************/
/*                                                                   */
/*           EINSS7_FUNC_POINTER   R E T U R N   C O D E S           */
/*                                                                   */
/*********************************************************************/

#define EINSS7_MGMTAPI_CALLBACK_FUNC_NOT_SET 250

#endif /* EINSS7_FUNC_POINTER */

/*********************************************************************/
/*                                                                   */
/*               R E Q U E S T    P R O T O T Y P E S                */
/*                                                                   */
/*********************************************************************/


extern USHORT_T EINSS7_MgmtApiSendBindReq(const USHORT_T senderId,
                                          const USHORT_T receiverId,
                                          const UCHAR_T xmRevision,
                                          const SYNC_T sync);

extern USHORT_T EINSS7_MgmtApiSendUnbindReq(const USHORT_T senderId,
                                            const USHORT_T receiverId);
                                
extern USHORT_T EINSS7_MgmtApiSendMgmtReq(const USHORT_T senderId,
                                          const USHORT_T receiverId,
                                          const UCHAR_T typeOfService,
                                          const USHORT_T dataLength,
                                          const UCHAR_T *data_p,
                                          const SYNC_T sync);

extern USHORT_T EINSS7_MgmtApiSendInitReq(const USHORT_T senderId,
                                          const USHORT_T receiverId, 
                                          const UCHAR_T lengthOfFilename,
                                          const UCHAR_T *fileName_p,
                                          const UCHAR_T noOfModules,
                                          const UCHAR_T *moduleId_p,
                                          const SYNC_T   sync);

extern USHORT_T EINSS7_MgmtApiSendStartReq(const USHORT_T senderId,
                                           const USHORT_T receiverId,
                                           const SYNC_T  sync);
                                           
extern USHORT_T EINSS7_MgmtApiSendStopReq(const USHORT_T senderId,
                                          const USHORT_T receiverId,
                                          const SYNC_T  sync);
                                           
extern USHORT_T EINSS7_MgmtApiSendAlarmReq(const USHORT_T senderId,
                                           const USHORT_T receiverId,
                                           const USHORT_T moduleId,
                                           const UCHAR_T noOfAlarms,
                                           const ALARM_T *alarm_sp,
                                           const SYNC_T  sync);
                                           
extern USHORT_T EINSS7_MgmtApiSendOrderReq(const USHORT_T senderId,
                                           const USHORT_T receiverId,
                                           const USHORT_T moduleId,
                                           const USHORT_T orderId,
                                           const USHORT_T lengthOfInfo,
                                           const UCHAR_T *orderInfo_p,
                                           const SYNC_T  sync);
                                           
extern USHORT_T EINSS7_MgmtApiSendXStatReq(const USHORT_T senderId,
                                           const USHORT_T receiverId,
                                           const USHORT_T moduleId,
                                           const USHORT_T xStatId,
                                           const USHORT_T resetOption,
                                           const USHORT_T lengthOfxStatParam,
                                           const UCHAR_T* xStat_sp,
                                           const SYNC_T sync);

extern USHORT_T EINSS7_MgmtApiSendStatisticsReq(const USHORT_T senderId,
                                                const USHORT_T receiverId,
                                                const USHORT_T moduleId,
                                                const UCHAR_T noOfStat,
                                                const UCHAR_T *stat_p,
                                                const SYNC_T sync);


/*********************************************************************/
/*                                                                   */
/*        C O N F I R M A T I O N   A N D  I N D I C A T I O N       */
/*                     P R O T O T Y P E S                           */
/*                                                                   */
/*********************************************************************/

extern USHORT_T EINSS7_MgmtApiReceivedXMMsg(MSG_T *msg_sp);

#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */

typedef USHORT_T (*XMBINDCONF_T)(UCHAR_T length,
                                 UCHAR_T result,
                                 UCHAR_T mmState,
                                 UCHAR_T xmRevision);

typedef USHORT_T (*XMMGMTCONF_T)(UCHAR_T typeOfService,
                                 USHORT_T length,
                                 UCHAR_T *data_p);

typedef USHORT_T (*XMINITCONF_T)(USHORT_T moduleId,
                                 UCHAR_T result,
                                 ULONG_T offset);

typedef USHORT_T (*XMSTARTCONF_T)(USHORT_T moduleId,
                                  UCHAR_T result);

typedef USHORT_T (*XMSTOPCONF_T)(USHORT_T moduleId,
                                 UCHAR_T result);

typedef USHORT_T (*XMALARMCONF_T)(USHORT_T moduleId,
                                  UCHAR_T noOfAlarms,
                                  ALARMS_T *alarms_sp);

typedef USHORT_T (*XMORDERCONF_T)(USHORT_T moduleId,
                                  UCHAR_T orderId,
                                  UCHAR_T orderResult,
                                  UCHAR_T resultInfo,
                                  UCHAR_T lengthOfInfo,
                                  UCHAR_T *orderInfo_p);

typedef USHORT_T (*XMXSTATCONF_T)(USHORT_T moduleId,
                                  USHORT_T xStatId,
                                  UCHAR_T statResult,
                                  UCHAR_T lastInd,
                                  USHORT_T totLength,
                                  XSTAT_T *xStat_sp);

typedef USHORT_T (*XMSTATISTICSCONF_T)(USHORT_T moduleId,
                                       UCHAR_T noOfStat,
                                       STAT_T *stat_sp);

typedef USHORT_T (*XMALARMIND_T)(USHORT_T moduleId,
                                 UCHAR_T alarmId,
                                 UCHAR_T alarmStatusLength,
                                 UCHAR_T *alarmStatus_p);

typedef USHORT_T (*XMERRORIND_T)(UCHAR_T length,
                                 USHORT_T errorCode,
                                 UCHAR_T state,
                                 UCHAR_T event);

typedef USHORT_T (*XMINDERROR_T)(USHORT_T errorCode,
                                 MSG_T *msg_sp);

typedef USHORT_T (*XMSYSINFOIND_T)(USHORT_T moduleId,
                                   SHORT_T fileNameLength,
                                   CHAR_T* fileName_p,
                                   USHORT_T line,
                                   UCHAR_T lengthOfInfo,
                                   LONG_T curState,
                                   LONG_T curEvent,
                                   LONG_T p1,
                                   LONG_T p2,
                                   LONG_T errorCode);

typedef struct MGMTAPIFUNCS
{
    XMBINDCONF_T        EINSS7_MgmtApiHandleBindConf; /* Pointer to a function */
    XMMGMTCONF_T        EINSS7_MgmtApiHandleMgmtConf;
    XMINITCONF_T        EINSS7_MgmtApiHandleInitConf;
    XMSTARTCONF_T       EINSS7_MgmtApiHandleStartConf;
    XMSTOPCONF_T        EINSS7_MgmtApiHandleStopConf;
    XMALARMCONF_T       EINSS7_MgmtApiHandleAlarmConf;
    XMORDERCONF_T       EINSS7_MgmtApiHandleOrderConf;
    XMXSTATCONF_T       EINSS7_MgmtApiHandleXStatConf;
    XMSTATISTICSCONF_T  EINSS7_MgmtApiHandleStatisticsConf;
    XMALARMIND_T        EINSS7_MgmtApiHandleAlarmInd;
    XMERRORIND_T        EINSS7_MgmtApiHandleErrorInd;
    XMINDERROR_T        EINSS7_MgmtApiIndError;
    XMSYSINFOIND_T      EINSS7_MgmtApiHandleSysInfoInd;
    XMINDERROR_T        EINSS7_MgmtApiSysInfoIndError;
}MGMTAPIFUNCS_T;

#else

extern USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,
                                             UCHAR_T result,
                                             UCHAR_T mmState,
                                             UCHAR_T xmRevision);

extern USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,
                                             USHORT_T length,
                                             UCHAR_T *data_p);
                                             
                                             
extern USHORT_T EINSS7_MgmtApiHandleInitConf(USHORT_T moduleId,
                                             UCHAR_T result,
                                             ULONG_T offset);

extern USHORT_T EINSS7_MgmtApiHandleStartConf(USHORT_T moduleId,
                                              UCHAR_T result);

extern USHORT_T EINSS7_MgmtApiHandleStopConf(USHORT_T moduleId,
                                             UCHAR_T result);

extern USHORT_T EINSS7_MgmtApiHandleAlarmConf(USHORT_T moduleId,
                                              UCHAR_T noOfAlarms,
                                              ALARMS_T *alarms_sp);
                                              
extern USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,
                                              UCHAR_T orderId,
                                              UCHAR_T orderResult,
                                              UCHAR_T resultInfo,
                                              UCHAR_T lengthOfInfo,
                                              UCHAR_T *orderInfo_p);
                                              
extern USHORT_T EINSS7_MgmtApiHandleXStatConf(USHORT_T moduleId,
                                              USHORT_T xStatId,
                                              UCHAR_T statResult,
                                              UCHAR_T lastInd,
                                              USHORT_T totLength,
                                              XSTAT_T *xStat_sp);

extern USHORT_T EINSS7_MgmtApiHandleStatisticsConf(USHORT_T moduleId,
                                                   UCHAR_T noOfStat,
                                                   STAT_T *stat_sp);

extern USHORT_T EINSS7_MgmtApiHandleAlarmInd(USHORT_T moduleId,
                                             UCHAR_T alarmId,
                                             UCHAR_T alarmStatusLength,
                                             UCHAR_T *alarmStatus_p);

extern USHORT_T EINSS7_MgmtApiHandleErrorInd(UCHAR_T length,
                                             USHORT_T errorCode,
                                             UCHAR_T state,
                                             UCHAR_T event);

extern USHORT_T EINSS7_MgmtApiIndError(USHORT_T errorCode,
                                       MSG_T *msg_sp);

extern USHORT_T EINSS7_MgmtApiHandleSysInfoInd(USHORT_T moduleId,
                                               SHORT_T fileNameLength,
                                               CHAR_T* fileName_p,
                                               USHORT_T line,
                                               UCHAR_T lengthOfInfo,
                                               LONG_T curState,
                                               LONG_T curEvent,
                                               LONG_T p1,
                                               LONG_T p2,
                                               LONG_T errorCode);

extern USHORT_T EINSS7_MgmtApiSysInfoInd(USHORT_T errorCode,
                                         MSG_T *msg_sp);

#endif


#ifdef EINSS7_FUNC_POINTER
/* The application must call this function to register the call back functions. */ 
        USHORT_T EINSS7_MgmtApiRegFunc(MGMTAPIFUNCS_T *);
#endif /* EINSS7_FUNC_POINTER */

/* C++ compability definitions */
#if defined (__cplusplus) || (c_plusplus)
}
#endif /* __cplusplus */ 
#endif /* __MGMTAPI_H__ */

/*------------------- End of header file mgmtapi.h -------------------*/


