#include "util/debug.h"
#include "system/smsc.hpp"
#include "system/state_machine.hpp"
#include <exception>

namespace smsc{
namespace system{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;

int StateMachine::Execute()
{
  Tuple t;
  StateType st;
  for(;;)
  {
    eq.selectAndDequeue(t,&isStopping);
    if(isStopping)break;
    switch(t.command->cmdid)
    {
      case SUBMIT:st=submit(t);break;
      case DELIVERY_RESP:st=deliveryResp(t);break;
      case FORWARD:st=forward(t);break;
      default:
        __warning__("UNKNOWN COMMAND");
        st=ERROR_STATE;
        break;
    }
    __trace2__("change state for %lld to %d",t.msgId,st);
    eq.changeState(t.msgId,st);
  }
  __trace__("exit state machine");
  return 0;
}

StateType StateMachine::submit(Tuple& t)
{
  __require__(t.state==UNKNOWN_STATE);
  SmeProxy *src_proxy,*dest_proxy=0;
  src_proxy=t.command.getProxy();
  __trace2__("StateMachine::submit:%lld",t.msgId);
  SMS* sms = t.command->get_sms();

  uint32_t dialogId =  t.command->get_dialogId();
  __trace2__("SUBMIT: seq=%d",dialogId);
  int dest_proxy_index;
  // route sms
  //SmeProxy* dest_proxy = 0;
  bool has_route = smsc->routeSms(sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    //send_no_route;
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
    src_proxy->putCommand(resp);
    __warning__("SUBMIT_SM: no route");
    return ERROR_STATE;
  }

  try{
    store->createSms(*sms,t.msgId);
  }catch(...)
  {
    __trace2__("failed to create sms with id %lld",t.msgId);
    return ERROR_STATE;
  }

  if(sms->getWaitTime()>time(NULL))
  {
    return ENROUTE_STATE;
  }

  if ( !dest_proxy )
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
    src_proxy->putCommand(resp);
    __warning__("SUBMIT_SM: SME is not connected");
    return ENROUTE_STATE;
  }
  // create task
  uint32_t dialogId2 = dest_proxy->getNextSequenceNumber();
  __trace2__("DELIVER: seq:%d",dialogId2);
  //Task task((uint32_t)dest_proxy_index,dialogId2);
  Task task(dest_proxy->getUniqueId(),dialogId2);
  task.messageId=t.msgId;
  if ( !smsc->tasks.createTask(task) )
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
    src_proxy->putCommand(resp);
    __warning__("SUBMIT_SM: can't create task");
    return ENROUTE_STATE;
  }
  // send delivery
  SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
  dest_proxy->putCommand(delivery);
  // send responce
  SmscCommand resp2 = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::OK);
  src_proxy->putCommand(resp2);

  __trace__("mainLoop:SUBMIT:OK");
  return DELIVERING_STATE;
}

StateType StateMachine::forward(Tuple& t)
{
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    return UNKNOWN_STATE;
  }
  if(sms.getState()!=ENROUTE_STATE)
  {
    return sms.getState();
  }
  SmeProxy *dest_proxy=0;
  int dest_proxy_index;
  bool has_route = smsc->routeSms(&sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    __warning__("FORWARD: No route");
    return ERROR_STATE;
  }
  // create task
  uint32_t dialogId2 = dest_proxy->getNextSequenceNumber();
  __trace2__("FORWARD: seq number:%d",dialogId2);
  //Task task((uint32_t)dest_proxy_index,dialogId2);
  Task task(dest_proxy->getUniqueId(),dialogId2);
  task.messageId=t.msgId;
  if ( !smsc->tasks.createTask(task) )
  {
    __warning__("FORWARD: can't create task");
    return ENROUTE_STATE;
  }
  // send delivery
  SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
  dest_proxy->putCommand(delivery);
  return DELIVERING_STATE;
}

StateType StateMachine::deliveryResp(Tuple& t)
{
  __trace2__("delivering resp for :%lld",t.msgId);
  __require__(t.state==DELIVERING_STATE);
  smsc::sms::Descriptor d;
  try{
    __trace__("change state to delivered");
    store->changeSmsStateToDelivered(t.msgId,d);
    __trace__("change state to delivered: ok");
  }catch(std::exception& e)
  {
    __trace2__("change state to delivered exception:%s",e.what());
    return DELIVERED_STATE;
  }
  return DELIVERED_STATE;
}


};//system
};//smsc
