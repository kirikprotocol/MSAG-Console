#include <stdio.h>
#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <exception>
#include "util/debug.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#ifndef _WIN32
#include <signal.h>
#endif
#include "core/buffers/IntHash.hpp"

using namespace smsc::core::buffers;
using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using smsc::sms::SMS;
using namespace std;
using namespace smsc::smpp;

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
#ifdef _WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
      printf("%d\n",cnt-last);
      last=cnt;
      if(quit)break;
    }
    return 0;
  }
};

static const int MAXUNRESPONDED=15;
class Watcher{
public:
  void lock()
  {
    m.Lock();
  }
  void unlock()
  {
    m.Unlock();
  }

  void add(int key)
  {
    MutexGuard g(m);
    if(ih.Count()>=MAXUNRESPONDED)
    {
      IntHash<time_t>::Iterator i=ih.First();
      int key;
      time_t val,t=time(NULL);
      while(i.Next(key,val))
      {
        if(t-val>8)
        {
          ih.Delete(key);
        }
      }
      trace("wait until lowmark");
      m.wait();
#ifndef _WIN32
      trace2("lowmark:%d",ih.Count());
#endif
    }
    ih.Insert(key,time(NULL));
#ifndef _WIN32
    trace2("add:%d(%d)\n",key,ih.Count());
#endif
  }
  void remove(int key)
  {
    MutexGuard g(m);
    ih.Delete(key);
#ifndef _WIN32
    trace2("remove:%d(%d)\n",key,ih.Count());
#endif
    if(ih.Count()<MAXUNRESPONDED)
    {
      trace("lowmakr: notify");
      m.notify();
    }
  }
protected:
  EventMonitor m;
  IntHash<time_t> ih;
}watcher;


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
      watcher.remove(pdu->get_sequenceNumber());
      disposePdu(pdu);
      cnt++;
    }
    }catch(exception& e)
    {
#ifndef _WIN32
      trace2("rd ex:%s\n",e.what());
#endif
    }
    quit=true;
    return 0;
  }
  TestSme& sme;
};

class Writer:public Thread{
public:
  Writer(TestSme& testsme,const char *newsrc,const char* newdst):
    sme(testsme),src(newsrc),dst(newdst)
  {
  }
  int Execute()
  {
    try{
    for(;;)
    {
      SMS s;
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
      unsigned char message[]="SME test message";
      s.setMessageBody(sizeof(message),1,false,message);
      s.setEServiceType("XXX");
      trace("try to send sms");
      int seq=sme.getNextSeq();
      watcher.add(seq);
      if(sme.sendSms(&s,seq))
      {
        trace("sms sent ok");
      }else
      {
        watcher.remove(seq);
      }
      if(quit)break;
    }
    }catch(exception& e)
    {
#ifndef _WIN32
      trace2("wr ex:%s\n",e.what());
#endif
    }
    quit=true;
    return 0;
  }
  TestSme& sme;
  const char* src;
  const char* dst;
};

static void disp(int sig)
{
  trace("got user signal");
}

int main(int argc,char* argv[])
{
  if(argc!=5)
  {
    printf("Usage\n%s host port sysid/src dst\n",argv[0]);
    return -1;
  }
#ifndef _WIN32
  sigset(16,disp);
#endif
  TestSme sme(argv[1],atoi(argv[2]),argv[3]);
  try{
    if(!sme.init())throw Exception("connect failed!");
    trace("binding\n");
    sme.bindsme();
    trace("bind ok\n");
    Reader r(sme);
    Writer w(sme,argv[3],argv[4]);
    Monitor m;
    m.Start();
    r.Start();
    w.Start();
    m.WaitFor();
#ifndef _WIN32
    w.Kill(16);
    r.Kill(16);
#endif
    w.WaitFor();
    r.WaitFor();
  }catch(std::exception& e)
  {
    fprintf(stderr,"EX:%s\n",e.what());
  }
  fprintf(stderr,"exiting\n");
  return 0;
}
