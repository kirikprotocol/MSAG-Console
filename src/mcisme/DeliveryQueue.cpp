//------------------------------------
//  DeliveryQueue.cpp
//  Routman Michael, 2005-2006
//------------------------------------

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
#include "DeliveryQueue.hpp"

namespace smsc {
namespace mcisme {

using std::multimap;
using std::map;
using std::pair;
using std::vector;
using namespace core::synchronization;
using namespace core::buffers;


char* cTime(const time_t* clock, char* buff, size_t bufSz)
{
  struct tm t;
  localtime_r(clock, &t);
  snprintf(buff, bufSz, "%.2d.%.2d.%4d %.2d:%.2d:%.2d", t.tm_mday, t.tm_mon+1, t.tm_year+1900, t.tm_hour, t.tm_min, t.tm_sec);								
  return buff;
}

void DeliveryQueue::AddScheduleRow(int error, time_t wait)
{
  MutexGuard lock(deliveryQueueMonitor);
  if(error == -1)
    dt = wait;
  else
    scheduleTable.insert(map<int, time_t>::value_type(error, wait));
}

void DeliveryQueue::SetSchedTimeOnBusy(time_t wait)
{
  MutexGuard lock(deliveryQueueMonitor);
  schedTimeOnBusy = wait;
}

void DeliveryQueue::OpenQueue(void)
{
  MutexGuard lock(deliveryQueueMonitor);
  isQueueOpen = true;
  deliveryQueueMonitor.notify();
}

void DeliveryQueue::CloseQueue(void)
{
  isQueueOpen = false;
}

bool DeliveryQueue::isQueueOpened(void)
{
  return isQueueOpen;
}

unsigned DeliveryQueue::GetAbntCount(void)
{
  MutexGuard lock(deliveryQueueMonitor);
  return AbntsStatus.GetCount();
}

unsigned DeliveryQueue::GetQueueSize(void)
{
  MutexGuard lock(deliveryQueueMonitor);
  return static_cast<unsigned>(deliveryQueue.size());
}

time_t DeliveryQueue::Schedule(const AbntAddr& abnt, bool onBusy, time_t schedTime, uint16_t lastError)
{
  string strAbnt = abnt.toString();
  MutexGuard lock(deliveryQueueMonitor);
  char curSchedTimeStr[32];
  char lastAttemptTimeStr[32];
  if(AbntsStatus.Exists(strAbnt.c_str()))
  {
    SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());
    time_t curSchedTime = schedParam->schedTime;
    time_t curTime = time(0);
    smsc_log_info(logger, "Subscriber %s already scheduled on %s. total = %d (%d)", strAbnt.c_str(), cTime(&curSchedTime, curSchedTimeStr, sizeof(curSchedTimeStr)), total, deliveryQueue.size());
    // sanity check for forgotten events
    if( (curTime - curSchedTime) > 900)
    {
      Resched(abnt, curSchedTime, curTime);
      schedParam->abntStatus = Idle;
      schedParam->schedTime = curTime;
      deliveryQueueMonitor.notify();
      smsc_log_info(logger, "Subscriber %s rescheduled on %s. Status = %d, lastAttempt = %s, total = %d (%d)", strAbnt.c_str(), cTime(&curTime, curSchedTimeStr, sizeof(curSchedTimeStr)), schedParam->abntStatus, cTime(&schedParam->lastAttempt, lastAttemptTimeStr, sizeof(lastAttemptTimeStr)), total, deliveryQueue.size());
    }
    return schedParam->schedTime;
  }
  if(-1 != schedTime)
  {
    smsc_log_info(logger, "Subscriber %s scheduling on time %s", strAbnt.c_str(), cTime(&schedTime, curSchedTimeStr, sizeof(curSchedTimeStr)));
  }
  else
  {
    schedTime = calculateSchedTime(onBusy);
    smsc_log_info(logger, "Subscriber %s scheduling on time %s", strAbnt.c_str(), cTime(&schedTime, curSchedTimeStr, sizeof(curSchedTimeStr)));
  }

  deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(schedTime, abnt));

  SchedParam schedParam = {Idle, schedTime, lastError, 0};
  AbntsStatus.Insert(strAbnt.c_str(), schedParam);
  deliveryQueueMonitor.notify();
  total++;

  return schedTime;
}

time_t DeliveryQueue::calculateSchedTime(bool onBusy) const
{
  time_t schedTime = time(0);
  if(onBusy)
    schedTime += schedTimeOnBusy;

  return schedTime;
}

