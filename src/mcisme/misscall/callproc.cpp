#include <unistd.h>
#include <vector>
#include <strings.h>
#include "callproc.hpp"
#include "clbks.hpp"
#include "util.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"
#include "util/regexp/RegExp.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include <utility>


using namespace std;
void mts(EINSS7_I97_ISUPHEAD_T *head,EINSS7_I97_CALLINGNUMB_T *calling,EINSS7_I97_CALLEDNUMB_T *called,EINSS7_I97_ORIGINALNUMB_T *original,EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,EINSS7_I97_REDIRECTINGNUMB_T *redirecting);
void taif(EINSS7_I97_ISUPHEAD_T *head,EINSS7_I97_CALLINGNUMB_T *calling,EINSS7_I97_CALLEDNUMB_T *called,EINSS7_I97_ORIGINALNUMB_T *original,EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,EINSS7_I97_REDIRECTINGNUMB_T *redirecting);
void taifmixed(EINSS7_I97_ISUPHEAD_T *head,EINSS7_I97_CALLINGNUMB_T *calling,EINSS7_I97_CALLEDNUMB_T *called,EINSS7_I97_ORIGINALNUMB_T *original,EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,EINSS7_I97_REDIRECTINGNUMB_T *redirecting);
void redirect_rule(EINSS7_I97_ISUPHEAD_T *head,EINSS7_I97_CALLINGNUMB_T *calling,EINSS7_I97_CALLEDNUMB_T *called,EINSS7_I97_ORIGINALNUMB_T *original,EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,EINSS7_I97_REDIRECTINGNUMB_T *redirecting);

