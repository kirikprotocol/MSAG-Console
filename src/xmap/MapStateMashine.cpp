
#include <list>
#include <thread.h>
#include <util/recoder/recode_dll.h>
#include "MapStateMashine.h"
//#include "MapDialog.h"
#include "common.h"

extern smsc::logger::Logger *logger;

//extern Event globa_levent;
extern bool stopProcess;
//class CMMessage;
Event map_stm_event;

using namespace smsc::sms;
using namespace smsc::sms::Tag;

#ifdef FULL_LOG
 #define __dump__ dumpmsg
#else
 #define __dump__
#endif

CMapStateMashine::CMapStateMashine()
{
 
 //procState =0;
// pInputQueue = 0;
// pOutQueue   = 0;
// processFlag = 0;
// pIncomingMSGQueue = 0;
// pOutgoingMSGQueue = 0;
// dialogindex = ET96MAP_MAX_DIALOGUES;
// dinfos.resize(ET96MAP_MAX_DIALOGUES);
   //pthread_mutex_init(&mutex,NULL);
   
 connected = false;
   quall=0;
}


CMapStateMashine::CMapStateMashine(qMessage_t *msg_store)
{
// pInputQueue2 = &msg_store->CQin;
// pOutQueue2   = &msg_store->CQout;
// pIncomingMSGQueue = &msg_store->qIncoming;
// pOutgoingMSGQueue = &msg_store->qQoutgoing;
// dialogindex = ET96MAP_MAX_DIALOGUES;
// event_in_q = &msg_store->event_in_queue;
// event_out_q = &msg_store->event_out_queue;
// event_outgoing_queue =&msg_store->event_outgoing_queue;
// event_incoming_queue =&msg_store->event_incoming_queue;
// globa_levent = &msg_store->globa_levent;
// dinfos.resize(ET96MAP_MAX_DIALOGUES);

 connected = false;
 quall = msg_store;
 processFlag = 0;

 
 //pthread_mutex_init(&mutex,NULL);
}
CMapStateMashine::~CMapStateMashine()
{
 
 //pthread_mutex_destroy(&mutex);
}

void CMapStateMashine:: SetDialogContainer(MapDialogContainer* _mdc)
{
 mdc = _mdc;
}

int CMapStateMashine::connect()
{
 USHORT_T err = MsgInit(MAXENTRIES);
 if(err!=RETURN_OK)
 {
  xmap_trace(logger, "error : %s::[EINSS7Cp]MsgInit[Inst]() return: %d",__func__,err);
  return err;
 }
 err = MsgOpen(MICROMAP_ID);

 if(err!=RETURN_OK)
 {
  xmap_trace(logger, "error %s::MsgOpen()... %d",__func__,err);
  return err;
 }
 return RETURN_OK;
}

//int CMapStateMashine::run()
//{
//// GREEN
// //if(pInputQueue2->Count()>0)
// { 
//  MSG_T * xmessage;
//#ifdef USE_ENEVTS_IN_QUEUE
//  xmap_trace(logger,"------------<stm run wait>");
//  event_in_q->Wait(MSTM_WAIT_TIMEOUT);
//  xmap_trace(logger,"------------</stm run wait>");
//#endif
//  while(pInputQueue2->Pop(xmessage,MSTM_WAIT_TIMEOUT))
//  {
//
// 
//   ////xmap_trace(logger,"CMapStateMashine::run()::prim=%d\n",xmessage->primitive);
// 
//   //TRACE_1("primitive RX dump:%s",xmessage->msg_p,xmessage->size); 
//
//   switch(xmessage->primitive)
//   {
//   case MAP_BIND_REQ:
//    sendBindConf(xmessage);
//    break;
//
//   case MAP_OPEN_REQ: 
//    onOpenDialog(xmessage);
//    break;
// /*  case MAP_CLOSE_REQ:
//    {
//     removeDialog(getMsgDialogId(xmessage));
//    }
//    break;*/
//   default:
//    processMessage(xmessage);
//    break;
//   }
//
//   delete xmessage->msg_p;
//   delete xmessage;
//
//  }
//    
// }
// //GREEN
// //else //от смс центра ничего нету, смотреть что есть от трубок
//  processMO();
//
// deleteExpiredDialogs();
//
// 
// return 0;
//}

/************************************************************************/
/* Recieve primitive from CP                                            */
/************************************************************************/