time_t DeliveryQueue::Reschedule(const AbntAddr& abnt, int resp_status) // bool toHead = false
{
  bool toHead = false;
  string strAbnt = abnt.toString();
  //	smsc_log_debug(logger, "Reschedule %s", strAbnt.c_str());
  MutexGuard lock(deliveryQueueMonitor);
  if(!AbntsStatus.Exists(strAbnt.c_str()))
  {
    smsc_log_debug(logger, "Rescheduling %s canceled (Subscriber is not in hash).", strAbnt.c_str());
    return 0;
  }

  if(resp_status == smsc::system::Status::OK)
  {
    smsc_log_debug(logger, "Previous SMS for Subscriber %s was delivered normally or no DATA_SM_RESP.", strAbnt.c_str());
    toHead = true;
  }

  SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());		
  if(schedParam->abntStatus == AlertHandled)
  {
    smsc_log_debug(logger, "ALERT_NOTIFICATION for %s has accepted previously.", strAbnt.c_str());
    toHead = true;
  }

  schedParam->abntStatus = Idle;
  time_t oldSchedTime = schedParam->schedTime;
  time_t newSchedTime;
  char newSchedTimeStr[32];
  if(toHead)
  {
    newSchedTime = time(0);
    smsc_log_info(logger, "Rescheduling %s to Head", strAbnt.c_str());
  }
  else 
  {
    newSchedTime = CalcTimeDelivery(resp_status);
    smsc_log_info(logger, "Rescheduling %s to %s by error %d", strAbnt.c_str(), cTime(&newSchedTime, newSchedTimeStr, sizeof(newSchedTimeStr)), resp_status);
  }

  Resched(abnt, oldSchedTime, newSchedTime);
  schedParam->schedTime = newSchedTime;
  schedParam->lastError = resp_status;
  deliveryQueueMonitor.notify();
  //	smsc_log_info(logger, "total = %d in queue = %d", total, deliveryQueue.size());
  return newSchedTime;
}

time_t DeliveryQueue::RegisterAlert(const AbntAddr& abnt)
{
  string strAbnt = abnt.toString();
  smsc_log_debug(logger, "RegisterAlert for %s, schedDelay=%u", strAbnt.c_str(), schedDelay);
  MutexGuard lock(deliveryQueueMonitor);

  if(!AbntsStatus.Exists(strAbnt.c_str()))
  {
    smsc_log_warn(logger, "Registration alert for %s canceled (Subscriber is not in hash).", strAbnt.c_str());
    return -1;
  }

  SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());		

  if(schedParam->abntStatus == Idle)
  {
    char newSchedTimeStr[128];
    time_t newSchedTime = time(0) + schedDelay;
    Resched(abnt, schedParam->schedTime, newSchedTime);
    schedParam->schedTime = newSchedTime;
    schedParam->lastError = -1;
    smsc_log_info(logger, "Registering Alert and rescheduling %s on %s",
                  strAbnt.c_str(), cTime(&newSchedTime, newSchedTimeStr, sizeof(newSchedTimeStr)));
    deliveryQueueMonitor.notify();
    return newSchedTime;
  }
  else
  {
    schedParam->abntStatus = AlertHandled;
    smsc_log_info(logger, "Alert received and subscriber %s is already in process", strAbnt.c_str());
    return schedParam->schedTime;
  }
}

bool DeliveryQueue::Get(AbntAddr& abnt)
{
  MutexGuard lock(deliveryQueueMonitor);
  int pause = static_cast<int>(GetDeliveryTime()-time(0));

  //	smsc_log_debug(logger, "pause = %d", pause);
  if(pause > 0)
  {
    deliveryQueueMonitor.wait(pause*1000);

    //if(0 == deliveryQueueMonitor.wait(pause*1000))
    //		smsc_log_debug(logger, "recieved a notify.");
    //else
    //	smsc_log_debug(logger, "timeout has passed.");
  }

  if(!isQueueOpen)
  {
    smsc_log_info(logger, "Queue was closed.");
    return false;
  }

  if(!deliveryQueue.empty())
  {
    multimap<time_t, AbntAddr>::iterator It;
    time_t	t;

    It = deliveryQueue.begin();
    t = It->first;

    time_t curTime = time(0);
    if(t > curTime)
    {
      smsc_log_debug(logger, "Delivery time is not reached yet.");
      return false;
    }
    abnt = It->second;
    deliveryQueue.erase(It);
    string strAbnt = abnt.toString();
    if(AbntsStatus.Exists(strAbnt.c_str()))
    {
      SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());
      if(schedParam->abntStatus == Idle)
      {	
        schedParam->abntStatus = InProcess;
        schedParam->schedTime = 0;
        schedParam->lastAttempt = curTime;
        smsc_log_info(logger, "Subscriber %s ready to delivery.", strAbnt.c_str());
        return true;
      }
      else
      {
        smsc_log_info(logger, "Subscriber %s already in delivery.", strAbnt.c_str());
        return false;
      }
      return true;
    }
    else
    {
      return false;
    }
  }

  smsc_log_debug(logger, "deliveryQueue is empty.");
  return false;
}

