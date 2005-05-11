/*

  Panin Gregory 
  green@sibinco.ru

  изменения
  в
1: JSPipe::sendSms вставлена переменная m_dialogid 
2: простая - не растянутая сессия USSD -> <PSSR/>                                                                  
   в скрипте проверять флажки  таким образом: 
         JSPipe::isUssdSessionClsoed(); -проверка на активность сессии
         JSPipe::isUssdRecieved(); - проверка приемного буффера, есть ли ответ от центра  
*/

#include <string>
#include <jsapi.h>

#include "syncque.hpp"
#include "jspipe.h"
#include "common.h"
#include "xmap.h"
//#include "sms.h"
#include <sms/sms.h>
#include <util/recoder/recode_dll.h>
using namespace smsc::sms;
using namespace smsc::sms::Tag;

////
//////////////////////////////////////////////////////////////////////////
// pipe
extern qMessage_t message_store;
extern MapDialogContainer * g_mdc;

extern void _mkMapAddress( ET96MAP_ADDRESS_T *addr, const char *saddr, unsigned len);


//#define SendCPMessage(__a__) jsMobileTSendCPMessage(__a__)



JSPropertySpec JSPipe::Pipe_properties[] = 
{ 
    { "msisdn", msisdn_prop, JSPROP_ENUMERATE },
 { "imsi", imsi_prop, JSPROP_ENUMERATE },
 { "text", text_prop, JSPROP_ENUMERATE },
 { "absent",absent_prop,JSPROP_ENUMERATE},
 { "probability",probability_prop,JSPROP_ENUMERATE},
 { "error", error_info_prop, JSPROP_ENUMERATE },
 { 0 }
};

JSFunctionSpec JSPipe::Pipe_methods[] = 
{
    { "sendSms", sendSms, 3, 0, 0 },
 {"isSmsDone",isSmsDone,1,0,0},
 {"isRecieved",isRecieved,1,0,0},
 {"Register",Register,1,0,0},
 {"generateMsisdn",generateMsisdn,1,0,0},
 {"generateImsi",generateImsi,1,0,0},
 {"openUssdSession",openUssdSession,1,0,0},
 {"sendUssdMessage",sendUssdMessage,1,0,0},
 {"isUssdRecieved",isUssdRecieved,1,0,0},
 {"isUssdSessionClosed",isUssdSessionClosed,1,0,0},
    { 0, 0, 0, 0, 0 }
};

JSClass JSPipe::Pipe_class = 
{ 
 "Mobilka", JSCLASS_HAS_PRIVATE, JS_PropertyStub, JS_PropertyStub,
 JSPipe::JSGetProperty, JSPipe::JSSetProperty,
        JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JSPipe::JSDestructor
};

JSBool JSPipe::JSGetProperty(JSContext *cx, JSObject *obj, jsval id,jsval *vp)
{
    if (JSVAL_IS_INT(id)) 
 {
  JSPipe *p = (JSPipe *) JS_GetPrivate(cx, obj);
  Pipe *pipe = p->getPipe();
        switch (JSVAL_TO_INT(id)) 
  {
  case text_prop:
   {
    std::string text = pipe->GetText();
    JSString *str = JS_NewStringCopyN(cx, text.c_str(), text.length());
    *vp = STRING_TO_JSVAL(str);
   }
   break;
  case imsi_prop:
   {
    std::string imsi = pipe->GetImsi();
    JSString *str = JS_NewStringCopyN(cx, imsi.c_str(), imsi.length());
    *vp = STRING_TO_JSVAL(str);
   }
   break;
  case msisdn_prop:
   {
    std::string msisdn = pipe->GetMsisdn();
    JSString *str = JS_NewStringCopyN(cx, msisdn.c_str(), msisdn.length());
    *vp = STRING_TO_JSVAL(str);
   }
   break;
  case error_info_prop:
   {
    std::string einfo = pipe->GetErrorInfo();
    JSString *str = JS_NewStringCopyN(cx, einfo.c_str(), einfo.length());
    *vp = STRING_TO_JSVAL(str);
   }

   break;
  case absent_prop:
   {
    *vp=BOOLEAN_TO_JSVAL(pipe->GetAbsent());
   }
   break;
  case probability_prop:
   {
    *vp=INT_TO_JSVAL(pipe->GetProbability());
   }
   break;


        }
    }
    return JS_TRUE;
}

