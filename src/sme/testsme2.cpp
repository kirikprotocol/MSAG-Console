#include <stdio.h>
#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <exception>
#include "util/debug.h"

using namespace smsc::util;
using smsc::sms::SMS;
using namespace smsc::smpp;

class TestSme:public smsc::sme::BaseSme{
public:
  TestSme(const char *host,int port,const char *sysid)
  :BaseSme(host,port,sysid){}
  bool processSms(smsc::sms::SMS *sms){}
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
    smsc::smpp::SmppHeader *pdu=sme.receiveSmpp(0);
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      SMS sms;
      if(fetchSmsFromSmppPdu(reinterpret_cast<PduXSm*>(pdu),&sms))
      {
        printf("Gotcha!\n");
        unsigned char buf[smsc::sms::MAX_SHORT_MESSAGE_LENGTH];
        int len=sms.getMessageBody().getData(buf);
        buf[len]=0;
        printf("%s\n",buf);
      }else
      {
        printf("Ooops.\n");
      }
    }
    //trace2("response status:%d\n",pdu->get_commandId());
  }catch(std::exception& e)
  {
    fprintf(stderr,"EX:%s\n",e.what());
  }
  fprintf(stderr,"exiting\n");
  return 0;
}