bool DeliveryQueue::Get(const AbntAddr& abnt, SchedItem& item)
{
  MutexGuard lock(deliveryQueueMonitor);
  string strAbnt = abnt.toString();
  if(AbntsStatus.Exists(strAbnt.c_str()))
  {
    SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());
    item.abnt = abnt;
    item.schedTime = schedParam->schedTime;
    item.lastError = schedParam->lastError;
    return true;
  }
  return false;
}

int DeliveryQueue::Get(vector<SchedItem>& items, int count)
{
  MutexGuard    lock(deliveryQueueMonitor);
  AbntAddr      abnt;
  DelQueueIter  It;
  int           i;

  It = deliveryQueue.begin();
  for(i = 0; i < count; i++)
  {	
    if(It == deliveryQueue.end()) break;
    uint32_t  abonentsCount = 0;
    time_t    schedTime = It->first;

    while(schedTime == It->first)
    {
      ++abonentsCount; ++It;
      if(It == deliveryQueue.end()) break;
    }
    SchedItem item;
    item.schedTime = schedTime;
    item.abonentsCount = abonentsCount;
    items.push_back(item);
  }
  return i;
}

void DeliveryQueue::Remove(const AbntAddr& abnt)
{
  string strAbnt = abnt.toString();
  smsc_log_debug(logger, "Remove %s",strAbnt.c_str());
  MutexGuard lock(deliveryQueueMonitor);
  if(AbntsStatus.Exists(strAbnt.c_str()))
  {
    AbntsStatus.Delete(strAbnt.c_str());
    total--;
    smsc_log_info(logger, "Remove %s total = %d (%d, %d)", strAbnt.c_str(), total, deliveryQueue.size(), AbntsStatus.GetCount());
  }
  else
    smsc_log_debug(logger, "Remove %s canceled (Subscriber is not in hash).", strAbnt.c_str());
}
void DeliveryQueue::Erase(void)
{
  smsc_log_debug(logger, "Erase");
  MutexGuard lock(deliveryQueueMonitor);
  smsc_log_info(logger, "Queue size = %d, Hash size = %d, total = %d", deliveryQueue.size(), AbntsStatus.GetCount(), total);
  deliveryQueue.erase(deliveryQueue.begin(), deliveryQueue.end());
  AbntsStatus.Empty();
  total = 0;
  smsc_log_info(logger, "Erased. (%d %d %d)", deliveryQueue.size(), AbntsStatus.GetCount(), total);
}

void DeliveryQueue::Resched(const AbntAddr& abnt, time_t oldSchedTime, time_t newSchedTime)
{
  if(0 != oldSchedTime)
  {
    pair<DelQueueIter, DelQueueIter> range= deliveryQueue.equal_range(oldSchedTime);
    for(DelQueueIter i = range.first; i != range.second; ++i)
      if((*i).second == abnt)
      {
        deliveryQueue.erase(i);
        break;
      }
  }
  deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(newSchedTime, abnt));
}

time_t DeliveryQueue::CalcTimeDelivery(int err)
{
  if(err == -1) return time(0) + dt;
  map<int, time_t>::iterator It;
  It = scheduleTable.find(err);
  if(It == scheduleTable.end())
    return time(0) + dt;
  return time(0) + It->second;
}

time_t DeliveryQueue::GetDeliveryTime(void)
{
  multimap<time_t, AbntAddr>::iterator It;
  time_t t = time(0) + default_wait;
  if(!deliveryQueue.empty())
  {
    It = deliveryQueue.begin();
    t = It->first;
  }
  return t;
}

}
}
