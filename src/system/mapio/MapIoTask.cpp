
#include "MapIoTask.h"
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

//struct SMSC_FORWARD_RESPONSE_T {
//  ET96MAP_DIALOGUE_ID_T dialogId;
//};

void MAPIO_TaskACVersionNotifier()
{
  __pingPongWaitCounter = 0;
}
extern void MAPIO_QueryMscVersionInternal();

void CloseDialog( ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  USHORT_T res = Et96MapCloseReq (SSN,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __trace2__("MAP::close error, code 0x%hx",res);
  }else{
    __trace2__("MAP::dialog closed");
  }
}

void CloseAndRemoveDialog(  ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  CloseDialog(lssn,dialogId);
  MapDialogContainer::getInstance()->dropDialog(dialogId);
  __trace2__("MAP::dialog 0x%hx destroed",dialogId);
}

extern "C" {

USHORT_T Et96MapBindConf(ET96MAP_LOCAL_SSN_T lssn, ET96MAP_BIND_STAT_T status)
{
  __trace2__("MAP::Et96MapBindConf confirmation received ssn=%x status=%x\n",lssn,status);
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
  __trace2__("MAP::Et96MapStateInd received ssn=%x user state=%x affected SSN=%d affected SPC=%ld local SPC=%ld\n",lssn,userState,affectedSSN,affectedSPC,localSPC);
  return ET96MAP_E_OK;
}

void Et96MapIndicationError(USHORT_T error,UCHAR_T* errString)
{
  if ( errString ) {
    __trace2__("MAP::Et96MapIndicationError: error 0x%hx text %s",error,errString);
  }else{
    __trace2__("MAP::Et96MapIndicationError: error 0x%hx",error);
  }
}

} // extern "C"

