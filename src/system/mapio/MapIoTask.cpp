#include "MapIoTask.h"
#include "logger/Logger.h"
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

#define MAXENTRIES 600
#define MY_USER_ID USER01_ID
using namespace std;

#ifdef USE_MAP

//#define SMSC_FORWARD_RESPONSE 0x001

static unsigned __global_bind_counter = 0;
static unsigned __pingPongWaitCounter = 0;
static bool MAP_dispatching = false;
static bool MAP_isAlive = false;
static bool MAP_aborting = false;
  #define CORRECT_BIND_COUNTER 2
  #define MAX_BIND_TIMEOUT 15

//struct SMSC_FORWARD_RESPONSE_T {
//  ET96MAP_DIALOGUE_ID_T dialogId;
//};

void MAPIO_TaskACVersionNotifier()
{
  __pingPongWaitCounter = 0;
}
extern void MAPIO_QueryMscVersionInternal();

/*
void CloseDialog( ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  MAPSTATS_Update(MAPSTATS_GSMDIALOG_CLOSE);
  USHORT_T res = Et96MapCloseReq (SSN,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __map_trace2__("close error, code 0x%hx",res);
  }else{
    __map_trace2__("dialog closed");
  }
}

void CloseAndRemoveDialog(  ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  CloseDialog(lssn,dialogId);
  MapDialogContainer::getInstance()->dropDialog(dialogId);
  __trace2__("MAP::dialog 0x%hx destroed",dialogId);
}
*/

extern "C" {

  USHORT_T Et96MapBindConf(ET96MAP_LOCAL_SSN_T lssn, ET96MAP_BIND_STAT_T status)
  {
    __map_trace2__("Et96MapBindConf confirmation received ssn=%d status=%d",lssn,status);
    ++__global_bind_counter;
    return ET96MAP_E_OK;
  }

  USHORT_T Et96MapStateInd (
                           ET96MAP_LOCAL_SSN_T lssn,
                           UCHAR_T userState,
                           UCHAR_T affectedSSN,
                           ULONG_T affectedSPC,
                           ULONG_T localSPC)
  {
    __map_trace2__("Et96MapStateInd received ssn=%d user state=%d affected SSN=%d affected SPC=%ld local SPC=%ld",lssn,userState,affectedSSN,affectedSPC,localSPC);
    return ET96MAP_E_OK;
  }

  void Et96MapIndicationError(USHORT_T error,UCHAR_T* errString)
  {
    if ( errString ) {
      __map_warn2__("Et96MapIndicationError: error 0x%hx text %s",error,errString);
    } else {
      __map_warn2__("Et96MapIndicationError: error 0x%hx",error);
    }
  }

} // extern "C"

void MapIoTask::deinit( bool connected )
{
  USHORT_T result;
  __map_trace__("deinitialize");
  __global_bind_counter = 0;
  MapDialogContainer::destroyInstance();
  if( connected ) {
      result = Et96MapUnbindReq(SSN);
      if ( result != ET96MAP_E_OK) {
  __map_trace2__("error at Et96MapUnbindReq SSN=%d errcode 0x%hx",SSN,result);
      }
      result = Et96MapUnbindReq(USSD_SSN);
      if ( result != ET96MAP_E_OK) {
  __map_trace2__("error at Et96MapUnbindReq SSN=%d errcode 0x%hx",USSD_SSN,result);
      }
      result = MsgRel(MY_USER_ID,ETSIMAP_ID);
      if ( result != MSG_OK) {
        __map_warn2__("error at MsgRel errcode 0x%hx",result);
        if ( !isStopping ) kill(getpid(),17);
        return;
      }
  }
  result = MsgClose(MY_USER_ID);
  if ( result != MSG_OK) {
    __map_warn2__("error at MsgClose errcode 0x%hx",result);
    if ( !isStopping ) kill(getpid(),17);
    return;
  }
  MsgExit();
}

