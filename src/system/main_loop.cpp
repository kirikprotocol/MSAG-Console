/*
  $Id$
*/

#include "smsc.hpp"
#include <memory>

#include <exception>

namespace smsc{
namespace system{

#define WAIT_DATA_TIMEOUT 100 /* ms */
using smsc::smeman::CommandId;
using smsc::smeman::SmscCommand;
using std::exception;
using smsc::sms::SMS;
using smsc::smeman::SmeIterator;
using std::auto_ptr;

#define __CMD__(x) smsc::smeman::x

bool Smsc::routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,
  smsc::router::RouteInfo* ri)
{
  //smeman.getSmeProxy(0)
  proxy = 0;
  bool ok = getRouterInstance()->lookup(org,dst,proxy,&dest_idx,ri);
  return ok;
}

#if 0
void Smsc::mainLoop()
{
  int src_proxy_index;
  int dest_proxy_index;
  SmeProxy* src_proxy;

  for(;;)
  {

    do
    {
      src_proxy = smeman.selectSmeProxy(WAIT_DATA_TIMEOUT,&src_proxy_index);
      if ( stopFlag ) return;
    }
    while(!src_proxy);

    SmscCommand cmd = src_proxy->getCommand();
    try
    {
      switch ( cmd->get_commandId() )
      {
      case __CMD__(SUBMIT):
        {
          __trace__("mainLoop:SUBMIT");
          SMS* sms = cmd->get_sms();
          uint32_t dialogId =  cmd->get_dialogId();
          // route sms
          SmeProxy* dest_proxy = 0;
          bool has_route = routeSms(sms,dest_proxy_index,dest_proxy);
          if ( !has_route )
          {
            //send_no_route;
            SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
            src_proxy->putCommand(resp);
            __warning__("SUBMIT_SM: no route");
            break;
          }
          else if ( !dest_proxy )
          {
            SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
            src_proxy->putCommand(resp);
            __warning__("SUBMIT_SM: SME is not connected");
            break;
          }
          // store sms
          // create task
          uint32_t dialogId2 = dest_proxy->getNextSequenceNumber();
          //Task task((uint32_t)dest_proxy_index,dialogId2);
          Task task(dest_proxy->getUniqueId(),dialogId2);
          if ( !tasks.createTask(task) )
          {
            SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
            src_proxy->putCommand(resp);
            __warning__("SUBMIT_SM: can't create task");
            break;
          }
          // send delivery
          SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
          dest_proxy->putCommand(delivery);
          // send responce
          SmscCommand resp2 = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::OK);
          src_proxy->putCommand(resp2);
          __trace__("mainLoop:SUBMIT:OK");
          break;
        }
      case __CMD__(DELIVERY_RESP):
        {
          __trace__("mainLoop:DELIVERY_RESP");
          //uint32_t status = cmd->get_resp()->get_status();
          uint32_t dialogId = cmd->get_dialogId();
          //const char* messageId = cmd->get_resp()->get_messageId();
          Task task;
          // find and remove task
          if (!tasks.findAndRemoveTask(src_proxy->getUniqueId(),dialogId,&task))
          {
            __warning__("responce on unpresent task");
            break;
          }
          // update sms state
          //......
          __trace__("mainLoop:DELIVERY_RESP:OK");
          break;
        }
      default:
        __warning__("received unsupported command");
        // drop command
      }
    }
    catch (exception& e)
    {
      __warning__(e.what());
    }
    catch (...)
    {
      __warning__("unknown exception catched");
    }
  }
}
#endif

void Smsc::mainLoop()
{
  SmeProxy* src_proxy;
  int src_proxy_index;

  for(;;)
  {

    do
    {
      // !!!TODO: taskcontainer expiration checks
      src_proxy = smeman.selectSmeProxy(WAIT_DATA_TIMEOUT,&src_proxy_index);
      if ( stopFlag ) return;
      Task task;
      while ( tasks.getExpired(&task) )
      {
        SMSId id = task.messageId;
        __trace__("enqueue timeout Alert");
        eventqueue.enqueue(id,SmscCommand::makeAlert());
      }
    }
    while(!src_proxy);

    SmscCommand cmd;

    SMSId id=0;
    try{
      cmd = src_proxy->getCommand();
      cmd.setProxy(src_proxy);
      //__trace2__("mainLoop: prio=%d",src_proxy->getPriority());
      //__require__(src_proxy->getPriority()>0);
      int prio=src_proxy->getPriority()/1024;
      if(prio<0)prio=0;
      if(prio>=32)prio=31;
      cmd->set_priority(prio);
      cmd->sourceId=src_proxy->getSystemId();
    }catch(...)
    {
      __trace2__("Source proxy died after selection");
      continue;
    }
    switch(cmd->get_commandId())
    {
      case __CMD__(SUBMIT):
      {
        id=store->getNextId();
        __trace2__("main loop submit: seq=%d, id=%lld",cmd->get_dialogId(),id);
        break;
      }
      case __CMD__(DELIVERY_RESP):
      {
        Task task;
        uint32_t dialogId = cmd->get_dialogId();
        __trace2__("delivery response received. id=%d",dialogId);

        if (!tasks.findAndRemoveTask(src_proxy->getUniqueId(),dialogId,&task))
        {
          __warning__("task not found for delivery response");
          continue; //jump to begin of for
        }
        id=task.messageId;
        break;
      }
      case __CMD__(REPLACE):
      {
        int pos;
        if(sscanf(cmd->get_replaceSm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
           cmd->get_replaceSm().messageId.get()[pos]!=0)
        {
          src_proxy->putCommand
          (
            SmscCommand::makeReplaceSmResp
            (
              cmd->get_dialogId(),
              SmscCommand::Status::INVALIDMSGID
            )
          );
        };
        break;
      }
      case __CMD__(QUERY):
      {
        int pos;
        if(sscanf(cmd->get_querySm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
           cmd->get_querySm().messageId.get()[pos]!=0)
        {
          src_proxy->putCommand
          (
            SmscCommand::makeQuerySmResp
            (
              cmd->get_dialogId(),
              SmscCommand::Status::INVALIDMSGID,
              0,0,0,0
            )
          );
        };
        break;
      }
      case __CMD__(CANCEL):
      {
        if((cmd->get_cancelSm().messageId.get() && cmd->get_cancelSm().serviceType.get()) ||
           (!cmd->get_cancelSm().messageId.get() && !cmd->get_cancelSm().serviceType.get()))
        {
            src_proxy->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                SmscCommand::Status::CANCELFAIL
              )
            );
            continue;
        }

        if(cmd->get_cancelSm().messageId.get())
        {
          int pos=0;
          if(sscanf(cmd->get_cancelSm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
             cmd->get_cancelSm().messageId.get()[pos]!=0)
          {
            src_proxy->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                SmscCommand::Status::INVALIDMSGID
              )
            );
            continue;
          };
        }else
        {
          if(!cmd->get_cancelSm().sourceAddr.get() || !cmd->get_cancelSm().sourceAddr.get()[0] ||
             !cmd->get_cancelSm().destAddr.get() || !cmd->get_cancelSm().destAddr.get()[0])
          {
            src_proxy->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                SmscCommand::Status::CANCELFAIL
              )
            );
            continue;
          }
          cancelAgent->putCommand(cmd);
          continue;
        }
        break;
      }
      case __CMD__(HLRALERT):
      {
        alertAgent->putCommand(cmd);
        continue;
      }
    }
    __require__(cmd.getProxy()==src_proxy);
    eventqueue.enqueue(id,cmd);
  }
}

};
};
