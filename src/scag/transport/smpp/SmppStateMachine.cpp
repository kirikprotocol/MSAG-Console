#include "SmppStateMachine.h"

namespace scag{
namespace transport{
namespace smpp{


int StateMachine::Execute()
{
  SmppCommand cmd;
  while(!isStopping)
  {
    try{
      if(queue->getCommand(cmd))
      {
        switch(cmd->get_commandId())
        {
          case SUBMIT:processSubmit(cmd);break;
          case SUBMIT_RESP:processSubmitResp(cmd);break;
          case DELIVERY:processDelivery(cmd);break;
          case DELIVERY_RESP:processDeliveryResp(cmd);break;
          default:
            smsc_log_warn(log,"Unprocessed command id %d",cmd->get_commandId());
            break;
        }
      }
    }catch(std::exception& e)
    {
      smsc_log_error(log,"Exception in state machine:%s",e.what());
    }
  }
  return 0;
}

void StateMachine::processSubmit(SmppCommand& cmd)
{
}
void StateMachine::processSubmitResp(SmppCommand& cmd)
{
}
void StateMachine::processDelivery(SmppCommand& cmd)
{
}
void StateMachine::processDeliveryResp(SmppCommand& cmd)
{
}


}//smpp
}//transport
}//scag
