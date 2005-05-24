#ifdef _WIN32
#include <winsock2.h>
#endif
#include "sme/SmppBase.hpp"
#include <stdio.h>
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/timeslotcounter.hpp"
#include "logger/Logger.h"
#include <vector>
#include "util/sleep.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace smsc::logger;

using namespace std;

const int MAX_UNRESPONDED_HIGH=5000;
const int MAX_UNRESPONDED_LOW=2000;


int stopped=0;

Mutex cntMutex;

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
      {
        MutexGuard g(cntMutex);
        reccnt++;
      }
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      __trace2__("received submit sm resp:%x, seq=%d\n",pdu->get_commandStatus(),pdu->get_sequenceNumber());
      //printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
      MutexGuard g(cntMutex);

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

struct Option{
  const char* name;
  char type;
  void* addr;
  int& asInt(){return *(int*)addr;}
  bool& asBool(){return *(bool*)addr;}
  string& asString(){return *(string*)addr;}
};

string host="sunfire";
int port=9001;

const int N=200;

int main(int argc,char* argv[])
{
  Logger::Init();
  try{

    SmeConfig cfg[N];

    for(int i=0;i<N;i++)
    {
      cfg[i].host=host;
      cfg[i].port=port;
      char buf[32];
      sprintf(buf,"testsme%d",i+1);
      cfg[i].sid=buf;
      cfg[i].timeOut=10;
      sprintf(buf,"sme%d",i+1);
      cfg[i].password=buf;
    }
    MyListener lst[N];

    SmppSession* ss[N];
    for(int i=0;i<N;i++)
    {
      ss[i]=new SmppSession(cfg[i],lst+i);
      lst[i].setTrans(ss[i]->getSyncTransmitter());
    }

    try{
      for(int i=0;i<N;i++)
      {
        printf("Connect: %d\r",i);fflush(stdout);
        ss[i]->connect();
      }
      printf("\n");

      PduSubmitSm sm;
      SMS s;
      char msc[]="123";
      char imsi[]="123";
      s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);

      s.setValidTime(0);

      s.setArchivationRequested(false);

      s.setIntProperty(Tag::SMPP_ESM_CLASS,0);

      s.setEServiceType("TEST");
      sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);

      int cnt=0;
      time_t lasttime=time(NULL);
      time_t starttime=lasttime;

      int overdelay=0;


      while(!stopped)
      {
        hrtime_t msgstart=gethrtime();
        string message;
        int msgLen=20+rand()%100;
        int wordLen=3+rand()%5;

        char letters[]="abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+|-=\\";

        for(int i=0;i<msgLen;i++)
        {
          if(wordLen)
          {
            message+=letters[rand()%sizeof(letters)];
            wordLen--;
          }else
          {
            message+=' ';
            wordLen=3+rand()%5;
          }
        }

        int org=rand()%N;
        int dst=rand()%N;
        char orgAddr[32];
        sprintf(orgAddr,"555%03d",org+1);
        char dstAddr[32];
        sprintf(dstAddr,"555%03d",dst+1);

        s.setDestinationAddress(orgAddr);
        s.setOriginatingAddress(dstAddr);

        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,message.c_str(),message.length());

        s.setIntProperty(Tag::SMPP_SM_LENGTH,0);

        fillSmppPduFromSms(&sm,&s);
        ss[org]->getAsyncTransmitter()->submit(sm);
        millisleep(9);
        cnt++;
        time_t now=time(NULL);
        if(now!=lasttime)
        {
          printf("Cnt:%d\r",cnt);fflush(stdout);
          lasttime=now;
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
    for(int i=0;i<N;i++)
    {
      ss[i]->close();
      delete ss[i];
    }
  }catch(exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  catch(...)
  {
    printf("unknown exception\n");
  }
  printf("Exiting\n");
  return 0;
}
