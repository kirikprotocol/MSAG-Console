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

  void MyListener::handleEvent(SmppHeader *pdu)
  {
   
 if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {

//////////////////////////////////////////////////////////////////////////
      PduDeliverySmResp dsmresp;
   dsmresp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
   dsmresp.set_messageId("");
   dsmresp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
   dsmresp.get_header().set_commandStatus(0);
   trans->sendDeliverySmResp(dsmresp);
//////////////////////////////////////////////////////////////////////////

   char msg[256];
   char message[256];
      getPduText(((PduXSm*)pdu),msg,sizeof(msg));

   //printf("%s recieved '%s'",__func__,msg);

   smsc_log_info(smelogger,"%s recieved '%s'",__func__,msg);
   fflush(stdout);
   sprintf(message,"%s",msg);
      int len=strlen((char*)message);

//////////////////////////////////////////////////////////////////////////

   
      fetchSmsFromSmppPdu(((PduXSm*)pdu),&xsms);


    Address src_addr;
       Address dest_addr;

    /* addressing exchange orig_new = dest_old etc...*/
       src_addr  = xsms.getDestinationAddress();//001
       dest_addr = xsms.getOriginatingAddress();//02001
    uint32_t mref =  xsms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
    
    
    //dest_addr.setValue(5,"02001");
       xsms.setDestinationAddress(dest_addr);
       xsms.setOriginatingAddress(src_addr);


       smsc_log_info(smelogger,"%s, ora '%s' dsta '%s' 0x%.4x   string length= %d",__func__,src_addr.toString().c_str(),dest_addr.toString().c_str(),(USHORT_T)mref,len);

       xsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)message,len);
       xsms.setIntProperty(Tag::SMPP_SM_LENGTH,len);
       xsms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_RESP);
    xsms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,mref);
       xsms.setIntProperty(Tag::SMPP_DATA_CODING,0);

    // signalize to sender thread
    bSend = true;
    ussd_evt.Signal();

    }
 else
 {
   smsc_log_info(smelogger,"%s cmd id 0x%.8x  recieved",__func__,pdu->get_commandId());
 
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

   tr=ss.getSyncTransmitter();
   lst.setTrans(tr);
   
   while(!stopProcess)
   {
    //  [5/17/2005]
    // wait while ...
    ussd_evt.Wait();

    //send all
    if(bSend)
    {
     PduSubmitSm sm;
     fillSmppPduFromSms(&sm,&xsms);

     SmppHeader* resp;
     resp=(SmppHeader*)tr->submit(sm); 
     
     smsc_log_info(smelogger,"%s resp is 0x%x trans is 0x%x",__func__,resp,tr);

     if(resp)
     {
      smsc_log_info(smelogger,"%s command status =%d",__func__,resp->get_commandStatus());
      disposePdu((SmppHeader*)resp);
     }
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
