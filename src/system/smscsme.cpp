#include "system/smscsme.hpp"
#include "util/smstext.h"
#include "util/Logger.h"
#include "smeman/smsccmd.h"

namespace smsc{
namespace system{

using smsc::util::getSmsText;
using smsc::smeman;

int SmscSme::Execute()
{
  SmscCommand cmd,resp;

  while(!isStopping)
  {
    waitFor();
    if(isStopping)break;
    while(hasOutput())cmd=getOutgoingCommand();

    SMS *s;
    while((s=getSms()))
    {
      s->setEServiceType(servType.c_str());
      s->setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
      resp=SmscCommand::makeSumbmitSm(*s,getNextSequenceNumber());
      delete s;
      putIncomingCommand(resp);
    }
  }
  return 0;
}

};//system
};//smsc
