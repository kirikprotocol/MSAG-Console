#include "system/abonentinfo/AbonentInfo.hpp"
#include "util/smstext.h"
#include "util/Logger.h"

namespace smsc{
namespace system{
namespace abonentinfo{

using smsc::util::getSmsText;
using smsc::smeman;

int AbonentInfoSme::Execute()
{
  SmscCommand cmd,resp,answ;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
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
  s.setEServiceType(servType.c_str());
  s.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.system.AbonentInfoSme");

  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid!=smsc::smeman::DELIVERY && cmd->cmdid!=smsc::smeman::QUERYABONENTSTATUS_RESP)
    {
      __trace2__("AbonentInfoSme: incorrect command submitted:%d",cmd->cmdid);
      log.warn("Incorrect command received");
      continue;
    }
    if(cmd->cmdid==smsc::smeman::DELIVERY)
    {
      sms=cmd->get_sms();
      resp=SmscCommand::makeDeliverySmResp(
        sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
        cmd->get_dialogId(),
        SmscCommand::Status::OK);
      putIncomingCommand(resp);

      getSmsText(sms,body,sizeof(body));
      try{
        Address a(body);
        Address d;
        if(!smsc->AliasToAddress(a,d))
        {
          d=a;
        }
        SmscCommand cmd=
          SmscCommand::makeQueryAbonentStatus
          (
            d,
            body,
            sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
          );
        putIncomingCommand(cmd);
      }
      catch(...)
      {
        __trace__("AbonentInfo: invalid address received. Ignored");
      }
    }else
    if(cmd->cmdid==smsc::smeman::QUERYABONENTSTATUS_RESP)
    {
      Address d=cmd->get_abonentStatus().addr;

      p=smsc->getProfiler()->lookup(d);

      char answ[MAX_SHORT_MESSAGE_LENGTH];
      sprintf(answ,"%s:%d,%d",
        cmd->get_abonentStatus().originalAddr.c_str(),
        cmd->get_abonentStatus().status,
        p.codepage);

      int len=strlen(answ);
      //char buf7[MAX_SHORT_MESSAGE_LENGTH];
      //int len7=ConvertTextTo7Bit(answ,len,buf7,sizeof(buf7),CONV_ENCODING_ANSI);

      s.setOriginatingAddress(sms->getDestinationAddress());
      s.setDestinationAddress(sms->getOriginatingAddress());
      s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,answ,len);
      s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
      s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
      s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
        cmd->get_abonentStatus().userMessageReference);


      resp=SmscCommand::makeSumbmitSm(s,getNextSequenceNumber());
      putIncomingCommand(resp);
    }

  }
  return 0;
}

};//abonentinfo
};//system
};//smsc