namespace smsc{
namespace misscall{

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::misscall::util::getReturnCodeDescription;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using std::vector;

#define MAXENTRIES 1000
#define USER USER02_ID
#define MGMT_VER 6
#define MCIERROR 999
#define handle(a,b,c,d,e) taif(a,b,c,d,e)

Logger* missedCallProcessorLogger = 0;
Mutex MissedCallProcessor::lock;
MissedCallProcessor* volatile MissedCallProcessor::processor = 0;

MissedCallProcessor::instance_type_t MissedCallProcessor::_instanceType;

int going;
char countryCode[COUNTRY_CODE_MAX_LEN + 1];

void detectDisconnect(USHORT_T result, const char *func) {
  if( (result == 1011) || (result == 1009) ) {
    smsc_log_error(missedCallProcessorLogger,
                   "Stack disconnect detected in %s",
                   func);
    going = 0;
  }
}

static smsc::util::regexp::RegExp maskRx;
static smsc::util::regexp::RegExp calledMaskRx;


enum State{
      INIT,
      MGMTBINDING,
      MGMTBOUND,
      MGMTBINDERROR,
      WAITINGSTACK,
      STACKOK,
      ISUPBINDING,
      ISUPBOUND,
      LINKUP,
      ISUPBINDERROR,
      UNBLOCKING,
      WORKING
} state;
std::string getStateDescription(State aState)
{
  switch(aState)
  {
    case INIT: return "INIT";
    case MGMTBINDING: return "MGMTBINDING";
    case MGMTBOUND: return "MGMTBOUND";
    case MGMTBINDERROR: return "MGMTBINDERROR";
    case WAITINGSTACK: return "WAITINGSTACK";
    case STACKOK: return "STACKOK";
    case ISUPBINDING: return "ISUPBINDING";
    case ISUPBOUND: return "ISUPBOUND";
    case ISUPBINDERROR: return "ISUPBINDERROR";
    case UNBLOCKING: return "UNBLOCKING";
    case WORKING: return "WORKING";
    case LINKUP: return "LINKUP";
  }
}
static void changeState(State nstate)
{
  state = nstate;
  smsc_log_debug(missedCallProcessorLogger,
                 "MissedCallProcessor:%s",
                 getStateDescription(state).c_str());
}
struct Timer {
  struct timeval time;
  int            status;
};

static Timer conftimer;
static Timer stacktimer;
static Timer waitstacktimer;

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
/*
 * @-> INIT -> MGMTBINDING --> MGMTBOUND
 *                         |
 *                         --> STACKOK
 * --> MGMTBOUND -> WAITINGSTACK --> STACKOK
 *  |                            |
 *  --------------<---------------
 * --> STACKOK -> ISUPBINDING -> ISUPBOUND
 * --> ISUPBOUND --(RGAVAIL)--> LINKUP
 *  |             |
 *  --(RGUNAVAIL)--
 * --> LINKUP -> UNBLOCKING -> WORKING
 */


bool setCallingMask(const char* rx)
{
  smsc::util::regexp::RegExp::InitLocale();
  return maskRx.Compile(rx)!=0;
}
bool setCalledMask(const char* rx) {
  smsc::util::regexp::RegExp::InitLocale();
  return calledMaskRx.Compile(rx)!=0;
}

struct Connection {
  Circuits cics;
  State status; /* INIT -> ISUPBOUND -> UNBLOCKING -> WORKING */
  Timer timer;
  Connection()
  {
    status = INIT;
  }
  void start()
  {
    status = ISUPBOUND;
    setTimer(&timer,120000); /* max wait for MTP_RESUME */
  }
  void tick()
  {
    switch(status)
    {
      case ISUPBOUND:
        if (checkTimer(&timer))
        {
          smsc_log_error(missedCallProcessorLogger,
                         "Resource group availability timer for span=%d is expired",
                         cics.spn);
          cancelTimer(&timer);
          status = ISUPBINDERROR;
        }
        break;
      case UNBLOCKING:
        if (checkTimer(&timer))
        {
          smsc_log_error(missedCallProcessorLogger,
                         "Unblock confirmation timer for span=%d is expired",
                         cics.spn);
          cancelTimer(&timer);
          status = ISUPBINDERROR;
        }
        break;
    }
  }
  void unblock()
  {

    UCHAR_T UBL[]={0x00/*HSN*/,
                   0x00/*SPN*/,
                   0xFE,
                   0xFF,
                   0xFE,
                   0xFF/*TS MASK*/,
                   0x00/*NO FORCE*/};

    UBL[0] = cics.hsn;
    UBL[1] = cics.spn;
    UBL[2] = cics.ts & 0xFF;
    UBL[3] = cics.ts >> 8 & 0xFF;
    UBL[4] = cics.ts >> 16 & 0xFF;
    UBL[5] = cics.ts >> 24 & 0xFF;

    status = UNBLOCKING;

    smsc_log_info(missedCallProcessorLogger,
                     "unblocking span=%d hsn=%d ts=0x%x",cics.spn,cics.hsn,cics.ts);
    USHORT_T result;
    result = EINSS7_MgmtApiSendOrderReq(
               USER,
               MGMT_ID,
               ISUP_ID,
               0x0B/*UNBLOCK CURCUITS*/,
               static_cast<USHORT_T>(sizeof(UBL)),
               UBL,
               NO_WAIT);

    if( result != EINSS7_MGMTAPI_REQUEST_OK )
    {
      detectDisconnect( result, __func__ );
      smsc_log_error(missedCallProcessorLogger,
                     "EINSS7_MgmtApiSendOrderReq() unblocking span=%d failed with code %d(%s)",
                     cics.spn,result,getReturnCodeDescription(result));
      status = ISUPBINDERROR;
    }
    else
    {
      setTimer(&timer,120000); /* max wait for success unblocking */
    }
  }
  void finish(int ublstatus)
  {
    if (ublstatus)
    {
      cancelTimer(&timer);
      status = WORKING;
      smsc_log_debug(missedCallProcessorLogger,
                     "MissedCallProcessor: span %d is %s",
                         cics.spn,getStateDescription(status).c_str());
    }
    else
    {
      cancelTimer(&timer);
      status = ISUPBINDERROR;
    }
  }
};

static char *rediraddr = 0;
static char  rediraddrbuf[32] = {0};

void MissedCallProcessor::setRedirectionAddress(const char* address)
{
  if (address && strlen(address) < 32)
  {
    strcpy(rediraddrbuf,address);
    rediraddr = rediraddrbuf;
  }
}

using namespace smsc::core::buffers;
static IntHash<Connection> connections;

void MissedCallProcessor::setCircuits(Hash<Circuits>& circuits)
{
  Circuits cics; char* circuitsMsc = 0; circuits.First();
  while (circuits.Next(circuitsMsc, cics))
  {
    Connection conn;
    conn.cics = cics;
    connections.Insert(cics.spn,conn);
  }
}

bool MissedCallProcessor::setCountryCode(const char* _countryCode)
{
  size_t len = strlen(_countryCode);
  if(len > COUNTRY_CODE_MAX_LEN)  return false;
  strncpy(countryCode, _countryCode, len);
  countryCode[len]=0;
  return true;
}

struct InternalRule {
  Rule   rule;
  RegExp exp;
  vector<SMatch> m;
  int n;
  InternalRule(Rule& _rule)
  {
    rule = _rule;
    if ( !exp.Compile(rule.rx.c_str()) )
      smsc_log_error(missedCallProcessorLogger,
                     "rule='%s' rx='%s' compiling failed",
                     rule.name.c_str(),rule.rx.c_str());
    m.resize(exp.GetBracketsCount());
    n=static_cast<int>(m.size());
  }
  int match(const char* rx,UCHAR_T* cause,UCHAR_T* inform)
  {
      int res = exp.Match(rx,&m[0],n);
      if (res)
      {
        *cause  = rule.cause;
        *inform = rule.inform;
      }
      return res;
  }
};

static vector<InternalRule*> rules;

void MissedCallProcessor::setRules(vector<Rule>& _rules)
{
  vector<Rule>::iterator it = _rules.begin(),end = _rules.end();
  for(;it != end;++it)
  {
    InternalRule* item = new InternalRule(*it);
    rules.push_back(item);
  }
}

static ReleaseSettings relCauses = {
/* strategy */ REDIRECT_STRATEGY,
/* busy     */ 0x11 /* called user busy    */, 0x01 /* inform              */,
/* no reply */ 0x13 /* no answer from user */, 0x01 /* inform              */,
/* uncond   */ 0x15 /* call rejected       */, 0x00 /* don't inform        */,
/* absent   */ 0x14 /* subscriber absent   */, 0x01 /* inform              */,
/* detach   */ 0x14 /* subscriber absent   */, 0x01 /* inform              */,
/* other    */ 0x15 /* call rejected       */, 0x01 /* inform just in case */,
/* skip uca */ false
};


static void (*strategy)(EINSS7_I97_ISUPHEAD_T *head,
                        EINSS7_I97_CALLINGNUMB_T *calling,
                        EINSS7_I97_CALLEDNUMB_T *called,
                        EINSS7_I97_ORIGINALNUMB_T *original,
                        EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
                        EINSS7_I97_REDIRECTINGNUMB_T *redirecting) = mts;

void MissedCallProcessor::setReleaseSettings(ReleaseSettings params)
{
  relCauses = params;
  if (relCauses.strategy == REDIRECT_STRATEGY) strategy = mts;
  if (relCauses.strategy == PREFIXED_STRATEGY) strategy = taif;
  if (relCauses.strategy == MIXED_STRATEGY) strategy = taifmixed;
  if (relCauses.strategy == REDIREC_RULE_STRATEGY) strategy = redirect_rule;
}

void
MissedCallProcessor::setInstanceType(instance_type_t instance_type)
{
  _instanceType = instance_type;
}

MissedCallProcessor* MissedCallProcessor::instance()
{
  if ( processor == 0 ) {
    MutexGuard g(lock);
    if ( processor == 0 )
    {
      if ( _instanceType == REAL_CALL_PROCESOR )
        processor = new MissedCallProcessor();
      else
        processor = new MissedCallProcessorEmulator();

      smsc::util::regexp::RegExp::InitLocale();
    }
  }
  return processor;
}

MissedCallProcessor::MissedCallProcessor()
{
  missedCallProcessorLogger = Logger::getInstance("smsc.misscall");
  memset(countryCode, 0x00, COUNTRY_CODE_MAX_LEN + 1);
  listener = 0;
}
void MissedCallProcessor::addMissedCallListener(MissedCallListener* _listener)
{
  MutexGuard g(lock);
  listener = _listener;
}
void MissedCallProcessor::removeMissedCallListener()
{
  MutexGuard g(lock);
  listener = 0;
}
void MissedCallProcessor::fireMissedCallEvent(const MissedCallEvent& event)
{
  if (listener)
  {
    {
      MutexGuard g(lock);
      if (listener) listener->missed(event);
    }
  }
}
void MissedCallProcessor::stop()
{
  going = 0;
}
int MissedCallProcessor::run()
{
  USHORT_T result;
  USHORT_T extresult;

  result = EINSS7CpMsgInitNoSig(MAXENTRIES);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,
                   "MsgInit Failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msginit;
  }

