#ifndef __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__

#include "SmppCommandQueue.h"
#include "SmppRouter.h"
#include "SmppCommand.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/IntHash.hpp"
#include "logger/Logger.h"

namespace scag{
namespace transport{
namespace smpp{

namespace thr=smsc::core::threads;
using namespace smsc::core::buffers;

class StateMachine:public thr::ThreadedTask{
public:
  StateMachine(SmppCommandQueue* argQueue,SmppRouter* argRouteMan)
  {
    queue=argQueue;
    routeMan=argRouteMan;
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
  SmppRouter* routeMan;
  smsc::logger::Logger* log;

  void SubmitResp(SmppCommand& cmd,int status);
  void DeliveryResp(SmppCommand& cmd,int status);

  struct ResponseRegistry;
  static ResponseRegistry reg;
  
  static IntHash<int> UMRtoUSR;
  static IntHash<int> USRtoUMR;
};

}//smpp
}//transport
}//scag


#endif
