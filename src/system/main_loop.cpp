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

/*SmeProxy* Smsc::routeSms(SMS* sms, int* dest_idx)
{
  smeman.getSmeProxy(0)
}*/

void Smsc::mainLoop()
{
  int src_proxy_index;
  int dest_proxy_index;
  SmeProxy* src_proxy = smeman.selectSmeProxy(WAIT_DATA_TIMEOUT,&src_proxy_index);
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
        //SmeProxy* dest_proxy = routeSms(sms,&dest_proxy_index);
        SmeProxy* dest_proxy = 0;
        auto_ptr<SmeIterator> it(smeman.iterator());
        while (it->next())
        {
          SmeProxy* proxy = it->getSmeProxy();
          if ( proxy != src_proxy )
          {
            dest_proxy = proxy;
            break;
          }
        }

        if ( !dest_proxy )
        {
          //send_no_route;
          SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
          src_proxy->putCommand(resp);
          __warning__("SUBMIT_SM: no route");
          break;
        }
        // store sms
        // create task
        uint32_t dialogId2 = dest_proxy->getNextSequenceNumber();
        Task task((uint32_t)dest_proxy_index,dialogId2);
        if ( !tasks.createTask(task) )
        {
          SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::ERROR);
          src_proxy->putCommand(resp);
          __warning__("SUBMIT_SM: no route");
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
        uint32_t status = cmd->get_resp()->get_status();
        uint32_t dialogId = cmd->get_dialogId();
        const char* messageId = cmd->get_resp()->get_messageId();
        Task task;
        // find and remove task
        if (!tasks.findAndRemoveTask((uint32_t)src_proxy_index,dialogId,&task))
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

};
};