  result = MsgOpen(USER);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,
                   "MsgOpen failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgexit;
  }

  result = MsgConn(USER,ISUP_ID);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,
                   "MsgConn to ISUP failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgclose;
  }
  result = MsgConn(USER,MGMT_ID);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,
                   "MsgConn to MGMT failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgrelisup;
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
        result = EINSS7_MgmtApiSendBindReq(USER,MGMT_ID,MGMT_VER,NO_WAIT);
        if( EINSS7_MGMTAPI_REQUEST_OK == result)
        {
          changeState(MGMTBINDING);
          setTimer(&conftimer,MAXCONFTIME);
        }
        else
        {
          smsc_log_error(missedCallProcessorLogger,
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
          smsc_log_error(missedCallProcessorLogger,
                         "ss7 stack is still not running, exiting...");
          result = MCIERROR;
          goto unbindmgmt;
        }
        if (checkTimer(&waitstacktimer))
        {
          cancelTimer(&waitstacktimer);
          result = EINSS7_MgmtApiSendMgmtReq(USER,
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
            smsc_log_error(missedCallProcessorLogger,
                           "EINSS7_MgmtApiSendMgmtReq failed with code %d(%s)",
                           result,getReturnCodeDescription(result));
            goto unbindmgmt;
          }
        }
        break;

      case STACKOK:
        result = EINSS7_I97IsupBindReq(USER);
        if( result != EINSS7_I97_REQUEST_OK )
        {
          smsc_log_error(missedCallProcessorLogger,
                         "EINSS7_I97IsupBindReg() failed with code %d(%s)",
                         result,getReturnCodeDescription(result));
          goto unbindmgmt;
        }
        else
        {
          changeState(ISUPBINDING);
          setTimer(&conftimer,MAXCONFTIME);
        }
        break;
      case ISUPBOUND:
        {
          int linkup = 0;
          Connection *conn = 0;
          int span;
          IntHash<Connection>::Iterator it=connections.First();
          while (it.Next(span, conn))
          {
            if(conn)
            {
              conn->tick();
              linkup |= (conn->status == WORKING);
            }
          }

          if(linkup) cancelTimer(&stacktimer);
          if (checkTimer(&stacktimer))
          {
            smsc_log_error(missedCallProcessorLogger,
                           "Resource group availability timer is expired",
                           result,getReturnCodeDescription(result));
            result = MCIERROR;
            goto unbindisup;
          }
          break;
        }

      /*
       * Timer events
       */
      case MGMTBINDING:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(missedCallProcessorLogger,
                         "EINSS7_MgmtApiSendBindReq() confirmation timer is expired");
          result = MCIERROR;
          goto unbindmgmt; /* just in case */
        }
        break;
      case ISUPBINDING:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(missedCallProcessorLogger,
                         "EINSS7_I97IsupBindReg() confirmation timer is expired");
          result = MCIERROR;
          goto unbindisup; /* just in case */
        }
        break;
      case WAITINGSTACK:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(missedCallProcessorLogger,
                         "EINSS7_MgmtApiSendMgmtReq() confirmation timer is expired");
          result = MCIERROR;
          goto unbindmgmt;
        }
        break;
      case MGMTBINDERROR:
        result = MCIERROR;
        goto unbindmgmt;
        break;
      case ISUPBINDERROR:
        result = MCIERROR;
        goto unbindisup;
        break;
    } /* end of switch ( state ) */

    message.receiver = USER;
    //result = EINSS7CpMsgRecv_r( &message, 1000 );
    result = MsgRecvEvent( &message, 0, 0, 1000 );
    if( result == MSG_TIMEOUT ) {
      continue;
    }
    if( result != MSG_OK ) {
      smsc_log_error(missedCallProcessorLogger,
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
        case ISUP_ID:
          try {
            result = EINSS7_I97IsupHandleInd(&message);
            if( EINSS7_I97_REQUEST_OK != result )
            {
              smsc_log_error(missedCallProcessorLogger,
                             "ISUP callback function return code %d(%s)",
                             result,getReturnCodeDescription(result));
            }
          }
          catch (std::exception& exc) {
            smsc_log_error(missedCallProcessorLogger, "Exception occured in call of EINSS7_I97IsupHandleInd. Reason: %s", exc.what());
            going = 0;
          }
          catch (...) {
            smsc_log_error(missedCallProcessorLogger, "Unknown exception occured in call of EINSS7_I97IsupHandleInd");
            going = 0;
          }
          break;
        case MGMT_ID:
          try {
            result = EINSS7_MgmtApiReceivedXMMsg(&message);
            if( EINSS7_MGMTAPI_RETURN_OK != result )
            {
              smsc_log_error(missedCallProcessorLogger,
                             "MGMT callback function return code %d(%s)",
                             result,getReturnCodeDescription(result));
            }
          }
          catch (std::exception& exc) {
            smsc_log_error(missedCallProcessorLogger, "Exception occured in call of EINSS7_MgmtApiReceivedXMMsg. Reason: %s", exc.what());
            going = 0;
          }
          catch (...) {
            smsc_log_error(missedCallProcessorLogger, "Unknown exception occured in call of EINSS7_MgmtApiReceivedXMMsg");
            going = 0;
          }
          break;
      }
      EINSS7CpReleaseMsgBuffer(&message);
    }
  }

unbindisup:
  extresult = EINSS7_I97IsupUnBindReq();
  if( extresult != 0 )
    smsc_log_error(missedCallProcessorLogger,
                   "EINSS7_I97IsupUnBindReq failed with code %d(%s)",
                    extresult,getReturnCodeDescription(extresult));
unbindmgmt:
  extresult = EINSS7_MgmtApiSendUnbindReq(USER,MGMT_ID);
  if( extresult != 0 )
    smsc_log_error(missedCallProcessorLogger,
                   "EINSS7_MgmtApiSendUnbindReq(%d,%d) failed with code %d(%s)",
                    USER,MGMT_ID,extresult,getReturnCodeDescription(extresult));
