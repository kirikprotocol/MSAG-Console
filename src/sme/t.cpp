#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include <unistd.h>

using namespace smsc::sms;
using namespace smsc::sme;

int stopped=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    printf("event!\n");
  }
  void handleError(int errorCode)
  {
    printf("error!\n");
    stopped=1;
  }
};

int main(int argc,char* argv[])
{
  SmeConfig cfg;
  cfg.host="smsc";
  cfg.port=9002;
  cfg.sid="1";
  cfg.timeOut=10;
  cfg.password="";
  MyListener lst;
  SmppSession ss(cfg,&lst);
  try{
    ss.connect();
    sleep(1);
  {
    unsigned char message[512];
    printf("Enter message:");fflush(stdout);
    fgets((char*)message,sizeof(message),stdin);
    printf("Accepted:%s\n",message);fflush(stdout);
  }
    PduSubmitSm sm;
    SMS s;
    const char *src="1";
    const char *dst="2";
    s.setOriginatingAddress(strlen(src),0,0,src);
    s.setDestinationAddress(strlen(dst),0,0,dst);
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    s.setWaitTime(0);
    time_t t=time(NULL)+60;
    s.setValidTime(t);
    //s.setSubmitTime(0);
    s.setPriority(0);
    s.setProtocolIdentifier(0);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);
    //unsigned char message[]="SME test message";
    SmppTransmitter *tr=ss.getSyncTransmitter();
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    while(!stopped)
    {
      unsigned char message[512];
      printf("Enter message:");fflush(stdout);
      fgets((char*)message,sizeof(message),stdin);
      printf("Accepted\n");fflush(stdout);
      for(int i=0;message[i];i++)
      {
        if(message[i]<32)message[i]=32;
      }
      s.setMessageBody(strlen((char*)message),1,false,message);
      fillSmppPduFromSms(&sm,&s);
      PduSubmitSmResp *resp=tr->submit(sm);
      disposePdu((SmppHeader*)resp);
    }
  }catch(...)
  {
    printf("exception\n");
  }
  return 0;
}
