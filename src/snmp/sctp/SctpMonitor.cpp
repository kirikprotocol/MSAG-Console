static char const ident[] = "$Id$";
#include "logger/Logger.h"
#include "snmp/sctp/SctpMonitor.hpp"
#include "snmp/sctp/util.hpp"
#include "ss7cp.h"
#include "mgmtapi.h"


namespace smsc{
namespace snmp{
namespace sctp{

using namespace smsc::core::synchronization;
using smsc::logger::Logger;

AssociationChangeListener* listener;
static SctpMonitor* volatile monitor;
static int going;
static Mutex lock;
Logger* logger = 0;

static bool needToStartAssociation = false;
static struct timeval lastassocstart;
void SctpMonitor::startAllAssociations()
{
  long period = 60000L; // in milliseconds
  struct timeval now;
  struct timeval tmp;

  tmp = lastassocstart;
  long sec = period / 1000L;
  long usec = (period % 1000L) * 1000L;
  tmp.tv_sec += sec;
  tmp.tv_usec += usec;
  if (tmp.tv_usec > 1000000L)
  {
    tmp.tv_usec -= 1000000L;
    tmp.tv_sec ++;
  }
  gettimeofday(&now, NULL);
  if (now.tv_sec > tmp.tv_sec ||
          (now.tv_sec == tmp.tv_sec && now.tv_usec >= tmp.tv_usec))
  {
    needToStartAssociation = true;
  }
}

SctpMonitor* SctpMonitor::instance()
{
  if ( monitor == 0 )
  {
    MutexGuard g(lock);
    if ( monitor == 0 )
    {
      monitor = new SctpMonitor();
    }
  }
  return monitor;
}

#define USERID SNMPAGENT_ID
#define MAXENTRIES 1000
#define MGMT_VER 6
#define MCIERROR 999

enum State{
      INIT,
      MGMTBINDING,
      MGMTBOUND,
      MGMTBINDERROR,
      WAITINGSTACK,
      STACKOK,
      SCCPBINDING,
      SCCPBINDERROR,
      WORKING
} state;

std::string getStateDescription(State astate)
{
  switch(astate)
  {
    case INIT: return "INIT";
    case MGMTBINDING: return "MGMTBINDING";
    case MGMTBOUND: return "MGMTBOUND";
    case MGMTBINDERROR: return "MGMTBINDERROR";
    case WAITINGSTACK: return "WAITINGSTACK";
    case STACKOK: return "STACKOK";
    case SCCPBINDING: return "SCCPBINDING";
    case SCCPBINDERROR: return "SCCPBINDERROR";
    case WORKING: return "WORKING";
    default: return "UNKNOWN";
  }
}

static void changeState(State nstate){ state = nstate;smsc_log_debug(logger,"SCTP monitor:%s",getStateDescription(state).c_str());}

struct Timer {
  struct timeval time;
  int            status;
};

static Timer conftimer;
static Timer stacktimer;
static Timer waitstacktimer;
static Timer assoctimer;
#define MAXCONFTIME 5000

static void setTimer(Timer *timer, long milliseconds)
{
  gettimeofday(&(timer->time), NULL);
  long sec = milliseconds / 1000L;
  long usec = (milliseconds % 1000L) * 1000L;
  timer->time.tv_sec += sec;
  timer->time.tv_usec += usec;
  if (timer->time.tv_usec > 1000000L)
  {
    timer->time.tv_usec -= 1000000L;
    timer->time.tv_sec ++;
  }
  timer->status = 1;
}
static void cancelTimer(Timer *timer)
{
  timer->status = 0;
}
static int checkTimer(Timer *timer)
{
  if (!(timer->status))
  {
    return 0;
  }
  struct timeval now;
  gettimeofday(&now, NULL);
  return (now.tv_sec > timer->time.tv_sec ||
          (now.tv_sec == timer->time.tv_sec && now.tv_usec >= timer->time.tv_usec));
}
SctpMonitor::SctpMonitor()
{
  logger = Logger::getInstance("sctp.mon");
  smsc_log_debug(logger,"\n************************\n* SIBINCO SCTP MONITOR *\n************************");
  listener = 0;
  lastassocstart.tv_sec = 0L;
  lastassocstart.tv_usec = 0L;
  cancelTimer(&assoctimer);
}
void SctpMonitor::addAssociationChangeListener(AssociationChangeListener* _listener)
{
  MutexGuard g(lock);
  listener = _listener;
}
void SctpMonitor::removeAssociationChangeListener()
{
  listener = 0;
}
void SctpMonitor::stop()
{
  going = 0;
}
USHORT_T setalarm()
{
  USHORT_T res;
  ALARM_T alarms[] = {{124,0,0}};
/*
  alarm.alarmId = 124;
  alarm.alarmInfoLength = 0;
  alarm.alarmInfo_p = 0; */

  res = EINSS7_MgmtApiSendAlarmReq(USERID,   /* senderId */
                                   MGMT_ID,  /*receiverID*/
                                   MTPL3_ID, /* moduleID = 6 = MTPL3 */
                                   (UCHAR_T)(sizeof(alarms)/sizeof(ALARM_T)),        /* noOfAlarms */
                                   alarms,
                                   NO_WAIT);
  return res;
}
/*
 * @-> INIT -> MGMTBINDING --> MGMTBOUND
 *                         |
 *                         --> WORKING
 * --> MGMTBOUND -> WAITINGSTACK --> WORKING
 *  |                            |
 *  --------------<---------------
 */
int SctpMonitor::run()
{
  USHORT_T result;
  USHORT_T extresult;

  result = EINSS7CpMsgInitNoSig(MAXENTRIES);
  if (result != 0) {
    smsc_log_error(logger,
                   "MsgInit Failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msginit;
  }

  result = MsgOpen(USERID);
  if (result != 0) {
    smsc_log_error(logger,
                   "MsgOpen failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgexit;
  }

  result = MsgConn(USERID,MGMT_ID);
  if (result != 0) {
    smsc_log_error(logger,
                   "MsgConn to MGMT failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgrelSCCP;
  }
  MSG_T message;

  going = 1;
  changeState(INIT);
  while (going) {
    switch ( state )
    {
      case INIT:
        /*
         * Bind to Management module, await confirmation
         * return if not confirmed in specified period of time
         */
        result = EINSS7_MgmtApiSendBindReq(USERID,MGMT_ID,MGMT_VER,NO_WAIT);
        if( EINSS7_MGMTAPI_REQUEST_OK == result)
        {
          changeState(MGMTBINDING);
          setTimer(&conftimer,MAXCONFTIME);
        }
        else
        {
          smsc_log_error(logger,
                         "EINSS7_MgmtApiSendBindReq() failed with code %d(%s)",
                         result,getReturnCodeDescription(result));
          goto msgrelmgmt;
        }
        break;
      case MGMTBOUND:
        /*
         * Query Stack state, awaiting ss7 stack running
         */
        if (checkTimer(&stacktimer))
        {
          cancelTimer(&stacktimer);
          smsc_log_error(logger,
                         "state=%s, ss7 stack is still not running, exiting...",getStateDescription(state).c_str());
          result = MCIERROR;
          goto unbindmgmt;
        }
        if (checkTimer(&waitstacktimer))
        {
          cancelTimer(&waitstacktimer);
          result = EINSS7_MgmtApiSendMgmtReq(USERID,
                                             MGMT_ID,
                                             9,    /*Stack State Query (SSQ)*/
                                             0,    /* length */
                                             NULL, /* data buffer */
                                             NO_WAIT);
          if( EINSS7_MGMTAPI_REQUEST_OK == result)
          {
            changeState(WAITINGSTACK);
            setTimer(&conftimer,MAXCONFTIME);
          }
          else
          {
            smsc_log_error(logger,
                           "EINSS7_MgmtApiSendMgmtReq failed with code %d(%s)",
                           result,getReturnCodeDescription(result));
            goto unbindmgmt;
          }
        }
        break;
      /*
       * Timer events
       */
      case MGMTBINDING:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(logger,
                         "EINSS7_MgmtApiSendBindReq() confirmation timer is expired, state=%s",getStateDescription(state).c_str());
          result = MCIERROR;
          goto unbindmgmt; /* just in case */
        }
        break;
      case WAITINGSTACK:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(logger,
                         "EINSS7_MgmtApiSendMgmtReq() confirmation timer is expired, state=%s",getStateDescription(state).c_str());
          result = MCIERROR;
          goto unbindmgmt;
        }
        break;
      case WORKING:
        if (checkTimer(&assoctimer))
        {
          setTimer(&assoctimer,60000);
          result = EINSS7_MgmtApiSendOrderReq(USERID,
                                              MGMT_ID, /*receiverID*/
                                              6, /* moduleID = MTPL3 */
                                              53, /* orderID = STATUSOFASSOCS_req */
                                              0, /*lengthOfInfo*/
                                              NULL, /* orderInfo_p*/
                                              NO_WAIT);

          if( EINSS7_MGMTAPI_REQUEST_OK != result)
          {
            smsc_log_error(logger,
                           "EINSS7_MgmtApiSendOrderReq failed with code %d(%s),state=%s,orderID = \"STATUSOFASSOCS_req\"",
                           result,getReturnCodeDescription(result),
                           getStateDescription(state).c_str());
            goto unbindmgmt;
          }
          setalarm();
        }
        if (needToStartAssociation)
        {
          needToStartAssociation = false;
          gettimeofday(&lastassocstart, NULL);
          result = EINSS7_MgmtApiSendOrderReq(USERID,
                                              MGMT_ID, /*receiverID*/
                                              6, /* moduleID = MTPL3 */
                                              39, /* orderID = ASSOC_START_ALL_req */
                                              0, /*lengthOfInfo*/
                                              NULL, /* orderInfo_p*/
                                              NO_WAIT);
          smsc_log_error(logger,"ASSOC_START_ALL_req has been sent");
          if( EINSS7_MGMTAPI_REQUEST_OK != result)
          {
            smsc_log_error(logger,
                           "EINSS7_MgmtApiSendOrderReq failed with code %d(%s),state=%s,orderID = \"ASSOC_START_ALL_req\"",
                           result,getReturnCodeDescription(result),
                           getStateDescription(state).c_str());
            goto unbindmgmt;
          }
        }
        break;
      case MGMTBINDERROR:
        result = MCIERROR;
        goto unbindmgmt;
        break;
      case SCCPBINDERROR:
        result = MCIERROR;
        goto unbindSCCP;
        break;
    } /* end of switch ( state ) */

    message.receiver = USERID;
#if EINSS7_THREADSAFE == 1
    result = EINSS7CpMsgRecv_r(&message,1000);
#else
    result = MsgRecvEvent( &message, 0, 0, 1000 );
#endif

    if( result == MSG_TIMEOUT ) {
      continue;
    }
    if( result != MSG_OK ) {
      smsc_log_error(logger,
                     "MsgRecvEvent failed: %d(%s)",
                     result,getReturnCodeDescription(result));
      going = 0;
      break;
    }
    // handle received message
    {
      //printf("MsgRecv rcvd msg 0x%hx->0x%hx prim=%d sz=%d\n",message.sender,message.receiver,message.primitive,message.size);
      switch(message.sender)
      {
        case MGMT_ID:
          result = EINSS7_MgmtApiReceivedXMMsg(&message);
          if( EINSS7_MGMTAPI_RETURN_OK != result )
          {
            smsc_log_error(logger,
                           "MGMT callback function return code %d(%s)",
                           result,getReturnCodeDescription(result));
          }
          break;
        default :
          smsc_log_error(logger,"MsgRecv receive message from unknown module \"%s\"",getModuleName(message.sender));
      }
      EINSS7CpReleaseMsgBuffer(&message);
    }
  }

unbindSCCP:
unbindmgmt:
  extresult = EINSS7_MgmtApiSendUnbindReq(USERID,MGMT_ID);
  if( extresult != 0 )
    smsc_log_error(logger,
                   "EINSS7_MgmtApiSendUnbindReq(%d,%d) failed with code %d(%s)",
                    USERID,MGMT_ID,extresult,getReturnCodeDescription(extresult));
msgrelmgmt:
  extresult = MsgRel(USERID,MGMT_ID);
  if( extresult != 0 )
    smsc_log_error(logger,
                   "MsgRel(%d,%d) failed with code %d(%s)",
                    USERID,MGMT_ID,extresult,getReturnCodeDescription(extresult));

msgrelSCCP:
msgclose:
  extresult = MsgClose(USERID);
  if( extresult != 0 )
    smsc_log_error(logger,
                   "MsgClose(%d) failed with code %d(%s)",
                    USERID,extresult,getReturnCodeDescription(extresult));
msgexit:
  MsgExit();
msginit:
  smsc_log_debug(logger,"RequestProcessor is down");
  return result;
}
}/* namespace sctp */
}/* namespace snmp */
}/* namespace smsc */

/*********************************************************************/
/*                                                                   */
/*                       C A L L B A C K S                           */
/*                                                                   */
/*********************************************************************/
using namespace smsc::snmp::sctp;
USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,
                                      UCHAR_T result,
                                      UCHAR_T mmState,
                                      UCHAR_T xmRevision)
{
  smsc_log_debug(logger,
                 "MgmtBindConf STATUS=%s STACK=%s MGMT_VER=%hu",
                 getResultDescription(result),
                 getStackStatusDescription(mmState),
                 xmRevision);
  if ( state == MGMTBINDING )
  {
    cancelTimer(&conftimer);
    if ( result == 0 ) /* Success Bind */
    {
      if ( mmState == 4 ) /* Stack is running */
      {
        changeState(WORKING);
        setTimer(&assoctimer,0); /* check association state immediately */
      }
      else
      {
        changeState(MGMTBOUND);
        setTimer(&stacktimer,20000); /* wait stack running during 20 sec*/
        setTimer(&waitstacktimer,0); /* check state immediately after mgmt bind */
      }
    }
    else
    {
      changeState(MGMTBINDERROR);
    }
  }
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,
                                      USHORT_T length,
                                      UCHAR_T *data_p)
{
  smsc_log_debug(logger,
                 "MgmtConf Service=%s DataLength=%d",
                  getTypeOfServiceDescription(typeOfService),
                  length);
  if ( state == WAITINGSTACK &&
       typeOfService == 9 /* type = "Stack State Query (SSQ)" */)
  {
    cancelTimer(&conftimer);
    if (*data_p == 4)       /* StackState = "Running" */
    {
      changeState(WORKING);
      setTimer(&assoctimer,0); /* check association state immediately */
      cancelTimer(&stacktimer);
    }
    else
    {
      changeState(MGMTBOUND);
      setTimer(&waitstacktimer,1500); /* try to check ss7 stack state after 1500 ms */
    }
  }
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleInitConf(USHORT_T moduleId,
                                      UCHAR_T result,
                                      ULONG_T offset)
{
  printf("EINSS7_MgmtApiHandleInitConf\n");
/*
  printf("INIT %s MODULE=%d OFFSET=%d\n",
          getResultDescription(result),
          moduleId,
          offset);

  if (status == SIDLE && result == 0) {
    status = SINIT;
  }
  else {
    fprintf(stderr, "FAILED: ss7 stack init for module %d file offset %d\n", moduleId,offset);
  }
*/
  return MSG_OK;
}

USHORT_T EINSS7_MgmtApiHandleStartConf(USHORT_T moduleId,
                                       UCHAR_T result)
{
  printf("EINSS7_MgmtApiHandleStartConf\n");
/*
  printf("START %s MODULE=%d\n",
          getResultDescription(result),
          moduleId);

  if (status == SINIT && result == 0) {
    status = SRUN;
  }
  else {
    fprintf(stderr, "FAILED: ss7 stack start for module %d\n", moduleId);
  }
*/
  return MSG_OK;
}


USHORT_T EINSS7_MgmtApiHandleStopConf(USHORT_T moduleId,
                                      UCHAR_T result)
{
  printf("EINSS7_MgmtApiHandleStopConf\n");
  return EINSS7_MGMTAPI_RETURN_OK;
}


USHORT_T EINSS7_MgmtApiHandleAlarmConf(USHORT_T moduleId,
                                       UCHAR_T noOfAlarms,
                                       ALARMS_T *alarms_sp)
{
  smsc_log_debug(logger,
                 "MgmtAlarmConf MODULE=%s ALARMRES[%d]={%s}",
                 getModuleName(moduleId),
                 noOfAlarms,
                 getAlarmResultDescription(noOfAlarms,alarms_sp).c_str());
  return EINSS7_MGMTAPI_RETURN_OK;
}

USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,
                                       UCHAR_T orderId,
                                       UCHAR_T orderResult,
                                       UCHAR_T resultInfo,
                                       UCHAR_T lengthOfInfo,
                                       UCHAR_T *orderInfo_p)
{
  smsc_log_debug(logger,
                 "MgmtOrderConf %s ORDER %d %s orderInfo_p[%d]={%s}",
                 getModuleName(moduleId),
                 orderId,
                 getResultDescription(orderResult),
                 lengthOfInfo,
                 dump(lengthOfInfo, orderInfo_p).c_str());
  switch (orderId)
  {
    case 53:
      {
        UCHAR_T* p = orderInfo_p;
        p++; //skip 'Last Confirmation' indicator
        uint8_t count = (lengthOfInfo - 1) / 3;
        for (int i =0; i < count; i++)
        {
          AssociationChangeEvent event;
          time(&event.time);
          event.said = p[i*3+0] | (p[i*3+1]<<8);
          event.state = p[i*3+2];
          if (listener) listener->associationChange(event);
        }
      }
      break;
  }
  return EINSS7_MGMTAPI_RETURN_OK;
#if 0
ucn1:/opt/EABss7009/bin/OAM root# ./assocst
ORDER module=6 orderid=53 result=Success resinfo=255 len=13
data:01 01 00 09 02 00 09 03 00 09 04 00 09

  UCHAR_T* buf = hexDump(orderInfo_p,lengthOfInfo);
  printf("ORDER module=%d orderid=%d result=%s len=%d data:%s\n",
          moduleId,
          orderId,
          getResultDescription(orderResult),
          lengthOfInfo,
          buf);
  free(buf);

  if (status == RUNNING &&
      orderId == 53   &&
      orderResult == 0) status = TRACEOK;
  return EINSS7_MGMTAPI_RETURN_OK;
#endif
}