void MapIoTask::dispatcher()
{
  MSG_T message;
  USHORT_T result;
  USHORT_T map_result;
  unsigned timecounter = 0;
  time_t last_msg;
  time_t cur_time;
  struct timeval utime, curtime;
  APP_EVENT_T *eventlist = NULL;
  INT_T        eventlist_len = 0;
  smsc::logger::Logger *time_logger = smsc::logger::Logger::getInstance("map.itime");

  message.receiver = MY_USER_ID;
  int bindTimer = 0;
  for (;;) {
    MAP_isAlive = true;
    if ( isStopping ) {
        deinit(true);
  return;
    }
    MAP_dispatching = true;
    gettimeofday( &curtime, 0 );
#if EINSS7_THREADSAFE == 1
    result = EINSS7CpMsgRecv_r(&message,1000);
#else
    result = MsgRecvEvent( &message, 0, 0, 1000 );
#endif
    if ( time_logger->isDebugEnabled() ) gettimeofday( &utime, 0 );

    MAP_dispatching = false;

    if ( result == MSG_TIMEOUT ) {
      if ( __global_bind_counter == CORRECT_BIND_COUNTER ) continue;
      __map_trace2__("MAP:: check binders %d", bindTimer);
      if ( ++bindTimer <= MAX_BIND_TIMEOUT ) continue;
      __map_warn2__("MAP:: not all binders binded in %d seconds. Restarting...", MAX_BIND_TIMEOUT);
      if ( !isStopping ) kill(getpid(),17);
      continue;
    }
    if ( result == MSG_BROKEN_CONNECTION ) {
      __map_warn2__("Broken connection %d", result);
      if ( !isStopping ) kill(getpid(),17);
      continue;
    }
    if ( result != MSG_OK ) {
      __map_warn2__("Error at MsgRecv with code %d",result);
      if ( !(MAP_aborting || isStopping) ) {
        MAP_aborting = true;
        abort();
      } else {
        return;
      }
    }

    MAPSTATS_Update(MAPSTATS_GSMRECV);

    __map_trace2__("MsgRecv receive msg with receiver 0x%hx sender 0x%hx prim 0x%hx size %d",message.receiver,message.sender,message.primitive,message.size);
    if ( message.primitive == 0x8b && message.msg_p[6] >= 0x04 ) {
      __map_trace__("MsgRecv hatching msg to reset priority order " );
      message.msg_p[6] = 0;
    } else if ( message.primitive == 0x8d && message.msg_p[4] >= 0x04 ) {
      __map_trace__("MsgRecv hatching msg to reset priority order " );
      message.msg_p[4] = 0;
    }
    if( message.primitive == 0x88 ) {
      // MapOpenInd
      if ( smsc::logger::_map_cat->isDebugEnabled() ) {
        {
          char *text = new char[message.size*4+1];
          int k = 0;
          for ( int i=0; i<message.size; i++) {
            k+=sprintf(text+k,"%02x ",(unsigned)message.msg_p[i]);
          }
          text[k]=0;
      __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, "Decoding openInd: %s",text);
          delete text;
        }
      }
      //
      const int destAddrPos = 6;
      const int destRefPos = destAddrPos+message.msg_p[destAddrPos]+1;
      __map_trace2__("destRefPos = %d", destRefPos);
      const int orgAddrPos = destRefPos+message.msg_p[destRefPos]+1;
      __map_trace2__("orgAddrPos = %d", orgAddrPos);
      const int orgRefPos = orgAddrPos+message.msg_p[orgAddrPos]+1;
      __map_trace2__("orgRefPos = %d", orgRefPos);
      const int specificInfoLenPos = orgRefPos+(message.msg_p[orgRefPos]+1)/2+1+(message.msg_p[orgRefPos]?1:0);
      __map_trace2__("specificInfoLenPos = %d", specificInfoLenPos);
      ET96MAP_USERDATA_T specificInfo;
      specificInfo.specificInfoLen = ((USHORT_T)message.msg_p[specificInfoLenPos])|(((USHORT_T)message.msg_p[specificInfoLenPos+1])<<8);
      __map_trace2__("specificInfo.specificInfoLen = %d", specificInfo.specificInfoLen);
      if( specificInfo.specificInfoLen > 0 ) {
        memcpy(specificInfo.specificData, message.msg_p+specificInfoLenPos+2, specificInfo.specificInfoLen );
      }
      const int ctx[2] = {(int)message.msg_p[4],(int)message.msg_p[5]};
      map_result = Et96MapOpenInd(
        (ET96MAP_LOCAL_SSN_T)message.msg_p[1], // SSN
        ((ET96MAP_DIALOGUE_ID_T)message.msg_p[2])|(((ET96MAP_DIALOGUE_ID_T)message.msg_p[3])<<8), // Dialogue ID
        (ET96MAP_APP_CNTX_T*)ctx, // AC version
        (message.msg_p[destAddrPos]>0)?(ET96MAP_SS7_ADDR_T*)(message.msg_p+destAddrPos):0, // dest ss7 addr
        (message.msg_p[orgAddrPos]>0)?(ET96MAP_SS7_ADDR_T*)(message.msg_p+orgAddrPos):0, // org ss7 addr
#ifdef MAP_R12
        (message.msg_p[destRefPos]>0)?(ET96MAP_IMSI_OR_MSISDN_T*)(message.msg_p+destRefPos):0, // dest ref
#else
        (message.msg_p[destRefPos]>0)?(ET96MAP_IMSI_T*)(message.msg_p+destRefPos):0, // dest ref
#endif
        (message.msg_p[orgRefPos]>0)?(ET96MAP_ADDRESS_T*)(message.msg_p+orgRefPos):0, // dest ref
        (specificInfo.specificInfoLen>0)?&specificInfo:0
      );
    } else {
      map_result = Et96MapHandleIndication(&message);

    }
    if ( map_result != ET96MAP_E_OK && smsc::logger::_map_cat->isWarnEnabled() ) {
      {
        char *text = new char[message.size*4+1];
        int k = 0;
        for ( int i=0; i<message.size; i++) {
          k+=sprintf(text+k,"%02x ",(unsigned)message.msg_p[i]);
        }
        text[k]=0;
        __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN, "error at Et96MapHandleIndication with code x%hx msg: %s",map_result,text);
        delete text;
      }
    }
