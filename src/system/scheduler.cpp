#include "system/smsc.hpp"
#include "system/scheduler.hpp"
#include <thread.h>

namespace smsc{
namespace system{

void Scheduler::Init(MessageStore* st,Smsc* psmsc)
{
  smsc=psmsc;
  smsc::store::TimeIdIterator* it=st->getReadyForRetry(time(NULL)+30*24*60*60);
  if(it)
  {
    MutexGuard guard(mon);
    SMSId id;
    try{
      StartupItem si;
      while(it->getNextId(id))
      {

        if(it->getDstSmeId(si.smeId))
        {
          si.id=id;
          si.schedTime=it->getTime();
          startupCache.Push(si);
          /*
          SmeIndex idx=psmsc->getSmeIndex(dstSmeId);
          if(idx!=INVALID_SME_INDEX)
          {
            timeLine.insert(TimeIdPair(it->getTime(),Data(id,idx)));
            CacheItem *ci=smeCountCache.GetPtr(idx);
            if(ci)
            {
              ci->totalCount++;
            }else
            {
              CacheItem c;
              c.totalCount=1;
              smeCountCache.Insert(idx,c);
            }
          }
          */
        }
      }
      __trace2__("Scheduler: init ok - %d messages for rescheduling",timeLine.size());
    }catch(std::exception& e)
    {
      __warning2__("Scheduler:failed to init scheduler timeline:%s",e.what());
    }catch(...)
    {
      __warning__("Scheduler:failed to init scheduler timeline:unknown");
    }
    delete it;
  }else
  {
    __trace__("Scheduler: init - No messages found");
  }
}


int Scheduler::Execute()
{
/*  time_t t,r;
  Event e;
  Array<Data> ids;
  while(!isStopping)
  {
    t=time(NULL);
    {
      MutexGuard guard(mon);
      if(timeLine.size())
      {
        __trace2__("Scheduler: start check - now=%d, first=%d",t,timeLine.begin()->first);
      }
      while(timeLine.size() && timeLine.begin()->first<=t && ids.Count()<rescheduleLimit)
      {
        ids.Push(timeLine.begin()->second);
        timeLine.erase(timeLine.begin());
      }
    }
    if(ids.Count())
    {
      __trace2__("Scheduler: %d ids for reschedule",ids.Count());
    }
    for(int i=0;i<ids.Count();i++)
    {
      SmscCommand cmd=SmscCommand::makeForward(ids[i].idx,ids[i].id,false);
      queue.enqueue(ids[i].id,cmd);
      CacheItem *ci=smeCountCache.GetPtr(ids[i].idx);
      if(ci)
      {
        if(ci->lastUpdate>t)
        {
          ci->count--;
        }
      }

      e.Wait(1000/rescheduleLimit);
      if(isStopping)break;
    }
    ids.Clean();

    if(isStopping)break;
    {
      MutexGuard guard(mon);
      if(timeLine.size())
      {
        r=timeLine.begin()->first;
      }else
      {
        r=0;
      }
      if(r==0)
        mon.wait();
      else
        if(r>t)mon.wait((r-t)*1000);
    }
  }
  return 0;
  */

  mon.Lock();
  for(int i=0;i<startupCache.Count();i++)
  {
    StartupItem& si=startupCache[i];
    SmeIndex idx=smsc->getSmeIndex(si.smeId);
    if(idx!=INVALID_SME_INDEX)
    {
      timeLine.insert(TimeIdPair(si.schedTime,Data(si.id,idx)));
      CacheItem *ci=smeCountCache.GetPtr(idx);
      if(ci)
      {
        ci->totalCount++;
      }else
      {
        CacheItem c;
        c.totalCount=1;
        smeCountCache.Insert(idx,c);
      }
    }
    mon.Unlock();
    thr_yield();
    mon.Lock();;
  }
  mon.Unlock();

  time_t r,t;
  while(!isStopping)
  {
    t=time(NULL);
    r=0;
    MutexGuard guard(mon);
    while(outQueue.Count())
    {
      SmscCommand cmd;
      outQueue.Shift(cmd);
      if(cmd->cmdid!=SMEALERT)continue;
      int idx=cmd->get_smeIndex();
      TimeLineMap::iterator it=timeLine.begin(),tmpit;
      while(it!=timeLine.end())
      {
        if(it->second.idx!=idx || it->first<=t)
        {
          it++;
          continue;
        }
        timeLine.insert(TimeIdPair(t,Data(it->second.id,idx,true)));
        tmpit=it;
        it++;
        timeLine.erase(tmpit);
      }
    }
    if(timeLine.size()>0 && timeLine.begin()->first<t)
    {
      if(prxmon)prxmon->Signal();
      mon.wait(100);
    }else
    {
      mon.wait(1000);
    }
  }
  return 0;
}

void Scheduler::ChangeSmsSchedule(SMSId id,time_t newtime,SmeIndex idx)
{
  __require__(id!=0);
  MutexGuard guard(mon);
  timeLine.insert(TimeIdPair(newtime,Data(id,idx)));
  __trace2__("Scheduler: changesmsschedule %lld -> %d",id,newtime);
  CacheItem *ci=smeCountCache.GetPtr(idx);
  if(ci)
  {
    ci->totalCount++;
  }else
  {
    CacheItem c;
    c.totalCount++;
    smeCountCache.Insert(idx,c);
  }
  mon.notify();
  __trace__("Scheduler: notify");
}

void Scheduler::UpdateSmsSchedule(time_t old,SMSId id,time_t newtime,SmeIndex idx)
{
  __require__(id!=0);
  MutexGuard guard(mon);
  std::pair<TimeLineMap::iterator,TimeLineMap::iterator> p;
  p=timeLine.equal_range(old);
  if(p.first!=timeLine.end())
  {
    for(TimeLineMap::iterator i=p.first;i!=p.second;i++)
    {
      if(i->second.id==id)
      {
        timeLine.erase(i);
        timeLine.insert(TimeIdPair(newtime,Data(id,idx,true)));
        CacheItem *ci=smeCountCache.GetPtr(idx);
        if(ci)
        {
          if(ci->lastUpdate>old && ci->lastUpdate<newtime)
          {
            ci->count--;
          }
        }
        mon.notify();
        __trace2__("Scheduler: updatesmsschedule: %lld: %d->%d",id,old,newtime);
        return;
      }
    };
  }
  //timeLine.insert(TimeIdPair(newtime,Data(id,idx)));
  //mon.notify();
}

int Scheduler::getSmeCount(SmeIndex idx,time_t time)
{
  MutexGuard guard(mon);
  if(!smeCountCache.Exist(idx))
  {
    CacheItem ci;
    ci.lastUpdate=0;
    ci.count=0;
    ci.totalCount=0;
    smeCountCache.Insert(idx,ci);
  }
  CacheItem *pci=smeCountCache.GetPtr(idx);
  if(time==0)
  {
    return pci->totalCount;
  }

  TimeLineMap::iterator from=timeLine.lower_bound(pci->lastUpdate);
  if(from==timeLine.end())from=timeLine.begin();
  TimeLineMap::iterator till=timeLine.upper_bound(time);
  if(till==timeLine.end())
  {
    return pci->count;
  }
  for(;from!=till;from++)
  {
    if(from->second.idx==idx)
    {
      pci->count++;
    }
  }
  pci->lastUpdate=time+1;
  return pci->count;
}


}//system
}//smsc