msgrelmgmt:
  extresult = MsgRel(USER,MGMT_ID);
  if( extresult != 0 )
    smsc_log_error(missedCallProcessorLogger,
                   "MsgRel(%d,%d) failed with code %d(%s)",
                    USER,MGMT_ID,extresult,getReturnCodeDescription(extresult));

msgrelisup:
  extresult = MsgRel(USER,ISUP_ID);
  if( extresult != 0 )
    smsc_log_error(missedCallProcessorLogger,
                   "MsgRel(%d,%d) failed with code %d(%s)",
                    USER,ISUP_ID,extresult,getReturnCodeDescription(extresult));
msgclose:
  extresult = MsgClose(USER);
  if( extresult != 0 )
    smsc_log_error(missedCallProcessorLogger,
                   "MsgClose(%d) failed with code %d(%s)",
                    USER,extresult,getReturnCodeDescription(extresult));
msgexit:
  MsgExit();
msginit:
  smsc_log_debug(missedCallProcessorLogger,"MissedCallProcessor is down");
  return result;
}
}//namespace misscall
}//namespace smsc

/*********************************************************************/
/* ISUP CALLBACKS IMPLEMENTATION                                     */
/*-------------------------------------------------------------------*/
/* IndError                                                          */
/* BindConf                                                          */
/* SetupInd                                                          */
/* ReleaseConf                                                       */
/* ReleaseInd                                                        */
/*********************************************************************/
using smsc::misscall::detectDisconnect;
using smsc::misscall::MissedCallEvent;
using smsc::misscall::MissedCallProcessor;
using smsc::misscall::missedCallProcessorLogger;
using smsc::misscall::ISUPBINDING;
using smsc::misscall::MGMTBINDING;
using smsc::misscall::MGMTBOUND;
using smsc::misscall::MGMTBINDERROR;
using smsc::misscall::WAITINGSTACK;
using smsc::misscall::STACKOK;
using smsc::misscall::ISUPBOUND;
using smsc::misscall::LINKUP;
using smsc::misscall::ISUPBINDERROR;
using smsc::misscall::UNBLOCKING;
using smsc::misscall::WORKING;
using smsc::misscall::state;
using smsc::logger::Logger;
using smsc::misscall::conftimer;
using smsc::misscall::stacktimer;
using smsc::misscall::waitstacktimer;
using smsc::misscall::relCauses;
using smsc::misscall::NONE;
using smsc::misscall::ABSENT;
using smsc::misscall::BUSY;
using smsc::misscall::NOREPLY;
using smsc::misscall::UNCOND;
using smsc::misscall::DETACH;
using smsc::misscall::strategy;
using smsc::misscall::Connection;
using smsc::misscall::connections;
using smsc::misscall::InternalRule;
using smsc::misscall::rules;
using smsc::misscall::countryCode;
using smsc::misscall::NON_ANTI_AON_FOR_CALLER;
using smsc::misscall::ANTI_AON_FOR_CALLER;

using namespace smsc::misscall::util;
using namespace smsc::core::buffers;
using smsc::sms::MAX_FULL_ADDRESS_VALUE_LENGTH;

USHORT_T EINSS7_I97IsupIndError(USHORT_T errorCode,MSG_T *message)
{
  char *msg = new char[message->size*4+1];
  int k = 0;
  for ( int i=0; i<message->size; i++){
    k+=sprintf(msg+k,"%02x ",(unsigned)message->msg_p[i]);
  }
  msg[k]=0;
  const char* reason;
  switch(errorCode)
  {
    case EINSS7_I97_IND_UNKNOWN_CODE : reason = "\"Unknown primitive code\"";break;
    case EINSS7_I97_IND_LENGTH_ERROR : reason = "\"Length error\"";break;
  }
  smsc_log_error(missedCallProcessorLogger,
                 "EINSS7_I97IsupHandleInd found error $s in msg: %s",
                 reason,
                 msg);
  delete msg;
  return EINSS7_I97_REQUEST_OK;
}