int CMapStateMashine::RecievePrimitive()
{
 USHORT_T res;

 MSG_T message;

 message.receiver=MICROMAP_ID;
 
 res = EINSS7CpMsgRecv_r(&message,100);
     

 switch(res)
 {
 case RETURN_OK:
   res = ProcessPrimitive(&message);
  break;
 case MSG_TIMEOUT:
   //xmap_trace(logger,"%s timeout",__func__);
  break;
 default:
  {
   xmap_trace(logger,"%s::MsgRecv failed (%d): %d->%d prim=%d sz=%d\n",__func__,res,message.sender,message.receiver,message.primitive,message.size);
   stopProcess=true;
   return res;
  }
 }
 
 return EINSS7CpReleaseMsgBuffer(&message); 
}
/************************************************************************/
/* Process primitive                                                    */
/************************************************************************/
int CMapStateMashine::ProcessPrimitive(MSG_T * message)
{
 
 /*switch(message->primitive)
 {
  
  case MAP_OPEN_REQ: 
   onOpenDialog(message);
    break;
  default:
   processMessage(message);
   break;
 }*/

  switch(message->primitive)
   {
   case MAP_BIND_REQ:
     sendBindConf(message);
    break;
   case MAP_OPEN_REQ:
     onOpenRequest(message);
    break;

   case MAP_SND_RINFO_SM_REQ:
     onSendRinfoRequest(message);
    break;

   case MAP_DELIMIT_REQ:
      onDelimiterRequest(message);//close_ind
    break;

   case MAP_FWD_SM_REQ:
     onForwardSMSRequest(message);
    break;

   case MAP_OPEN_RSP:
     onOpenResponce(message);
    break;

   case MAP_FWD_SM_RSP:
   case MAP_V1_FWD_SM_RSP:
     onForwardMOSMSResp(message);
    break;

   case MAP_CLOSE_REQ:
     onCloseReq(message);//statePosition = DIALOG_FINISH;
    break;

   case MAP_U_ABORT_REQ:
     onUserAbortRequest(message);
    break;

   case MAP_USSD_REQ:
     onUssdRequest(message);
    break;
   case MAP_USSD_RSP:
     onUssdResponce(message);
    break;

   case MAP_PROC_USSD_RSP:
     onUssdClosing(message);
    break;

   case MAP_USSD_NOTIFY_REQ:
     onUssdNotifyRequest(message);
    break;
   case MAP_USSD_NOTIFY_RSP:
     onUssdNotifyResponce(message);
    break;
    
   default:
    
    xmap_trace(logger,"%s warning: unknown primitive recieved. %d\n",__func__,message->primitive);
    __dump__("<dump> %s",message);
    break;
   }


  dumpmsg("process primitive() %s",message);
 USHORT_T di = getDialogId(message);

 if(mdc->getState(di)==DIALOG_FINISH)
    mdc->deleteDialog(di);

 return RETURN_OK;
}
/************************************************************************/
/* Execute thread main proc                                             */
/************************************************************************/
int CMapStateMashine::Execute()
{
  for(;!stopProcess;)
  {
  
    // onli sleep 100 ms in EINSS7MSGRecv_r(...)
       //run();
  RecievePrimitive();
  
  //processMO();
  //quall->map_statemachine_event.Wait(MSTM_WAIT_TIMEOUT);
  
  }
 return 1;
}

USHORT_T CMapStateMashine::getDialogId(MSG_T* message)
{
 USHORT_T res = message->msg_p[3];
 res<<=8;
 res+= message->msg_p[2];
 return res; 
}


//DEL MapDialog * CMapStateMashine::getDialog(USHORT_T dlgid)
//DEL {
//DEL  return (MapDialog*)quall->pDialogs[dlgid];
//DEL }

//DEL void CMapStateMashine::deleteExpiredDialogs()
//DEL {
//DEL 
//DEL     MutexGuard g(mtx);
//DEL 
//DEL  for(int i=0;i<0xffff;i++)
//DEL  {
//DEL   
//DEL   if((MapDialog*)quall->pDialogs[i])
//DEL   {
//DEL    
//DEL    if( ((MapDialog*)quall->pDialogs[i])->statePosition==DIALOG_FINISH)
//DEL    {
//DEL 
//DEL     delete (MapDialog*)quall->pDialogs[i];
//DEL     removeDialog(i);
//DEL    }
//DEL   }
//DEL  }
//DEL 
//DEL   
//DEL }

