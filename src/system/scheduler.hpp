#ifndef __SMSC_SYSTEM_SCHEDULER_HPP__
#define __SMSC_SYSTEM_SCHEDULER_HPP__

#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Event.hpp"
#include "system/sched_timer.hpp"
#include "store/StoreManager.h"
#include <map>

namespace smsc{
namespace system{

using smsc::store::MessageStore;
using namespace smsc::smeman;
using smsc::core::buffers::Array;
using smsc::core::buffers::IntHash;
using smsc::core::synchronization::Event;

class Smsc;

class Scheduler:public smsc::core::threads::ThreadedTask//, public SmeProxy
{
public:
  Scheduler(EventQueue& eq):
    queue(eq),rescheduleLimit(10){}
  int Execute();
  const char* taskName(){return "scheduler";}

  void Init(MessageStore* st,Smsc* psmsc);

  void notify()
  {
    //
  }
  void setRescheduleLimit(int rl)
  {
    rescheduleLimit=rl;
  }
  void ChangeSmsSchedule(SMSId id,time_t newtime,SmeIndex idx);

  void UpdateSmsSchedule(time_t old,SMSId id,time_t newtime,SmeIndex idx);

  int getSmeCount(SmeIndex idx,time_t time);

  int getSmsCount()
  {
    return timeLine.size();
  }

protected:
  EventQueue &queue;
  EventMonitor mon;
  int rescheduleLimit;
  struct Data{
    Data():id(0),idx(0){}
    Data(SMSId id,SmeIndex idx):id(id),idx(idx){}
    SMSId id;
    SmeIndex idx;
  };
  typedef std::multimap<time_t,Data> TimeLineMap;
  typedef std::pair<time_t,Data> TimeIdPair;

  struct CacheItem{
    time_t lastUpdate;
    int count;
  };
  IntHash<CacheItem> smeCountCache;

  TimeLineMap timeLine;
};

};//system
};//smsc


#endif
