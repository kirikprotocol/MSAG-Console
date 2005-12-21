#include "Processor.h"
#include "clbks.hpp"
#include "util.hpp"
#include "logger/Logger.h"
#include "MTRequest.hpp"
#include "TCO.hpp"

using namespace std;
namespace smsc{
namespace mtsmsme{
namespace processor{

using namespace smsc::mtsmsme::processor::decode;
using namespace smsc::mtsmsme::processor::encode;

class MtSmsProcessor : public RequestProcessor{
  public:
    virtual void setRequestSender(RequestSender* sender);
    virtual int Run();
    virtual void Stop();
    TCO*    getCoordinator();
    MtSmsProcessor();
    ~MtSmsProcessor();
  private:
    RequestSender* sender;
    TCO* coordinator;

};

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::mtsmsme::processor::util::getReturnCodeDescription;

#define MAXENTRIES 1000
#define USER USER04_ID
#define MAXSEGM 272
#define SSN 191
#define TCINST 0
#define MGMT_VER 6
#define MCIERROR 999
int going;

static MtSmsProcessor* volatile processor = 0;
Logger* MtSmsProcessorLogger = 0;
Mutex lock;


RequestProcessor* RequestProcessor::getInstance()
{
  if ( processor == 0 ) {
    MutexGuard g(lock);
    if ( processor == 0 )
    {
      processor = new MtSmsProcessor();
    }
  }
  return processor;
}

MtSmsProcessor::MtSmsProcessor()
{
  MtSmsProcessorLogger = Logger::getInstance("mt.sme.pr");
  smsc_log_debug(MtSmsProcessorLogger,"\n**********************\n* SIBINCO MT SMS SME *\n**********************");
  sender = 0;
  coordinator = new TCO(10);
}
MtSmsProcessor::~MtSmsProcessor()
{
  delete(coordinator);
}
TCO     *MtSmsProcessor::getCoordinator() {return coordinator;}

void MtSmsProcessor::setRequestSender(RequestSender* _sender)
{
  MutexGuard g(lock);
  coordinator->setRequestSender(_sender);
}


void MtSmsProcessor::Stop()
{
  going = 0;
}
/*
 * @-> INIT -> MGMTBINDING --> MGMTBOUND
 *                         |
 *                         --> STACKOK
 * --> MGMTBOUND -> WAITINGSTACK --> STACKOK
 *  |                            |
 *  --------------<---------------
 * --> STACKOK -> SCCPBINDING -> WORKING
 */
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

std::string getStateDescription(State state)
{
  switch(state)
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
  }
}

static void changeState(State nstate)
{
  state = nstate;
  smsc_log_debug(MtSmsProcessorLogger,
                 "RequestProcessor:%s",
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

int MtSmsProcessor::Run()
{
  USHORT_T result;
  USHORT_T extresult;

  result = EINSS7CpMsgInitNoSig(MAXENTRIES);
  if (result != 0) {
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgInit Failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msginit;
  }

  result = MsgOpen(USER);
  if (result != 0) {
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgOpen failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgexit;
  }

  result = MsgConn(USER,SCCP_ID);
  if (result != 0) {
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgConn to SCCP failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msgclose;
  }
  result = MsgConn(USER,MGMT_ID);
  if (result != 0) {
    smsc_log_error(MtSmsProcessorLogger,
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
        result = EINSS7_MgmtApiSendBindReq(USER,MGMT_ID,MGMT_VER,NO_WAIT);
        if( EINSS7_MGMTAPI_REQUEST_OK == result)
        {
          changeState(MGMTBINDING);
          setTimer(&conftimer,MAXCONFTIME);
        }
        else
        {
          smsc_log_error(MtSmsProcessorLogger,
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
          smsc_log_error(MtSmsProcessorLogger,
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
            smsc_log_error(MtSmsProcessorLogger,
                           "EINSS7_MgmtApiSendMgmtReq failed with code %d(%s)",
                           result,getReturnCodeDescription(result));
            goto unbindmgmt;
          }
        }
        break;
      case STACKOK:
        result = EINSS7_I96SccpBindReq(SSN,USER,MAXSEGM);
        if( result != EINSS7_I96SCCP_REQUEST_OK )
        {
          smsc_log_error(MtSmsProcessorLogger,
                         "EINSS7_I96SccpBindReq() failed with code %d(%s)",
                         result,getReturnCodeDescription(result));
          goto unbindmgmt;
        }
        else
        {
          changeState(SCCPBINDING);
          setTimer(&conftimer,MAXCONFTIME);
        }
        break;
      /*
       * Timer events
       */
      case MGMTBINDING:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(MtSmsProcessorLogger,
                         "EINSS7_MgmtApiSendBindReq() confirmation timer is expired");
          result = MCIERROR;
          goto unbindmgmt; /* just in case */
        }
        break;
      case WAITINGSTACK:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(MtSmsProcessorLogger,
                         "EINSS7_MgmtApiSendMgmtReq() confirmation timer is expired");
          result = MCIERROR;
          goto unbindmgmt;
        }
        break;
      case SCCPBINDING:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(MtSmsProcessorLogger,
                         "EINSS7_I97IsupBindReg() confirmation timer is expired");
          result = MCIERROR;
          goto unbindSCCP; /* just in case */
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

    message.receiver = USER;
#if EINSS7_THREADSAFE == 1
    result = EINSS7CpMsgRecv_r(&message,1000);
#else
    result = MsgRecvEvent( &message, 0, 0, 1000 );
#endif

    if( result == MSG_TIMEOUT ) {
      continue;
    }
    if( result != MSG_OK ) {
      smsc_log_error(MtSmsProcessorLogger,
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
        case SCCP_ID:
          result = EINSS7_I96SccpHandleInd(&message);
          if( EINSS7_I96SCCP_REQUEST_OK != result )
          {
            smsc_log_error(MtSmsProcessorLogger,
                           "SCCP callback function return code %d(%s)",
                           result,getReturnCodeDescription(result));
          }
          break;
        case MGMT_ID:
          result = EINSS7_MgmtApiReceivedXMMsg(&message);
          if( EINSS7_MGMTAPI_RETURN_OK != result )
          {
            smsc_log_error(MtSmsProcessorLogger,
                           "MGMT callback function return code %d(%s)",
                           result,getReturnCodeDescription(result));
          }
          break;
      }
      EINSS7CpReleaseMsgBuffer(&message);
    }
  }

unbindSCCP:
  extresult = EINSS7_I96SccpUnBindReq(SSN);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "EINSS7_I96SccpUnBindReq(%d) failed with code %d(%s)",
                    SSN,extresult,getReturnCodeDescription(extresult));
unbindmgmt:
  extresult = EINSS7_MgmtApiSendUnbindReq(USER,MGMT_ID);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "EINSS7_MgmtApiSendUnbindReq(%d,%d) failed with code %d(%s)",
                    USER,MGMT_ID,extresult,getReturnCodeDescription(extresult));
msgrelmgmt:
  extresult = MsgRel(USER,MGMT_ID);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgRel(%d,%d) failed with code %d(%s)",
                    USER,MGMT_ID,extresult,getReturnCodeDescription(extresult));

msgrelSCCP:
  extresult = MsgRel(USER,SCCP_ID);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgRel(%d,%d) failed with code %d(%s)",
                    USER,SCCP_ID,extresult,getReturnCodeDescription(extresult));
msgclose:
  extresult = MsgClose(USER);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgClose(%d) failed with code %d(%s)",
                    USER,extresult,getReturnCodeDescription(extresult));