USHORT_T EINSS7_I97IsupBindConf(USHORT_T isupUserID,UCHAR_T result)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "IsupBindConf %s",
                 getIsupBindStatusDescription(result));
  if ( state == ISUPBINDING )
  {
    cancelTimer(&conftimer);
    if (result == EINSS7_I97_BIND_OK ||
        result == EINSS7_I97_SAME_CC_BOUND)
    {
      changeState(ISUPBOUND);
      Connection *conn = 0;
      int span;
      IntHash<Connection>::Iterator it=connections.First();
      while (it.Next(span, conn))
      {
        if(conn) conn->start();
      }
      setTimer(&stacktimer,120000); /* wait at least one span workinfg */
    }
    else
    {
      changeState(ISUPBINDERROR);
    }
  }
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T releaseConnection(EINSS7_I97_ISUPHEAD_T *isupHead_sp, UCHAR_T causeValue)
{
  char r[]="9139254896";
  USHORT_T res;
  EINSS7_I97_CAUSE_T cause;
  cause.location = EINSS7_I97_ISUP_USER;
  cause.codingStd = EINSS7_I97_ITU_STAND;
  cause.rec = 0;
  cause.causeValue = causeValue;
  cause.lengthOfDiagnostics = 0;
  cause.diagnostics_p = 0;

  EINSS7_I97_REDIRECTIONINFO_T redirectionInfo;
  redirectionInfo.redirecting = EINSS7_I97_CALL_FORW;
  redirectionInfo.origReason = EINSS7_I97_REASON_UNKNOWN;
  redirectionInfo.counter = 2;
  redirectionInfo.lastReason = EINSS7_I97_MOB_NOT_REACHED;

  EINSS7_I97_REDIRECTIONNUMB_T redirectionNumber;
  redirectionNumber.natureOfAddr = EINSS7_I97_NATIONAL_NO;
  redirectionNumber.numberPlan = EINSS7_I97_ISDN_PLAN;
  redirectionNumber.internalNetwNumb = EINSS7_I97_INTERNAL_ROUT;
  redirectionNumber.noOfAddrSign = static_cast<UCHAR_T>(strlen(r));
  UCHAR_T* addr = new UCHAR_T[(strlen(r)+1)/2];
  pack_addr(addr,r,static_cast<int>(strlen(r)));
  redirectionNumber.addrSign_p = addr;
  smsc_log_debug(missedCallProcessorLogger,"IsupReleaseReq %s cause=%u",getHeadDescription(isupHead_sp).c_str(),cause.causeValue);
  res = EINSS7_I97IsupReleaseReq(isupHead_sp, /*to do ����� �� �� ���� �������*/
                                  &cause,
                                  0, /* autoCongestLevel_p  */
                                  0, /* &redirectionNumber, */
                                  0, /* &redirectionInfo,   */
                                  0  /* extraOpts_sp        */
                                 );
  delete addr;
  if (res != 0)
  {
    detectDisconnect( res, __func__ );
    smsc_log_error(
                   missedCallProcessorLogger,
                   "IsupReleaseReq %s failed with error code %d(%s)",
                   getHeadDescription(isupHead_sp).c_str(),
                   res,getReturnCodeDescription(res)
                  );
  }
  return res;
}
void registerEvent(EINSS7_I97_CALLINGNUMB_T *calling, EINSS7_I97_CALLEDNUMB_T *called, uint8_t eventType)
{
  MissedCallEvent event;
  event.cause = eventType;
  time(&event.time);
  if (calling &&
      calling->noOfAddrSign > 0 &&
      calling->noOfAddrSign <= MAX_FULL_ADDRESS_VALUE_LENGTH)
  {

    /*max length of .ton.npi.digits = 1+3+1+1+1+20 = 27*/
    char cgaddr[32] = {0};
    int pos = snprintf(cgaddr,sizeof(cgaddr),".%d.%d.",calling->natureOfAddr,calling->numberPlan);
    unpack_addr(cgaddr+pos, calling->addrSign_p, calling->noOfAddrSign);

    using smsc::misscall::maskRx;
    using smsc::util::regexp::SMatch;
    std::vector<SMatch> m(maskRx.GetBracketsCount());

    int n=static_cast<int>(m.size());
    if(maskRx.Match(cgaddr,&m[0],n))
    {
      cgaddr[0] = 0;
      if (calling->natureOfAddr == EINSS7_I97_NATIONAL_NO)
      {
        size_t len = strlen(countryCode);
        cgaddr[0] = '+';
        strncpy(&cgaddr[1], countryCode, len);
        unpack_addr(cgaddr+len+1, calling->addrSign_p, calling->noOfAddrSign);
      }
      else if (calling->natureOfAddr == EINSS7_I97_INTERNATIONAL_NO)
      {
        cgaddr[0] = '+';
        unpack_addr(cgaddr+1, calling->addrSign_p, calling->noOfAddrSign);
      }
      /*
       * FIX: allow SUBSCRIBER and UNKNOWN type of calling address
       * For example prefix of PBX number
       * Change request from MTS MR URAL
       */
      else if (calling->natureOfAddr == EINSS7_I97_SUBSCRIBER_NO ||
               calling->natureOfAddr == EINSS7_I97_NA_UNKNOWN)
      {
        unpack_addr(cgaddr, calling->addrSign_p, calling->noOfAddrSign);
      }

      event.from = cgaddr;
      if ( calling->presentationRestr == EINSS7_I97_PRES_ALLOWED ) {
        event.flags = NON_ANTI_AON_FOR_CALLER;
      } else {
        event.flags = ANTI_AON_FOR_CALLER;
      }
    }
  }
  /* skip prohibited or unknown caller */
  if (relCauses.skipUnknownCaller && event.from.length() == 0)
  {
    return;
  }
  if (called &&
      called->noOfAddrSign > 0 &&
      called->noOfAddrSign <= MAX_FULL_ADDRESS_VALUE_LENGTH)
  {
    char cdaddr[32] = {0};
    int pos = snprintf(cdaddr,sizeof(cdaddr),".%d.%d.",called->natureOfAddr,called->numberPlan);
    unpack_addr(cdaddr+pos, called->addrSign_p, called->noOfAddrSign);

    using smsc::misscall::calledMaskRx;
    using smsc::util::regexp::SMatch;
    std::vector<SMatch> m(calledMaskRx.GetBracketsCount());

    int n=static_cast<int>(m.size());
    if(calledMaskRx.Match(cdaddr,&m[0],n))
    {
      cdaddr[0] = 0;
      if (called->natureOfAddr == EINSS7_I97_NATIONAL_NO)
      {
        size_t len = strlen(countryCode);
        cdaddr[0] = '+';
        strncpy(&cdaddr[1], countryCode, len);
        unpack_addr(cdaddr+len+1, called->addrSign_p, called->noOfAddrSign);
      }
      else if (called->natureOfAddr == EINSS7_I97_INTERNATIONAL_NO)
      {
        cdaddr[0] = '+';
        unpack_addr(cdaddr+1, called->addrSign_p, called->noOfAddrSign);
      }
      event.to = cdaddr;
      event.gotFromIAMSME = false;
      smsc_log_debug(missedCallProcessorLogger,"send event: %s->%s, flags=0x%x, type=0x%x",event.from.c_str(),event.to.c_str(), event.flags, event.cause);
      MissedCallProcessor::instance()->fireMissedCallEvent(event);
    }
  }
}

