
#include "MapIoTask.h"

#define MAXENTRIES 10
#define MY_USER_ID USER01_ID 
using namespace std;

#ifdef USE_MAP

#define SMSC_FORWARD_RESPONSE 0x001

static unsigned __global_bind_counter = 0;
static bool MAP_dispatching = false;
static bool MAP_isAlive = false;
#define CORRECT_BIND_COUNTER 2

struct SMSC_FORWARD_RESPONSE_T {
  ET96MAP_DIALOGUE_ID_T dialogId;
};

void CloseDialog(	ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  USHORT_T res = Et96MapCloseReq (SSN,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __trace2__("MAP::close error, code 0x%hx",res);
  }else{
    __trace2__("MAP::dialog closed");
  }
}

void CloseAndRemoveDialog(	ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  CloseDialog(lssn,dialogId);
  MapDialogContainer::getInstance()->dropDialog(dialogId);
  __trace2__("MAP::dialog 0x%hx destroed",dialogId);
}

void MapIoTask::deinit()
{
  USHORT_T result;
  warning_if(Et96MapUnbindReq(SSN)!=ET96MAP_E_OK);  
  result = MsgRel(MY_USER_ID,ETSIMAP_ID);
  if ( result != MSG_OK){
    __trace2__("MAP::error at MsgRel errcode 0x%hx",result);
  }
  warning_if(MsgClose(MY_USER_ID)!=MSG_OK);
  MsgExit();
}

USHORT_T  Et96MapCloseInd(ET96MAP_LOCAL_SSN_T ssn,
                         ET96MAP_DIALOGUE_ID_T dialogId,
                         ET96MAP_USERDATA_T *ud,
                         UCHAR_T priorityOrder)
{
  __trace2__("MAP::Et96MapCloseInd did 0x%x",dialogId);
  try{
    DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogId));
    if ( !mdci.isnull() ){
      try{
        if ( mdci->Et96MapCloseInd(ssn,
                            dialogId,
                            ud,
                            priorityOrder) )
          MapDialogContainer::getInstance()->dropDialog(mdci->getDialogId());
      }catch(...){
        MapDialogContainer::getInstance()->dropDialog(mdci->getDialogId()); 
      }
    }
  }catch(...){
    __trace2__("MAP::Et96MapCloseInd: catch exception when processing did 0x%x",dialogId);
    MapDialogContainer::getInstance()->dropDialog(dialogId);
  }
  return ET96MAP_E_OK;
}

void MapIoTask::dispatcher()
{
  MSG_T message;
  USHORT_T result;
  unsigned timecounter = 0;
  message.receiver = MY_USER_ID;
  for(;;){
    MAP_isAlive = true;
    if ( isStopping ) return;
    MAP_dispatching = true;
    result = EINSS7CpMsgRecv_r(&message,1000);
    MAP_dispatching = false;
    if ( ++timecounter == 60 ) {
      __trace2__("MAP: EINSS7CpMsgRecv_r TICK-TACK");
      if ( __global_bind_counter != CORRECT_BIND_COUNTER ){
        result = MSG_BROKEN_CONNECTION;
      }
      timecounter = 0;
    }
    if ( result == MSG_TIMEOUT ) continue;
    if ( result == MSG_BROKEN_CONNECTION ){
      __trace2__("MAP: Broken connection");
      warning_if(MsgRel(MY_USER_ID,ETSIMAP_ID)!=MSG_OK);
      bool ok = false;
      while ( !ok ){
        try{
          deinit();
          init(30);
          timecounter = 0;
          /*if ( __global_bind_counter != CORRECT_BIND_COUNTER ){
            __trace2__("MAP:: waiting bind confirm");
            sleep(3);
            if ( __global_bind_counter != CORRECT_BIND_COUNTER ){
              throw 0;
            }
          }*/
          ok = true;
        }catch(...){
          __trace2__("MAP:: Error reinitialization");
          sleep(1);
        }
      }
      continue;
      /*result = MsgConn(USER01_ID,ETSIMAP_ID);
       ( result != MSG_OK ) { 
        __trace2__("MAP: Error at MsgConn, code 0x%hx",result); 
        throw runtime_error("MAP::MapIoTask: MsgConn error"); 
      }
      continue;*/
    }
    if ( result != MSG_OK ) {
      __trace2__("MAP: error at MsgRecv with code x%hx",result);
      return;
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
    Et96MapHandleIndication(&message);
  }
}

USHORT_T  Et96MapOpenConf (
  ET96MAP_LOCAL_SSN_T ssn,
  ET96MAP_DIALOGUE_ID_T dialogId,
  ET96MAP_OPEN_RESULT_T openResult,
  ET96MAP_REFUSE_REASON_T *refuseReason_p,
  ET96MAP_SS7_ADDR_T *respondingAddr_sp,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_USERDATA_T *specificInfo_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  __trace2__("MAP::Et96MapOpenConf did 0x%x",dialogId);
  try{
    DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogId));
    if ( !mdci.isnull() ){
      mdci->Et96MapOpenConf(ssn,
                            dialogId,
                            openResult,
                            refuseReason_p,
                            respondingAddr_sp,
                            appContext_sp,
                            specificInfo_sp,
                            provErrCode_p);
     if ( openResult != ET96MAP_RESULT_OK )
      MapDialogContainer::getInstance()->dropDialog(dialogId);
    }
  }catch(...){
    __trace2__("MAP::Et96MapCloseInd: catch exception when processing did 0x%x",dialogId);
    MapDialogContainer::getInstance()->dropDialog(dialogId);
  }
  return ET96MAP_E_OK;
}

void MapIoTask::init(unsigned timeout)
{
  USHORT_T err;
  __global_bind_counter = 0;
  err = MsgInit(MAXENTRIES);
  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgInit, code 0x%hx",err); throw runtime_error("MsgInit error"); }
	err = MsgOpen(MY_USER_ID);
  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgOpen, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  err = MsgConn(USER01_ID,ETSIMAP_ID);
  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgConn, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  __trace2__("MAP:: pause self and wait map initialization");
  sleep(timeout);
  __trace2__("MAP:: continue self initialization");
//  err = MsgConn(USER01_ID,USER01_ID);
//  if ( err != MSG_OK ) { __trace2__("MAP: Error at MsgConn on self, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  MsgTraceOn( MY_USER_ID );
  MsgTraceOn( ETSIMAP_ID );
  MsgTraceOn( TCAP_ID );
  __trace__("MAP: Bind");
  USHORT_T bind_res = Et96MapBindReq(MY_USER_ID, SSN);
  if(bind_res!=ET96MAP_E_OK){
    __trace2__("MAP: SSN Bind error 0x%hx",bind_res);
    throw runtime_error("bind error");
  }
  bind_res = Et96MapBindReq(MY_USER_ID, 147);
  if(bind_res!=ET96MAP_E_OK){
    __trace2__("MAP: 147 Bind error 0x%hx",bind_res);
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

void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId)
{
  __trace2__("MAP::freeDialogueId: 0x%x",dialogueId);
  MapDialogContainer::getInstance()->dialogId_pool.push_back(dialogueId);
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
    //__trace2__("MAP tracker:: alive %d dispatching %d",MAP_isAlive,MAP_dispatching);
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