JSBool JSPipe::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, 
         jsval *vp)
{
    if (JSVAL_IS_INT(id)) 
 {
  JSPipe *p = (JSPipe *) JS_GetPrivate(cx, obj);
  Pipe *pip= p->getPipe();

        switch (JSVAL_TO_INT(id)) 
  {
  case imsi_prop:
   {
    JSString *str = JS_ValueToString(cx, *vp);
    std::string imsi = JS_GetStringBytes(str);
    pip->SetImsi(imsi);
   }
   break;

   case msisdn_prop:
   {
    JSString *str = JS_ValueToString(cx, *vp);
    std::string msisdn = JS_GetStringBytes(str);
    pip->SetMsisdn(msisdn);


   }
   break;
   // comment if recieved text is readonly
   case text_prop:
   {
    JSString *str = JS_ValueToString(cx, *vp);
    std::string text = JS_GetStringBytes(str);
    pip->SetText(text);
   }
   break;
   case error_info_prop:
    {
     //JSString *str = JS_ValueToString(cx, *vp);
     //std::string text = JS_GetStringBytes(str);
     //pip->SetText(text);
    }
   break;

   case absent_prop:
    {
   
     pip->SetAbsent(JSVAL_TO_BOOLEAN(*vp));
         
    }
    break;
   case probability_prop:
    {
     pip->SetProbability(JSVAL_TO_INT(*vp));
      
    }
    break;


  }
    }
    return JS_TRUE;
}

JSBool JSPipe::JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
         jsval *argv, jsval *rval)
{
 JSPipe *priv = new JSPipe();
 priv->setPipe(new Pipe());
 JS_SetPrivate(cx, obj, (void *) priv);

 priv->m_dialogid=0;

 //priv->getQall()->vPipesInform.insert(pi);

 return JS_TRUE;
}

void JSPipe::JSDestructor(JSContext *cx, JSObject *obj)
{
 JSPipe *priv = (JSPipe*) JS_GetPrivate(cx, obj);
 delete priv;
 priv = NULL;
}

JSObject *JSPipe::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
    JSObject *newProtoObj = JS_InitClass(cx, obj, proto, &Pipe_class, 
             JSPipe::JSConstructor, 0,
           NULL, JSPipe::Pipe_methods,
           NULL, NULL);
 JS_DefineProperties(cx, newProtoObj, JSPipe::Pipe_properties);


 return newProtoObj;
}

//////////////////////////////////////////////////////////////////////////

JSBool JSPipe::sendSms(JSContext *cx, JSObject *obj, uintN argc,
         jsval *argv, jsval *rval)
{
     JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);

  
  if(argc!=2) 
   return JS_FALSE;

////xmap_trace(logger,"jspipe:send sms 1");
  //CMMessage * message = new CMMessage();

  std::string s_destination_address = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
  std::string s_message_text= JS_GetStringBytes(JS_ValueToString(cx, argv[1]));
  std::string s_originating_address= p->getPipe()->GetMsisdn();
  std::string s_imsi= p->getPipe()->GetImsi();
  std::string s_mscaddr =p->getPipe()->GetMSCAddr();

/*  p->getQall()->PipesList.setDestAddr(s_imsi,(std::string)s_destination_address);
   p->getQall()->PipesList.setText(s_imsi,(std::string)s_message_text,PF_SMS_SEND,PL_FLAG_TX_FLAG);

  p->opendialog(s_destination_address,s_originating_address,s_message_text,s_imsi);

    //p->getQall()->qQoutgoing.Push(p->getPipe()->GetImsi());
#ifdef USE_ENEVTS_IN_QUEUE
  p->getQall()->event_outgoing_queue.Signal();
#endif*/


  MapDialogContainer * mdc =  p->getDialogConatiner();
  
  if(!mdc) 
   return JS_FALSE;

 p->m_dialogid = mdc->addDialog(s_imsi,s_originating_address);

 if(p->m_dialogid==0)// overflow dlgid 0xffff/2 max
 {
   *rval = BOOLEAN_TO_JSVAL(false);
   return JS_TRUE;
 }

 mdc->setMultiple(p->m_dialogid,false);

 ET96MAP_APP_CNTX_T cntx;
 cntx.acType = ET96MAP_SHORT_MSG_MO_RELAY;
 cntx.version= ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;

 p->sendOpenDialogInd(p->m_dialogid,cntx,s_originating_address,s_destination_address);//cdinfo status++ (in RX if confirmed --)
 p->forwardSMS(p->m_dialogid,s_originating_address,s_destination_address,s_message_text,s_mscaddr);
 p->sendDelimiterInd(p->m_dialogid);//cdinfo status++     (in RX if confirmed --)

 p->getQall()->map_statemachine_event.Signal();

  *rval = BOOLEAN_TO_JSVAL(true); 

 return JS_TRUE;
}