USHORT_T EINSS7_MgmtApiHandleXStatConf(USHORT_T moduleId,
                                       USHORT_T xStatId,
                                       UCHAR_T statResult,
                                       UCHAR_T lastInd,
                                       USHORT_T totLength,
                                       XSTAT_T *xStat_sp)
{
  printf("EINSS7_MgmtApiHandleXStatConf\n");
  return EINSS7_MGMTAPI_RETURN_OK;
}


USHORT_T EINSS7_MgmtApiHandleStatisticsConf(USHORT_T moduleId,
                                            UCHAR_T noOfStat,
                                            STAT_T *stat_sp)
{
  printf("EINSS7_MgmtApiHandleStatisticsConf\n");
  return EINSS7_MGMTAPI_RETURN_OK;
}


USHORT_T EINSS7_MgmtApiHandleAlarmInd(USHORT_T moduleId,
                                      UCHAR_T alarmId,
                                      UCHAR_T alarmStatusLength,
                                      UCHAR_T *alarmStatus_p)
{
  smsc_log_debug(logger,
                 "MgmtAlarmInd MODULE=%s ALARM=%d alarmStatus_p[%d]={%s}",
                 getModuleName(moduleId),
                 alarmId,
                 alarmStatusLength,
                 dump(alarmStatusLength, alarmStatus_p).c_str());
  switch (moduleId)
  {
    case MTPL3_ID:
      switch (alarmId)
      {
        case 124 : /*association state change */
          if (alarmStatusLength && alarmStatus_p)
          {
            UCHAR_T* p = alarmStatus_p;
            AssociationChangeEvent event;
            time(&event.time);
            event.said = p[0] | (p[1]<<8);
            switch (p[3])
            {
              case 0: event.state = 0; break;
              case 1: event.state = 3; break;
              case 2: event.state = 6; break;
              case 3: event.state = 9; break;
            }
            if (listener) listener->associationChange(event);
          }
          break;
      }
      break;
  }
  return EINSS7_MGMTAPI_RETURN_OK;
}


