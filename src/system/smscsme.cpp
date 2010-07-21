#include "system/smscsme.hpp"
#include "util/smstext.h"
#include "logger/Logger.h"
#include "smeman/smsccmd.h"

namespace smsc{
namespace system{

using namespace smsc::smeman;

int SmscSme::Execute()
{
  SmscCommand cmd,resp;

  while(!isStopping)
  {
    waitFor();
    if(isStopping)break;
    /*
    while(hasOutput())
    {
      cmd=getOutgoingCommand();
      if(cmd->get_commandId()==DELIVERY)
      {
        putIncomingCommand(
          SmscCommand::makeDeliverySmResp("0",cmd->get_dialogId(),
            MAKE_COMMAND_STATUS(CMD_ERR_PERM,SmppStatusSet::ESME_RX_P_APPN))
        );
      }
    }

    SMS *s;
    while((s=getSms()))
    {
      s->setEServiceType(servType.c_str());
      s->setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
      resp=SmscCommand::makeSumbmitSm(*s,getNextSequenceNumber());
      delete s;
      putIncomingCommand(resp);
    }
    */
  }
  return 0;
}

}//system
}//smsc
