#include "system/status_sme.hpp"
#include "util/smstext.h"

namespace smsc{
namespace system{

using namespace smsc::util;

int StatusSme::Execute()
{
  SmscCommand cmd,resp;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
  SMS *sms;
  SMS s;
  string answer;
  char msc[]="";
  char imsi[]="";
  s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  s.setDeliveryReport(0);
  s.setArchivationRequested(false);
  s.setEServiceType(servType.c_str());
  s.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.system.StatusSme");

  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid==smsc::smeman::SUBMIT_RESP)continue;
    if(cmd->cmdid!=smsc::smeman::DELIVERY)
    {
      __trace2__("StatusSme: incorrect command submitted:%d",cmd->cmdid);
      log.warn("Incorrect command received");
      continue;
    }
    sms=cmd->get_sms();
    resp=SmscCommand::makeDeliverySmResp(
      sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
      cmd->get_dialogId(),
      Status::OK);
    putIncomingCommand(resp);

    getSmsText(sms,body,sizeof(body));
    string request=body;
    string answer;

    if(request=="scheduler")
    {
      char buf[32];
      sprintf(buf,"%d",smsc->getSchedulesSmsCount());
      answer=buf;
      answer+=" sms in scheduler";
    }else
    {
      answer="unknown command";
    }


    time_t t=time(NULL)+60;
    s.setValidTime(t);
    s.setOriginatingAddress(sms->getDestinationAddress());
    s.setDestinationAddress(sms->getOriginatingAddress());
    s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    fillSms(&s,answer.c_str(),answer.length(),CONV_ENCODING_CP1251,DataCoding::UCS2);

    s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
      sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));

    resp=SmscCommand::makeSumbmitSm(s,getNextSequenceNumber());
    putIncomingCommand(resp);
  }
  return 0;
}
};//system
};//smsc
