#include "MapIoTask.h"
#define SSN 8
#define MAXENTRIES 10
 
#define _CB(a) USHORT_T a() {fprintf(stderr,"MAP::%s callback was called\n",#a);return MSG_OK;}

#ifdef USE_MAPIO

static void CloseDialog(	ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  Et96MapCloseReq (lssn,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
}

static void CloseAndRemoveDialog(	ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId)
{
  Et96MapCloseReq (lssn,dialogId,ET96MAP_NORMAL_RELEASE,0,0,0);
  MapDialogContainer::getInstance()->dropDialog(dialogId);
}

_CB(MapSubmitSMInd)              
_CB(MapReportSMInd)
_CB(MapAlertInd)
_CB(MapBeginInd)
_CB(MapErrorInd)
_CB(MapUssdReqConf)
_CB(MapUssdNotifyConf)
_CB(MapProcUssdDataInd)
_CB(MapSubmitConf)
_CB(MapForwardMTPositioningConf)
_CB(MapProvideSubscriberLCSConf)
_CB(MapSendRoutingInfoForLCSConf)
_CB(MapPerformLocationLCSInd)
_CB(MapBindConf)
_CB(MapIndicationError)
_CB(Et96MapGetACVersionConf)
_CB(Et96MapV1ForwardSmMT_MOConf)
_CB(Et96MapV2ForwardSmMTConf)
_CB(Et96MapV2ForwardSmMOConf)
_CB(Et96MapOpenConf)
_CB(Et96MapV1ReportSmDelStatConf)
_CB(Et96MapV2ReportSmDelStatConf)
_CB(Et96MapV1SendRInfoForSmConf)
_CB(Et96MapV2SendRInfoForSmConf)   
_CB(Et96MapV1AlertSCInd)
_CB(Et96MapV2AlertSCInd)
_CB(Et96MapCloseInd)
_CB(Et96MapDelimiterInd)
_CB(Et96MapV1ForwardSmMOInd)
_CB(Et96MapV2InformSCInd)
_CB(Et96MapNoticeInd)
_CB(Et96MapPAbortInd)
_CB(Et96MapUAbortInd) 
_CB(Et96MapIndicationError)
_CB(Et96MapV3ProvideSubscriberLCSConf) 
_CB(Et96MapV3SendRInfoForLCSConf)
_CB(Et96MapV3SubscriberLCSReportInd)
_CB(Et96MapV3ForwardSmMTConf)    
_CB(Et96MapV3SendRInfoForSmConf)
_CB(Et96MapV3ForwardSmMOInd)
_CB(Et96MapV3ReportSmDelStatConf)   
_CB(Et96MapV3InformSCInd)
_CB(Et96MapV1BeginSubscriberActivityInd)
_CB(Et96MapV1ProcessUnstructuredSSDataInd)
_CB(Et96MapV2ProcessUnstructuredSSRequestInd)
_CB(Et96MapV2UnstructuredSSRequestConf)
_CB(Et96MapV2UnstructuredSSNotifyConf)
_CB(Et96MapV3AnyTimeInterrogationConf)
_CB(Et96MapV3AnyTimeInterrogationInd)
_CB(Et96MapV3ProvideSubscriberInfoConf)

USHORT_T Et96MapBindConf(ET96MAP_LOCAL_SSN_T lssn, ET96MAP_BIND_STAT_T status)
{
  __trace2__("MAP::Et96MapBindConf confirmation received ssn=%x status=%x\n",lssn,status);
  if (status == 0) return MSG_OK;
  else return status;
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
	__trace2__("MAP::Et96MapOpenInd ssn 0x%x, dalogid 0x%x",dialogId);
	try{
    MapDialogCntItem* mdci = 
		  MapDialogContainer::getInstance()->createDialog(dialogId);
  	mdci->localSsn = lssn;
  }catch(...){
    CloseDialog(lssn,dialogId);
  }
  return MSG_OK;
}
																															    
/*USHORT_T  Et96MapV2ForwardSmInd( 
	ET96MAP_LOCAL_SSN_T lssn, 
	ET96MAP_DIALOGUE_ID_T dialogId,
	ET96MAP_INVOKE_ID_T invokeId, 
	ET96MAP_SM_RP_DA_T* dstAddr, 
	ET96MAP_SM_RP_OA_T* srcAddr,  
	ET96MAP_SM_RP_UI_T* ud ) 
{
	__trace2__("MAP::Et96MapV2ForwardSmMOInd ssn 0x%x, dalogid 0x%x",lssn,dialogId);
  MapDialogCntItem* mdci = MapDialogContainer::getInstance()->getDialog(dialogId);
  if ( !mdci ) CloseDialog(lssn,dialogId);
	try{
    mdci->dialogue->Et96MapV2ForwardSmMOInd(
      lssn,dialogId,invokeId,dstAddr,srcAddr,ud);
    USHORT_T err = Et96MapV2ForwardSmResp(lssn,dialogId,invokeId,0);
    if ( err != MSG_OK ) {
      __trace2__("broken response with error 0x%hx",err);
      throw runtime_error("MAPIO::ERR broken response");
    }
	}catch(...){
		__trace__("MAP::Et96MapV2ForwardSmMOInd catch exception");
    CloseAndRemoveDialog(lssn,dialogId);
	}
  return MSG_OK;
} */

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
  if ( !mdci ) CloseDialog(lssn,dialogId);
	try{
    mdci->dialogue->Et96MapV2ForwardSmMOInd(
      lssn,dialogId,invokeId,dstAddr,srcAddr,ud);
    USHORT_T err = Et96MapV2ForwardSmMOResp(lssn,dialogId,invokeId,0);
    if ( err != MSG_OK ) {
      __trace2__("broken response with error 0x%hx",err);
      throw runtime_error("MAPIO::ERR broken response");
    }
	}catch(...){
		__trace__("MAP::Et96MapV2ForwardSmMOInd catch exception");
    CloseAndRemoveDialog(lssn,dialogId);
	}
  return MSG_OK;
}

USHORT_T Et96MapStateInd (
	ET96MAP_LOCAL_SSN_T lssn,
	UCHAR_T userState,
	UCHAR_T affectedSSN,
	ULONG_T affectedSPC,
	ULONG_T localSPC) 
{
  __trace2__("MAP::Et96MapStateInd received ssn=%x user state=%x affected SSN=%d affected SPC=%ld local SPC=%ld\n",lssn,userState,affectedSSN,affectedSPC,localSPC);
  return MSG_OK;
}


void MapIoTask::deinit()
{
  warning_if(!Et96MapUnbindReq(SSN));  
  warning_if(!MsgRel(USER01_ID,ETSIMAP_ID));
  warning_if(!MsgClose(USER01_ID));
  MsgExit();
}

void MapIoTask::dispatcher()
{
  int going;
  MSG_T message;
  USHORT_T result;
	
	going = 1;
  message.receiver = USER01_ID;

  while(going) {
    if (MsgRecv(&message)!=MSG_OK) return;
    if ( isStopping ) return;
    if(!Et96MapHandleIndication(&message)) return;
  }
}

void MapIoTask::init()
{
  throw_if(!MsgInit(MAXENTRIES));
	throw_if(!MsgOpen(USER01_ID));
  throw_if(!MsgConn(USER01_ID,ETSIMAP_ID));
  MsgTraceOn( USER01_ID );
  MsgTraceOn( ETSIMAP_ID );
  MsgTraceOn( TCAP_ID );
  throw_if(Et96MapBindReq(USER01_ID, SSN)!=ET96MAP_E_OK);
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

MapIoTask::MapDialogContainer* container = 0;
MapIoTask::Mutex sync_object;

