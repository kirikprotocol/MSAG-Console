#ifndef __SMSC_SYSTEM_CANCELAGENT_HPP__
#define __SMSC_SYSTEM_CANCELAGENT_HPP__

#include "smsc/scheduler/scheduler.hpp"
#include "smsc/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "smsc/smeman/smsccmd.h"

namespace smsc{
namespace agents{

using namespace smsc::smeman;
using namespace smsc::scheduler;
using smsc::core::buffers::Array;
using smsc::core::synchronization::Event;

class CancelAgent:public smsc::core::threads::ThreadedTask{
public:
  CancelAgent(EventQueue& eq,Scheduler* argSched,int argEventQueueLimit):
    eventQueue(eq),sched(argSched),eventQueueLimit(argEventQueueLimit){}

  const char *taskName()
  {
    return "CancelAgent";
  }

  int Execute()
  {
    mon.Lock();
    Array<SMSId> ids;
    SMS dummySms;
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
          dummySms.setOriginatingAddress(c.sourceAddr.get());
          dummySms.setDealiasedDestinationAddress(c.destAddr.get());
          dummySms.setEServiceType(c.serviceType.get());

          __trace2__("CANCELAGENT: mass cancel request - %s/%s/%s",
            dummySms.getOriginatingAddress().toString().c_str(),
            dummySms.getDealiasedDestinationAddress().toString().c_str(),
            dummySms.getEServiceType()
          );

          //sched->getMassCancelIds(dummySms,ids);

          __trace2__("CANCELAGENT: found %d ids to cancel",ids.Count());

          if(ids.Count()==0)
          {
            cmd.getProxy()->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                Status::CANCELFAIL
              )
            );
          }else
          {
            cmd.getProxy()->putCommand
            (
              SmscCommand::makeCancelSmResp
              (
                cmd->get_dialogId(),
                Status::OK
              )
            );
          }
          for(int i=0;!isStopping && i<ids.Count();i++)
          {
            __trace2__("CANCELAGENT: sending command for %lld",ids[i]);
            while
            (
              !eventQueue.enqueueEx
               (
                  ids[i],
                  SmscCommand::makeCancel
                  (
                    ids[i],
                    dummySms.getOriginatingAddress(),
                    dummySms.getDealiasedDestinationAddress()
                  ),
                  eventQueueLimit/2
               )
            )
            {
#ifdef linux
              timespec tv={0,10000000};
#else
              timestruc_t tv={0,10000000};
#endif
              nanosleep(&tv,0);
            }
          }
          ids.Empty();
        }

      }catch(...)
      {
        __trace__("CANCELAGENT: batch cancel processing error");
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeCancelSmResp
          (
            cmd->get_dialogId(),
            Status::CANCELFAIL
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
  Scheduler* sched;
  EventMonitor mon;
  Array<SmscCommand> queue;
  int eventQueueLimit;
};

}//system
}//smsc

#endif