JSBool JSPipe::generateImsi(JSContext *cx, JSObject *obj, uintN argc,
       jsval *argv, jsval *rval)
{
 JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);


 if(argc!=2) 
  return JS_FALSE;
 
 int count = JSVAL_TO_INT(argv[0]);
 int value = JSVAL_TO_INT(argv[1]);

 char fmtstr[10];

 sprintf(fmtstr,"%s%dd","%.",count);

 char str[17];

 sprintf(str,fmtstr,value);

 std::string imsistr = str;
  p->getPipe()->SetImsi(imsistr);

 return JS_TRUE;
}

JSBool JSPipe::generateMsisdn(JSContext *cx, JSObject *obj, uintN argc,
       jsval *argv, jsval *rval)
{
 JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);


 if(argc!=2) 
  return JS_FALSE;

 int count = JSVAL_TO_INT(argv[0]);
 int value = JSVAL_TO_INT(argv[1]);

 char fmtstr[10];

 sprintf(fmtstr,"%s%dd","%.",count);
 
 char str[17];

 sprintf(str,fmtstr,value);

 std::string msisdnstr = str;
 p->getPipe()->SetMsisdn(msisdnstr);

 return JS_TRUE;
}


JSBool JSPipe::Register(JSContext *cx, JSObject *obj, uintN argc,
      jsval *argv, jsval *rval)
{

 JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);
  p->setDialogContainer(g_mdc);

 PipeInfo * pi = (PipeInfo*)malloc(sizeof(PipeInfo));

 pi->send_flag=PF_SMS_SEND_CONFIRM;
 
 pi->recv_flag=PF_SMS_RECIEVED;
  
 pi->error_flag=PF_NOTHING;

 std::string s_imsi   = p->getPipe()->GetImsi();
 std::string s_msisdn = p->getPipe()->GetMsisdn();

 if(s_imsi.length()==0 || s_msisdn.length()==0)
 {
  xmap_trace(logger,"error Rgistering mobile into HLR imsi ='%s' msisdn ='%s'",s_imsi.c_str(),s_msisdn.c_str());
  printf("\nerror Rgistering mobile into HLR imsi ='%s' msisdn ='%s'\n",s_imsi.c_str(),s_msisdn.c_str());
  fflush(stdout);
  
  return JS_FALSE;
 }

 pi->absent = p->getPipe()->GetAbsent();
 pi->probability = p->getPipe()->GetProbability();

 pi->imsi = new char[s_imsi.length()+1];
 memcpy(pi->imsi,s_imsi.c_str(),s_imsi.length()); 

 pi->msisdn = new char[s_msisdn.length()+1];
 memcpy(pi->msisdn,s_msisdn.c_str(),s_msisdn.length()); 

 pi->msisdn[s_msisdn.length()]=0;
 pi->imsi[s_imsi.length()]=0;

 pi->dialogid=0;
  
  
 p->setQall(&message_store);
 p->getQall()->PipesList.InserPipeInfo(pi);
 
 return JS_TRUE;
}
JSBool JSPipe::isSmsDone(JSContext *cx, JSObject *obj, uintN argc,
         jsval *argv, jsval *rval)
{
     JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);

  
  long flag = p->getQall()->PipesList.getFlag(p->getPipe()->GetImsi(),PL_FLAG_TX_FLAG);
      p->getPipe()->SetErrorInfo("");

  if(flag!=-1)
  {
   switch(flag)
   {
   case PF_SMS_SEND_CONFIRM:
    *rval = BOOLEAN_TO_JSVAL(true); 
    
    break;
   case PF_SMS_SEND_ERROR:
     p->getPipe()->SetErrorInfo("SMS_SEND_ERROR");
      *rval = BOOLEAN_TO_JSVAL(false);   
    break;
   default:
     
      *rval = BOOLEAN_TO_JSVAL(false);   
    break;
   }
   
   
   p->getQall()->PipesList.setFlag(p->getPipe()->GetImsi(),PF_NOTHING,PL_FLAG_TX_FLAG);
   
  }
   return JS_TRUE;
}