//unused
//DEL void CMapStateMashine::processMessage(MSG_T * message )
//DEL {
//DEL   
//DEL   MapDialog * md = getDialog(getDialogId(message));
//DEL 
//DEL   if(!md) 
//DEL   {
//DEL    md = new MapDialog(DO_MAPSTATM,quall,&dinfos);
//DEL    md->dialogID = getDialogId(message);
//DEL    
//DEL    AddDialog(md,md->dialogID);
//DEL    
//DEL    // pDialogs.insert(std::pair<USHORT_T,MapDialog*>(md->dialogID,md));
//DEL 
//DEL   }
//DEL  
//DEL   md->step(message);
//DEL   
//DEL 
//DEL }
//DEL void CMapStateMashine::processMO()
//DEL {
//DEL  //map_stm_event.Wait(MSTM_WAIT_TIMEOUT);
//DEL  //map_stm_event.Wait();
//DEL //GREEN
//DEL  //if(quall->qQoutgoing.Count()>0)
//DEL  {
//DEL  // тут еще проверять надо!!!
//DEL 
//DEL #ifdef USE_ENEVTS_IN_QUEUE
//DEL   xmap_trace(logger,"--------------<processMO wait>");
//DEL   quall->event_outgoing_queue.Wait(MSTM_WAIT_TIMEOUT);
//DEL   xmap_trace(logger,"--------------</processMO wait>");
//DEL #endif
//DEL   //CMMessage * mm_message;
//DEL 
//DEL   std::string originator_imsi="";
//DEL   //
//DEL    
//DEL   while(quall->qQoutgoing.Pop(originator_imsi,MSTM_WAIT_TIMEOUT))
//DEL   {
//DEL   
//DEL    dialogindex--; //ET96MAP_MAX_DIALOGUES--
//DEL 
//DEL    dinfos[dialogindex].MMessageID=0;//.getSize();
//DEL    dinfos[dialogindex].Type =0;//mm_message->type;// message.Type;//MAP_SND_RINFO_SM_IND //example
//DEL    dinfos[dialogindex].DialogId = dialogindex;
//DEL    dinfos[dialogindex].Status = MAP_OPEN_IND;//MAP_OPEN_CONF if step done //example
//DEL    
//DEL    //dinfos[dialogindex]. .srcaddrlen = message.srcaddrlen;
//DEL 
//DEL 
//DEL    //memcpy(dinfos[dialogindex].srcaddr, &mm_message->fromAddress ,mm_message->fromAddress.ss7AddrLen+1 );
//DEL    
//DEL    dinfos[dialogindex].destination_address=quall->PipesList.getByImsi(originator_imsi)->destaddr; //mm_message->destination_address;
//DEL    dinfos[dialogindex].originating_address=quall->PipesList.getByImsi(originator_imsi)->msisdn; //mm_message->originating_address;
//DEL    dinfos[dialogindex].message_text=quall->PipesList.getByImsi(originator_imsi)->text; //mm_message->message_text;
//DEL    dinfos[dialogindex].imsi = quall->PipesList.getByImsi(originator_imsi)->imsi; //mm_message->imsi;
//DEL    dinfos[dialogindex].mscaddr =quall->config->getMscAddr(); //mm_message->mscaddr;
//DEL 
//DEL    //dinfos[dialogindex].destraddrlen = message.destraddrlen;
//DEL    //memcpy(dinfos[dialogindex].destaddr, &mm_message->toAddress, mm_message->toAddress.ss7AddrLen+1);
//DEL 
//DEL    //{TRACE_1("dstAddress %s",dinfos[dialogindex].destaddr,dinfos[dialogindex].destraddrlen);}
//DEL    //{TRACE_1("srcAddress %s",dinfos[dialogindex].srcaddr,dinfos[dialogindex].srcaddrlen);}
//DEL 
//DEL    //processed_messages.push_back(message);
//DEL   
//DEL    //if(mm_message->data)
//DEL    //  delete mm_message->data;
//DEL 
//DEL    //delete mm_message;
//DEL   
//DEL    onOpenDialog(0);
//DEL   }
//DEL   //sendOpenDialogInd(dialogindex);
//DEL 
//DEL   //SendInd();
//DEL  }
//DEL }
//DEL void CMapStateMashine::removeDialog(USHORT_T DialogId)
//DEL {
//DEL  
//DEL // delete pDialogs[DialogId];
//DEL  
//DEL  dinfos[DialogId].DialogId=0;
//DEL  dinfos[DialogId].MMessageID=-1;
//DEL  dinfos[DialogId].Status=0;
//DEL  dinfos[DialogId].Type=0;
//DEL  dinfos[DialogId].imsi="";
//DEL  dinfos[DialogId].mscaddr="";
//DEL  dinfos[DialogId].originating_address="";
//DEL  dinfos[DialogId].destination_address="";
//DEL  dinfos[DialogId].message_text="";
//DEL  
//DEL  dialogindex++;//restore ID
//DEL }

//unused
//DEL void CMapStateMashine::onOpenDialog(MSG_T * message)
//DEL {
//DEL  MapDialog * md = new MapDialog(DO_MAPSTATM,quall,&dinfos);//pOutQueue2,&dinfos,pIncomingMSGQueue,event_out_q);
//DEL 
//DEL     if(message!=0) //MT priznak
//DEL  {
//DEL   md->dialogID = getDialogId(message);
//DEL   //xmap_trace(logger,"<md->dialogID %d>",md->dialogID);
//DEL  }
//DEL  else     //MO priznak
//DEL  {
//DEL   md->dialogID = dialogindex;
//DEL 
//DEL   //xmap_trace(logger,"<dialogindex %d>",dialogindex);
//DEL 
//DEL  }
//DEL 
//DEL 
//DEL  md->step(message);//openning dialog confirmation or error
//DEL 
//DEL  AddDialog(md,md->dialogID);
//DEL 
//DEL  //pDialogs.insert(std::pair<USHORT_T,MapDialog*>(md->dialogID,md));
//DEL }
 