void registerEvent(EINSS7_I97_CALLINGNUMB_T *calling,
                   EINSS7_I97_REDIRECTINGNUMB_T *called,
                   uint8_t eventType)
{
  MissedCallEvent event;
  event.cause = eventType;
  time(&event.time);

  /*
   * FIX: remove unnecessary country code (7) digits from national address
   * It's a known bug in Siemens mobile switch
   * Change request from URALTEL
   */
  EINSS7_I97_REDIRECTINGNUMB_T fixed_called;
  UCHAR_T fixed_called_p[32] = {0};
  if (called &&
      called->natureOfAddr == EINSS7_I97_NATIONAL_NO)
  {
    char cdaddr[32] = {0};
    unpack_addr(cdaddr, called->addrSign_p, called->noOfAddrSign);
    int addrlen = static_cast<int>(strlen(cdaddr));
    if (addrlen > 10)
    {
      int skip = 0; while(cdaddr[skip] == '7') skip++;
      pack_addr(fixed_called_p, cdaddr + skip, addrlen - skip);
      fixed_called.natureOfAddr = called->natureOfAddr;
      fixed_called.presentationRestr = called->presentationRestr;
      fixed_called.numberPlan = called->numberPlan;
      fixed_called.noOfAddrSign = addrlen - skip;
      fixed_called.addrSign_p = fixed_called_p;
      smsc_log_debug(missedCallProcessorLogger,
                     "modify original called address: %s -> %s",
                     getRedirectingNumberDescription(called).c_str(),
                     getRedirectingNumberDescription(&fixed_called).c_str());
      called = &fixed_called;
    }
  }
  /*
   * FIX: remove trailing 0 from calling national address
   * Change request from MTS Belarus
   */
  EINSS7_I97_CALLINGNUMB_T fixed_calling;
  UCHAR_T fixed_calling_p[32] = {0};
  if (calling &&
      calling->natureOfAddr == EINSS7_I97_NATIONAL_NO)
  {
    char claddr[32] = {0};
    unpack_addr(claddr, calling->addrSign_p, calling->noOfAddrSign);
    int addrlen = static_cast<int>(strlen(claddr));

    int skip = 0; while(claddr[skip] == '0') skip++;

    if (skip)
    {
      pack_addr(fixed_calling_p, claddr + skip, addrlen - skip);
      fixed_calling.natureOfAddr = calling->natureOfAddr;
      fixed_calling.screening = calling->screening;
      fixed_calling.presentationRestr = calling->presentationRestr;
      fixed_calling.numberPlan = calling->numberPlan;
      fixed_calling.numbIncompl = calling->numbIncompl;
      fixed_calling.noOfAddrSign = addrlen - skip;
      fixed_calling.addrSign_p = fixed_calling_p;
      smsc_log_debug(missedCallProcessorLogger,
                     "modify calling address: %s -> %s",
                     getCallingNumberDescription(calling).c_str(),
                     getCallingNumberDescription(&fixed_calling).c_str());
      calling = &fixed_calling;
    }
  }

  if (calling &&
      calling->noOfAddrSign > 0 &&
      calling->noOfAddrSign <= MAX_FULL_ADDRESS_VALUE_LENGTH)
  {
    /*max length of .ton.npi.digits = 1+3+1+1+1+20 = 27*/
    char cgaddr[32] = {0};
    int pos = snprintf(cgaddr,sizeof(cgaddr),".%d.%d.",calling->natureOfAddr,calling->numberPlan);
    unpack_addr(cgaddr+pos, calling->addrSign_p, calling->noOfAddrSign);

    using smsc::misscall::maskRx;
    using smsc::util::regexp::SMatch;
    std::vector<SMatch> m(maskRx.GetBracketsCount());

    int n=static_cast<int>(m.size());
    if(maskRx.Match(cgaddr,&m[0],n))
    {
      cgaddr[0] = 0;
      if (calling->natureOfAddr == EINSS7_I97_NATIONAL_NO)
      {
        size_t len = strlen(countryCode);
        cgaddr[0] = '+';
        strncpy(&cgaddr[1], countryCode, len);
        unpack_addr(cgaddr+len+1, calling->addrSign_p, calling->noOfAddrSign);
      }
      else if (calling->natureOfAddr == EINSS7_I97_INTERNATIONAL_NO)
      {
        cgaddr[0] = '+';
        unpack_addr(cgaddr+1, calling->addrSign_p, calling->noOfAddrSign);
      }
      /*
       * FIX: allow SUBSCRIBER and UNKNOWN type of calling address
       * For example prefix of PBX number
       * Change request from MTS MR URAL
       */
      else if (calling->natureOfAddr == EINSS7_I97_SUBSCRIBER_NO ||
               calling->natureOfAddr == EINSS7_I97_NA_UNKNOWN)
      {
        unpack_addr(cgaddr, calling->addrSign_p, calling->noOfAddrSign);
      }

      event.from = cgaddr;
      if ( calling->presentationRestr == EINSS7_I97_PRES_ALLOWED ) {
        event.flags = NON_ANTI_AON_FOR_CALLER;
      } else {
        event.flags = ANTI_AON_FOR_CALLER;
      }
    }
  }
  /* skip prohibited or unknown caller */
  if (relCauses.skipUnknownCaller && event.from.length() == 0)
  {
    return;
  }
  if (called &&
      called->noOfAddrSign > 0 &&
      called->noOfAddrSign <= MAX_FULL_ADDRESS_VALUE_LENGTH)
  {
    char cdaddr[32] = {0};
    int pos = snprintf(cdaddr,sizeof(cdaddr),".%d.%d.",called->natureOfAddr,called->numberPlan);
    unpack_addr(cdaddr+pos, called->addrSign_p, called->noOfAddrSign);

    using smsc::misscall::calledMaskRx;
    using smsc::util::regexp::SMatch;
    std::vector<SMatch> m(calledMaskRx.GetBracketsCount());

    int n=static_cast<int>(m.size());
    if(calledMaskRx.Match(cdaddr,&m[0],n))
    {
      cdaddr[0] = 0;
      if (called->natureOfAddr == EINSS7_I97_NATIONAL_NO)
      {
        size_t len = strlen(countryCode);
        cdaddr[0] = '+';
        strncpy(&cdaddr[1], countryCode, len);
        unpack_addr(cdaddr+len+1, called->addrSign_p, called->noOfAddrSign);
      }
      else if (called->natureOfAddr == EINSS7_I97_INTERNATIONAL_NO)
      {
        cdaddr[0] = '+';
        unpack_addr(cdaddr+1, called->addrSign_p, called->noOfAddrSign);
      }
      event.to = cdaddr;
      event.gotFromIAMSME = false;
      smsc_log_debug(missedCallProcessorLogger,"send event: %s->%s, flags=0x%x, type=0x%x",event.from.c_str(),event.to.c_str(), event.flags, event.cause);
      MissedCallProcessor::instance()->fireMissedCallEvent(event);
    }
  }
}
void taif(
          EINSS7_I97_ISUPHEAD_T *head,
          EINSS7_I97_CALLINGNUMB_T *calling,
          EINSS7_I97_CALLEDNUMB_T *called,
          EINSS7_I97_ORIGINALNUMB_T *original,
          EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
          EINSS7_I97_REDIRECTINGNUMB_T *redirecting
         )
{
  UCHAR_T causeValue = relCauses.otherCause; /* must be 121 */
  UCHAR_T inform = 0;        /* need to register event */
  uint8_t eventType = NONE;
  if ( called &&
       called->noOfAddrSign >=2 &&
       called->addrSign_p !=0 )
  {
    char prefix[3] = {0};
    unpack_addr(prefix, called->addrSign_p, 2);

    /* prefix 21 means detach IMSI */
    if (strncmp(prefix,"21",2) == 0 )
    {
      causeValue = relCauses.detachCause;
      eventType  = DETACH;
      inform     = relCauses.detachInform;
    }

    /* prefix 22 means absent subscriber */
    if (strncmp(prefix,"22",2) == 0 )
    {
      causeValue = relCauses.absentCause;
      eventType  = ABSENT;
      inform     = relCauses.absentInform;
    }

    /* prefix 23 means busy */
    if (strncmp(prefix,"23",2) == 0 )
    {
      causeValue = relCauses.busyCause;
      eventType  = BUSY;
      inform     = relCauses.busyInform;
    }
  }
  releaseConnection(head,causeValue);
  if (inform)
  {
    char cdaddr[32] = {0};
    UCHAR_T tmpcdaddr[32] = {0};
    unpack_addr(cdaddr, called->addrSign_p, called->noOfAddrSign);
    pack_addr(tmpcdaddr, cdaddr + 2, static_cast<int>(strlen(cdaddr) - 2));

    EINSS7_I97_CALLEDNUMB_T tmpcalled;
    tmpcalled.natureOfAddr = called->natureOfAddr;
    tmpcalled.numberPlan = called->numberPlan;
    tmpcalled.internalNetwNumb = called->internalNetwNumb;
    tmpcalled.noOfAddrSign = static_cast<UCHAR_T>(strlen(cdaddr) - 2);
    tmpcalled.addrSign_p = tmpcdaddr;

    registerEvent(calling,&tmpcalled,eventType);
  }
}
void mts(
         EINSS7_I97_ISUPHEAD_T *head,
         EINSS7_I97_CALLINGNUMB_T *calling,
         EINSS7_I97_CALLEDNUMB_T *called,
         EINSS7_I97_ORIGINALNUMB_T *original,
         EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
         EINSS7_I97_REDIRECTINGNUMB_T *redirecting
        )
{
  UCHAR_T causeValue = relCauses.otherCause;
  UCHAR_T inform = 0;        /* need to register event */
  uint8_t eventType = NONE;

  /*
   * some exchange doesn't provide redirection information
   * so let's use presence of redirecting number as
   * call forward/divert indication
   */
  if (redirecting)
  {
    inform = relCauses.otherInform;
    if (redirectionInfo_sp)
    {
      switch (redirectionInfo_sp->lastReason)
      {
        case EINSS7_I97_USER_BUSY:
          causeValue = relCauses.busyCause;
          eventType  = BUSY;
          inform     = relCauses.busyInform;
          break;
        case EINSS7_I97_NO_REPLY:
          causeValue = relCauses.noReplyCause;
          eventType  = NOREPLY;
          inform     = relCauses.noReplyInform;
          break;
        case EINSS7_I97_UNCOND:
          causeValue = relCauses.unconditionalCause;
          eventType  = UNCOND;
          inform     = relCauses.unconditionalInform;
          break;
        case EINSS7_I97_MOB_NOT_REACHED:
          causeValue = relCauses.absentCause;
          eventType  = ABSENT;
          inform     = relCauses.absentInform;
          break;
        default:
          causeValue = relCauses.otherCause;
          inform     = relCauses.otherInform;
          break;
      }
    }
  }
  releaseConnection(head,causeValue);


  if (redirecting && inform)
  {
    registerEvent(calling,redirecting,eventType);
  }

}
void redirect_rule(
         EINSS7_I97_ISUPHEAD_T *head,
         EINSS7_I97_CALLINGNUMB_T *cg,
         EINSS7_I97_CALLEDNUMB_T *called,
         EINSS7_I97_ORIGINALNUMB_T *original,
         EINSS7_I97_REDIRECTIONINFO_T *redirinf,
         EINSS7_I97_REDIRECTINGNUMB_T *redirecting
        )
{
  UCHAR_T causeValue = relCauses.otherCause;
  UCHAR_T inform = 0;        /* need to register event */
  uint8_t eventType = NONE;

  /*
   * some exchange doesn't provide redirection information
   * so let's use presence of redirecting number as
   * call forward/divert indication
   */

  if (redirecting)
  {
    inform = relCauses.otherInform;
    if (redirinf)
    {
      switch (redirinf->lastReason)
      {
        case EINSS7_I97_USER_BUSY:       eventType  = BUSY;    break;
        case EINSS7_I97_NO_REPLY:        eventType  = NOREPLY; break;
        case EINSS7_I97_UNCOND:          eventType  = UNCOND;  break;
        case EINSS7_I97_MOB_NOT_REACHED: eventType  = ABSENT;  break;
      }

      /* FILL CALL INFO STRING */
      /* format of call string: 'spn,cause,a-number */
      /*max length of 'spn,cause,.ton.npi.digits' = 3+1+3+1+1+3+1+1+1+20 = 35*/
      char callinfo[36] = {0};int pos;
      pos = snprintf(callinfo,sizeof(callinfo),
                     "%d,%d,",
                     head->span,
                     redirinf->lastReason);
      if (cg)
      {
        pos += snprintf(callinfo+pos,sizeof(callinfo),
                        ".%d.%d.",
                        cg->natureOfAddr,
                        cg->numberPlan);
        unpack_addr(callinfo+pos, cg->addrSign_p, cg->noOfAddrSign);
      }

      vector<InternalRule*>::iterator it = rules.begin(),end = rules.end();
      for(;it != end;++it)
      {
        InternalRule* rule = *it;
        if (rule->match(&callinfo[0],&causeValue,&inform))
        {
          smsc_log_debug(missedCallProcessorLogger,
                         "match rule \"%s\" cause=%d inform=%d",
                          rule->rule.name.c_str(),
                          causeValue,
                          inform);
          break;
        }
      }
      if( it == end)
      {
        causeValue = relCauses.otherCause;
        inform     = relCauses.otherInform;
      }
    }
  }
  releaseConnection(head,causeValue);


  if (redirecting && inform)
  {
    registerEvent(cg,redirecting,eventType);
  }

}

