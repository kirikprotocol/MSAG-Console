#ifndef __SMSC_SYSTEM_TRAFFIC_CONTROL_HPP__
#define __SMSC_SYSTEM_TRAFFIC_CONTROL_HPP__

#include "util/timeslotcounter.hpp"
#include "core/buffers/IntHash.hpp"
#include "smeman/smsccmd.h"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace smsc{

namespace store{
class MessageStore;
}

namespace system{

class SmeManager;

using smsc::smeman::SmscCommand;
using smsc::core::buffers::IntHash;
using smsc::smeman::SmeIndex;
using namespace smsc::core::synchronization;

class Smsc;

class TrafficControl{
protected:
  typedef smsc::util::TimeSlotCounter<> IntTimeSlotCounter;
public:
  struct TrafficControlConfig{
    int maxSmsPerSecond;
    int protectThreshold;
    int allowedDeliveryFailures;
    time_t protectTimeFrame;
    time_t shapeTimeFrame;
    time_t lookAheadTime;
    time_t smoothTimeFrame;
    Smsc *smsc;
    smsc::store::MessageStore *store;
  };
  TrafficControl(const TrafficControlConfig& cfg):
    cfg(cfg),
    totalCounter(cfg.shapeTimeFrame,10),
    totalCounterLong(cfg.smoothTimeFrame,10)
  {
    log = smsc::logger::Logger::getInstance("smsc.tc");
  }


  bool processCommand(SmscCommand& cmd);

  IntTimeSlotCounter* getTSC(IntHash<IntTimeSlotCounter*>& hash,SmeIndex idx)
  {
    IntTimeSlotCounter** pptr=hash.GetPtr(idx);
    if(pptr)return *pptr;
    IntTimeSlotCounter* ptr=new IntTimeSlotCounter(cfg.protectTimeFrame);
    hash.Insert(idx,ptr);
    return ptr;
  }

  int getTotalCount()
  {
    MutexGuard g(mtx);
    return totalCounter.Get();
  }

  int getTotalCountLong()
  {
    MutexGuard g(mtx);
    return totalCounterLong.Get();
  }

  void incTotalCount(int v)
  {
    MutexGuard g(mtx);
    totalCounter.Inc(v);
    totalCounterLong.IncDistr(v*1000,1000*cfg.maxSmsPerSecond/100);
  }



  TrafficControlConfig& getConfig()
  {
    return cfg;
  }

protected:
  TrafficControlConfig cfg;
  IntHash<IntTimeSlotCounter*> deliverCnt,responseCnt;
  IntTimeSlotCounter totalCounter;
  IntTimeSlotCounter totalCounterLong;
  Mutex mtx;
  smsc::logger::Logger* log;
  friend class StatusSme;
};

}//system
}//smsc

#endif
