#ifndef __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__

#include "SmppCommandQueue.h"
#include "SmppCommand.h"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace scag{
namespace transport{
namespace smpp{

namespace thr=smsc::core::threads;

class StateMachine:public thr::ThreadedTask{
public:
  StateMachine(SmppCommandQueue* argQueue)
  {
    queue=argQueue;
    log=smsc::logger::Logger::getInstance("statmach");
  }

  const char* taskName(){return "StateMachine";}
  int Execute();

  void processSubmit(SmppCommand& cmd);
  void processSubmitResp(SmppCommand& cmd);
  void processDelivery(SmppCommand& cmd);
  void processDeliveryResp(SmppCommand& cmd);

protected:
  SmppCommandQueue* queue;
  smsc::logger::Logger* log;
};

}//smpp
}//transport
}//scag


#endif
