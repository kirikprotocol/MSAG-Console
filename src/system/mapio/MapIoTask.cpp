
#include "MapIoTask.h"

#define MAXENTRIES 10
#define MY_USER_ID USER01_ID 
using namespace std;

#ifdef USE_MAP

#define SMSC_FORWARD_RESPONSE 0x001

static unsigned __global_bind_counter = 0;

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

extern "C"{

USHORT_T Et96MapBindConf(ET96MAP_LOCAL_SSN_T lssn, ET96MAP_BIND_STAT_T status)
{
  __trace2__("MAP::Et96MapBindConf confirmation received ssn=%x status=%x\n",lssn,status);
  return ET96MAP_E_OK;
}

USHORT_T Et96MapUAbortInd(ET96MAP_LOCAL_SSN_T lssn,
                          ET96MAP_DIALOGUE_ID_T dialogid,
                          ET96MAP_USER_REASON_T *reason,
                          ET96MAP_DIAGNOSTIC_INFO_T* diag,
                          ET96MAP_USERDATA_T *ud,
                          UCHAR_T priorityOrder)
{
  __trace2__("MAP::Et96MapUAbortInd");
  const char* szDiag = "<Diag-Unknown>";
  const char* szReason = "<Reason-Unknown>";
  if ( *reason == ET96MAP_RESOURCE_UNAVAIL )
  {
    szReason = "ET96MAP_RESOURCE_UNAVAIL";
    switch(*diag){
    case ET96MAP_SHORT_TERM_PROBLEM: 
      szDiag = "ET96MAP_SHORT_TERM_PROBLEM";
      break;
    case ET96MAP_LONG_TERM_PROBLEM:
      szDiag = "ET96MAP_LONG_TERM_PROBLEM";
      break;
    }
  }
  else if ( *reason == ET96MAP_APPL_PROC_CANCEL)
  {
    szReason = "ET96MAP_APPL_PROC_CANCEL";
    switch(*diag){
    case /*ET96MAP_HANDOVER_CANCELLATION*/0:
      szDiag = "ET96MAP_HANDOVER_CANCELLATION";
      break;
    case /*ET96MAP_RADIO_CHANNEL_RELEASED*/1:
      szDiag = "ET96MAP_RADIO_CHANNEL_RELEASED";
      break;
    case /*ET96MAP_NETWORK_PATH_RELEASED*/2:
      szDiag = "ET96MAP_NETWORK_PATH_RELEASED";
      break;
    case /*ET96MAP_CALL_RELEASED*/3:
      szDiag = "ET96MAP_CALL_RELEASED";
      break;
    case /*ET96MAP_ASSOCIATED_PROC_FAILURE*/4:
      szDiag = "ET96MAP_ASSOCIATED_PROC_FAILURE";
      break;
    case /*ET96MAP_TANDEM_DIALOGUE_RELEASED*/5:
      szDiag = "ET96MAP_TANDEM_DIALOGUE_RELEASED";
      break;
    case /*ET96MAP_REMOTE_OPERATIONS_FAILURE*/6:
      szDiag = "ET96MAP_REMOTE_OPERATIONS_FAILURE";
      break;
    }
  }
  else if ( *reason == ET96MAP_RESOURCE_LIMIT ){
    szReason = "ET96MAP_RESOURCE_LIMIT";
  }
  else if ( *reason == ET96MAP_PROC_ERROR ){
    szReason = "ET96MAP_PROC_ERROR";
  }
  __trace2__("MAP::Et96MapUAbortInd: did 0x%x, reson 0x%x:'%s', diag 0x%x:'%s'",
             dialogid,
             *reason,
             szReason,
             *diag,
             szDiag);
  try{
    DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogid));
    if ( !mdci.isnull() ){
      mdci->Et96MapUAbortInd(lssn,
                            dialogid,
                            reason,
                            diag,
                            ud,
                            priorityOrder);
      MapDialogContainer::getInstance()->dropDialog(dialogid);
    }
  }catch(...){
    __trace2__("MAP::Et96MapUAbort: catch exception when processing did 0x%x",dialogid);
    MapDialogContainer::getInstance()->dropDialog(dialogid);
  }
  return ET96MAP_E_OK;
}

