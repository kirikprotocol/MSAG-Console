//------------------------------------
//  DeliveryQueue.hpp
//  Routman Michael, 2005-2006
//------------------------------------


#ifndef ___DELIVERYQUEUE_H
#define ___DELIVERYQUEUE_H

#include <map>
#include <vector>
#include <sys/types.h>
#include <time.h>

#include <logger/Logger.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <core/buffers/Hash.hpp>

#include <mcisme/AbntAddr.hpp>
#include <system/status.h>

namespace smsc {
namespace mcisme {

using std::multimap;
using std::map;
using std::pair;
using std::vector;
using namespace core::synchronization;
using namespace core::buffers;

typedef uint8_t abnt_stat_t;

const abnt_stat_t  Idle         = 0x00;
const abnt_stat_t  InProcess    = 0x01;
const abnt_stat_t  AlertHandled	= 0x02;

const time_t	default_wait = 60;

struct SchedItem
{
  time_t       schedTime;

  AbntAddr     abnt;
  uint8_t      eventsCount;
  uint32_t     lastError;

  uint32_t     abonentsCount;

  SchedItem(){}
  SchedItem(time_t t, const AbntAddr& _abnt, uint8_t ec, uint32_t le, uint32_t ac):
    schedTime(t), abnt(_abnt), eventsCount(ec), lastError(le), abonentsCount(ac){}
  SchedItem(const SchedItem& item):
    schedTime(item.schedTime), abnt(item.abnt), eventsCount(item.eventsCount), lastError(item.lastError), abonentsCount(item.abonentsCount){}
  SchedItem& operator=(const SchedItem& item)
  {
    if(this != &item)
    {
      schedTime = item.schedTime;
      abnt = item.abnt;
      eventsCount = item.eventsCount;
      lastError = item.lastError;
      abonentsCount = item.abonentsCount;
    }
    return *this;
  }
  bool operator<(const SchedItem& item) const
  {return schedTime < item.schedTime;}
};

struct SchedParam
{
  abnt_stat_t abntStatus;
  time_t      schedTime;
  uint16_t    lastError;
  time_t      lastAttempt;
};

char* cTime(const time_t* clock, char* buff, size_t bufSz);

class DeliveryQueue
{
  typedef multimap<time_t, AbntAddr>    DelQueue;
  typedef map<int, time_t>              SchedTable;
  typedef DelQueue::iterator            DelQueueIter;

  DelQueue          deliveryQueue;
  SchedTable        scheduleTable;
  time_t            schedTimeOnBusy;
  EventMonitor      deliveryQueueMonitor;
  Hash<SchedParam>  AbntsStatus;

  time_t   dt;
  uint32_t total;
  bool     isQueueOpen;
  time_t   responseWaitTime, schedDelay;

  smsc::logger::Logger *logger;

public:

  DeliveryQueue(time_t _dt, time_t onBusy, time_t sched_delay):
    dt(_dt), schedTimeOnBusy(onBusy), total(0), isQueueOpen(false), responseWaitTime(60),
    schedDelay(sched_delay), logger(smsc::logger::Logger::getInstance("mci.DlvQueue")){}
  virtual ~DeliveryQueue(){Erase();}

  void AddScheduleRow(int error, time_t wait);
  void SetSchedTimeOnBusy(time_t wait);
  void SetResponseWaitTime(time_t _responseWaitTime){responseWaitTime = _responseWaitTime;}

  void OpenQueue(void);
  void CloseQueue(void);
  bool isQueueOpened(void);
  unsigned GetAbntCount(void);
  unsigned GetQueueSize(void);

  time_t Schedule(const AbntAddr& abnt, bool onBusy=false, time_t schedTime=-1, uint16_t lastError=-1);
  time_t Reschedule(const AbntAddr& abnt, int resp_status = smsc::system::Status::OK); // bool toHead = false
  time_t RegisterAlert(const AbntAddr& abnt);
  bool Get(AbntAddr& abnt);
  bool Get(const AbntAddr& abnt, SchedItem& item);
  //	int Get(vector<SchedItem>& items, int count);
  int Get(vector<SchedItem>& items, int count);
  void Remove(const AbntAddr& abnt);
  void Erase(void);
  time_t calculateSchedTime(bool onBusy) const;
private:

  void Resched(const AbntAddr& abnt, time_t oldSchedTime, time_t newSchedTime);
  time_t CalcTimeDelivery(int err = -1);
  time_t GetDeliveryTime(void);

};

};	//  namespace msisme
};	//  namespace smsc
#endif