void CMapStateMashine::sendBindConf(MSG_T* message)
{


 MSG_T msg;

 UCHAR_T dl_start_prim[3] = {MAP_BIND_CONF,message->msg_p[1],0};

 msg.receiver = GSMSC_ID;
 msg.sender   = MICROMAP_ID;
 msg.primitive=MAP_BIND_CONF;
 msg.msg_p = new UCHAR_T[3];
 msg.size = 3;

 USHORT_T res;

 if(!connected)
 {
  res = MsgConn(MICROMAP_ID,GSMSC_ID);
  
 }

 if(res==RETURN_OK || connected)
 {
  connected  =true;
  res = EINSS7CpGetMsgBuffer(&msg);

  xmap_trace(logger,"%s get buffer (mstm) %d ",__func__,res);

  memcpy(msg.msg_p,dl_start_prim,3);

  if(res==EINSS7CP_MSG_GETBUF_FAIL)
  {
   xmap_trace(logger,"%s error sendBindConf->EINSS7CpGetMsgBuffer-> return: %d ",__func__,res);
  }
  
  res = MsgSend(&msg); 

  if(res != RETURN_OK)
  {
   EINSS7CpReleaseMsgBuffer(&msg);
   xmap_trace(logger,"%s sendBindConf->MsgSend->return = %d",__func__,res);
  }

  
 }
 else
 {
   
  xmap_trace(logger,"error %s unable to connect-> return: %d ",__func__,res);
 }

 //xmap_trace(logger,"CMapStateMashine::sendBindConf. %d->%d prim = %d",message->sender,message->receiver,message->primitive);

 delete msg.msg_p;

 quall->bindOK++;
 
}


//DEL cdinfo CMapStateMashine::getDialogInfo(int dlgid)
//DEL {
//DEL  cdinfo di;
//DEL  
//DEL  if(dlgid<=0xffff && dlgid>=0)
//DEL   di = dinfos[dlgid];
//DEL 
//DEL  return di;
//DEL }  

void _mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, const ET96MAP_ADDRESS_T *saddr, ET96MAP_LOCAL_SSN_T ssn) {
 addr->ss7AddrLen = 5+(saddr->addressLength+1)/2;
 addr->ss7Addr[0] = 0x12; // SSN & GT
 addr->ss7Addr[1] = ssn;
 addr->ss7Addr[2] = 0;
 addr->ss7Addr[3] = (saddr->typeOfAddress<<4)|(saddr->addressLength%2==0?0x02:0x01); // NP & GT coding
 addr->ss7Addr[4] = 0x04; // | (saddr->addressLength%2==0?0x80:0x00); high bit always 0 see 15517-CAA901437, 3.3.8
 memcpy( addr->ss7Addr+5, saddr->address, (saddr->addressLength+1)/2 );
 if( saddr->addressLength%2!=0 ) {
  addr->ss7Addr[5+(saddr->addressLength+1)/2-1] &= 0x0f;
 }
}

void _mkMapAddress(ET96MAP_ADDRESS_T *addr, const char *saddr, unsigned len) 
{
 unsigned i;
 int sz = (len+1)/2;
 addr->addressLength = len;
 addr->typeOfAddress = 0x81; // InterNational, ISDN
 for( i = 0; i < len; i++ ) {
  int bi = i/2;
  if( i%2 == 1 ) {
   addr->address[bi] |= ((saddr[i]-'0')<<4); // fill high octet
  } else {
   addr->address[bi] = (saddr[i]-'0')&0x0F; // fill low octet
  }
 }
 if( len%2 != 0 ) {
  addr->address[sz-1] |= 0xF0;
 }
}

//DEL void CMapStateMashine::AddDialog(MapDialog* pDlg,USHORT_T index)
//DEL {
//DEL  MutexGuard g(mtx);
//DEL  quall->pDialogs[index]=(MapDialogPtr)pDlg;
//DEL 
//DEL }
 

//// primitive management ////

