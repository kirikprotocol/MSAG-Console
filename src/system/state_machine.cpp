#include "util/debug.h"
#include "system/smsc.hpp"
#include "system/state_machine.hpp"
#include <exception>
#include "system/rescheduler.hpp"

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
      case ALERT:st=alert(t);break;
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

  time_t now=time(NULL);

  uint32_t dialogId =  t.command->get_dialogId();
  __trace2__("SUBMIT: seq=%d",dialogId);
  int dest_proxy_index;
  // route sms
  //SmeProxy* dest_proxy = 0;
  Address dst;
  __trace2__("AliasToAddress: %d.%d.%.20s",
    sms->getDestinationAddress().type,
    sms->getDestinationAddress().plan,
    sms->getDestinationAddress().value);
  if(smsc->AliasToAddress(sms->getDestinationAddress(),dst))
  {
    __trace2__("ALIAS:%20s->%20s",sms->getDestinationAddress().value,dst.value);
  }
  else
  {
    dst=sms->getDestinationAddress();
  }
  sms->setDealiasedDestinationAddress(dst);

  bool has_route = smsc->routeSms(sms->getOriginatingAddress(),
                          dst,
                          dest_proxy_index,dest_proxy);
  //smsc->routeSms(sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    //send_no_route;
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::NOROUTE);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    __warning__("SUBMIT_SM: no route");
    return ERROR_STATE;
  }

  if(sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime)
  {
    sms->setValidTime(now+maxValidTime);
  }

  __trace2__("Valid time for sms %lld=%u\n",t.msgId,sms->getValidTime());

  if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::INVALIDSCHEDULE);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    return ERROR_STATE;
  }

  __trace2__("Replace if present for message %lld=%d",t.msgId,sms->getIntProperty("SMPP_REPLACE_IF_PRESENT_FLAG"));

  try{
    if(sms->getNextTime()<now)
    {
      sms->setNextTime(RescheduleCalculator::calcNextTryTime(now,1));
    }
    store->createSms(*sms,t.msgId,
      sms->getIntProperty("SMPP_REPLACE_IF_PRESENT_FLAG")?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
  }catch(...)
  {
    __trace2__("failed to create sms with id %lld",t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::DBERROR);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    return ERROR_STATE;
  }

  {
    // sms ��࠭��� � ����, � ���⠢����� Next Time, ⠪�� ��ࠧ��
    // ���� �᫨ ����� ��-� ���������, ��⮬ �㤥� �� ����⪠ ��᫠�� ��
    // � ���� �� �ਭ﫨 sms � ��ࠡ���, ����� ᫠�� ok.
    char buf[64];
    sprintf(buf,"%lld",t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp(buf, dialogId, SmscCommand::Status::OK);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
  }

  __trace2__("Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(sms->getNextTime()>now)
  {
    smsc->notifyScheduler();
    return ENROUTE_STATE;
  }


  if ( !dest_proxy )
  {
    __warning__("SUBMIT_SM: SME is not connected");
    return ENROUTE_STATE;
  }
  // create task
  uint32_t dialogId2;
  try{
     dialogId2=dest_proxy->getNextSequenceNumber();
  }catch(...)
  {
    return ENROUTE_STATE;
  }
  __trace2__("DELIVER: seq:%d",dialogId2);
  //Task task((uint32_t)dest_proxy_index,dialogId2);
  try{
  Task task(dest_proxy->getUniqueId(),dialogId2);
  task.messageId=t.msgId;
  if ( !smsc->tasks.createTask(task) )
  {
    __warning__("SUBMIT_SM: can't create task");
    return ENROUTE_STATE;
  }
  }catch(...)
  {
    return ENROUTE_STATE;
  }
  try{
    // send delivery
    Address src;
    if(smsc->AddressToAlias(sms->getOriginatingAddress(),src))
    {
      sms->setOriginatingAddress(src);
    }
    sms->setDestinationAddress(dst);
    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    dest_proxy->putCommand(delivery);
  }catch(...)
  {
    return ENROUTE_STATE;
  }
  __trace__("mainLoop:SUBMIT:OK");
  return DELIVERING_STATE;
}

StateType StateMachine::forward(Tuple& t)
{
  SMS sms;
  __trace__("FORWARD!!!");
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    return UNKNOWN_STATE;
  }
  if(sms.getValidTime()<time(NULL))
  {
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
    }
    return EXPIRED_STATE;
  }
  if(sms.getState()!=ENROUTE_STATE)
  {
    return sms.getState();
  }
  SmeProxy *dest_proxy=0;
  int dest_proxy_index;

  bool has_route = smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    __warning__("FORWARD: No route");
    try{
      time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,0,RescheduleCalculator::calcNextTryTime(now,sms.getAttemptsCount()));
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    return ERROR_STATE;
  }
  if(!dest_proxy)
  {
    __trace__("FORWARD: no proxy");
    try{
      time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,0,RescheduleCalculator::calcNextTryTime(now,sms.getAttemptsCount()));
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task

  uint32_t dialogId2;
  try{
    dialogId2 = dest_proxy->getNextSequenceNumber();
    __trace2__("FORWARD: seq number:%d",dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(dest_proxy->getUniqueId(),dialogId2);
    task.messageId=t.msgId;
    if ( !smsc->tasks.createTask(task) )
    {
      __warning__("FORWARD: can't create task");
      return ENROUTE_STATE;
    }
  }catch(...)
  {
    return ENROUTE_STATE;
  }
  try{
    // send delivery
    Address src;
    if(smsc->AddressToAlias(sms.getOriginatingAddress(),src))
    {
      sms.setOriginatingAddress(src);
    }
    sms.setDestinationAddress(sms.getDealiasedDestinationAddress());
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    dest_proxy->putCommand(delivery);
  }catch(...)
  {
    //TODO!!!: remove task and reschedule
    return ENROUTE_STATE;
  }
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

StateType StateMachine::alert(Tuple& t)
{
  __trace__("ALERT!!!");
  time_t now=time(NULL);
  try{
    Descriptor d;
    SMS sms;
    store->retriveSms((SMSId)t.msgId,sms);
    store->changeSmsStateToEnroute(t.msgId,d,0,RescheduleCalculator::calcNextTryTime(now,sms.getAttemptsCount()));
    smsc->notifyScheduler();
  }catch(...)
  {
  };
  return UNKNOWN_STATE;
}


};//system
};//smsc
