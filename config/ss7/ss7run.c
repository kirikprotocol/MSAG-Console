#include "ss7osdpn.h"
#include "ss7tmc.h"
#include "portss7.h"
#include "ss7msg.h"
#include "mgmtapi.h"


#define USERID XMGMT_ID
#define QUEUE_SIZE 10000

char* getResultDescription(UCHAR_T result);
char * getStackStatusDescription(UCHAR_T mmState);
char * getTypeOfServiceDescription(UCHAR_T typeOfService);
UCHAR_T *hexDump( UCHAR_T* src, USHORT_T len );
void deinitcp();
int ss7run(char* cfg, USHORT_T modnum, char* argv[]);

typedef struct modescr {
  char* name;
  USHORT_T id;
} modesc;

modesc moduledesc[] = {{"TCAP",   4    },
                       {"SCCP",   5    },
                       {"MTPL3",  6    },
                       {"MTPL2",  13   },
                       {"ETSIMAP",107  },
                       {"",       65535}};

enum {UNBOUND,BOUND,SIDLE,SINIT,SRUN,LINKRUN} status;

void usage() {
  printf("Usage:   ss7run cfgname modname [modname...]\n");
  printf("Example: ss7run ss7.cnf ETSIMAP TCAP SCCP MTPL3 MTPL2\n");
}

int verify(int argc, char* argv[]) {

  USHORT_T res = 0;
  USHORT_T i,j,id;

  if (argc < 3) {
    usage();
    return -1;
  }

  for(i=0; i< argc - 2; i++) {
    for(j = 0; j < sizeof(moduledesc)/sizeof(modesc); j++) {
      id = moduledesc[j].id;
      if (strcasecmp(argv[i+2],moduledesc[j].name) == 0) break;
    }
    if (id == 65535) {
      fprintf(stderr,"FAILED: unsupported module %s\n",argv[i+2]);
      res = -1;
      break;
    }
  }

  return res;
}
int main(int argc, char* argv[]) {

  USHORT_T res = 0;

  if ((res = verify(argc,argv)) != 0) {
    return res;
  }

  if ((res = initcp()) != 0) {
    deinitcp();
    return res;
  }

  if ((res = ss7run(argv[1],argc-2,&argv[2])) != 0) {
    deinitcp();
    return res;
  }
  return res;
}

int initcp() {
  USHORT_T res;
  if ( (res = EINSS7CpMsgInitNoSig(QUEUE_SIZE)) != MSG_INIT_OK) {
    fprintf(stderr, "FAILED: initializing message handling. code %hu \n", res);
    return res;
  }

  if ( (res = MsgOpen(USERID)) != MSG_OPEN_OK) {
    fprintf(stderr, "FAILED: opening message port for user %d. code %hu \n", USERID, res);
    return res;
  }

  if( (res = MsgConn( USERID, MGMT_ID)) != RETURN_OK) {
    fprintf(stderr, "FAILED: connecting to user %d. code %hu\n", MGMT_ID, res);
    return res;
  }
  return 0;
}

void deinitcp() {
  EINSS7_MgmtApiSendUnbindReq(USERID,MGMT_ID);
  MsgRel(USERID, MGMT_ID);
  MsgClose(USERID);
  MsgExit();
}