void CMapStateMashine::onOpenRequest(MSG_T* message)
{//add dialog to dialog container if dlgid< 0xffff/2

 USHORT_T dlgid = getDialogId(message);

 mdc->inSertAt("","",dlgid);

 //  version !!!!!!!!!!!!!!!!
 // ET96MAP_APP_CNTX_T cntx= getDlgAcntx(message);
 // if(cntx.acType ==  ET96MAP_SHORT_MSG_MT_RELAY || cntx.acType == ET96MAP_SHORT_MSG_MO_RELAY ||
 // cntx.acType ==ET96MAP_SHORT_MSG_GATEWAY_CONTEXT)
 // && version ==...!!!!
 //
 //  acNotSupported(message)
 // else
            

 __assign_message_(MAP_OPEN_CONF);

 SS7MapMsg messmaker;
 messmaker.insertUChar(MAP_OPEN_CONF);
 messmaker.insertUChar(SSN);
 messmaker.insertUShort(dlgid);
 messmaker.insertUChar(1);
 
 messmaker.insertUChar(message->msg_p[4]);//ctnx
 messmaker.insertUChar(message->msg_p[5]);//ver
 messmaker.insertUChar(0);
 messmaker.insertUChar(0);
 messmaker.insertUChar(0);

 #ifndef __no_stupid__
  messmaker.insertUChar(0);
  messmaker.insertUChar(0);
  messmaker.insertUChar(0);
  messmaker.insertUChar(0);
  messmaker.insertUChar(0);
 #endif

__send_message_();

xmap_trace(logger,"MAP_OPEN_CONF dialog %d",dlgid);

 mdc->setState(dlgid,map_Opened);

   
}
void CMapStateMashine::onSendRinfoRequest(MSG_T* message)
{
 USHORT_T dlgid = getDialogId(message);//->msg_p[2];
 UCHAR_T state =mdc->getState(dlgid);

 if(state!=map_Opened)
 {
  bad_state(state);
  return;
 }
 
 
 UCHAR_T  lssn     = message->msg_p[1];
 UCHAR_T  invokeid = message->msg_p[4];
 UCHAR_T  LenMSISDN = message->msg_p[5];
 UCHAR_T  taMSISDN;

 int pos=6;

 char sMSISDN[21] = {0,};
 char sSCNTRADDR[21] = {0,};

 //MOBILE ST. ISDN ADDRESS 
 if (LenMSISDN != 0 )
 {
  taMSISDN  = message->msg_p[pos++];
//304585

  int i = 0;
  for ( ;i<(LenMSISDN);i++)
  {
   if( i % 2 == 0 ) 
   {
    sMSISDN[i] = (message->msg_p[pos+i/2]&0x0f)+0x30;//cifirya
   } else 
   {
    sMSISDN[i] = (message->msg_p[pos+i/2]>>4)+0x30;//osi
   }
   if(sMSISDN[i] > 0x39 ) 
   {
    //botvaaaaaaaaaaaa!!!!
   }
  }
  {
   char b[256] = {0,};
   memcpy(b,sMSISDN,i);
  }
 
 // ////xmap_trace(logger," MSISDN(adr.type=%02x) address: %s",taMSISDN,sMSISDN);
 
  pos+=(LenMSISDN%2==0)?LenMSISDN/2:LenMSISDN/2+1;

 }

 UCHAR_T sm_rp_pri = message->msg_p[pos++];
 UCHAR_T LenSCADDR= message->msg_p[pos++];
 UCHAR_T taSCADDR;

 //SERVICE CENTER ADDRESS

 if(LenSCADDR !=0)
 {
  taSCADDR = message->msg_p[pos++];

  char sSCNTRADDR[21] = {0,};
  int i = 0;
  for ( ;i<(LenSCADDR);i++)
  {
   if( i % 2 == 0 ) 
   {
    sSCNTRADDR[i] = (message->msg_p[pos+i/2]&0x0f)+0x30;//cifirya
   } else 
   {
    sSCNTRADDR[i] = (message->msg_p[pos+i/2]>>4)+0x30;//osi
   }
   if(sSCNTRADDR[i] > 0x39 ) 
   {
    //botvaaaaaaaaaaaa!!!!
   }
  }
  {
   char b[256] = {0,};
   memcpy(b,sSCNTRADDR,i);
  }
 
  //////xmap_trace(logger," ServiceCenter(adr.type=%02x) address: %s",taSCADDR, sSCNTRADDR);
 
 }


 std::string msisdn = (std::string)sMSISDN;

 //here get pipe from HLR by msisdn
 //cpipe pipe =pstoremessages->config->getPipeByMSISDN(msisdn);
 SS7MapMsg messmaker;

 std::string __str__mscaddr = quall->config->getMscAddr();
 unsigned mscaddrlen =  __str__mscaddr.length()/2;

 ET96MAP_ADDRESS_T mscadr;
 _mkMapAddress(&mscadr,__str__mscaddr.c_str(), __str__mscaddr.length());//12/2+1

 __assign_message_(MAP_SND_RINFO_SM_CONF);

 messmaker.insertUChar(MAP_SND_RINFO_SM_CONF);
 messmaker.insertUChar(SSN);
 messmaker.insertUShort(dlgid);
 messmaker.insertUChar(invokeid);

 xmap_trace(logger,"msisdn_string %s",msisdn.c_str());
 int i_pipe_val = quall->PipesList.hasPipe((const char *)msisdn.c_str());

 if(i_pipe_val==PIPE_ACTIVE || i_pipe_val==PIPE_IS_ABSENT)
 {
  
  //std::string msgstr=(*pDinfos)[dialogID].message_text;

  std::string pipe_imsi =quall->PipesList.getImsiByMsisdn(msisdn);

  UCHAR_T imsilen = pipe_imsi.length()/2;
  
  UCHAR_T *imsiaddr=new UCHAR_T[pipe_imsi.length()/2];
  
  int k=0;
  for(int i=0;i<(pipe_imsi.length()/2);i++)
  {
   imsiaddr[i]=(pipe_imsi.at(i*2)-0x30)| ((pipe_imsi.at(i*2+1)-0x30)<<4);

  }

  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T error_code;

  if(i_pipe_val==PIPE_IS_ABSENT)
   error_code.errorCode=27;//absent subscriber
  else
   error_code.errorCode=0;

  
  
  messmaker.insertUChar(imsilen);
  messmaker.insertPtr(imsiaddr,imsilen);
  messmaker.insertPtr((UCHAR_T*)&mscadr,mscadr.addressLength/2+mscadr.addressLength%2+2);
  messmaker.insertUChar(0);
  messmaker.insertUChar(error_code.errorCode);
  messmaker.insertUChar(0);


  delete imsiaddr;
 

  
 }
 else 
 {

  ////xmap_trace(logger,"error! wrong routing way");
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T error_code;

  
  error_code.errorCode=1;//unk subscriber
  messmaker.insertUChar(1);//imsilen
  messmaker.insertUChar(0x99);//imsi
  
  messmaker.insertPtr((UCHAR_T*)&mscadr,mscadr.addressLength/2+mscadr.addressLength%2+2);
  messmaker.insertUChar(0);
  messmaker.insertUChar(error_code.errorCode);
  messmaker.insertUChar(0);

  xmap_trace(logger,"%s:warning: HLR has no mt with MSISDN ='%s'",__func__,(const char *)msisdn.c_str());
 }

 __send_message_();
 mdc->setState(dlgid, map_After_Rinfo);
 xmap_trace(logger,"    MAP_SND_RINFO_SM_CONF dialog %d",dlgid);
}
void CMapStateMashine::onDelimiterRequest(MSG_T* message)
{
 //close_ind

 
 USHORT_T dlgid = getDialogId(message);//->msg_p[2];
 UCHAR_T state =mdc->getState(dlgid);
 bool d_expanded = mdc->isMultiple(dlgid);

 UCHAR_T indication = (d_expanded)?MAP_DELIMIT_IND:MAP_CLOSE_IND;

 __assign_message_(indication);

 USHORT_T specInfoLen=0;
 UCHAR_T priorityOrder=0;
 SS7MapMsg messmaker;

 messmaker.insertUChar(indication);
 messmaker.insertUChar(SSN);
 messmaker.insertUShort(dlgid);
 messmaker.insertUShort(specInfoLen);
 messmaker.insertUChar(priorityOrder);

 __send_message_();

 xmap_trace(logger,"MAP_CLOSE_IND dialog %d",dlgid);



 // if not exteneded sms marked into FWD_sm_REQ then drop map dialog!

 if(!d_expanded)
  mdc->setState(dlgid,DIALOG_FINISH);
 

// ////xmap_trace(logger,"MapDialog::sendCloseInd()");
 
 
}

