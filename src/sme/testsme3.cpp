#include <stdio.h>
#include "../smpp/smpp.h"
#include "SmppBase.hpp"
#include "../sms/sms.h"
#include <unistd.h>

#include "../core/synchronization/Event.hpp"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

int stopped=0;
SmppTransmitter *tr;
int count=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
      SMS xsms;
      fetchSmsFromSmppPdu(((PduXSm*)pdu),&xsms);
      ////
      // processing here
      __trace2__("MAP::get SMS");
      {
        Address src_addr;
        Address dest_addr;
        dest_addr = xsms.getDestinationAddress();
        src_addr = xsms.getOriginatingAddress();
        xsms.setDestinationAddress(src_addr);
        xsms.setOriginatingAddress(dest_addr);
        xsms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_RESP);
      }
      // end of processing
      ////
      __trace2__("MAP::send SMS");
      PduSubmitSm sm;
      sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
      fillSmppPduFromSms(&sm,&xsms);
      PduSubmitSmResp *resp2 = tr->submit(sm);
    }
  }
  void handleError(int errorCode)
  {
    printf("error!\n");
    stopped=1;
  }

  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
};

int main(int argc,char* argv[])
{
  if(argc<4)
  {
    printf("usage: %s host port systemid(source_addres) destination_address [-start]\n",argv[0]);
    return -1;
  }
  SmeConfig cfg;
  cfg.host=argv[1];
  cfg.port=atoi(argv[2]);
  cfg.sid=argv[3];
  cfg.timeOut=10;
  cfg.password="";
  MyListener lst;
  SmppSession ss(cfg,&lst);
  try{
    ss.connect();
    /*    
    PduSubmitSm sm;
    SMS s;
    s.setOriginatingAddress(cfg.sid.length(),1,1,cfg.sid.c_str());
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    time_t t=time(NULL)+60;
    s.setValidTime(t);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    int send=(argc==6)&&!strcmp(argv[5],"-start");
    char message[64];
    */
    tr=ss.getSyncTransmitter();
    lst.setTrans(tr);
    while(!stopped)
    {
      /*
      if(send)
      {
        s.setDestinationAddress(strlen(argv[4]),1,1,argv[4]);
        sprintf(message,"%d",count);
        int len=strlen((char*)message);
        s.setBinProperty("SMPP_SHORT_MESSAGE",(char*)message,len);
        s.setIntProperty("SMPP_SM_LENGTH",len);

        fillSmppPduFromSms(&sm,&s);
        PduSubmitSmResp *resp=tr->submit(sm);
        if(resp)disposePdu((SmppHeader*)resp);

      }
      printf("%d\r",count);fflush(stdout);
      e.Wait();
      send=1;
      */
      sleep(1);
    }
  }
  catch(std::exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  catch(...)
  {
    printf("unknown exception\n");
  }
  ss.close();
  printf("Exiting\n");
  return 0;
}
