static char const ident[] = "$Id$";
#include "Processor.h"
#include "clbks.hpp"
#include "util.hpp"
#include "logger/Logger.h"
#include "MTRequest.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "TCO.hpp"
#include "sms/sms.h"

//delete after
#include "mtsmsme/comp/UpdateLocation.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
//delete above

using namespace std;
namespace smsc{
namespace mtsmsme{
namespace processor{

using namespace smsc::mtsmsme::processor::decode;
using namespace smsc::mtsmsme::processor::encode;
using smsc::sms::Address;
using smsc::sms::AddressValue;

class MtSmsProcessor : public RequestProcessor{
  public:
    virtual void setRequestSender(RequestSender* sender);
    void configure(int user_id, int ssn,Address& msc, Address& vlr, Address& hlr);
    virtual HLROAM* getHLROAM();
    virtual int Run();
    virtual void Stop();
    TCO*    getCoordinator();
    MtSmsProcessor();
    ~MtSmsProcessor();
  private:
    RequestSender* sender;
    TCO* coordinator;
    SubscriberRegistrator* registrator;
};

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::mtsmsme::processor::util::getReturnCodeDescription;

#define MAXENTRIES 1000
#define MAXSEGM 272
#define TCINST 0
#define MGMT_VER 6
#define MCIERROR 999
int going;

static MtSmsProcessor* volatile processor = 0;
Logger* MtSmsProcessorLogger = 0;
Mutex lock;

static UCHAR_T USER = USER04_ID;
static UCHAR_T SSN = 191;
static AddressValue vlrnumber;
static AddressValue mscnumber;
static AddressValue hlrnumber;

void MtSmsProcessor::configure(int user_id, int ssn, Address& msc, Address& vlr, Address& hlr)
{
  USER = user_id; SSN = ssn;
  msc.getValue(mscnumber);
  vlr.getValue(vlrnumber);
  hlr.getValue(hlrnumber);
  registrator->configure(msc,vlr);
}

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
  //smsc_log_debug(MtSmsProcessorLogger,"\n**********************\n* SIBINCO MT SMS SME *\n**********************");
  sender = 0;
  coordinator = new TCO(100,SSN);
  registrator = new SubscriberRegistrator(coordinator);
}
HLROAM* MtSmsProcessor::getHLROAM() { return registrator; }
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
 * @-> INIT -> SCCPBINDING -> WORKING
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
    goto msg_init_error;
  }

  result = MsgOpen(USER);
  if (result != 0) {
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgOpen failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msg_open_error;
  }

  result = MsgConn(USER,SCCP_ID);
  if (result != 0) {
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgConn to SCCP failed with code %d(%s)",
                   result,getReturnCodeDescription(result));
    goto msg_conn_sccp_error;
  }
  MSG_T message;

  going = 1;
  changeState(INIT);
  while (going) {
    switch ( state )
    {
      case INIT:
        result = EINSS7_I96SccpBindReq(SSN,
                                       #ifdef SCCP_R9
                                       0,
                                       #endif 
                                       USER,MAXSEGM);
        if( result != EINSS7_I96SCCP_REQUEST_OK )
        {
          smsc_log_error(MtSmsProcessorLogger,
                         "EINSS7_I96SccpBindReq() failed with code %d(%s)",
                         result,getReturnCodeDescription(result));
          goto msg_rel_sccp;
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
      case SCCPBINDING:
        if (checkTimer(&conftimer))
        {
          smsc_log_error(MtSmsProcessorLogger,
                         "EINSS7_I96SccpBindReq() confirmation timer is expired");
          result = MCIERROR;
          goto unbind_sccp; /* just in case */
        }
        break;
      case SCCPBINDERROR:
        result = MCIERROR;
        goto unbind_sccp;
        break;
      case WORKING:
        registrator->process();
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
      }
      EINSS7CpReleaseMsgBuffer(&message);
    }
  }

unbind_sccp:
  extresult = EINSS7_I96SccpUnBindReq(SSN
                                       #ifdef SCCP_R9
                                       , 0
                                       #endif 
                                      );
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "EINSS7_I96SccpUnBindReq(%d) failed with code %d(%s)",
                    SSN,extresult,getReturnCodeDescription(extresult));
msg_rel_sccp:
  extresult = MsgRel(USER,SCCP_ID);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgRel(%d,%d) failed with code %d(%s)",
                    USER,SCCP_ID,extresult,getReturnCodeDescription(extresult));
msg_conn_sccp_error:
  extresult = MsgClose(USER);
  if( extresult != 0 )
    smsc_log_error(MtSmsProcessorLogger,
                   "MsgClose(%d) failed with code %d(%s)",
                    USER,extresult,getReturnCodeDescription(extresult));
msg_open_error:
  MsgExit();
msg_init_error:
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
using smsc::mtsmsme::processor::util::modifyssn;

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
                                #ifdef SCCP_R9                                                                                       
                                EINSS7INSTANCE_T sccpInstanceId,
                                #endif 
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
        /* 
         * Useless to allow ALREADY_IN_USE treated as OK because get BROKEN CONNECTION on next MsgRecvEvent
         * result == EINSS7_I96SCCP_SSN_ALREADY_IN_USE)
         */
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
                                #ifdef SCCP_R9                                                                                       
                                EINSS7INSTANCE_T sccpInstanceId,
                                #endif 
                                UCHAR_T userstatus,
                                UCHAR_T assn,
                                ULONG_T aspc,
                                ULONG_T lspc)
{
  smsc_log_warn(MtSmsProcessorLogger,
                "SccpStateInd SSN=%d STATUS=%s AFFECTED(SSN=%d,SPC=%d) LOCAL(SPC=%d)",
                ssn,
                userstatus==0?"\"User in service\"":"\"User out of service\"",
                assn,aspc,lspc);
  return EINSS7_I96SCCP_REQUEST_OK;
}



USHORT_T EINSS7_I96SccpUnitdataInd(UCHAR_T ssn,
                                   #ifdef SCCP_R9                                                                                       
                                   EINSS7INSTANCE_T sccpInstanceId,
                                   #endif 
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

    /* fix ssn for configured numbers */
    if (modifyssn(cd, cdlen, mscnumber, 8))
      smsc_log_debug(MtSmsProcessorLogger,
                    "Cd equals to %s, set SSN = 8, new Cd(%s)",
                    mscnumber,getAddressDescription(cdlen,cd).c_str());
    if (modifyssn(cd, cdlen, vlrnumber, 7))
      smsc_log_debug(MtSmsProcessorLogger,
                    "Cd GT equals to %s, set SSN = 7, new Cd(%s)",
                    vlrnumber,getAddressDescription(cdlen,cd).c_str());

    //cd[1] = 0x08;
    if ( coordinator )
    {
      coordinator->NUNITDATA(cdlen,cd,cllen,cl,ulen,udp);
    }

  }
  return EINSS7_I96SCCP_REQUEST_OK;
}