void CMapStateMashine::onForwardSMSRequest(MSG_T* message)
{
 USHORT_T dlgid = getDialogId(message);//->msg_p[2];
 UCHAR_T state =mdc->getState(dlgid);

 if(state==map_Opened)
 {
  
 __dump__("dump recieved sms %s",message);

 int pos=5;
 if(message->msg_p[pos]==ET96MAP_ADDRTYPE_IMSI)
 {
  pos++;
  UCHAR_T imsiLength = message->msg_p[pos];//len
  UCHAR_T * imsi = new UCHAR_T [imsiLength]; //addr
  pos++;
  memcpy(imsi,message->msg_p+pos,imsiLength);
  pos += imsiLength;

  if(message->msg_p[pos]==ET96MAP_ADDRTYPE_SCADDR)
  {
   pos++;

   UCHAR_T scaddrlen = message->msg_p[pos];
   UCHAR_T * scaddr = new UCHAR_T[scaddrlen];

   pos++;
   memcpy(scaddr,message->msg_p+pos,scaddrlen);
   pos+=scaddrlen;

   UCHAR_T pduilen = message->msg_p[pos];
   UCHAR_T *pdu = new UCHAR_T[pduilen];
   pos++;
   memcpy(pdu,message->msg_p+pos,pduilen);

   UCHAR_T pdupos=0;
   SMS_DELIVERY_FORMAT_HEADER hdr;
   memcpy(&hdr,pdu + pdupos,sizeof(SMS_DELIVERY_FORMAT_HEADER));;
   pdupos++;
   
   /* multi message sending?*/
   if(!hdr.uu.s.mms)
    mdc->setMultiple(dlgid,true);
   else
    mdc->setMultiple(dlgid,false);


   /* address calculating  */
   UCHAR_T o_half_o_leh = pdu[pdupos++];
   UCHAR_T oraddrlen = 1 + o_half_o_leh/2 + ((o_half_o_leh % 2)?1:0);
   UCHAR_T * oraddr  =  new UCHAR_T[oraddrlen];
  // ////xmap_trace(logger," ---- %d oalen",oraddrlen);
   /* */

   memcpy(oraddr,pdu+pdupos,oraddrlen);
   pdupos+=oraddrlen;

   UCHAR_T protocol_idf = pdu[pdupos++];
   UCHAR_T dcs = pdu[pdupos++];

   MAP_TIMESTAMP mptime;
   memcpy(&mptime,pdu+pdupos,sizeof(MAP_TIMESTAMP));
   pdupos+=sizeof(MAP_TIMESTAMP);

   UCHAR_T userdatalen = pdu[pdupos++];

   char * lsztext = new char[MAX_SHORT_MESSAGE_LENGTH];
   
   int len = Convert7BitToText(( const char *)(pdu+pdupos),userdatalen,lsztext,MAX_SHORT_MESSAGE_LENGTH);

            int k=0;
   
   char * di=new char[200];
   std::string dest_imsi="";  
   int i=0;
                                                                                                                                            
   for (i=0;i<imsiLength;i++)
   {
    sprintf(di+i*2,"%d%d",imsi[i] & 0x0f,(imsi[i] & 0xf0)>>4);
   }
   di[i*2+1]=0;

   dest_imsi=di;
   delete di;

   lsztext[userdatalen]=0;

   std::string str_message =lsztext ;

   //PipeInfo * pi = pstoremessages->PipesList.getByImsi();//getPipeInfoByImsi(dest_imsi);
   
    if(!quall->PipesList.setText(dest_imsi,str_message,PF_SMS_RECIEVED,PL_FLAG_RX_FLAG))
    {
     xmap_trace(logger,"error! no <PipeInfoPtr> for IMSI'%s' ",dest_imsi.c_str());

    }
    
   
    quall->event_rx.Signal();
       
    xmap_trace(logger,"the %s recieved short message =\"%s\" len = %d userdatalen =%d",dest_imsi.c_str(),lsztext,len,userdatalen);


    //printf("\n%s %s\n",text.bytes,dest_imsi.c_str());
    //fflush(stdout);
    delete lsztext;
    delete scaddr;
    delete pdu;
    delete oraddr;

   }

  delete imsi;

  }else
  {
   xmap_trace(logger,"send fwd sm conf error_1");

  }


  __assign_message_(MAP_FWD_SM_CONF);

  SS7MapMsg messmaker;
  
  messmaker.insertUChar(MAP_FWD_SM_CONF);
  messmaker.insertUChar(SSN);
  messmaker.insertUShort(dlgid);

  messmaker.insertUChar(message->msg_p[4]);//invokeid

  messmaker.insertUChar(0);//error forward MO ERROR
  messmaker.insertUChar(0);//provider error

  
  __send_message_();

  mdc->setState(dlgid,map_After_RxSms);
 }
 else
 {
  bad_state(state);
 }
}

