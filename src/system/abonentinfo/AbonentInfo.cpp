#include "system/abonentinfo/AbonentInfo.hpp"
#include "util/smstext.h"

namespace smsc{
namespace system{
namespace abonentinfo{

using smsc::util::getSmsText;
using smsc::smeman;

int AbonentInfoSme::Execute()
{
  SmscCommand cmd,resp,answ;
  char body[MAX_SHORT_MESSAGE_LENGTH];
  SMS *sms;
  Profile p;
  SMS s;
  char msc[]="123";
  char imsi[]="123";
  s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  time_t t=time(NULL)+60;
  s.setValidTime(t);
  s.setDeliveryReport(0);
  s.setArchivationRequested(false);
  s.setEServiceType("XXX");

  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid!=smsc::smeman::DELIVERY)
    {
      __trace2__("Profiler: incorrect command submitted");
      continue;
    }
    sms=cmd->get_sms();
    resp=SmscCommand::makeDeliverySmResp(
      sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
      cmd->get_dialogId(),
      SmscCommand::Status::OK);
    putIncomingCommand(resp);
    
    getSmsText(sms,body);
    Address a(body);
    p=profiler->lookup(a);
    char answ[MAX_SHORT_MESSAGE_LENGTH];
    sprintf(answ,"%d,%d",1,p.codepage);
    
    int len=strlen(answ);
    char buf7[MAX_SHORT_MESSAGE_LENGTH];
    int len7=ConvertTextTo7Bit(answ,len,buf7,sizeof(buf7),CONV_ENCODING_ANSI);

    s.setOriginatingAddress(sms->getDestinationAddress());
    s.setDestinationAddress(sms->getOriginatingAddress());
    s.setStrProperty(Tag::SMPP_SHORT_MESSAGE,buf7);
    s.setIntProperty(Tag::SMPP_SM_LENGTH,len7);
    s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);

    resp=SmscCommand::makeSumbmitSm(s,getNextSequenceNumber());
    putIncomingCommand(resp);
  }
  return 0;
}

};//abonentinfo
};//system
};//smsc
