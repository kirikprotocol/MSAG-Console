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

bool Smsc::routeSms(SMS* sms, int& dest_idx,SmeProxy*& proxy)
{
  //smeman.getSmeProxy(0)
  proxy = 0;
  bool ok = router.lookup(sms->getOriginatingAddress(),
                          sms->getDestinationAddress(),
                          proxy,
                          &dest_idx);
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
				eventqueue.enqueue(id,SmscCommand::makeAlert());
			}
    }
    while(!src_proxy);

    SmscCommand cmd = src_proxy->getCommand();
    cmd.setProxy(src_proxy);
    SMSId id=0;
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
      }
    }
    eventqueue.enqueue(id,cmd);
  }
}

};
};
