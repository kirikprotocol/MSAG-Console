#include "system/smsc.hpp"
#include "system/scheduler.hpp"
#include <sched.h>

namespace smsc{
namespace system{

smsc::logger::Logger* Scheduler::log;

void Scheduler::Init(MessageStore* st,Smsc* psmsc)
{
  smsc=psmsc;
  smsc::store::TimeIdIterator* it=st->getReadyForRetry(time(NULL)+30*24*60*60);
  if(it)
  {
    debug1(log,"Scheduler: start init");
    MutexGuard guard(mon);
    SMSId id;
    try{
      StartupItem si;
      while(it->getNextId(id))
      {
        FullAddressValue ddabuf;
        if(it->getDstSmeId(si.smeId) && it->getDda(ddabuf))
        {
          si.id=id;
          si.schedTime=it->getTime();
          si.attCount=it->getAttempts();
          si.addr=ddabuf;
          si.validTime=it->getValidTime();
          startupCache.Push(si);
          if(startupCache.Count()%1000==0)debug2(log,"Loading scheduler - %d",startupCache.Count());
        }
      }
      info2(log,"Scheduler: init ok - %d messages for rescheduling",startupCache.Count());
    }catch(std::exception& e)
    {
      warn2(log,"Scheduler:failed to init scheduler timeline:%s",e.what());
    }catch(...)
    {
      warn1(log,"Scheduler:failed to init scheduler timeline:unknown");
    }
    delete it;
  }else
  {
    info1(log,"Scheduler: init - No messages found");
  }
}


int Scheduler::Execute()
{
  mon.Lock();
  try{
    for(int i=0;i<startupCache.Count();i++)
    {
      StartupItem& si=startupCache[i];
      SmeIndex idx=INVALID_SME_INDEX;
      try{
        idx=smsc->getSmeIndex(si.smeId);
      }catch(...)
      {
        warn2(log,"failed to get sme index for %s",si.smeId);
        continue;
      }

      if(idx!=INVALID_SME_INDEX)
      {
        Chain* c=GetChain(si.addr);
        if(!c)
        {
          c=CreateChain(si.schedTime,si.addr,idx);
        }
        if(si.attCount==0)
        {
          ChainAddTimed(c,si.schedTime,SchedulerData(si.id,si.validTime));
        }else
        {
          ChainPush(c,SchedulerData(si.id,si.validTime));
        }
        timeLine.Add(c,this);
        //smsCount++;
      }
      mon.Unlock();
      sched_yield();
      mon.Lock();;
    }
  }catch(std::exception& e)
  {
    warn2(log,"error during scheduler initialization: %s",e.what());
  }
  mon.Unlock();

  time_t t;
  while(!isStopping)
  {
    t=time(NULL);
    MutexGuard guard(mon);
    while(outQueue.Count())
    {
      SmscCommand cmd;
      outQueue.Shift(cmd);
      if(cmd->cmdid==SMEALERT)
      {
        int idx=cmd->get_smeIndex();
        debug2(log,"SMEALERT for %d",idx);
        int cnt=0;
        time_t sctime=time(NULL);
        SmeStatMap::iterator it=smeStatMap.find(idx);
        if(it==smeStatMap.end())continue;
        SmeStat::ChainSet::iterator cit=it->second.chainSet.begin();
        for(;cit!=it->second.chainSet.end();cit++)
        {
          Chain* c=*cit;
          RescheduleChain(c,sctime);
          cnt++;
          if(cnt==5)
          {
            sctime++;
            cnt=0;
          }
        }
      }else if(cmd->cmdid==HLRALERT)
      {
        debug2(log,"HLRALERT for %s",cmd->get_address().toString().c_str());
        Chain* c=GetChain(cmd->get_address());
        if(!c)continue;
        if(c->inProcMap)continue;
        RescheduleChain(c,time(NULL));
      }
    }
    if(timeLine.size()>0 && timeLine.headTime()<t)
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


}//system
}//smsc