void CMapStateMashine::onOpenResponce(MSG_T* message)
{
 USHORT_T dlgid = getDialogId(message);
 
 if(message->msg_p[7]==RETURN_OK)
 {
  mdc->setState(dlgid,map_Opened);
 }
 else
 {
  xmap_trace(logger,"%s error!: in <open responce> for dialog %d",__func__,dlgid);
  mdc->setState(dlgid,DIALOG_FINISH);
 }

}
void CMapStateMashine::onForwardMOSMSResp(MSG_T* message)
{
 USHORT_T dlgid = getDialogId(message);

 if(mdc->getImsi(dlgid)==0)
 {
  xmap_trace(logger,"%s error imsi is empty for dialog %d",__func__,dlgid);

  return;
 }
 
 std:string imsi = (std::string)mdc->getImsi(dlgid); /*quall->PipesList.getImsiByDialogId(dlgid);*///getPipeInfoByImsi((*pDinfos)[dialogID].imsi);

 if(message->msg_p[5]==RETURN_OK)
 {
 
  if(imsi.length()>0)
  {
   quall->PipesList.setFlag(imsi,PF_SMS_SEND_CONFIRM,PL_FLAG_TX_FLAG);
  }


 }
 else
 {
  if(imsi.length()>0)
  {
   
  quall->PipesList.setFlag(imsi,PF_SMS_SEND_ERROR,PL_FLAG_TX_FLAG);
  }

   
  xmap_trace(logger,"error! forward sms for dialog %d %d",dlgid,message->msg_p[5] );

 }

 quall->event_tx.Signal();
}

void CMapStateMashine::onCloseReq(MSG_T* message)
{
 USHORT_T dlgid = getDialogId(message);
 mdc->setState(dlgid,DIALOG_FINISH);
}

void CMapStateMashine::onUserAbortRequest(MSG_T* message)
{
 onCloseReq(message);
}


// ussd primitives //////////////////////////////////////////////////////

void CMapStateMashine::onUssdRequest(MSG_T* message)
{

 smsc_log_info(logger,"%s",__func__);
 parseUssdReqResp(message);
 
}
void CMapStateMashine::onUssdResponce(MSG_T* message)
{
 smsc_log_info(logger,"%s",__func__);
 parseUssdReqResp(message);
}
void CMapStateMashine::onUssdNotifyRequest(MSG_T* message)
{
 smsc_log_info(logger,"%s",__func__);
 parseUssdReqResp(message);

}
void CMapStateMashine::onUssdNotifyResponce(MSG_T* message)
{
 smsc_log_info(logger,"%s",__func__);
 parseUssdReqResp(message);
  
}

// process_ussd_request responce recieved </PSSR>
void CMapStateMashine::onUssdClosing(MSG_T*message)
{
 smsc_log_info(logger,"%s",__func__);
 parseUssdReqResp(message);

 USHORT_T dlgid = getDialogId(message);
 mdc->setState(dlgid,DIALOG_FINISH);
 smsc_log_info(logger,"%d ussd dialog closed",dlgid);
}
//////////////////////////////////////////////////////////////////////////
// service tools