#if EINSS7_THREADSAFE == 1
    EINSS7CpReleaseMsgBuffer(&message);
#endif
    if ( time_logger->isDebugEnabled() ) {
      long usecs;
      gettimeofday( &curtime, 0 );
      usecs = curtime.tv_usec < utime.tv_usec?(1000000+curtime.tv_usec)-utime.tv_usec:curtime.tv_usec-utime.tv_usec;
      smsc_log_debug(time_logger, "prim=%d s=%ld us=%ld", message.primitive, curtime.tv_sec-utime.tv_sec, usecs );
    }
  }
}

void MapIoTask::init(unsigned timeout)
{
  USHORT_T err;
  __global_bind_counter = 0;
  __pingPongWaitCounter = 0;
  err = EINSS7CpMsgInitNoSig(MAXENTRIES);
//  err = MsgInit(MAXENTRIES);
  if ( err != MSG_OK ) {
    __map_warn2__("MAP: Error at MsgInit, code 0x%hx",err); throw runtime_error("MsgInit error");
  }
  err = MsgOpen(MY_USER_ID);
  if ( err != MSG_OK ) {
    __map_warn2__("MAP: Error at MsgOpen, code 0x%hx",err); throw runtime_error("MsgOpen error");
  }
  err = MsgConn(MY_USER_ID,ETSIMAP_ID);
  if ( err != MSG_OK ) {
    __map_warn2__("MAP: Error at MsgConn, code 0x%hx",err); throw runtime_error("MsgConn error");
  }
  __map_trace__("MAP:: pause self and wait map initialization");
  sleep(timeout);
  __map_trace__("MAP:: continue self initialization");
//  err = MsgConn(USER01_ID,USER01_ID);
//  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgConn on self, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  __map_trace__("Bind");
  USHORT_T bind_res = Et96MapBindReq(MY_USER_ID, SSN);
  if (bind_res!=ET96MAP_E_OK) {
    __map_trace2__("SSN Bind error 0x%hx",bind_res);
    throw runtime_error("bind error");
  }
  bind_res = Et96MapBindReq(MY_USER_ID, USSD_SSN);
  if (bind_res!=ET96MAP_E_OK) {
    __map_trace2__("USSD Bind error 0x%hx",bind_res);
    throw runtime_error("bind error");
  }
  MAPSTATS_Restart();
  __map_trace__("Ok");
}

