//#define NOMINMAX
#include "sme/SmppBase.hpp"
#include <stdio.h>
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/buffers/Array.hpp"
#include "core/synchronization/EventMonitor.hpp"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;

const int MAX_UNRESPONDED_HIGH=5000;
const int MAX_UNRESPONDED_LOW=2000;


int stopped=0;

int rcnt=0;
int scnt=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      //char buf[256];
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
      scnt++;
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      //printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
      rcnt++;
    }
    disposePdu(pdu);
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
  if(argc!=6 && argc!=7 && argc!=8)
  {
    printf("usage: %s systemid host[:port] sourceaddr addrlistfile message [num=1 [delay=10]]\n",argv[0]);
    return -1;
  }
  SmeConfig cfg;
  string host=argc>2?argv[2]:"smsc";
  int pos=host.find(":");
  int port=9001;
  if(pos>0)
  {
    port=atoi(host.substr(pos+1).c_str());
    host.erase(pos);
  }
  string source=argv[1];
  if(argc>3)source=argv[3];
  cfg.host=host;
  cfg.port=port;
  cfg.sid=argv[1];
  cfg.timeOut=10;
  cfg.password=cfg.sid;
  MyListener lst;
  Array<string> addrs;
  FILE *f=fopen(argv[4],"rt");
  if(!f)
  {
    printf("failed to open addresslistfile\n");
    return -1;
  }
  char buf[1024];
  while(fgets(buf,sizeof(buf),f))
  {
    addrs.Push(buf);
    if(addrs[-1][addrs[-1].length()-1]==0x0a)addrs[-1].erase(addrs[-1].length()-1);
  }

  string msg=argv[5];

  int n=1;
  if(argc==7)
  {
    n=atoi(argv[6]);
  }
  int delay=10;
  if(argc==8)
  {
    delay=atoi(argv[7]);
  }

  SmppSession ss(cfg,&lst);
  try{
    ss.connect();
    PduSubmitSm sm;
    SMS s;
    Address addr(source.c_str());
    s.setOriginatingAddress(addr);
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    //s.setWaitTime(0);
    //time_t t=time(NULL)+60;
    s.setValidTime(0);
    //s.setSubmitTime(0);
    //s.setPriority(0);
    //s.setProtocolIdentifier(0);
    s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);
    s.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,3);
    //unsigned char message[]="SME test message";
    SmppTransmitter *tr=ss.getSyncTransmitter();
    SmppTransmitter *atr=ss.getAsyncTransmitter();
    lst.setTrans(tr);
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    int cnt=0;
    time_t lasttime=time(NULL);
    while(!stopped)
    {
      for(int i=0;i<addrs.Count();i++)
      {
        //Address dst(addrs[i].c_str());
        s.setDestinationAddress(addrs[i].c_str());
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
        s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msg.c_str(),msg.length());
        s.setIntProperty(Tag::SMPP_SM_LENGTH,msg.length());
        fillSmppPduFromSms(&sm,&s);
        atr->submit(sm);
        //Array<SMS*> smsarr;
        /*splitSms(&s,msg.c_str(),msg.length(),CONV_ENCODING_KOI8R,DataCoding::DEFAULT,smsarr);
        for(int x=0;x<smsarr.Count();x++)
        {
          fillSmppPduFromSms(&sm,smsarr[x]);
          atr->submit(sm);
          delete smsarr[x];
        }*/
        cnt++;
      }
      usleep(delay*1000);
      if((cnt%500)==0 || time(NULL)-lasttime>5)
      {
        printf("%d/%d/%d                       \r",rcnt,scnt,cnt);
        fflush(stdout);
        lasttime=time(NULL);
      }

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
