/********************************************************************
 created: 2005/05/17
 created: 17:5:2005   16:19
 filename:  D:\users\green\projects\smsc\xmap\smet.cpp
 file path: D:\users\green\projects\smsc\xmap
 file base: smet
 file ext: cpp
 author:  Gregory Panin
 
 purpose: 
*********************************************************************/

#include "smet.h"
#include "syncque.hpp"

smsc::logger::Logger* smelogger;
extern bool stopProcess;
Event ussd_evt;
SmppTransmitter *tr;
extern qMessage_t message_store;
SMS xsms;
bool bSend;
/** Listener to SME for USSD processing.
Main trend is RX message are moved to TX message.*/

typedef struct sm_st
{
 // for 1st send
 uint32_t seguencenumber;

 //for 2st send
 SMS s;

};

SyncQeuue<sm_st> smque;

void MyListener::handleEvent(SmppHeader *pdu)
{
   
 if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
  sm_st sm;
  sm.seguencenumber = pdu->get_sequenceNumber();
  fetchSmsFromSmppPdu(((PduXSm*)pdu),&sm.s);
  smque.Push(sm);

   // signalize to sender thread
    bSend = true;
    ussd_evt.Signal();

    }
 else
 {
   smsc_log_info(smelogger,"%s cmd id 0x%.8x  recieved",__func__,pdu->get_commandId());
   ussd_evt.Signal();
 }
 
  
}

void MyListener::handleError(int errorCode)
{
   printf("error!\n");
   stopProcess=1;
}

void MyListener::setTrans(SmppTransmitter *t)
{
    trans=t;
}

/**
Thread for running SME listener
*/


int UssdSmeRunner::Execute()
{

 bSend=false;
 // wait smsc powering
 while(message_store.bindOK!=2 && !stopProcess)
 {
  sleep(1);
 }
 smelogger =  Logger::getInstance("xmapussd");
 smsc_log_info(smelogger, "Starting...");

 SmeConfig cfg;
 cfg.host=message_store.config->getSmeHost();
 cfg.port=message_store.config->getSmePort();
 cfg.sid=message_store.config->getSmeSid();
 cfg.timeOut=10;
 cfg.password="";
 cfg.origAddr=message_store.config->getSmeAddress();

 MyListener lst;
 SmppSession ss(cfg,&lst);

 try{

   ss.connect();
   smsc_log_info(smelogger, "SME was connected.");

   tr=ss.getAsyncTransmitter();//Sync
   lst.setTrans(tr);
   
   while(!stopProcess)
   {
    //  [5/17/2005]
    // wait while ...
    ussd_evt.Wait();

    //send all
    if(bSend)
    {
     Reply();
     bSend = false;
    }
    
    
   }
  }

 catch(std::exception& e)
 {
  
 printf("%s Exception: %s\n",__func__,e.what());
 fflush(stdout);
 smsc_log_info(smelogger, "%s Exception %s",__func__,e.what());
 
  stopProcess=true;
 }
 catch(...)
 {
 printf("unknown exception\n");
 fflush(stdout);
  stopProcess=true;
 }

 ss.close();

return 1;
};


void UssdSmeRunner::Reply()
{

 sm_st it;
 if(smque.Pop(it,100))
 {
    PduDeliverySmResp dsmresp;
    dsmresp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
    dsmresp.set_messageId("");
    dsmresp.get_header().set_sequenceNumber(it.seguencenumber);
    dsmresp.get_header().set_commandStatus(0);
    tr->sendDeliverySmResp(dsmresp);
//////////////////////////////////////////////////////////////////////////
/*

   char msg[256];
   char message[256];
      getPduText(((PduXSm*)pdu),msg,sizeof(msg));

   //printf("%s recieved '%s'",__func__,msg);

   smsc_log_info(smelogger,"%s recieved '%s'",__func__,msg);
   fflush(stdout);
   sprintf(message,"%s",msg);
      int len=strlen((char*)message);
   fetchSmsFromSmppPdu(((PduXSm*)pdu),&xsms);
*/

//////////////////////////////////////////////////////////////////////////

    Address src_addr;
       Address dest_addr;

    /* addressing exchange orig_new = dest_old etc...*/
       src_addr  = it.s.getDestinationAddress();//001
       dest_addr = it.s.getOriginatingAddress();//02001
    uint32_t mref =  it.s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
    
    
    //dest_addr.setValue(5,"02001");
       it.s.setDestinationAddress(dest_addr);
       it.s.setOriginatingAddress(src_addr);


    //   it.s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)message,len);
    //   it.s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
       it.s.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_RESP);
    it.s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,mref);
       it.s.setIntProperty(Tag::SMPP_DATA_CODING,0);


    PduSubmitSm sm;
  fillSmppPduFromSms(&sm,&it.s);

  SmppHeader* resp;
  resp=(SmppHeader*)tr->submit(sm); 

  xmap_trace(smelogger,"%s resp is 0x%x trans is 0x%x",__func__,resp,tr);

   if(resp)
   {
    xmap_trace(smelogger,"%s command status =%d",__func__,resp->get_commandStatus());
    disposePdu((SmppHeader*)resp);
   }
    }

}
