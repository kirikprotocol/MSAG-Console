#include "system/scheduler.hpp"

namespace smsc{
namespace system{

void Scheduler::Init(MessageStore* st)
{
  smsc::store::TimeIdIterator* it=st->getReadyForRetry(time(NULL)+30*24*60*60);
  if(it)
  {
    MutexGuard guard(mon);
    SMSId id;
    try{
      while(it->getNextId(id))
      {
        timeLine.insert(TimeIdPair(it->getTime(),id));
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
  time_t t,r;
  Event e;
  Array<SMSId> ids;
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
      SmscCommand cmd=SmscCommand::makeForward();
      queue.enqueue(ids[i],cmd);
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
}

void Scheduler::ChangeSmsSchedule(SMSId id,time_t newtime)
{
  MutexGuard guard(mon);
  timeLine.insert(TimeIdPair(newtime,id));
  __trace2__("Scheduler: changesmsschedule %lld -> %d",id,newtime);
  mon.notify();
  __trace2__("Scheduler: notify");
}

void Scheduler::UpdateSmsSchedule(time_t old,SMSId id,time_t newtime)
{
  MutexGuard guard(mon);
  std::pair<TimeLineMap::iterator,TimeLineMap::iterator> p;
  p=timeLine.equal_range(old);
  if(p.first!=timeLine.end())
  {
    for(TimeLineMap::iterator i=p.first;i!=p.second;i++)
    {
      if(i->second==id)
      {
        timeLine.erase(i);
        timeLine.insert(TimeIdPair(newtime,id));
        mon.notify();
        __trace2__("Scheduler: updatesmsschedule: %lld: %d->%d",id,old,newtime);
        break;
      }
    };
  }
}


};//system
};//smsc
