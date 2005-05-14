/*
Ussd Sme for XMAP application
by Mc.Green green@sibinco.ru
*/
#include "smet.h"
#include "syncque.hpp"

smsc::logger::Logger* smelogger;
extern bool stopProcess;
Event ussd_evt;
SmppTransmitter *tr;
extern qMessage_t message_store;

/** Listener to SME for USSD processing.
Main trend is RX message are moved to TX message.*/

  void MyListener::handleEvent(SmppHeader *pdu)
  {
   
 if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
   char msg[256];
   char message[256];
      getPduText(((PduXSm*)pdu),msg,sizeof(msg));
//////////////////////////////////////////////////////////////////////////
   printf("%s recieved '%s'",__func__,msg);
   smsc_log_info(smelogger,"%s recieved '%s'",__func__,msg);
   fflush(stdout);
   sprintf(message,"USSD response '%s'",msg);
      int len=strlen((char*)message);

//////////////////////////////////////////////////////////////////////////

   SMS xsms;
      fetchSmsFromSmppPdu(((PduXSm*)pdu),&xsms);


    Address src_addr;
       Address dest_addr;

    /* addressing exchange orig_new = dest_old etc...*/
       src_addr  = xsms.getDestinationAddress();//001
       dest_addr = xsms.getOriginatingAddress();//02001
    uint32_t mref =  xsms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
    
    
       xsms.setDestinationAddress(dest_addr);
       xsms.setOriginatingAddress(src_addr);

       smsc_log_info(smelogger,"%s, ora '%s' dsta '%s' 0x%.4x",__func__,src_addr.toString().c_str(),dest_addr.toString().c_str(),(USHORT_T)mref);

       xsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)message,len);
       xsms.setIntProperty(Tag::SMPP_SM_LENGTH,len);
       xsms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_RESP);
    xsms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,mref);
       xsms.setIntProperty(Tag::SMPP_DATA_CODING,0);

    
    PduSubmitSm sm;

    fillSmppPduFromSms(&sm,&xsms);

       PduSubmitSmResp *resp=trans->submit(sm); 
    
    smsc_log_info(smelogger,"%s SUBMIT STATUS = %d",__func__,resp->get_header().get_commandStatus());
    if(resp)disposePdu((SmppHeader*)resp);
        
    ussd_evt.Signal();
    }
 else
 {
   char msg[256];
   char message[256];
   getPduText(((PduXSm*)pdu),msg,sizeof(msg));
   printf("recieved '%s'",msg);
   smsc_log_info(smelogger,"%s cmd id %d recieved '%s'",__func__,pdu->get_commandId(),msg);
   fflush(stdout);
 
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
    ussd_evt.Wait();
    sleep(1);
    
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
