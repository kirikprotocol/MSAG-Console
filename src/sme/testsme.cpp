#include <stdio.h>
#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <exception>
#include "util/debug.h"

using namespace smsc::util;
using smsc::sms::SMS;

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
    SMS s;
    char oa[]="1",da[]="2";
    s.setOriginatingAddress(strlen(oa),1,1,oa);
    s.setDestinationAddress(strlen(da),1,1,da);
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
    sme.sendSms(&s);
    smsc::smpp::SmppHeader *pdu=sme.receiveSmpp(0);
    trace2("response status:%d\n",pdu->get_commandStatus());
  }catch(std::exception& e)
  {
    fprintf(stderr,"EX:%s\n",e.what());
  }
  fprintf(stderr,"exiting\n");
  return 0;
}