#define ERROR_RESOLVED_1

JSBool JSPipe::isRecieved(JSContext *cx, JSObject *obj, uintN argc,
         jsval *argv, jsval *rval)
{
     JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);
 
   
  long flag = p->getQall()->PipesList.getFlag(p->getPipe()->GetImsi(),PL_FLAG_RX_FLAG);

  *rval = BOOLEAN_TO_JSVAL(false);

  
  if(flag!=-1)
  {
  if (flag==PF_SMS_RECIEVED)
  {
#ifdef ERROR_RESOLVED_1
   std::string txt = p->getQall()->PipesList.getText(p->getPipe()->GetImsi());
   p->getPipe()->SetText(txt);
#endif

    *rval = BOOLEAN_TO_JSVAL(true); 
   p->getQall()->PipesList.setFlag(p->getPipe()->GetImsi(),PF_NOTHING,PL_FLAG_RX_FLAG);
    
  }
    
  }

  return JS_TRUE;
}


void JSPipe::sendOpenDialogInd(USHORT_T dlgid,ET96MAP_APP_CNTX_T cntx,std::string orastr,std::string dstastr)
{
 __assign_message_(MAP_OPEN_IND);

  SS7MapMsg messmaker;
  UCHAR_T applContext=cntx.acType;
  UCHAR_T acVersion=cntx.version;
  
  UCHAR_T oraddr_len = orastr.length();
  UCHAR_T dstaddr_len= dstastr.length();
 
  messmaker.insertUChar(MAP_OPEN_IND);
  messmaker.insertUChar(SSN);
  messmaker.insertUShort(dlgid);
  messmaker.insertUChar(applContext);
  messmaker.insertUChar(acVersion);

  ET96MAP_ADDRESS_T oraddr;
  ET96MAP_ADDRESS_T dsaddr;

  _mkMapAddress(&oraddr,orastr.c_str(),oraddr_len);
  _mkMapAddress(&dsaddr,dstastr.c_str(),dstaddr_len);

#ifdef OLD_ADDRESSING
  
  messmaker.insertUChar(dsaddr.addressLength/2+dsaddr.addressLength%2+1);
  messmaker.insertUChar(dsaddr.typeOfAddress);
  messmaker.insertPtr((UCHAR_T*)&dsaddr.addressLength,dsaddr.addressLength/2+dsaddr.addressLength%2);
#else
       int len = dsaddr.addressLength/2+dsaddr.addressLength%2+2;
   dsaddr.addressLength=len-1;; 
  messmaker.insertPtr((unsigned char *)&dsaddr,len);

#endif
  ///
  //messmaker.insertPtr((UCHAR_T*)&dsaddr ,dsaddr.addressLength%2+dsaddr.addressLength/2+2);
  //
  messmaker.insertUChar(0);//additional address len+addr
  //
  //messmaker.insertPtr((UCHAR_T*)&oraddr ,oraddr.addressLength%2+oraddr.addressLength/2+2);
  //
#ifdef  OLD_ADDRESSING
  
  messmaker.insertUChar(oraddr.addressLength/2+oraddr.addressLength%2+1);
  messmaker.insertUChar(oraddr.typeOfAddress);
  messmaker.insertPtr((UCHAR_T*)&oraddr.addressLength,oraddr.addressLength/2+oraddr.addressLength%2);
#else
  len = oraddr.addressLength/2+oraddr.addressLength%2+2;
  oraddr.addressLength=len-1;; 
  messmaker.insertPtr((unsigned char *)&oraddr,len);

#endif

  messmaker.insertUChar(0);//oref infolen
  messmaker.insertUChar(0);//spec info len 1
  messmaker.insertUChar(0);//spec info len 2

  __send_message_();

  xmap_trace(logger,"MAP_OPEN_IND dialog id   %d", dlgid);
}