USHORT_T Et96MapV2ForwardSmMTConf (
  ET96MAP_LOCAL_SSN_T lssn,
  ET96MAP_DIALOGUE_ID_T dialogid,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  __trace2__("MAP::Et96MapV2ForwardSmMTConf: did 0x%x",dialogid);
  try{
    DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogid));
    if ( !mdci.isnull() ){
      mdci->Et96MapV2ForwardSmMTConf(lssn,
                                    dialogid,
                                    invokeId,
                                    errorForwardSMmt_sp,
                                    provErrCode_p);
    }
  }catch(...){
    __trace2__("MAP::Et96MapV2ForwardSmMTConf: catch exception when processing did 0x%x",dialogid);
    //MapDialogContainer::getInstance()->dropDialog(dialogid);
  }
  return ET96MAP_E_OK;
}

USHORT_T Et96MapPAbortInd(ET96MAP_LOCAL_SSN_T lssn,
                          ET96MAP_DIALOGUE_ID_T dialogid,
                          ET96MAP_PROV_REASON_T reason,
                          ET96MAP_SOURCE_T source,
                          UCHAR_T priorityOrder)
{
  __trace2__("MAP::Et96MapPAbortInd")
  const char* szReason = "<Reason-Unknown>";
  switch( reason ){
  case ET96MAP_RESOURCE_UNAVAIL:
    szReason = "ET96MAP_RESOURCE_UNAVAIL";
    break;
  case ET96MAP_APPL_PROC_CANCEL:
    szReason = "ET96MAP_APPL_PROC_CANCEL";
    break;
  case ET96MAP_RESOURCE_LIMIT:
    szReason = "ET96MAP_RESOURCE_LIMIT";
    break;
  case ET96MAP_PROC_ERROR:
    szReason = "ET96MAP_PROC_ERROR";
    break;
  }
  __trace2__("MAP::Et96MapPAbortInd: did 0x%x, reson 0x%x:'%s'",
             dialogid,
             reason,
             szReason);
  try{
    DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogid));
    if ( !mdci.isnull() ){
      mdci->Et96MapPAbortInd(lssn,
                            dialogid,
                            reason,
                            source,
                            priorityOrder);
      MapDialogContainer::getInstance()->dropDialog(dialogid);
    }
  }catch(...){
    __trace2__("MAP::Et96MaPAbort: catch exception when processing did 0x%x",dialogid);
    MapDialogContainer::getInstance()->dropDialog(dialogid);
  }
  return ET96MAP_E_OK;
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
  __trace2__("MAP::Et96MapOpenInd appCtx->type:0x%x, appCtx->version:0x%x ",appCtx->acType,appCtx->version); 
  try{
    //char abonent[32] = {0,};
    //mkAbonent(abonent,0/*srcAddr*/);
    DialogRefGuard mdci(MapDialogContainer::getInstance()->createDialog(dialogId,SSN/*,0*/));
    __trace2__("MAP:: create dialog with ptr 0x%p, dialogid 0x%x",mdci.get(),dialogId);
  	//mdci->localSsn = SSN;
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    USHORT_T result = Et96MapOpenResp(SSN,dialogId,ET96MAP_RESULT_OK,&reason,0,0,0);
    if ( result != ET96MAP_E_OK )
    {
      __trace2__("MAP::Et96MapOpenInd dialog opened error 0x%x",result);
      throw 0;                                    
    }
    __trace2__("MAP::Et96MapOpenInd dialog opened");
  }catch(...){
    __trace2__("MAP::Et96MapOpenInd error open connection");
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    Et96MapOpenResp(SSN,dialogId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
  }
  return ET96MAP_E_OK;
}
																															    