#else
void MapIoTask::deinit(bool)
     {
  __map_trace__("MapIoTask::deinit: no map stack on this platform");
}

void MapIoTask::dispatcher()
{
  Event e;
  __map_trace__("MapIoTask::dispatcher: no map stack on this platform");
  e.Wait();
}

void MapIoTask::init(unsigned)
{
  __map_trace__("MapIoTask::init: no map stack on this platform");
}

#endif

MapDialogContainer* MapDialogContainer::container = 0;
Mutex MapDialogContainer::sync_object;

int MapIoTask::Execute(){
  try {
    try {
      init();
    } catch (exception& e) {
      __map_warn2__("exception in mapio, restarting: %s",e.what());
      kill(getpid(),9);
    }
    is_started = true;
    __trace2__("signal mapiotask start:%p",startevent);
    startevent->SignalAll();
    dispatcher();
  } catch (exception& e) {
    __map_warn2__("exception in mapio: %s",e.what());
  }
  return 0;
}

string MapDialogContainer::SC_ADRESS_VALUE = "79029869999";
string MapDialogContainer::USSD_ADRESS_VALUE = "79029869998";
ET96MAP_LOCAL_SSN_T MapDialogContainer::ussdSSN = 6;
MapProxy* MapDialogContainer::proxy = 0;

void MapDialogContainer::abort()
{
#ifdef USE_MAP
  MAP_aborting = true;
  EINSS7CpMsgClean();
#endif
}

int MapTracker::Execute(){
  Event e;
#ifdef USE_MAP
  for (;;) {
    time_t t = time(0);
    while (time(0)<(t+15)&&!isStopping) {
      time_t xx = time(0);
      if ( xx > t+15 ) break;
      e.Wait(1000*(t-xx+15));
    }
    __map_trace2__("MAP tracker:: alive %d dispatching %d",MAP_isAlive,MAP_dispatching);
    if ( isStopping ) return 0;
    if ( MAP_dispatching && !MAP_isAlive ) {
      __map_warn__("\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                    "MAP is DEAD"
                    "\n\n\n\n");
      abort();
    } else MAP_isAlive = false;
  }
#else
  e.Wait();
#endif
  return 0;
}

void MapDialogContainer::registerSelf(SmeManager* smeman)
{
  proxy->init();
  __map_trace__("register MAP_PROXY");
//#if defined USE_MAP // !!!! temporary !!!!!
//  smeman->registerSmeProxy("MAP_PROXY",proxy);
//#else
  smeman->registerInternallSmeProxy("MAP_PROXY",proxy);
  proxy->assignSmeRegistrar(smeman);
//#endif
  __map_trace__("register MAP_PROXY OK");
}

void MapDialogContainer::unregisterSelf(SmeManager* smeman)
{
  //proxy.init();
  __map_trace__("unregister MAP_PROXY");
//#if defined USE_MAP // !!!! temporary !!!!!
//  smeman->registerSmeProxy("MAP_PROXY",&proxy);
//#else
  smeman->unregisterSmeProxy("MAP_PROXY");
//#endif
  __map_trace__("unregister MAP_PROXY OK");
}


Mutex& MAPSTATS_GetMutex(){
  static Mutex mutex;
  return mutex;
}

smsc::logger::Logger* MAPSTATS_GetLoggerSec() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.sec");
  return logger;
}
smsc::logger::Logger* MAPSTATS_GetLoggerMin() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.min");
  return logger;
}
smsc::logger::Logger* MAPSTATS_GetLoggerHour() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.hour");
  return logger;
}
smsc::logger::Logger* MAPSTATS_GetLoggerDlg() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.dlg");
  return logger;
}

static time_t MAPSTATS_last_time_sec = 0;
static time_t MAPSTATS_last_time_min = 0;
static time_t MAPSTATS_last_time_hour = 0;