msgexit:
  MsgExit();
msginit:
  smsc_log_debug(MtSmsProcessorLogger,"RequestProcessor is down");
  return result;
}
}//namespace processor
}//namespace mtsmsme
}//namespace smsc

/*********************************************************************/
/* SCCP CALLBACKS IMPLEMENTATION                                     */
/*-------------------------------------------------------------------*/
/* IndError                                                          */
/* BindConf                                                          */
/* SccpStateInd                                                      */
/*********************************************************************/
using smsc::mtsmsme::processor::MtSmsProcessorLogger;
using smsc::mtsmsme::processor::SCCPBINDING;
using smsc::mtsmsme::processor::MGMTBINDING;
using smsc::mtsmsme::processor::MGMTBOUND;
using smsc::mtsmsme::processor::MGMTBINDERROR;
using smsc::mtsmsme::processor::WAITINGSTACK;
using smsc::mtsmsme::processor::STACKOK;
using smsc::mtsmsme::processor::SCCPBINDERROR;
using smsc::mtsmsme::processor::WORKING;
using smsc::mtsmsme::processor::state;
using smsc::mtsmsme::processor::conftimer;
using smsc::mtsmsme::processor::stacktimer;
using smsc::mtsmsme::processor::waitstacktimer;
using namespace smsc::mtsmsme::processor::util;
using namespace smsc::mtsmsme::processor::decode;

