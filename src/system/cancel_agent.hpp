#ifndef __SMSC_SYSTEM_CANCELAGENT_HPP__
#define __SMSC_SYSTEM_CANCELAGENT_HPP__

#include "store/StoreManager.h"
#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "smeman/smsccmd.h"

namespace smsc{
namespace system{

using smsc::store::MessageStore;
using namespace smsc::smeman;
using smsc::core::buffers::Array;
using smsc::core::synchronization::Event;
using namespace smsc::smeman;

class CancelAgent:public smsc::core::threads::ThreadedTask{
public:
  CancelAgent(EventQueue& eq,MessageStore* st):
    eventQueue(eq),store(st){}

  const char *taskName()
  {
    return "CancelAgent";
  }

  int Execute()
  {
    mon.Lock();
    Array<SMSId> ids;
    while(!isStopping)
    {
      if(queue.Count()==0)mon.wait();
      if(queue.Count()==0)continue;
      SmscCommand cmd;
      queue.Shift(cmd);
      mon.Unlock();
      ////
      // processing here

      try{
        const smsc::smeman::CancelSm& c=cmd->get_cancelSm();
        if(c.sourceAddr.get() && c.destAddr.get())
        {
          Address oa(c.sourceAddr.get());
          Address da(c.destAddr.get());
          smsc::store::IdIterator *it=store->getReadyForCancel(oa,da,c.serviceType.get());
          SMSId id;
          while(it->getNextId(id))
          {
            ids.Push(id);
          }
          __trace2__("CANCELAGENT: found %d ids to cancel",ids.Count());
          delete it;
          if(ids.Count()==0)
          {
            cmd.getProxy()->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                SmscCommand::Status::CANCELFAIL
              )
            );
          }else
          {
            cmd.getProxy()->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                SmscCommand::Status::OK
              )
            );
          }
          for(int i=0;i<ids.Count();i++)
          {
            __trace2__("CANCELAGENT: sending command for %lld",ids[i]);
            eventQueue.enqueue(ids[i],SmscCommand::makeCancel(ids[i],oa,da));
          }
          ids.Clean();
        }

      }catch(...)
      {
        __trace2__("CANCELAGENT: batch cancel processing error\n");
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeCancelSmResp
          (
            cmd->get_dialogId(),
            SmscCommand::Status::CANCELFAIL
          )
        );
      }

      // end of processing
      ////
      mon.Lock();
    }
    mon.Unlock();
    return 0;
  }

  void putCommand(SmscCommand& cmd)
  {
    MutexGuard g(mon);
    mon.notify();
    queue.Push(cmd);
  }

protected:
  EventQueue &eventQueue;
  MessageStore* store;
  EventMonitor mon;
  Array<SmscCommand> queue;
};

};//system
};//smsc

#endif
