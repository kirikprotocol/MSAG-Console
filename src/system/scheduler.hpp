#ifndef __SMSC_SYSTEM_SCHEDULER_HPP__
#define __SMSC_SYSTEM_SCHEDULER_HPP__

#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Event.hpp"
#include "system/sched_timer.hpp"
#include "store/StoreManager.h"
#include <map>

namespace smsc{
namespace system{

using smsc::store::MessageStore;
using namespace smsc::smeman;
using smsc::core::buffers::Array;
using smsc::core::synchronization::Event;

class Scheduler:public smsc::core::threads::ThreadedTask,public SchedTimer{
public:
  Scheduler(EventQueue& eq):
    queue(eq),rescheduleLimit(10){}
  int Execute();
  const char* taskName(){return "scheduler";}

  void Init(MessageStore* st);

  void notify()
  {
    //
  }
  void setRescheduleLimit(int rl)
  {
    rescheduleLimit=rl;
  }
  void ChangeSmsSchedule(SMSId id,time_t newtime);


protected:
  EventQueue &queue;
  EventMonitor mon;
  int rescheduleLimit;
  typedef std::multimap<time_t,SMSId> TimeLineMap;
  typedef std::pair<SMSId,time_t> TimeIdPair;
  TimeLineMap timeLine;
};

};//system
};//smsc


#endif
