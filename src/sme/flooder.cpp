//#define NOMINMAX
#include "sme/SmppBase.hpp"
#include <stdio.h>
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/buffers/Array.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/timeslotcounter.hpp"
#include "logger/Logger.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace smsc::logger;

const int MAX_UNRESPONDED_HIGH=5000;
const int MAX_UNRESPONDED_LOW=2000;


int stopped=0;

int sokcnt=0;
int serrcnt=0;
int reccnt=0;

TimeSlotCounter<> sok_time_cnt(30,100);

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
      resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      trans->sendDeliverySmResp(resp);
      reccnt++;
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      __trace2__("received submit sm resp:%x, seq=%d\n",pdu->get_commandStatus(),pdu->get_sequenceNumber());
      //printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
      if(pdu->get_commandStatus()==SmppStatusSet::ESME_ROK)
      {
        sokcnt++;
        sok_time_cnt.Inc();
      }else
      {
        serrcnt++;
      }
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
    printf("usage: %s systemid host[:port] sourceaddr addrlistfile message/file [num=1 [delay=10]]\n",argv[0]);
    return -1;
  }
  Logger::Init("log4cpp.flooder");
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
  cfg.smppTimeOut=120;
  cfg.password=cfg.sid;
  MyListener lst;
  Array<char*> addrs;
  FILE *f=fopen(argv[4],"rt");
  if(!f)
  {
    printf("failed to open addresslistfile\n");
    return -1;
  }
  char buf[4096];
  while(fgets(buf,sizeof(buf),f))
  {
    int l=strlen(buf);
    if(buf[l-1]==0x0a){buf[l-1]=0;}
    addrs.Push(strdup(buf));
  }
  fclose(f);

  Array<string> msgs;

  f=fopen(argv[5],"rt");
  if(f)
  {
    while(fgets(buf,sizeof(buf),f))
    {
      msgs.Push(buf);
      if(msgs[-1][msgs[-1].length()-1]==0x0a)msgs[-1].erase(msgs[-1].length()-1);
    }
    fclose(f);
  }else
  {
    msgs.Push(argv[5]);
  }

  if(msgs.Count()==0)
  {
    msgs.Push("Empty message");
  }

  int n=1;
  if(argc>=7)
  {
    n=atoi(argv[6]);
  }
  int delay=10;
  if(argc==8)
  {
    delay=atoi(argv[7]);
  }

  Event slev;

  SmppSession ss(cfg,&lst);
  SmppTransmitter *tr=ss.getSyncTransmitter();
  SmppTransmitter *atr=ss.getAsyncTransmitter();
  lst.setTrans(tr);
  int msgidx=0;
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
    s.setIntProperty(Tag::SMPP_ESM_CLASS,1);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);
    s.setIntProperty(Tag::SMPP_MS_VALIDITY,3);

    //unsigned char message[]="SME test message";
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    int cnt=0;
    time_t lasttime=time(NULL);
    time_t starttime=lasttime;
    int i=0;
    while(!stopped)
    {
      for(int j=0;j<n;j++)
      {
        //for(int i=0;i<addrs.Count();i++)
        {
          //Address dst(addrs[i].c_str());
          s.setDestinationAddress(addrs[i]);
          s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msgs[msgidx].c_str(),msgs[msgidx].length());
          s.setIntProperty(Tag::SMPP_SM_LENGTH,0);
          fillSmppPduFromSms(&sm,&s);
          atr->submit(sm);
          cnt++;
          msgidx++;
          if(msgidx>=msgs.Count())msgidx=0;
          i++;
          if(i>=addrs.Count())i=0;
        }
      }
      slev.Wait(delay);
      time_t now=time(NULL);
      if((cnt%500)==0 || now-lasttime>5)
      {
        printf("ut:%d sbm:%d sok:%d serr:%d recv:%d avgsp: %lf lstsp:%lf\n",
          now-starttime,cnt,sokcnt,serrcnt,reccnt,
          (double)sokcnt/(now-starttime),
          (double)sok_time_cnt.Get()/30.0
          );
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
