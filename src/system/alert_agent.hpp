#ifndef __SMSC_SYSTEM_ALERTAGENT_HPP__
#define __SMSC_SYSTEM_ALERTAGENT_HPP__

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

class AlertAgent:public smsc::core::threads::ThreadedTask{
public:
  AlertAgent(EventQueue& eq,MessageStore* st):
    eventQueue(eq),store(st){}

  const char *taskName()
  {
    return "AlertAgent";
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
      queue.Pop(cmd);
      mon.Unlock();
      ////
      // processing here



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