ET96MAP_SM_RP_UI_T*  JSPipe::mkSubmitPDU(Address destaddr,const char * text,ET96MAP_SM_RP_UI_T* pdu,bool mms)
{

 memset(pdu,0,sizeof(ET96MAP_SM_RP_UI_T));

 SMS_SUMBMIT_FORMAT_HEADER* header = (SMS_SUMBMIT_FORMAT_HEADER*)pdu->signalInfo;

 
 header->u.head.mg_type_ind = 1;
 header->u.head.reject_dupl = 0;
 header->u.head.tp_vp=0; 
 header->u.head.reply_path = 0;
 header->u.head.udhi = 0;
 header->u.head.srr=0;
 header->mr=1; 


 SS7MapMsg messamaker;

 messamaker.insertPtr((UCHAR_T*)header,sizeof(SMS_SUMBMIT_FORMAT_HEADER));
 
 MAP_SMS_ADDRESS* da=(MAP_SMS_ADDRESS*)malloc(sizeof(MAP_SMS_ADDRESS)) ;

 
 da->st.ton = destaddr.getTypeOfNumber();//international isdn
 da->st.npi = destaddr.getNumberingPlan();

 da->st.reserved_1 = 1;

 unsigned da_length = (da->len+1)/2;

 da->len = destaddr.getLength();
 da_length = (da->len+1)/2;

 char* sval = destaddr.value;

 for ( int i=0; i<da->len; ++i )
 {
  int bi = i/2;
  if( i%2 == 1 )
  {
   da->val[bi] |= ((sval[i]-'0')<<4); // fill high octet
  }
  else
  {
   da->val[bi] = (sval[i]-'0')&0x0F; // fill low octet
  }
 }
 int p=0;
 if( da->len%2 != 0 ) 
 {
  da->val[da_length-1] |= 0xF0;
  p=1;
 }

 messamaker.insertPtr((UCHAR_T*)da,da->len);
 messamaker.insertUChar(0);//protocol identifier
 messamaker.insertUChar(0);//data coding


 UCHAR_T * tp_user_data = new UCHAR_T[200];
 unsigned elen=0;

 
 unsigned _7bit_text_len = ConvertTextTo7Bit((  const char*)text ,strlen(text)+1,(char * )tp_user_data,200,CONV_ENCODING_ANSI);


 messamaker.insertUChar(_7bit_text_len );
    messamaker.insertPtr(tp_user_data,_7bit_text_len);

 //////xmap_trace(logger,"7bittext len = %d",_7bit_text_len);
 
 pdu->signalInfoLen = messamaker.getallsize();

 messamaker.getAll(pdu->signalInfo);
 messamaker.free();

 delete da;
 delete tp_user_data;

 return pdu;

}

void JSPipe::forwardSMS(USHORT_T dlgid,std::string str_oa,std::string str_da,std::string str_text,std::string str_msc_addr)
{
 ET96MAP_SM_RP_UI_T pdu;
 ET96MAP_ADDRESS_T SM_RP_OA,SM_RP_DA;
 ET96MAP_SS7_ADDR_T fromAddress,toAddress;
 UCHAR_T InvokedId=0;


 smsc::sms::Address addr(str_da.c_str());

 mkSubmitPDU(addr,str_text.c_str(),&pdu,false);
 

  __assign_message_(MAP_FWD_SM_IND);
 
  _mkMapAddress( &SM_RP_OA,str_oa.c_str(),str_oa.length());//orig address
 _mkMapAddress( &SM_RP_DA, str_msc_addr.c_str(),str_msc_addr.length());//sms center address

  SS7MapMsg messmaker;
 
 messmaker.insertUChar(MAP_FWD_SM_IND);//MAP_FWD_SM_IND
 messmaker.insertUChar(SSN);
 messmaker.insertUShort(dlgid);
 messmaker.insertUChar(InvokedId);

 messmaker.insertUChar(ET96MAP_ADDRTYPE_SCADDR);

#ifdef OLD_ADDRESSING
 messmaker.insertUChar(SM_RP_DA.addressLength/2+2);
 messmaker.insertUChar(SM_RP_DA.typeOfAddress);
 messmaker.insertPtr((unsigned char*)&SM_RP_DA.address,SM_RP_DA.addressLength/2+1);
#else
    int len = SM_RP_DA.addressLength/2+SM_RP_DA.addressLength%2+2;
  SM_RP_DA.addressLength=len-1; 
 messmaker.insertPtr((unsigned char *)&SM_RP_DA,len);
#endif

 messmaker.insertUChar(ET96MAP_ADDRTYPE_SCADDR);

#ifdef OLD_ADDRESSING
 messmaker.insertUChar(SM_RP_OA.addressLength/2+2);
 messmaker.insertUChar(SM_RP_OA.typeOfAddress);
 messmaker.insertPtr((unsigned char*)&SM_RP_OA.address,SM_RP_OA.addressLength/2+1);
#else
 len = SM_RP_OA.addressLength/2+SM_RP_OA.addressLength%2+2;
  SM_RP_OA.addressLength=len-1;
 messmaker.insertPtr((unsigned char *)&SM_RP_OA,len);
#endif
 messmaker.insertUChar(pdu.signalInfoLen);
 messmaker.insertPtr((UCHAR_T*)pdu.signalInfo,pdu.signalInfoLen);

 __send_message_();

 xmap_trace(logger,"MAP_FWD_SM_IND dialog %d",dlgid);
 
}


