#include <stdio.h>
#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <exception>
#include "util/debug.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using smsc::sms::SMS;
using namespace std;

class TestSme:public smsc::sme::BaseSme{
public:
  TestSme(const char *host,int port,const char *sysid)
  :BaseSme(host,port,sysid){}
  bool processSms(smsc::sms::SMS *sms){return false;}
};


int cnt=0;
bool quit=false;

class Monitor:public Thread{
public:
  int Execute()
  {
    int last=0;
    for(;;)
    {
      sleep(1);
      printf("%d\n",cnt-last);
      last=cnt;
      if(quit)break;
    }
    return 0;
  }
};

class Counter{
  static const int HIGHMARK=50;
  static const int LOWWARK=49;
public:

  Counter():count(0){}
  void inc()
  {
    MutexGuard g(m);
    if(count>=HIGHMARK)
    {
      trace("wait until lowmark");
      m.wait();
      trace2("lowmark:%d",count);
    }
    count++;
    trace2("inc:%d\n",count);
  }
  void dec()
  {
    MutexGuard g(m);
    count--;
    trace2("dec:%d\n",count);
    if(count<HIGHMARK)
    {
      trace("lowmakr: notify");
      m.notify();
    }
  }
protected:
  EventMonitor m;
  int count;
}counter;

class Reader:public Thread{
public:
  Reader(TestSme& testsme):sme(testsme)
  {
  }
  int Execute()
  {
    try{
    for(;;)
    {
      smsc::smpp::SmppHeader *pdu=sme.receiveSmpp(8000);
      if(quit)break;
      __trace2__("SUBMIT resp: seq=%d",pdu->get_sequenceNumber());
      disposePdu(pdu);
      counter.dec();
    }
    }catch(exception& e)
    {
      trace2("ex:%s\n",e.what());
    }
    return 0;
  }
  TestSme& sme;
};

class Writer:public Thread{
public:
  Writer(TestSme& testsme):sme(testsme)
  {
  }
  int Execute()
  {
    try{
    for(;;)
    {
      SMS s;
      char oa[]="1",da[]="2";
      s.setOriginatingAddress(strlen(oa),0,0,oa);
      s.setDestinationAddress(strlen(da),0,0,da);
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
      unsigned char message[]="SME test message";
      s.setMessageBody(sizeof(message),1,false,message);
      s.setEServiceType("XXX");
      trace("try to send sms");
      sme.sendSms(&s);
      cnt++;
      counter.inc();
    }
    }catch(exception& e)
    {
      trace2("ex:%s\n",e.what());
    }
    return 0;
  }
  TestSme& sme;
};

int main(int argc,char* argv[])
{
  if(argc!=4)
  {
    printf("Usage\n%s host port sysid\n",argv[0]);
    return -1;
  }
  TestSme sme(argv[1],atoi(argv[2]),argv[3]);
  try{
    if(!sme.init())throw Exception("connect failed!");
    trace("binding\n");
    sme.bindsme();
    trace("bind ok\n");
    Reader r(sme);
    Writer w(sme);
    Monitor m;
    m.Start();
    r.Start();
    w.Start();
    w.WaitFor();
    quit=true;
    r.Kill(16);
    r.WaitFor();
    m.WaitFor();
  }catch(std::exception& e)
  {
    fprintf(stderr,"EX:%s\n",e.what());
  }
  fprintf(stderr,"exiting\n");
  return 0;
}