void CMapStateMashine::SendCPMessage(MSG_T * msg)
{
  MSG_T message_to_send;
  message_to_send.sender = msg->sender;
  message_to_send.receiver = msg->receiver;
  message_to_send.primitive = msg->primitive;
  message_to_send.remoteInstance = msg->remoteInstance;
  message_to_send.size= msg->size;

  USHORT_T res=0;

  xmap_trace(logger,"%s ",__func__);


  if(!connected)

  {
   res = MsgConn(MICROMAP_ID,GSMSC_ID);
   
  }

  if(res==RETURN_OK)
  {
   message_to_send.msg_p = (UCHAR_T*)malloc(msg->size);

   res = EINSS7CpGetMsgBuffer(&message_to_send);


   memcpy(message_to_send.msg_p,msg->msg_p,msg->size);

   if(res==EINSS7CP_MSG_GETBUF_FAIL)
   {
    xmap_error(logger,"error SendCPMessage-> return: %d ",res);
   }
  
   __dump__("<dump> %s",&message_to_send);

   
   res = MsgSend(&message_to_send); 


   if(res != RETURN_OK)
   {
    EINSS7CpReleaseMsgBuffer(&message_to_send);
    xmap_error(logger,"SendCPMessage->MsgSend->return = %d",res);
   }

   delete msg->msg_p;

   connected = true;
  }
  else
  {
   xmap_error(logger,"error SendCPMessage->unable to connect-> return: %d ",res);
  }
}

void CMapStateMashine::dumpmsg(const char *fmtstr,MSG_T* message)
{
int k=0; char str[256];
 for(int i=0;i<message->size;i++)
 {
  k += sprintf(str+k,"%02x ",message->msg_p[i]);
 }
 str[k]=0;


 xmap_trace(logger,fmtstr,str);

}

//////////////////////////////////////////////////////////////////////////



UCHAR_T CMapStateMashine::parseUssdReqResp(MSG_T *message)
{

  
 dumpmsg("USSD  %s",message);

 USHORT_T dlgid = getDialogId(message);


 if(mdc->getImsi(dlgid)==0)
 {
  xmap_error(logger,"%s error imsi is empty for dialog %d",__func__,dlgid);

  return 0xff;
 }

 UCHAR_T state =mdc->getState(dlgid);

 if(state==map_Opened)
 {
 
  UCHAR_T lssn  = message->msg_p[1];
  UCHAR_T invokeid = message->msg_p[4];
  UCHAR_T DCS = message->msg_p[5];

  if(DCS!=DataCoding::SMSC7BIT)
  {
   xmap_error(logger,"error %s %s DCS=%d",__func__,"error data coding unsupported",DCS);
   
  // quall->PipesList.setText(dest_imsi,"error",PF_USSD_RECIEVED,PL_FLAG_USSD_FLAG);
  // return 0xff;

  }

  /*else*/
  {
   UCHAR_T ussdStrLen = message->msg_p[6];
   std::string dest_imsi = mdc->getImsi(dlgid);
   UCHAR_T aePattern=0;
   
   if(ussdStrLen>0)
   {
    char * lsztext = new char[MAX_SHORT_MESSAGE_LENGTH];

    int len = Convert7BitToText(( const char *)(message->msg_p+7),
                ussdStrLen,
                lsztext,
                MAX_SHORT_MESSAGE_LENGTH);
                
 
    lsztext[ussdStrLen]=0;


    std::string ussd_str;

    if(lsztext && ussdStrLen)
       ussd_str = lsztext;
  
    delete lsztext;
    

    if(message->size==7+ussdStrLen+1) // alerting paatern if end =0 on error !=0
    {
      aePattern = message->msg_p[7+ussdStrLen];
      
      mdc->setState(dlgid,DIALOG_FINISH);
      quall->PipesList.setFlag(dest_imsi,PF_SMS_SEND_CONFIRM,PL_FLAG_TX_FLAG);

    }
    xmap_trace(logger,"%s ussd string is '%s' len1=%d len2=%d",__func__,ussd_str.c_str(),len,ussdStrLen);
    
    if(aePattern!=0)
    {
     xmap_trace(logger,"%s exror, Alerting pattern recieved  %d",__func__,aePattern);
    }
    
    

    if(!quall->PipesList.setText(dest_imsi,ussd_str,PF_USSD_RECIEVED,PL_FLAG_USSD_FLAG))
    {
     xmap_error(logger,"error! no <PipeInfoPtr> for IMSI'%s' ",dest_imsi.c_str());
     return 0xff;

    }
    

   }
   else
   {
    UCHAR_T ap;
    if(message->size==8) // alerting paatern if end =0 on error !=0
    {
      aePattern= message->msg_p[7];
      if(!aePattern)
       mdc->setState(dlgid,DIALOG_FINISH);

    }
    xmap_error(logger,"%s empty ussd responce, alerting patter =%d",__func__,aePattern);
    std::string strerr = "<null>";
    quall->PipesList.setText(dest_imsi,strerr,PF_USSD_RECIEVED,PL_FLAG_USSD_FLAG);
    return 0xff;

   }

  }


 }
 else
 {
  bad_state(state);
  return 0xff;
 }

 return 0;
}