void JSPipe::sendDelimiterInd(USHORT_T dlgid)
{
 __assign_message_(MAP_DELIMIT_IND);

 SS7MapMsg messmaker;
 UCHAR_T priorityOrder=0;

 messmaker.insertUChar(MAP_DELIMIT_IND);
 messmaker.insertUChar(SSN);
 messmaker.insertUShort(dlgid);
 messmaker.insertUChar(priorityOrder);

 __send_message_();
 xmap_trace(logger,"MAP_DELIMIT_IND dialog %d",dlgid);
 
};

void JSPipe::SendCPMessage(MSG_T * msg)
{
 MSG_T message_to_send;
 message_to_send.sender = msg->sender;
 message_to_send.receiver = msg->receiver;
 message_to_send.primitive = msg->primitive;
 message_to_send.remoteInstance = msg->remoteInstance;
 message_to_send.size= msg->size;

 USHORT_T res=0;

// if(!connected)
// {
  res = MsgConn(MICROMAP_ID,GSMSC_ID);
  
// }

 if(res==RETURN_OK)
 {
  message_to_send.msg_p = (UCHAR_T*)malloc(msg->size);

  res = EINSS7CpGetMsgBuffer(&message_to_send);


  memcpy(message_to_send.msg_p,msg->msg_p,msg->size);

  if(res==EINSS7CP_MSG_GETBUF_FAIL)
  {
   xmap_trace(logger,"%s error SendCPMessage-> return: %d ",__func__,res);
  }
 
 // dumpmsg("<dump> %s",&message_to_send);

  
  res = MsgSend(&message_to_send); 


  if(res != RETURN_OK)
  {
   EINSS7CpReleaseMsgBuffer(&message_to_send);
   xmap_trace(logger,"%s SendCPMessage->MsgSend->return = %d",__func__,res);
  }

  delete msg->msg_p;

  
  int k=0; char str[256];
  for(int i=0;i<message_to_send.size;i++)
  {
   k += sprintf(str+k,"%02x ",message_to_send.msg_p[i]);
  }
  str[k]=0;


  xmap_trace(logger,"dump in %s message %s",__func__,str);

//  connected = true;
 }
 else
 {
  xmap_trace(logger,"%s error SendCPMessage->unable to connect-> return: %d ",__func__,res);
 }
}

//////////////////////////////////////////////////////////////////////////
// USSD Jspipe routines
//////////////////////////////////////////////////////////////////////////

JSBool JSPipe::openUssdSession(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);
  
  if(argc<1) 
   return JS_FALSE;

  std::string s_message_text= JS_GetStringBytes(JS_ValueToString(cx, argv[0]));

  std::string s_originating_address= p->getPipe()->GetMsisdn();
  std::string s_imsi= p->getPipe()->GetImsi();
  std::string s_mscaddr =p->getPipe()->GetMSCAddr();


  MapDialogContainer * mdc =  p->getDialogConatiner();
  
  if(!mdc) 
   return JS_FALSE;

 p->m_dialogid = mdc->addDialog(s_imsi,s_originating_address);

 if(p->m_dialogid==0)// overflow dlgid 0xffff/2 max
 {
   *rval = BOOLEAN_TO_JSVAL(false);
   return JS_TRUE;
 }
 
 mdc->setMultiple(p->m_dialogid,false);
 ET96MAP_APP_CNTX_T cntx;
 cntx.acType = ET96MAP_SHORT_MSG_MO_RELAY;
 cntx.version= ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;

 /** send <PSSR> tag opening */
 p->sendOpenDialogInd(p->m_dialogid,cntx,s_originating_address,s_mscaddr);//cdinfo status++ (in RX if confirmed --)
 p->send_PSSR_or_USSR_UssdRequestInd(SSN,true,p->m_dialogid,s_originating_address,s_message_text);
    p->sendDelimiterInd(p->m_dialogid);
 
 //unused
 p->getQall()->map_statemachine_event.Signal();

    *rval = BOOLEAN_TO_JSVAL(true); 

 return JS_TRUE;
}