void taifmixed(
          EINSS7_I97_ISUPHEAD_T *head,
          EINSS7_I97_CALLINGNUMB_T *calling,
          EINSS7_I97_CALLEDNUMB_T *called,
          EINSS7_I97_ORIGINALNUMB_T *original,
          EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
          EINSS7_I97_REDIRECTINGNUMB_T *redirecting
         )
{
  UCHAR_T causeValue = relCauses.otherCause;
  if (redirecting)
  {
    mts(head,calling,called,original,redirectionInfo_sp,redirecting);
  }
  else
  {
    taif(head,calling,called,original,redirectionInfo_sp,redirecting);
  }
}
USHORT_T EINSS7_I97IsupSetupInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                USHORT_T resourceGroup,
                                EINSS7_I97_NATUREOFCONN_T *natureOfConn_sp,
                                EINSS7_I97_FORWARD_T *forward_sp,
                                UCHAR_T *callingPartyCat_p,
                                UCHAR_T *transmissionReq_p,
                                EINSS7_I97_CALLEDNUMB_T *called,
                                EINSS7_I97_CALLINGNUMB_T *calling,
                                EINSS7_I97_OPTFORWARD_T *optForward_sp,
                                EINSS7_I97_ACCESS_T *access_sp,
                                EINSS7_I97_REDIRECTINGNUMB_T *redirecting,
                                EINSS7_I97_REDIRECTIONINFO_T *redirinfo,
                                EINSS7_I97_ORIGINALNUMB_T *original,
                                EINSS7_I97_USERINFORMATION_T *userInformation_sp,
                                EINSS7_I97_USERSERVICE_T *userService_sp,
                                EINSS7_I97_OPTPARAMS_T *extraOpts)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "IsupSetupInd %s RG=%d %s %s %s %s %s %s",
                 getHeadDescription(isupHead_sp).c_str(),
                 resourceGroup,
                 getCalledNumberDescription(called).c_str(),
                 getCallingNumberDescription(calling).c_str(),
                 getRedirectionInfoDescription(redirinfo).c_str(),
                 getRedirectingNumberDescription(redirecting).c_str(),
                 getOriginalNumberDescription(original).c_str(),
                 getExtraOptionsDescription(extraOpts).c_str()
                );
  strategy(isupHead_sp,calling,called,original,redirinfo,redirecting);
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupReleaseConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                   EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "ReleaseConf %s",
                 getHeadDescription(isupHead_sp).c_str());
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T confirmReleaseConnection(EINSS7_I97_ISUPHEAD_T* isupHead)
{
  USHORT_T res;
  res = EINSS7_I97IsupReleaseResp(isupHead,
                                  0  /* extraOpts_sp */
                                 );
  if (res != 0)
  {
    detectDisconnect( res, __func__ );
    smsc_log_error(
                   missedCallProcessorLogger,
                   "IsupReleaseResp %s failed with error code %d(%s)",
                   getHeadDescription(isupHead).c_str(),
                   res,getReturnCodeDescription(res)
                  );
  }
  return res;
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
  smsc_log_debug(missedCallProcessorLogger,
                 "ReleaseInd %s %s %s %s %s %s %s",
                 getHeadDescription(isupHead_sp).c_str(),
                 getResponseIndicatorDescription(responseInd).c_str(),
                 getSourceIndicatorDescription(sourceInd).c_str(),
                 getCauseDescription(cause_sp).c_str(),
                 getAutoCongLevelDescription(autoCongestLevel_p).c_str(),
                 getRedirectionNumberDescription(redirectionNumb_sp).c_str(),
                 getRedirectionInfoDescription(redirectionInfo_sp).c_str()
                );
  if (responseInd & 0x01)
  {
    confirmReleaseConnection(isupHead_sp);
  }
  return EINSS7_I97_REQUEST_OK;
}
/*********************************************************************/
/* MGMT CALLBACKS                                                    */
/*-------------------------------------------------------------------*/
/* BindConf                                                          */
/* OrderConf                                                         */
/* MgmtConf                                                          */
/* ResourceInd                                                       */
/*********************************************************************/
using smsc::misscall::util::getResultDescription;
using smsc::misscall::util::getStackStatusDescription;
using smsc::misscall::util::getModuleName;
USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,
                                      UCHAR_T result,
                                      UCHAR_T mmState,
                                      UCHAR_T xmRevision)
{
  smsc_log_debug(missedCallProcessorLogger,
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
        changeState(STACKOK);
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
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,
                                       UCHAR_T orderId,
                                       UCHAR_T orderResult,
                                       UCHAR_T resultInfo,
                                       UCHAR_T lengthOfInfo,
                                       UCHAR_T *orderInfo)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "MgmtOrderConf %s ORDER %d %s",
                 getModuleName(moduleId),
                 orderId,
                 getResultDescription(orderResult));
  if ( moduleId == ISUP_ID &&
       orderId == 0x0B /*UNBLOCK CURCUITS*/ && lengthOfInfo && orderInfo)
  {
    int span = orderInfo[1];
     Connection *conn = 0;
     conn = connections.GetPtr(span);
     if(conn)
       conn->finish(orderResult == 0); /* Success */
  }
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,
                                      USHORT_T length,
                                      UCHAR_T *data_p)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "MgmtConf Service=%s DataLength=%d",
                  getTypeOfServiceDescription(typeOfService).c_str(),
                  length);
  if ( state == WAITINGSTACK &&
       typeOfService == 9 /* type = "Stack State Query (SSQ)" */)
  {
    cancelTimer(&conftimer);
    if (*data_p == 4)       /* StackState = "Running" */
    {
      changeState(STACKOK);
      cancelTimer(&stacktimer);
    }
    else
    {
      changeState(MGMTBOUND);
      setTimer(&waitstacktimer,1500);
    }
  }
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_I97IsupResourceInd(USHORT_T resourceGroup,
                                   UCHAR_T rgstate)
{
  smsc_log_debug(missedCallProcessorLogger, "ResourceInd RG=%d is %s",resourceGroup,rgstate?"available":"unavailable");
  if ( rgstate )
  {
    int span = resourceGroup;
     Connection *conn = 0;
     conn = connections.GetPtr(span);
     if(conn)
       conn->unblock(); /* Success */
  }
  return EINSS7_I97_REQUEST_OK;
}

void _mprint_msg(MSG_T* message)
{
  char *text = new char[message->size*4+1];
  int k = 0;
  for ( int i=0; i<message->size; i++){
    k+=sprintf(text+k,"%02x ",(unsigned)message->msg_p[i]);
  }
  text[k]=0;
  smsc_log_debug(missedCallProcessorLogger,
                 "msg: %s",
                 text);
  delete text;
}