int ss7run(char* cfg, USHORT_T modnum, char* argv[]) {

  USHORT_T res;
  USHORT_T i,j,id;
  UCHAR_T* modids;
  /* Присоединяемся к модулю управления*/

  res = EINSS7_MgmtApiSendBindReq(USERID,MGMT_ID,6,WAIT);
  if (res != EINSS7_MGMTAPI_RETURN_OK) {
    fprintf(stderr, "FAILED: binding to user %d. code %hu\n", MGMT_ID, res);
    return res;
  }

  if (status != BOUND) {
    fprintf(stderr, "FAILED: binding to user %d.\n", MGMT_ID);
    return -1;
  }

  /* Запрашиваем текущее состояние стэка */

  res = EINSS7_MgmtApiSendMgmtReq(USERID,
                                  MGMT_ID,
                                  9 /*Stack State Query (SSQ)*/,
                                  0, 
                                  NULL,
                                  WAIT);

  if (res != EINSS7_MGMTAPI_RETURN_OK) {
    fprintf(stderr, "FAILED: ss7 stack state query. code %hu\n", res);
    return res ;
  }

  if (status != SIDLE) {
    fprintf(stderr, "FAILED: ss7 stack isn't IDLE.\n");
    return -1 ;
  }

  /* Проводим инициализацию модулей */

  modids = malloc(modnum*2);
  for(i=0; i< modnum; i++) {
    for(j = 0; j < sizeof(moduledesc)/sizeof(modesc); j++) {
      id = moduledesc[j].id;
      if (strcasecmp(argv[i],moduledesc[j].name) == 0) break;
    }
    modids[i*2] = (UCHAR_T)(id & 0xFF);
    modids[i*2+1] = (UCHAR_T)(id >>8);
  }

  res = EINSS7_MgmtApiSendInitReq(USERID, /*senderID*/
                                  MGMT_ID, /*receiverID*/
                                  strlen(cfg)+1, /*lengthOfFilename,*/
                                  cfg, /* fileName_p*/
                                  modnum, /*noOfModules*/
                                  modids,
                                  WAIT);
  free(modids);
  if (res != EINSS7_MGMTAPI_RETURN_OK) {
    fprintf(stderr, "FAILED: ss7 stack init. code %hu\n", res);
    return res ;
  }

  if (status != SINIT) {
    fprintf(stderr, "FAILED: ss7 stack init.\n");
    return -1 ;
  }

  res = EINSS7_MgmtApiSendStartReq(USERID, /*senderID*/
                                   MGMT_ID, /*receiverID*/
                                   WAIT);

  if (res != EINSS7_MGMTAPI_RETURN_OK) {
    fprintf(stderr, "FAILED: ss7 stack start. code %hu\n", res);
    return res ;
  }

  if (status != SRUN) {
    fprintf(stderr, "FAILED: ss7 stack start.\n");
    return -1 ;
  }
  res = EINSS7_MgmtApiSendOrderReq(USERID, /*senderID*/
                                   MGMT_ID, /*receiverID*/
                                   6, /*moduleID = MTPL3*/
                                   11, /*orderID = start all links*/
                                   0, /*lengthOfInfo*/
                                   NULL, /* orderInfo_p*/
                                   WAIT);

  if (res != EINSS7_MGMTAPI_RETURN_OK) {
    fprintf(stderr, "FAILED: start all links. code %hu\n", res);
    return res ;
  }

  if (status != LINKRUN) {
    fprintf(stderr, "FAILED: start all links.\n");
    return -1 ;
  }
}

/*********************************************************************/
/*                                                                   */
/*                       C A L L B A C K S                           */
/*                                                                   */
/*********************************************************************/

USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,
                                      UCHAR_T result,
                                      UCHAR_T mmState,
                                      UCHAR_T xmRevision)
{
  if (status == UNBOUND && result == 0) status = BOUND;
/*
  printf("STATUS=%s STACK=%s MGMT VERSION=%hu\n",
          getResultDescription(result),
          getStackStatusDescription(mmState),
          xmRevision);
*/
  return MSG_OK;
}

USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,
                                      USHORT_T length,
                                      UCHAR_T *data_p)
{
/*
  UCHAR_T* buf = hexDump(data_p,length);
  printf("LEN = %d DATA={%s}\n",length,buf);
  free(buf);
  switch(typeOfService) {
    case 9:  printf("TYPE=%s STATUS=%s\n",
                     getTypeOfServiceDescription(typeOfService),
                     getStackStatusDescription(*data_p));
             break;
    default: printf("UNKNOWN typeOfService\n");
  };
*/
  if (status == BOUND && typeOfService == 9 && *data_p == 0) status = SIDLE;
  return MSG_OK;
}
                                             
USHORT_T EINSS7_MgmtApiHandleInitConf(USHORT_T moduleId,
                                      UCHAR_T result,
                                      ULONG_T offset)
{
/*
  printf("INIT %s MODULE=%d OFFSET=%d\n",
          getResultDescription(result),
          moduleId,
          offset);
*/
  if (status == SIDLE && result == 0) {
    status = SINIT;
  }
  else {
    fprintf(stderr, "FAILED: ss7 stack init for module %d file offset %d\n", moduleId,offset);
  }
  return MSG_OK;
}

USHORT_T EINSS7_MgmtApiHandleStartConf(USHORT_T moduleId,
                                       UCHAR_T result)
{
/*
  printf("START %s MODULE=%d\n",
          getResultDescription(result),
          moduleId);
*/
  if (status == SINIT && result == 0) {
    status = SRUN;
  }
  else {
    fprintf(stderr, "FAILED: ss7 stack start for module %d\n", moduleId);
  }
  return MSG_OK;
}


USHORT_T EINSS7_MgmtApiHandleStopConf(USHORT_T moduleId,
                                      UCHAR_T result)
{
  printf("EINSS7_MgmtApiHandleStopConf\n");
}


