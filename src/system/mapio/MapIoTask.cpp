//#define EINSS7_THREADSAFE 1

#include "MapIoTask.h"

#define SSN 8
#define MAXENTRIES 10
#define My_USER_ID USER01_ID 
using namespace std;

#ifdef USE_MAPIO

#define SMSC_FORWARD_RESPONSE 0x001

struct SMSC_FORWARD_RESPONSE_T {
  ET96MAP_DIALOGUE_ID_T dialogId;
};

static void CloseDialog(	ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  Et96MapCloseReq (SSN,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
}

static void CloseAndRemoveDialog(	ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  USHORT_T res = Et96MapCloseReq (SSN,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __trace2__("close error, code 0x%hx",res);
  }
  MapDialogContainer::getInstance()->dropDialog(dialogId);
}

void ForwardResponse(ET96MAP_DIALOGUE_ID_T dialogId){
  MapDialogCntItem* mdci = 
    MapDialogContainer::getInstance()->createDialog(dialogId);
  if ( mdci ) {
    if ( !mdci ){
      __trace2__("MAP::bad dialogid 0x%x",dialogId);
      throw runtime_error("MAP::bad dialogid");
    }
  }
  USHORT_T result = Et96MapV2ForwardSmMOResp(SSN,dialogId,mdci->invokeId,0);
  if ( result != ET96MAP_E_OK ){
    __trace2__("MAP::Et96MapV2ForwardSmMOInd error when send response on forward_sm");
    throw runtime_error("MAP::Et96MapV2ForwardSmMOInd error when send response on forward_sm");
  }
  CloseAndRemoveDialog(SSN,dialogId);
}

extern "C"{

USHORT_T Et96MapBindConf(ET96MAP_LOCAL_SSN_T lssn, ET96MAP_BIND_STAT_T status)
{
  __trace2__("MAP::Et96MapBindConf confirmation received ssn=%x status=%x\n",lssn,status);
  if (status == 0) return ET96MAP_E_OK;
  else if ( status == 1 ){
    __trace__("MAP: Unbind");
    Et96MapUnbindReq(SSN);
    //__trace__("MAP: Bind ");
    //if ( Et96MapBindReq(USER01_ID, SSN)!=ET96MAP_E_OK ){
    //  return 0;
    //}
    return 1;//ET96MAP_E_OK;
  }else{
    return status;
  }
}

USHORT_T  Et96MapOpenInd(
	ET96MAP_LOCAL_SSN_T lssn, 
	ET96MAP_DIALOGUE_ID_T dialogId, 
	ET96MAP_APP_CNTX_T* appCtx, 
	ET96MAP_SS7_ADDR_T* dstAddr, 
	ET96MAP_SS7_ADDR_T* srcAddr,
	ET96MAP_IMSI_T* imsi, 
	ET96MAP_ADDRESS_T* mapAddress, 
	ET96MAP_USERDATA_T *ud ) 
{
	__trace2__("MAP::Et96MapOpenInd ssn 0x%x, dalogid 0x%x",lssn,dialogId);
	try{
    MapDialogCntItem* mdci = 
		  MapDialogContainer::getInstance()->createDialog(dialogId);
  	mdci->localSsn = SSN;
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    USHORT_T result = Et96MapOpenResp(lssn,dialogId,ET96MAP_RESULT_OK,&reason,0,0,0);
    if ( result != ET96MAP_E_OK )
    {
      __trace2__("MAP::Et96MapOpenInd dialog opened error 0x%x",result);
      throw 0;                                    
    }
    __trace2__("MAP::Et96MapOpenInd dialog opened");
  }catch(...){
    __trace2__("MAP::Et96MapOpenInd error open connection");
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    Et96MapOpenResp(lssn,dialogId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
  }
  return ET96MAP_E_OK;
}
																															    
USHORT_T  Et96MapV2ForwardSmMOInd( 
	ET96MAP_LOCAL_SSN_T lssn, 
	ET96MAP_DIALOGUE_ID_T dialogId,
	ET96MAP_INVOKE_ID_T invokeId, 
	ET96MAP_SM_RP_DA_T* dstAddr, 
	ET96MAP_SM_RP_OA_T* srcAddr,  
	ET96MAP_SM_RP_UI_T* ud ) 
{
	__trace2__("MAP::Et96MapV2ForwardSmMOInd ssn 0x%x, dalogid 0x%x",lssn,dialogId);
  MapDialogCntItem* mdci = MapDialogContainer::getInstance()->getDialog(dialogId);
  if ( !mdci ) {
    __trace2__("dialog is not present")
  }else{
  	try{
      mdci->invokeId = invokeId;
      mdci->dialogue->Et96MapV2ForwardSmMOInd(
        SSN,dialogId,invokeId,dstAddr,srcAddr,ud);
      SMSC_FORWARD_RESPONSE_T* p = new SMSC_FORWARD_RESPONSE_T();
      p->dialogId = dialogId;
      MSG_T msg;
      msg.primitive = SMSC_FORWARD_RESPONSE;
      msg.sender = MY_USER_ID;
      msg.receiver = MY_USER_ID;
      msg.msg_p = (USHORT_T*)p;
      msg.size = sizeof(SMSC_FORWARD_RESPONSE_T);
      result =  MsgSend(&msg);
      if ( result != MSG_OK ){
        __trace2__("MAP::Et96MapV2ForwardSmMOInd MsgSend broken with code 0x%x",result);
        throw 0;
      }
  	}catch(...){
  		__trace__("MAP::Et96MapV2ForwardSmMOInd catch exception");
      CloseAndRemoveDialog(SSN,dialogId);
  	}
  }
  return ET96MAP_E_OK;
}

USHORT_T Et96MapDelimiterInd(
  ET96MAP_LOCAL_SSN_T lssn,
  ET96MAP_DIALOGUE_ID_T dialogId,
  UCHAR_T priorityOrder)
{
  __trace2__("MAP::Et96MapDelimiterInd lssn 0x%hx, dialogId 0x%hx",lssn,dialogId);
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

} // extern "C"

void MapIoTask::deinit()
{
  warning_if(Et96MapUnbindReq(SSN)!=ET96MAP_E_OK);  
  warning_if(MsgRel(USER01_ID,ETSIMAP_ID)!=MSG_OK);
  warning_if(MsgClose(USER01_ID)!=MSG_OK);
  MsgExit();
}

void MapIoTask::dispatcher()
{
  //int going;
  MSG_T message;
  USHORT_T result;
	
	//going = 0;
  message.receiver = MY_USER_ID;

  for(;;){
    result = MsgRecv(&message);
    if ( result != MSG_OK ) {
      __trace2__("MAP: error at MsgRecv with code x%hx",result);
      return;
    }
    //if (EINSS7CpMsgRecv_r(&message,MSG_INFTIM)!=MSG_OK) return;
    if ( isStopping ) return;
    if ( message.sender == message.receiver &&
         message.sender == USER01_ID )
    {
      try{
        if ( message.primitive == SMSC_FORWAR_RESPONSE ){
          SMSC_FORWARD_RESPONSE_T* response = (SMSC_FORWARD_RESPONSE_T*)message.msg_p;
          if ( response == 0 ) {
            __trace2__("MAP::MessageProcessing Opss, forward response has zero data");
            throw 0;
          }
          ForwardResponse(response->dialogId);
        }
      }catch(...){
      }
      delete message.msg_p;
      message.size = 0;
    }
    else
      Et96MapHandleIndication(&message);
    //result = Et96MapHandleIndication(&message);
    //if ( result != ET96MAP_E_OK ) {
    //  __trace2__("MAP: error at Et96MapHandleIndication with code x%hx",result);
      //return;
    //}
  }
}

void MapIoTask::init()
{
  USHORT_T err;
  err = MsgInit(MAXENTRIES);
  if ( err != MSG_OK ) { __trace2__("MAP: Erroat at MsgInit, code 0x%hx",err); throw runtime_error("MsgInit error"); }
	err = MsgOpen(MY_USER_ID);
  if ( err != MSG_OK ) { __trace2__("MAP: Erroat at MsgOpen, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  err = MsgConn(USER01_ID,ETSIMAP_ID);
  if ( err != MSG_OK ) { __trace2__("MAP: Erroat at MsgConn, code 0x%hx",err); throw runtime_error("MsgInit error"); }
  MsgTraceOn( MY_USER_ID );
  MsgTraceOn( ETSIMAP_ID );
  MsgTraceOn( TCAP_ID );
  __trace__("MAP: Bind");
  USHORT_T bind_res = Et96MapBindReq(MY_USER_ID, SSN);
  if(bind_res!=ET96MAP_E_OK){
    __trace2__("MAP: Bind error 0x%hx",bind_res);
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

void MapIoTask::init()
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