int MAPSTATS_open_in[3] = {0,0,0};
int MAPSTATS_open_out[3] = {0,0,0};
int MAPSTATS_close_in[3] = {0,0,0};
int MAPSTATS_close_out[3] = {0,0,0};
int MAPSTATS_recv[3] = {0,0,0};
int MAPSTATS_dialogs_no = 0;
int MAPSTATS_reassign[3] = {0,0,0};

enum {
  MAPSTATS__SEC,
  MAPSTATS__MIN,
  MAPSTATS__HOUR
};

void MAPSTATS_DumpDialogLC(MapDialog* dialog)
{
  struct timeval tv;
  gettimeofday( &tv, 0 );
  long long maked_mks = dialog->maked_at_mks;
  long long mks = ((long long)tv.tv_sec)*1000*1000 + (long long)tv.tv_usec;
  long long cl = mks-maked_mks;
  smsc_log_info(MAPSTATS_GetLoggerDlg(), "dlg=%p (%x/%x) sec=%ld usec=%ld src=%s dst=%s",
                                dialog,dialog->dialogid_map,dialog->dialogid_smsc,
                                long(cl/(1000*1000)),long(cl%(1000*1000)),
                                dialog->sms.get()?dialog->sms->getOriginatingAddress().value:"???",
                                dialog->sms.get()?dialog->sms->getDestinationAddress().value:"???");
}

void MAPSTATS_Flush(unsigned x,bool dump)
{
  if ( dump ) {
    switch ( x ) {
    case MAPSTATS__SEC:
      {
        smsc::logger::Logger* log = MAPSTATS_GetLoggerSec();
        smsc_log_info(log, "op(i/o) %d/%d, clo(i/o) %d/%d, dlg %d/%d, rcv %d",
                  MAPSTATS_open_in[0],
                  MAPSTATS_open_out[0],
                  MAPSTATS_close_in[0],
                  MAPSTATS_close_out[0],
                  MAPSTATS_dialogs_no,
                  MapDialogContainer::getInstance()->getDialogCount(),
                  MAPSTATS_recv[0]
                 );
      }
      break;
    case MAPSTATS__MIN:
      {
        smsc::logger::Logger* log = MAPSTATS_GetLoggerMin();
        smsc_log_info(log, "op(i/o) %d/%d, clo(i/o) %d/%d, dlg %d, rcv %d",
                  MAPSTATS_open_in[1],
                  MAPSTATS_open_out[1],
                  MAPSTATS_close_in[1],
                  MAPSTATS_close_out[1],
                  MAPSTATS_dialogs_no,
                  MAPSTATS_recv[1]
                 );
      }
      break;
    case MAPSTATS__HOUR:
      {
        smsc::logger::Logger* log = MAPSTATS_GetLoggerHour();
        smsc_log_info(log, "op(i/o) %d/%d, clo(i/o) %d/%d, dlg %d, rcv %d",
                  MAPSTATS_open_in[2],
                  MAPSTATS_open_out[2],
                  MAPSTATS_close_in[2],
                  MAPSTATS_close_out[2],
                  MAPSTATS_dialogs_no,
                  MAPSTATS_recv[2]
                 );
      }
      break;
    }
  }
  int from = 0, to = 0;
  switch ( x ) {
  case MAPSTATS__SEC: from = 0; to = 1; break;
  case MAPSTATS__MIN: from = 1; to = 2; break;
  case MAPSTATS__HOUR: from = 2; to = -1; break;
  }
  if ( to != -1 ) {
    MAPSTATS_open_in[to] += MAPSTATS_open_in[from];
    MAPSTATS_open_out[to] += MAPSTATS_open_out[from];
    MAPSTATS_close_in[to] += MAPSTATS_close_in[from];
    MAPSTATS_close_out[to] += MAPSTATS_close_out[from];
    MAPSTATS_recv[to] += MAPSTATS_recv[from];
  }
  MAPSTATS_open_in[from] = 0;
  MAPSTATS_open_out[from] = 0;
  MAPSTATS_close_in[from] = 0;
  MAPSTATS_close_out[from] = 0;
  MAPSTATS_recv[from] = 0;
}

