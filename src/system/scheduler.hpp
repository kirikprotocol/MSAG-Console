#ifndef __SMSC_SYSTEM_SCHEDULER_HPP__
#define __SMSC_SYSTEM_SCHEDULER_HPP__

#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
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
using smsc::core::synchronization::MutexGuard;

class Smsc;

class Scheduler:public smsc::core::threads::ThreadedTask, public SmeProxy
{
public:
  Scheduler(EventQueue& eq):queue(eq)
  {
    rescheduleLimit=10;
    seq=0;
    prxmon=0;
    lastCheck=0;
  }
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

  virtual void close(){}
  virtual void putCommand(const SmscCommand& command){}
  virtual SmscCommand getCommand()
  {
    MutexGuard guard(mon);
    time_t t=time(NULL);
    if(timeLine.size()==0 || timeLine.begin()->first>t)
    {
      return SmscCommand();
    }
    Data d=timeLine.begin()->second;
    CacheItem *ci=smeCountCache.GetPtr(d.idx);
    if(ci)
    {
      if(ci->lastUpdate>t)
      {
        ci->count--;
      }
    }
    timeLine.erase(timeLine.begin());
    return SmscCommand::makeForward(d.idx,d.id,d.resched);
  }
  virtual SmeProxyState getState()const
  {
    return VALID;
  };
  virtual void init(){};
  virtual bool hasInput()const
  {
    MutexGuard guard(mon);
    return timeLine.size()>0 && timeLine.begin()->first<=time(NULL);
  }
  virtual void attachMonitor(ProxyMonitor* monitor)
  {
    prxmon=monitor;
  }

  virtual bool attached()
  {
    return prxmon!=NULL;
  }
  virtual uint32_t getNextSequenceNumber()
  {
    MutexGuard g(seqMtx);
    return seq++;
  }
  virtual const char * getSystemId()const
  {
    return "scheduler";
  }

protected:
  EventQueue &queue;
  mutable EventMonitor mon;
  int rescheduleLimit;
  time_t lastCheck;
  struct Data{
    Data():id(0),idx(0),resched(false){}
    Data(const Data& d)
    {
      id=d.id;
      idx=d.idx;
      resched=d.resched;
    }
    Data(SMSId id,SmeIndex idx,bool res=false):id(id),idx(idx),resched(res){}
    SMSId id;
    SmeIndex idx;
    bool resched;
  };
  typedef std::multimap<time_t,Data> TimeLineMap;
  typedef std::pair<time_t,Data> TimeIdPair;

  struct CacheItem{
    time_t lastUpdate;
    int count;
  };
  IntHash<CacheItem> smeCountCache;

  TimeLineMap timeLine;
  Mutex seqMtx;
  int seq;
  ProxyMonitor* prxmon;
};

};//system
};//smsc


#endif
