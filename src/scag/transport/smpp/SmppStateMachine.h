#ifndef __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSTATEMACHINE_H__

#include <vector>
#include "SmppCommandQueue.h"
#include "SmppRouter.h"
#include "SmppCommand.h"
#include "scag/sessions/Session.h"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace scag{
namespace transport{
namespace smpp{

namespace thr=smsc::core::threads;
using namespace scag::sessions;

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
  void processAlertNotification(SmppCommand& cmd);

  static void addTransitOptional(int tag)
  {
    allowedUnknownOptionals.push_back(tag);
    smsc_log_debug(smsc::logger::Logger::getInstance("statmach"), "Transitional tag added: %x", tag);
  }

protected:
  SmppCommandQueue* queue;
  SmppRouter* routeMan;
  smsc::logger::Logger* log;

  static sync::Mutex expMtx;
  static bool expProc;

  void SubmitResp(SmppCommand& cmd,int status);
  void DeliveryResp(SmppCommand& cmd,int status);
  void DataResp(SmppCommand& cmd,int status);

  void registerEvent(int event, SmppEntity* src, SmppEntity* dst, const char* rid, int errCode);
  uint32_t putCommand(CommandId cmdType, SmppEntity* src, SmppEntity* dst, router::RouteInfo& ri, SmppCommand& cmd);

  bool makeLongCall(SmppCommand& cx, SessionPtr& session);

  struct ResponseRegistry;
  static ResponseRegistry reg;

  static std::vector<int> allowedUnknownOptionals;

  void sendReceipt(SmppCommand& cmd);
};

}//smpp
}//transport
}//scag


#endif