USHORT_T  Et96MapV2SendRInfoForSmConf ( ET96MAP_LOCAL_SSN_T lssn,
				       ET96MAP_DIALOGUE_ID_T dialogId,
				       ET96MAP_INVOKE_ID_T invokeId,
				       ET96MAP_IMSI_T *imsi_sp,
				       ET96MAP_ADDRESS_T *mscNumber_sp,
				       ET96MAP_LMSI_T *lmsi_sp,
				       ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
				       ET96MAP_PROV_ERR_T *provErrCode_p )
{
	__trace2__("MAP::Et96MapV2SendRInfoForSmConf ssn 0x%x, dalogid 0x%x",lssn,dialogId);
  DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogId));
  __trace2__("MAP:: dialog with ptr 0x%p, dialogid 0x%x",mdci.get(),dialogId);
  if ( mdci.isnull() ) {
    __trace2__("MAP::dialog is not present");
  }else{
  	try{
      __trace2__("MAP::mdci->Et96MapV2SendRInfoForSmConf");
      mdci->Et96MapV2SendRInfoForSmConf(
        SSN,dialogId,invokeId,imsi_sp,mscNumber_sp,lmsi_sp,errorSendRoutingInfoForSm_sp,provErrCode_p);
      __trace2__("MAP::mdci->Et96MapV2SendRInfoForSmConf OK");
  	}catch(...){
  		__trace__("MAP::Et96MapV2SendRInfoForSmConf catch exception");
      CloseAndRemoveDialog(SSN,dialogId);
  	}
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
  DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogId));
  __trace2__("MAP:: dialog with ptr 0x%p, dialogid 0x%x",mdci.get(),dialogId);
  if ( mdci.isnull() ) {
    __trace2__("MAP::dialog is not present");
  }else{
  	try{
      __trace2__("MAP::mdci->Et96MapV2ForwardSmMOInd");
      mdci->Et96MapV2ForwardSmMOInd(
        SSN,dialogId,invokeId,dstAddr,srcAddr,ud);
      __trace2__("MAP::mdci->Et96MapV2ForwardSmMOInd OK");
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
  //USHORT_T result = Et96MapCloseReq( SSN, dialogId, ET96MAP_NORMAL_RELEASE, 0, priorityOrder, 0 );
  //if( result != ET96MAP_E_OK ) return result;
  DialogRefGuard mdci(MapDialogContainer::getInstance()->getDialog(dialogId));
  __trace2__("MAP:: dialog with ptr 0x%p, dialogid 0x%x",mdci.get(),dialogId);
  if ( mdci.isnull() ) {
    __trace2__("MAP::dialog is not present");
  }else{
  	try{
      __trace2__("MAP::mdci->Et96MapDelimiterInd");
      mdci->Et96MapDelimiterInd(SSN,dialogId,priorityOrder);
      __trace2__("MAP::mdci->Et96MapDelimiterInd OK");
  	}catch(...){
  		__trace__("MAP::Et96MapDelimiterInd catch exception");
      CloseAndRemoveDialog(SSN,dialogId);
  	}
  }
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
      if ( mdci->Et96MapCloseInd(ssn,
                            dialogId,
                            ud,
                            priorityOrder) )
        MapDialogContainer::getInstance()->dropDialog(dialogId);
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
  message.receiver = MY_USER_ID;
  unsigned timecounter = 0;
  for(;;){
    if ( isStopping ) return;
    result = EINSS7CpMsgRecv_r(&message,1000);
    if ( ++timecounter == 60 ) {
      __trace2__("MAP: EINSS7CpMsgRecv_r TICK-TACK");
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
          if ( __global_bind_counter != CORRECT_BIND_COUNTER ){
            __trace2__("MAP:: waiting bind confirm");
            sleep(3);
            if ( __global_bind_counter != CORRECT_BIND_COUNTER ){
              throw 0;
            }
          }
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



