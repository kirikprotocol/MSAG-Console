#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include <unistd.h>
#include "util/smstext.h"

#include "core/synchronization/Event.hpp"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;
using namespace smsc::util;

int stopped=0;
SmppTransmitter *tr;
Event e;
int count=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      char msg[256];
      getPduText(((PduXSm*)pdu),msg,sizeof(msg));
      count=atoi(msg)+1;
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
      e.Signal();
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
    PduSubmitSm sm;
    SMS s;
    Address addr(cfg.sid.c_str());
    s.setOriginatingAddress(addr);
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    time_t t=time(NULL)+60;
    s.setValidTime(t);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);
    tr=ss.getSyncTransmitter();
    lst.setTrans(tr);
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    int send=(argc==6)&&!strcmp(argv[5],"-start");
    char message[64];
    s.setDestinationAddress(argv[4]);

    while(!stopped)
    {
      if(send)
      {
        sprintf(message,"%d",count);
        int len=strlen((char*)message);
        s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)message,len);
        s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
        s.setIntProperty(Tag::SMPP_DATA_CODING,0);

        fillSmppPduFromSms(&sm,&s);
        PduSubmitSmResp *resp=tr->submit(sm);
        if(resp)disposePdu((SmppHeader*)resp);

      }
      printf("%d\r",count);fflush(stdout);
      e.Wait();
      send=1;
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