USHORT_T EINSS7_MgmtApiHandleAlarmConf(USHORT_T moduleId,
                                       UCHAR_T noOfAlarms,
                                       ALARMS_T *alarms_sp)
{
  printf("EINSS7_MgmtApiHandleAlarmConf\n");
}

                                              
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,
                                       UCHAR_T orderId,
                                       UCHAR_T orderResult,
                                       UCHAR_T resultInfo,
                                       UCHAR_T lengthOfInfo,
                                       UCHAR_T *orderInfo_p)
{
/*
  UCHAR_T* buf = hexDump(orderInfo_p,lengthOfInfo);
  printf("ORDER module=%d orderid=%d result=%s resinfo=%d len=%d data:%s\n",
          moduleId,
          orderId,
          getResultDescription(orderResult),
          resultInfo,
          lengthOfInfo,
          buf);
  free(buf);
*/
  if (status == SRUN && 
      orderId == 11  && 
      orderResult == 0) status = LINKRUN;
  return MSG_OK;
}

                                              
USHORT_T EINSS7_MgmtApiHandleXStatConf(USHORT_T moduleId,
                                       USHORT_T xStatId,
                                       UCHAR_T statResult,
                                       UCHAR_T lastInd,
                                       USHORT_T totLength,
                                       XSTAT_T *xStat_sp)
{
  printf("EINSS7_MgmtApiHandleXStatConf\n");
}


USHORT_T EINSS7_MgmtApiHandleStatisticsConf(USHORT_T moduleId,
                                            UCHAR_T noOfStat,
                                            STAT_T *stat_sp)
{
  printf("EINSS7_MgmtApiHandleStatisticsConf\n");
}


USHORT_T EINSS7_MgmtApiHandleAlarmInd(USHORT_T moduleId,
                                      UCHAR_T alarmId,
                                      UCHAR_T alarmStatusLength,
                                      UCHAR_T *alarmStatus_p)
{
  printf("EINSS7_MgmtApiHandleAlarmInd\n");
}


USHORT_T EINSS7_MgmtApiHandleErrorInd(UCHAR_T length,
                                      USHORT_T errorCode,
                                      UCHAR_T state,
                                      UCHAR_T event)
{
  printf("EINSS7_MgmtApiHandleErrorInd\n");
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
  printf("EINSS7_MgmtApiHandleSysInfoInd\n");
}

USHORT_T EINSS7_MgmtApiIndError(USHORT_T errorCode,
                                MSG_T *msg_sp)
{
  printf("EINSS7_MgmtApiIndError\n");
}



USHORT_T EINSS7_MgmtApiSysInfoInd(USHORT_T errorCode,
                                  MSG_T *msg_sp)
{
  printf("EINSS7_MgmtApiSysInfoInd\n");
}

/*********************************************************************/
/*                                                                   */
/*                            U T I L I T Y                          */
/*                                                                   */
/*********************************************************************/

char* getResultDescription(UCHAR_T result) {
  switch(result) {
    case 0: return "Success";
    case 1: return "File not found";
    case 2: return "File could not be opened";
    case 3: return "Order could not be performed";
    case 4: return "Bad configuration";
    case 5: return "Statistics ID not implemented";
    case 6: return "Order ID not implemented";
    case 7: return "Alarm ID not implemented";
    case 8: return "Format error";
    case 9: return "Resources Unavailable";
    case 10: return "SSN has not made a bind";
    case 11: return "Illegal state";
    case 12: return "MM user is already bound";
  }
}
char * getStackStatusDescription(UCHAR_T mmState) {
  switch (mmState) {
    case 0: return "IDLE";
    case 1: return "INITIALIZING";
    case 2: return "INITIALIZED";
    case 3: return "STARTING";
    case 4: return "RUNNING";
    case 5: return "TERMINATING";
    case 6: return "TERMINATED";
  }
}
char * getTypeOfServiceDescription(UCHAR_T typeOfService) {
  switch(typeOfService) {
    case 0:  return "End Of Service Request (EOSR)";
    case 1:  return "SysLog Info Service (SLIS)";
    case 2:  return "Reserved for future use";
    case 3:  return "Service Request Failed (SRF)";
    case 4:  return "Reserved for future use";
    case 5:  return "Reserved for future use";
    case 6:  return "Configuration Query (CONQ)";
    case 7:  return "Configuration Index Query (CIQ)";
    case 8:  return "System Trace Query (SYSTRCQ)";
    case 9:  return "Stack State Query (SSQ)";
    case 10: return "Set Config Info Query (SCIQ)";
    case 11: return "Get Config Info Query (GCIQ)";
    case 12: return "Read Binary Data (RBD)";
    case 13: return "Write Binary Data (WBD)";
  }
}
UCHAR_T *hexDump( UCHAR_T* src, USHORT_T len ) {
  int i;
  UCHAR_T *buf = (UCHAR_T*)malloc( len*3+1 );
  for( i = 0; i < len; i++ ) {
    sprintf( buf+(i*3), "%02x ", src[i] );
  }
  buf[len*3]=0;
  return buf;
}

