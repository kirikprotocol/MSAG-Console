#ifndef __SMSC_SYSTEM_SCHEDULER_HPP__
#define __SMSC_SYSTEM_SCHEDULER_HPP__

#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Event.hpp"
#include "system/sched_timer.hpp"

namespace smsc{
namespace system{

using smsc::store::MessageStore;
using namespace smsc::smeman;
using smsc::core::buffers::Array;
using smsc::core::synchronization::Event;

class Scheduler:public smsc::core::threads::ThreadedTask,public SchedTimer{
public:
  Scheduler(EventQueue& eq,MessageStore* st):
    queue(eq),store(st),rescheduleLimit(10){}
  int Execute();
  const char* taskName(){return "scheduler";}

  void notify()
  {
    event.Signal();
  }
  void setRescheduleLimit(int rl)
  {
    rescheduleLimit=rl;
  }
protected:
  EventQueue &queue;
  MessageStore* store;
  Event event;
  int rescheduleLimit;
};

};//system
};//smsc


#endif