USHORT_T EINSS7_I96SccpIndError(USHORT_T errorCode,MSG_T *message)
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
    case EINSS7_I96SCCP_IND_UNKNOWN_CODE : reason = "\"Unknown primitive code\"";break;
    case EINSS7_I96SCCP_IND_LENGTH_ERROR : reason = "\"Length error\"";break;
  }
  smsc_log_error(MtSmsProcessorLogger,
                 "EINSS7_I96SccpHandleInd found error $s in msg: %s",
                 reason,
                 msg);
  delete msg;
  return EINSS7_I96SCCP_REQUEST_OK;
}
extern "C"
USHORT_T EINSS7_I96SccpBindConf(UCHAR_T ssn,
                                UCHAR_T result,
                                USHORT_T maxSegmSize)
{
  smsc_log_debug(MtSmsProcessorLogger,
                 "SccpBindConf SSN=%d MAXSEGMSZ=%d STATUS=%s",
                 ssn,
                 maxSegmSize,
                 getSccpBindStatusDescription(result));
  if ( state == SCCPBINDING )
  {
    cancelTimer(&conftimer);
    if (result == EINSS7_I96SCCP_NB_SUCCESS)
    {
      changeState(WORKING);
    }
    else
    {
      changeState(SCCPBINDERROR);
    }
  }
  return EINSS7_I96SCCP_REQUEST_OK;
}
USHORT_T EINSS7_I96SccpStateInd(UCHAR_T ssn,
                                UCHAR_T userstatus,
                                UCHAR_T assn,
                                ULONG_T aspc,
                                ULONG_T lspc)
{
  smsc_log_warn(MtSmsProcessorLogger,
                "SccpStateInd SSN=%d STATUS=%s AFFECTED(SSN=%d,SPC=%ld) LOCAL(SPC=%ld)",
                ssn,
                userstatus==0?"\"User in service\"":"\"User out of service\"",
                assn,aspc,lspc);
  return EINSS7_I96SCCP_REQUEST_OK;
}



USHORT_T EINSS7_I96SccpUnitdataInd(UCHAR_T ssn,
                                   UCHAR_T seq,
                                   UCHAR_T ropt,
                                   UCHAR_T messPriImportance,
                                   UCHAR_T cdlen,
                                   UCHAR_T *cd,
                                   UCHAR_T cllen,
                                   UCHAR_T *cl,
                                   USHORT_T ulen,
                                   UCHAR_T *udp)
{
  smsc_log_debug(MtSmsProcessorLogger,
                "SccpUnitdataInd SSN=%d %s RET=%s Cd(%s) Cl(%s)\ndata[%d]={%s}",
                ssn,
                getSequenceControlDescription(seq),
                getReturnOptionDescription(ropt),
                getAddressDescription(cdlen,cd).c_str(),
                getAddressDescription(cllen,cl).c_str(),
                ulen,dump(ulen,udp).c_str());
  {
    using namespace smsc::mtsmsme::processor;
    MtSmsProcessor *proc = (MtSmsProcessor*)RequestProcessor::getInstance();
    TCO *coordinator = proc->getCoordinator();

    /* fix ssn from 191 to 8 */
    cd[1] = 0x08;

    if ( coordinator )
    {
      coordinator->NUNITDATA(cdlen,cd,cllen,cl,ulen,udp);
    }

  }
  return EINSS7_I96SCCP_REQUEST_OK;
}

/*********************************************************************/
/* MGMT CALLBACKS                                                    */
/*-------------------------------------------------------------------*/
/* BindConf                                                          */
/* MgmtConf                                                          */
/*********************************************************************/
using namespace smsc::mtsmsme::processor::util;
USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,
                                      UCHAR_T result,
                                      UCHAR_T mmState,
                                      UCHAR_T xmRevision)
{
  smsc_log_debug(MtSmsProcessorLogger,
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
USHORT_T EINSS7_MgmtApiHandleMgmtConf(UCHAR_T typeOfService,
                                      USHORT_T length,
                                      UCHAR_T *data_p)
{
  smsc_log_debug(MtSmsProcessorLogger,
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
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,
                                       UCHAR_T orderId,
                                       UCHAR_T orderResult,
                                       UCHAR_T resultInfo,
                                       UCHAR_T lengthOfInfo,
                                       UCHAR_T *orderInfo_p)
{
  smsc_log_debug(MtSmsProcessorLogger,
                 "MgmtOrderConf %s ORDER %d %s",
                 getModuleName(moduleId),
                 orderId,
                 getResultDescription(orderResult));
  return EINSS7_MGMTAPI_RETURN_OK;
}
