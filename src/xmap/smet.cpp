/*
Ussd Sme for XMAP application
by Mc.Green green@sibinco.ru
*/
#include "smet.h"
smsc::logger::Logger* smelogger;
extern bool stopProcess;
Event e;

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
	  printf("recieved '%s'",msg);
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
       src_addr  = xsms.getDestinationAddress();
       dest_addr = xsms.getOriginatingAddress();

	   
       xsms.setDestinationAddress(src_addr);
       xsms.setOriginatingAddress(dest_addr);
       xsms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)message,len);
       xsms.setIntProperty(Tag::SMPP_SM_LENGTH,len);
       xsms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_RESP);

	   PduSubmitSm sm;

	   fillSmppPduFromSms(&sm,&xsms);

       PduSubmitSmResp *resp=tr->submit(sm);	

	   if(resp)disposePdu((SmppHeader*)resp);

      e.Signal();
    }
  }
  void MyListener::handleError(int errorCode)
  {
    printf("error!\n");
    stopped=1;
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

	smelogger =  Logger::getInstance("xmapussd");
	smsc_log_info(smelogger, "Starting...");

	SmeConfig cfg;
	cfg.host="localhost";
	cfg.port=29012;
	cfg.sid="t1";
	cfg.timeOut=10;
	cfg.password="";
	cfg.origAddr="001";
	MyListener lst;
	SmppSession ss(cfg,&lst);

	try{
			ss.connect();
			smsc_log_info(smelogger, "SME was connected.");
			while(!stopProcess)
			{
				e.Wait();
				
			}
		}

	catch(std::exception& e)
	{
		
	printf("%s Exception: %s\n",__func__,e.what());
	smsc_log_info(smelogger, "%s Exception %s",__func__,e.what());
	
	stopProcess=true;
	}
	catch(...)
	{
	printf("unknown exception\n");
	}

	ss.close();

return 1;
};