void MapIoTask::deinit()
{
  USHORT_T result;
  __trace2__("MAP::deinitialize");
  result = Et96MapUnbindReq(SSN);
  if ( result != ET96MAP_E_OK){
    __trace2__("MAP::error at Et96MapUnbindReq SSN=%d errcode 0x%hx",SSN,result);
//    return;
  }
  result = Et96MapUnbindReq(USSD_SSN);
  if ( result != ET96MAP_E_OK){
    __trace2__("MAP::error at Et96MapUnbindReq SSN=%d errcode 0x%hx",USSD_SSN,result);
//    return;
  }
  result = MsgRel(MY_USER_ID,ETSIMAP_ID);
  if ( result != MSG_OK){
    __trace2__("MAP::error at MsgRel errcode 0x%hx",result);
    kill(getpid(),17);
    return;
  }
  result = MsgClose(MY_USER_ID);
  if ( result != MSG_OK){
    __trace2__("MAP::error at MsgClose errcode 0x%hx",result);
    kill(getpid(),17);
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
  INT_T	       eventlist_len = 0;
  
  message.receiver = MY_USER_ID;
  gettimeofday( &utime, 0 );
  for(;;){
    MAP_isAlive = true;
    if ( isStopping ) return;
    MAP_dispatching = true;
    gettimeofday( curtime, 0 );
    printf( "MsgRecv begin: %ld.%ld\n", curtime.tv_sec-utime.tv_sec, curtime.tv_usec-utime.tv_usec );
    utime=curtime;
    result = EINSS7CpMsgRecv_r(&message,1000);
    gettimeofday( curtime, 0 );
    printf( "MsgRecv end: %ld.%ld\n", curtime.tv_sec-utime.tv_sec, curtime.tv_usec-utime.tv_usec );
    utime=curtime;
//    result = MsgRecvEvent( &message, eventlist, &eventlist_len, 1000 );
    MAP_dispatching = false;
/*    if ( ++timecounter == 60 ) {
      __trace2__("MAP: EINSS7CpMsgRecv_r TICK-TACK");
      time( &cur_time );
      if( cur_time-last_msg > 120 ) {
        result = MSG_BROKEN_CONNECTION;
        kill(getpid(),17);
        __trace2__("MAP:: no messages received in 2 minutes");
      } else{
        try{
          MAPIO_QueryMscVersionInternal();
        }catch(exception& e){
          result = MSG_BROKEN_CONNECTION;
        }
      }
      if ( __global_bind_counter != CORRECT_BIND_COUNTER ){
        result = MSG_BROKEN_CONNECTION;
        __trace2__("MAP:: not all binders dinded");
      }
      timecounter = 0;
    }
*/
    if ( result == MSG_TIMEOUT ) continue;
    if ( result == MSG_BROKEN_CONNECTION ){
      __trace2__("MAP: Broken connection");
restart:
      __trace2__("MAP:: try restart MAP service");
//      warning_if(MsgRel(MY_USER_ID,ETSIMAP_ID)!=MSG_OK);
      bool ok = false;
      while ( !ok ){
        __trace2__("MAP:: check stopped flag");
        if ( isStopping ) return;
        try{
          __trace2__("MAP:: deinit MAP service");
          deinit();
          __trace2__("MAP:: init MAP service");
          init(30);
          __trace2__("MAP:: waiting binds");
          timecounter = 0;
          ok = true;
        }catch(...){
          __trace2__("MAP:: Error reinitialization");
          sleep(1);
        }
      }
      continue;
    }
    if ( result != MSG_OK ) {
      __trace2__("MAP: error at MsgRecv with code x%hx",result);
      if( !MAP_aborting ) {
        abort();
      }
    }

    __trace2__("MAP: MsgRecv receive msg with "
               "recver 0x%hx,sender 0x%hx,prim 0x%hx, size %d",message.receiver,message.sender,message.primitive,message.size);
    if( message.primitive == 0x8b && message.msg_p[6] >= 0x04 ) {
      __trace2__("MAP: MsgRecv hatching msg to reset priority order " );
      message.msg_p[6] = 0;
    }
    else if( message.primitive == 0x8d && message.msg_p[4] >= 0x04 ) {
      __trace2__("MAP: MsgRecv hatching msg to reset priority order " );
      message.msg_p[4] = 0;
    }
    map_result = Et96MapHandleIndication(&message);
    if( map_result != ET96MAP_E_OK ) {
     {
      char *text = new char[message.size*4+1];
      int k = 0;
      for ( int i=0; i<message.size; i++){
        k+=sprintf(text+k,"%02x ",(unsigned)message.msg_p[i]);
      }
      text[k]=0;
      __trace2__("MAP: WARN: error at Et96MapHandleIndication with code x%hx msg: %s",map_result,text);
      delete text;
     }
    }
    EINSS7CpReleaseMsgBuffer(&message);
  }
}

void MapIoTask::init(unsigned timeout)
{
  USHORT_T err;
  __global_bind_counter = 0;
  __pingPongWaitCounter = 0;
  err = EINSS7CpMsgInitNoSig(MAXENTRIES);
  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgInit, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  err = MsgOpen(MY_USER_ID);
  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgOpen, code 0x%hx",err); throw runtime_error("MsgOpen error"); }
  err = MsgConn(MY_USER_ID,ETSIMAP_ID);
  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgConn, code 0x%hx",err); throw runtime_error("MsgConn error"); }
  __trace2__("MAP:: pause self and wait map initialization");
  sleep(timeout);
  __trace2__("MAP:: continue self initialization");
//  err = MsgConn(USER01_ID,USER01_ID);
//  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgConn on self, code 0x%hx",err); throw runtime_error("MsgInit error"); }
#ifndef DISABLE_TRACING
  MsgTraceOn( MY_USER_ID );
  MsgTraceOn( ETSIMAP_ID );
  MsgTraceOn( TCAP_ID );
#endif
  __trace__("MAP: Bind");
  USHORT_T bind_res = Et96MapBindReq(MY_USER_ID, SSN);
  if(bind_res!=ET96MAP_E_OK){
    __trace2__("MAP: SSN Bind error 0x%hx",bind_res);
    throw runtime_error("bind error");
  }
  bind_res = Et96MapBindReq(MY_USER_ID, USSD_SSN);
  if(bind_res!=ET96MAP_E_OK){
    __trace2__("MAP: USSD Bind error 0x%hx",bind_res);
    throw runtime_error("bind error");
  }
  __trace__("MAP: Ok");
}

#else
void MapIoTask::deinit()
{
  __trace2__("MapIoTask::deinit: no map stack on this platform");
}

void MapIoTask::dispatcher()
{
  Event e;
  __trace2__("MapIoTask::dispatcher: no map stack on this platform");
  e.Wait();
}

void MapIoTask::init(unsigned)
{
  __trace2__("MapIoTask::init: no map stack on this platform");
}

#endif

MapDialogContainer* MapDialogContainer::container = 0;
Mutex MapDialogContainer::sync_object;

int MapIoTask::Execute(){
  try{
    init();
    is_started = true;
    startevent->Signal();
    dispatcher();
    //deinit();
  }catch(exception& e){
    __trace2__("exception in mapio: %s",e.what());
  }
  return 0;
}

string MapDialogContainer::SC_ADRESS_VALUE = "79029869999";

void MapDialogContainer::abort()
{
#ifdef USE_MAP  MAP_aborting = true;
  EINSS7CpMsgClean();
#endif
}

int MapTracker::Execute(){
  Event e;
#ifdef USE_MAP
  for(;;){
    time_t t = time(0);
    while(time(0)<(t+15)&&!isStopping){
      time_t xx = time(0);
      if ( xx > t+15 ) break;
      e.Wait(1000*(t-xx+15));
    }
    __trace2__("MAP tracker:: alive %d dispatching %d",MAP_isAlive,MAP_dispatching);
    if ( isStopping ) return 0;
    if ( MAP_dispatching && !MAP_isAlive ) {
      __trace2__("\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                 "MAP is DEAD"
                 "\n\n\n\n");
      abort();
    }else MAP_isAlive = false;
  }
#else
  e.Wait();
#endif
  return 0;
}

void MapDialogContainer::registerSelf(SmeManager* smeman)
{
  proxy.init();
  __trace2__("MAP::register MAP_PROXY");
//#if defined USE_MAP // !!!! temporary !!!!!
//  smeman->registerSmeProxy("MAP_PROXY",&proxy);
//#else
  smeman->registerInternallSmeProxy("MAP_PROXY",&proxy);
//#endif
  __trace2__("MAP::register MAP_PROXY OK");
}

void MapDialogContainer::unregisterSelf(SmeManager* smeman)
{
  //proxy.init();
  __trace2__("MAP::unregister MAP_PROXY");
//#if defined USE_MAP // !!!! temporary !!!!!
//  smeman->registerSmeProxy("MAP_PROXY",&proxy);
//#else
  smeman->unregisterSmeProxy("MAP_PROXY");
//#endif
  __trace2__("MAP::unregister MAP_PROXY OK");
}