void MAPSTATS_Update_(MAPSTATS stats)
{
  switch ( stats ) {
  case MAPSTATS_GSMDIALOG_OPENIN:   ++MAPSTATS_open_in[0]; break;
  case MAPSTATS_GSMDIALOG_OPENOUT:  ++MAPSTATS_open_out[0]; break;
  case MAPSTATS_GSMDIALOG_CLOSEIN:  ++MAPSTATS_close_in[0]; break;
  case MAPSTATS_GSMDIALOG_CLOSEOUT: ++MAPSTATS_close_out[0]; break;
  case MAPSTATS_GSMRECV:            ++MAPSTATS_recv[0]; break;
  case MAPSTATS_DISPOSEDIALOG:      --MAPSTATS_dialogs_no; break;
  case MAPSTATS_NEWDIALOG:          ++MAPSTATS_dialogs_no; break;
  default:; // nothing
  }
}

void MAPSTATS_Restart()
{
  MutexGuard _mg(MAPSTATS_GetMutex());
  MAPSTATS_Flush(MAPSTATS__SEC,false);
  MAPSTATS_Flush(MAPSTATS__MIN,false);
  MAPSTATS_Flush(MAPSTATS__HOUR,false);
  time_t cur_time = time(0);
  MAPSTATS_last_time_sec = cur_time;
  MAPSTATS_last_time_min = cur_time;
  MAPSTATS_last_time_hour = cur_time;
}

void MAPSTATS_Update(MAPSTATS stats)
{
  MutexGuard _mg(MAPSTATS_GetMutex());
  time_t cur_time = time(0);
  if ( cur_time > MAPSTATS_last_time_hour+60*60 ) {
    // dump one hour stats
    MAPSTATS_last_time_hour = cur_time;
    MAPSTATS_Flush(MAPSTATS__HOUR,true);
  }
  if ( cur_time > MAPSTATS_last_time_min+60 ) {
    // dump one minute stats
    MAPSTATS_last_time_min = cur_time;
    MAPSTATS_Flush(MAPSTATS__MIN,true);
  }
  if ( cur_time >= MAPSTATS_last_time_sec+1 ) {
    // dump one second stats
    MAPSTATS_last_time_sec = cur_time;
    MAPSTATS_Flush(MAPSTATS__SEC,true);
  }
  MAPSTATS_Update_(stats);
}

void MAPSTATS_DumpDialog(MapDialog* dlg)
{
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.dlgdump");
  smsc_log_info(logger, "dlg/map/smsc 0x%x/0x%x/0x%x(%s) state: %d, %ld sec, {%s->%s}",
               dlg,
               dlg->dialogid_map,
               dlg->dialogid_smsc,
               dlg->isUSSD?"USSD":"OTHER",
               (int)dlg->state,
               (long)(time(0)-(dlg->maked_at_mks/1000000)),
               dlg->sms.get()?dlg->sms->getOriginatingAddress().value:"???",
               dlg->sms.get()?dlg->sms->getDestinationAddress().value:"???");
}

void MapProxy::checkLogging() {
#ifdef USE_MAP
  if ( smsc::logger::Logger::getInstance("map.trace.user1")->isDebugEnabled() ) {
    __map_trace__("Enable trace for system USER01");
    MsgTraceOn( USER01_ID );
  } else {
    __map_trace__("Disable trace for system USER01");
    MsgTraceOff( USER01_ID );
  }
  if ( smsc::logger::Logger::getInstance("map.trace.etsimap")->isDebugEnabled() ) {
    __map_trace__("Enable trace for system ETSIMAP_ID");
    MsgTraceOn( ETSIMAP_ID );
  } else {
    __map_trace__("Disable trace for system ETSIMAP_ID");
    MsgTraceOff( ETSIMAP_ID );
  }
  if ( smsc::logger::Logger::getInstance("map.trace.tcap")->isDebugEnabled() ) {
    __map_trace__("Enable trace for system TCAP_ID");
    MsgTraceOn( TCAP_ID );
  } else {
    __map_trace__("Disable trace for system TCAP_ID");
    MsgTraceOff( TCAP_ID );
  }
#endif
}

bool isMapBound() {
#ifdef USE_MAP
  return __global_bind_counter == CORRECT_BIND_COUNTER;
#else
  return false;
#endif
}
