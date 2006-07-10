#ifndef __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__

#include "SmppCommandQueue.h"
#include "SmppRouter.h"
#include "SmppCommand.h"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace scag{
namespace transport{
namespace smpp{

namespace thr=smsc::core::threads;

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
  void processDataSm(SmppCommand& cmd);
  void processDataSmResp(SmppCommand& cmd);
  void processExpiredResps();

protected:
  SmppCommandQueue* queue;
  SmppRouter* routeMan;
  smsc::logger::Logger* log;

  void SubmitResp(SmppCommand& cmd,int status);
  void DeliveryResp(SmppCommand& cmd,int status);

  void registerEvent(int event, SmppEntity* sme, SmppEntity* sc, int errCode);
  void registerEvent(int event, SmppEntity* sme, SmppEntity* sc, const char* rid, int errCode);

  struct ResponseRegistry;
  static ResponseRegistry reg;
};

}//smpp
}//transport
}//scag


#endif