USHORT_T EINSS7_MgmtApiHandleErrorInd(UCHAR_T len,
                                      USHORT_T ec,
                                      UCHAR_T astate,
                                      UCHAR_T event)
{
  printf("EINSS7_MgmtApiHandleErrorInd(len=%d,errorCode=%d,state=%d,event=%d)\n",len,ec,astate,event);
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
  printf("EINSS7_MgmtApiHandleSysInfoInd\n");
  return EINSS7_MGMTAPI_RETURN_OK;
}

USHORT_T EINSS7_MgmtApiIndError(USHORT_T errorCode,
                                MSG_T *msg_sp)
{
  printf("EINSS7_MgmtApiIndError\n");
  return EINSS7_MGMTAPI_RETURN_OK;
}



USHORT_T EINSS7_MgmtApiSysInfoInd(USHORT_T errorCode,
                                  MSG_T *msg_sp)
{
  printf("EINSS7_MgmtApiSysInfoInd\n");
  return EINSS7_MGMTAPI_RETURN_OK;
}

UCHAR_T *hexDump( UCHAR_T* src, USHORT_T len ) {
  int i;
  char *buf = (char*)malloc( len*3+1 );
  for( i = 0; i < len; i++ ) {
    sprintf( buf+(i*3), "%02x ", src[i] );
  }
  buf[len*3]=0;
  return (UCHAR_T*)buf;
}
