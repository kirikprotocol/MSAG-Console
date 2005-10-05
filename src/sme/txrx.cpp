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


int stopped=0;

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
      resp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      trans->sendDeliverySmResp(resp);
    }
    disposePdu(pdu);
  }
  void handleError(int errorCode)
  {
    //printf("\nError:%d\n\n",errorCode);
    printf("?");fflush(stdout);
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

const int N=10;


void shuffle(int n,vector<int>& v)
{
  int idx;
  v.resize(n);
  for(int i=0;i<n;i++)
  {
    idx=1.0*rand()*i/RAND_MAX;
    v[i]=v[idx];
    v[idx]=i;
  }
}

int main(int argc,char* argv[])
{
  Logger::Init();
  try{

    SmeConfig cfg[N*2];

    for(int i=0;i<N*2;i++)
    {
      cfg[i].host=host;
      cfg[i].port=port;
      char buf[32];
      sprintf(buf,"testsme%03d",(i/2)+1);
      cfg[i].sid=buf;
      cfg[i].timeOut=10;
      sprintf(buf,"sme%03d",(i/2)+1);
      cfg[i].password=buf;
    }
    MyListener lst[N*2];

    SmppSession* ss[N*2];
    for(int i=0;i<N*2;i++)
    {
      ss[i]=new SmppSession(cfg[i],lst+i);
      lst[i].setTrans(ss[i]->getSyncTransmitter());
    }

    try{
      for(;;)
      {
        vector<int> v;
        shuffle(N*2,v);
        printf("Connect:");
        for(int i=0;i<N*2;i++)
        {
          printf(".",i);fflush(stdout);
          int idx=v[i];
          for(;;)
          {
            try{
              __trace2__("bind %d as %s",idx,(idx&1)?"BindType::Receiver":"BindType::Transmitter");
              ss[idx]->connect((idx&1)?BindType::Receiver:BindType::Transmitter);
              break;
            }catch(...)
            {
              printf("!");fflush(stdout);
              continue;
            }
          }
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

        try{
          stopped=0;
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
            ss[org*2]->getAsyncTransmitter()->submit(sm);
            millisleep(9);
            cnt++;
            //time_t now=time(NULL);
            //if(now!=lasttime)
            {
              printf("Cnt:%d\r",cnt);fflush(stdout);
//              lasttime=now;
            }
            if(cnt==100)break;
          }
        }
        catch(std::exception& e)
        {
          printf("\nException: %s\n\n",e.what());
        }
        catch(...)
        {
          printf("\nunknown exception\n\n");
        }
        printf("\n");
        shuffle(N*2,v);
        for(int i=0;i<N*2;i++)
        if(rand()&1)
        {
          PduUnbind pdu;
          pdu.get_header().set_commandId(SmppCommandSet::UNBIND);
          pdu.get_header().set_sequenceNumber(ss[v[i]]->getNextSeq());
          ss[v[i]]->getAsyncTransmitter()->sendPdu((SmppHeader*)&pdu);
        }
        shuffle(N*2,v);
        for(int i=0;i<N*2;i++)ss[v[i]]->close();
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