/************************************************************************/
/*@@JSBool JSPipe::sendUssdMessage()         */
/* для растянутого обмена                                               */
/************************************************************************/
JSBool JSPipe::sendUssdMessage(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 /* ответить можно только когда прийдет MAP_USSD_REQ*/

    *rval = BOOLEAN_TO_JSVAL(true); 
 return JS_TRUE;
}

/************************************************************************/
JSBool JSPipe::isUssdRecieved(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

 JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);
 
   
  long flag = p->getQall()->PipesList.getFlag(p->getPipe()->GetImsi(),PL_FLAG_USSD_FLAG);

  *rval = BOOLEAN_TO_JSVAL(false);

  
  if(flag!=-1)
  {
  if (flag==PF_USSD_RECIEVED)
  {

#ifdef ERROR_RESOLVED_1
   std::string txt = p->getQall()->PipesList.getText(p->getPipe()->GetImsi());
   p->getPipe()->SetText(txt);
#endif

    *rval = BOOLEAN_TO_JSVAL(true); 
   p->getQall()->PipesList.setFlag(p->getPipe()->GetImsi(),PF_NOTHING,PL_FLAG_RX_FLAG);
    
  }
    
  }

 return JS_TRUE;
}
/************************************************************************/
JSBool JSPipe::isUssdSessionClosed(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 
    JSPipe *p = (JSPipe*) JS_GetPrivate(cx, obj);
    MapDialogContainer * mdc =  p->getDialogConatiner();

 *rval = BOOLEAN_TO_JSVAL(mdc->hasDialog(p->m_dialogid)); 

 return JS_TRUE;
}
/************************************************************************/
void JSPipe::send_PSSR_or_USSR_UssdRequestInd(UCHAR_T subsistem_number, bool isPSSR,USHORT_T dlgid,std::string str_da,std::string str_text)
{
 ET96MAP_ADDRESS_T SM_RP_DA;
 UCHAR_T InvokedId=0;
 UCHAR_T primitive = isPSSR?MAP_PROC_USSD_IND:MAP_USSD_IND;
 
 
  __assign_message_(primitive);

 if(str_da.length()>0)
  _mkMapAddress(&SM_RP_DA,str_da.c_str(),str_da.length());//sms center address

  SS7MapMsg messmaker;
 
 messmaker.insertUChar(primitive);
 messmaker.insertUChar(subsistem_number);
 messmaker.insertUShort(dlgid);
 messmaker.insertUChar(InvokedId);

 messmaker.insertUChar(DataCoding::SMSC7BIT);

 UCHAR_T * tp_user_data = new UCHAR_T[200];
 unsigned elen=0;

 
 if(str_text.length()>0)
 {
  unsigned _7bit_text_len = ConvertTextTo7Bit((  const char*)str_text.c_str(),
             str_text.length() +1,
             (char * )tp_user_data,
             200,CONV_ENCODING_ANSI);


  messmaker.insertUChar(_7bit_text_len );
  messmaker.insertPtr(tp_user_data,_7bit_text_len);
 }
 else
 {
  messmaker.insertUChar(0); //empty string ????
 }

 if(str_da.length()>0)
 {
#ifdef OLD_ADDRESSING
  messmaker.insertUChar(SM_RP_DA.addressLength/2+2);
  messmaker.insertUChar(SM_RP_DA.typeOfAddress);
  messmaker.insertPtr((unsigned char*)&SM_RP_DA.address,SM_RP_DA.addressLength/2+1);
#else
  // тут MSISDN аддресс - длин а вполуоктеттах не включая типа адреса.
  
        int len = SM_RP_DA.addressLength/2+SM_RP_DA.addressLength%2+2;
  
  messmaker.insertPtr((unsigned char *)&SM_RP_DA,len);

#endif
 }
 

 __send_message_();

 delete tp_user_data;
 
 xmap_trace(logger,"%s dialog %d",__func__,dlgid);
}
