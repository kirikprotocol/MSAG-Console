#include <stdio.h>
#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <exception>
#include "util/debug.h"
#include "core/threads/Thread.hpp"

using namespace smsc::util;
using namespace smsc::core::threads;
using smsc::sms::SMS;
using namespace smsc::smpp;

class TestSme:public smsc::sme::BaseSme{
public:
  TestSme(const char *host,int port,const char *sysid)
  :BaseSme(host,port,sysid){}
  bool processSms(smsc::sms::SMS *sms){return false;}
};

int cnt=0;

class Monitor:public Thread{
  bool stopped;
public:
  Monitor():stopped(false){}
  int Execute()
  {
    for(;!stopped;)
    {
      printf("%d          \r",cnt);fflush(stdout);
      sleep(1);
    }
  }
  void stop(){stopped=true;}
};


int main(int argc,char* argv[])
{
  if(argc!=4)
  {
    printf("Usage\n%s host port sysid\n",argv[0]);
    return -1;
  }
  TestSme sme(argv[1],atoi(argv[2]),argv[3]);
  Monitor m;
  m.Start();
  try{
    if(!sme.init())throw Exception("connect failed!");
    trace("binding\n");
    sme.bindsme();
    trace("bind ok\n");
    //sleep(20);
    //return -1;
    for(;;)
    {
      smsc::smpp::SmppHeader *pdu=sme.receiveSmpp(0);
      trace("smpp received");
      if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
      {
        SMS sms;
        if(fetchSmsFromSmppPdu(reinterpret_cast<PduXSm*>(pdu),&sms))
        {
          trace("Gotcha!\n");
          unsigned char buf[smsc::sms::MAX_SHORT_MESSAGE_LENGTH];
          int len=sms.getMessageBody().getData(buf);
          buf[len]=0;
          trace2("%d:%s\n",len,buf);
          PduDeliverySmResp resp;
          resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
          resp.set_messageId("0");
          trace2("DELIVER: seq=%d",pdu->get_sequenceNumber());
          resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
          sme.sendSmpp(&resp.get_header());
          cnt++;
        }else
        {
          trace("Ooops.\n");
        }
      }else
      {
        trace2("??? Command id:%d\n",pdu->get_commandId());
      }
      disposePdu(pdu);
    }
    //trace2("response status:%d\n",pdu->get_commandId());
  }catch(std::exception& e)
  {
    fprintf(stderr,"EX:%s\n",e.what());
  }
  fprintf(stderr,"exiting\n");
  return 0;
}
