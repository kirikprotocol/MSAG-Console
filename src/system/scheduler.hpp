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
#include "logger/Logger.h"
#include <map>
#include <list>
#include <vector>

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
  //Scheduler(EventQueue& eq):queue(eq)
  static smsc::logger::Logger* log;
  Scheduler():firstQueue(1000)
  {
    seq=0;
    prxmon=0;
    smsCount=0;
    log=smsc::logger::Logger::getInstance("sched");
    queueOrder=qoTimeLine;
  }
  int Execute();
  const char* taskName(){return "scheduler";}

  void Init(MessageStore* st,Smsc* psmsc);

  void AddScheduledSms(SMSId id,const SMS& sms,SmeIndex idx)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);

    if(!c)c=GetChain(sms.getDealiasedDestinationAddress());
    if(!c)
    {
      c=CreateChain(sms.getNextTime(),sms.getDealiasedDestinationAddress(),idx);
    }
    ChainAddTimed(c,sms.getNextTime(),SchedulerData(id,sms.getValidTime()));
    debug2(log,"AddScheduledSms: time=%d, id=%lld,addr=%s,c=%p",sms.getNextTime(),id,sms.getDealiasedDestinationAddress().toString().c_str(),c);
    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }

  void UpdateSmsSchedule(SMSId id,const SMS& sms)
  {
    MutexGuard guard(mon);

    Chain* c=GetChain(sms.getDealiasedDestinationAddress());
    if(!c)
    {
      warn2(log,"UpdateSmsSchedule: chain for %s not found",sms.getDealiasedDestinationAddress().toString().c_str());
      return;
    }
    debug2(log,"UpdateSmsSchedule: time=%d,id=%lld,addr=%s,c=%p",sms.getNextTime(),id,sms.getDealiasedDestinationAddress().toString().c_str(),c);
    if(c->CancelMsgId(id))DecSme(c);
    ChainAddTimed(c,sms.getNextTime(),SchedulerData(id,sms.getValidTime()));
    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }

  void CancelSms(SMSId id,const Address& addr)
  {
    MutexGuard guard(mon);

    Chain* c=GetChain(addr);
    if(!c)
    {
      warn2(log,"CancelSms: chain for %s not found",addr.toString().c_str());
      return;
    }
    debug2(log,"CancelSms: id=%lld, c=%p, addr=%s",id,c,addr.toString().c_str());
    if(c->CancelMsgId(id))DecSme(c);
    if(c->Count()==0)
    {
      debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
      DeleteChain(c);
      return;
    }
    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }



  void DeliveryOk(SMSId id)
  {
    MutexGuard guard(mon);


    Chain* c=GetProcessingChain(id);
    if(!c)
    {
      info2(log,"DeliverOk - chain not found for %lld",id);
      return;
    }
    debug2(log,"DeliverOk: id=%lld, c=%p",id,c);
    if(c->Count()==0)
    {
      debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
      DeleteChain(c);
      return;
    }
    RescheduleChain(c,time(NULL));
  }

  void InvalidSms(SMSId id)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);
    if(!c)
    {
      return;
    }
    if(c->Count()==0)
    {
      debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
      DeleteChain(c);
      return;
    }

    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }

  void AddFirstTimeForward(SMSId id,const SMS& sms)
  {
    MutexGuard guard(mon);
    firstQueue.Push(SchedulerData(id,sms.getValidTime()));
  }

  time_t RescheduleSms(SMSId id,SMS& sms,int smeIndex)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);

    if(!c)
    {
      c=GetChain(sms.getDealiasedDestinationAddress());
    }
    if(!c)
    {
      c=CreateChain(sms.getNextTime(),sms.getDealiasedDestinationAddress(),smeIndex);
    }

    if(sms.attempts==0)
    {
      ChainPush(c,SchedulerData(id,sms.getValidTime()));
      debug2(log,"Resched: push sms %lld to tail (%d), c=%p",id,c->headTime,c);
    }else
    {
      debug2(log,"Resched: set sms %lld as head (%d),c=%p",id,sms.getNextTime(),c);
      ChainSetHead(c,sms.getNextTime(),SchedulerData(id,sms.getValidTime()));
    }
    //smsCount++;



    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);

    if(c->timedQueue.size()>0)
      return c->headTime;
    else
      return 0;
  }

  int getSmeCount(SmeIndex idx,time_t time)
  {
    MutexGuard g(mon);
    SmeStatMap::iterator it=smeStatMap.find(idx);
    if(it!=smeStatMap.end())
      return it->second.count;
    else
      return -1;
  }

  struct SchedulerCounts{
    int timeLineCount;
    int timeLineSize;
    int firstTimeCount;
    int regSize;
    int inProcCount;
  };

  void getCounts(SchedulerCounts& scCnt)
  {
    MutexGuard g(mon);

    scCnt.timeLineCount=smsCount;
    scCnt.timeLineSize=timeLine.size();
    scCnt.firstTimeCount=firstQueue.Count();
    scCnt.regSize=chainRegistry.size();
    scCnt.inProcCount=procMap.size();
  }

  virtual void close(){}
  virtual void putCommand(const SmscCommand& command)
  {
    MutexGuard g(mon);
    outQueue.Push(command);
    mon.notify();

  }


  virtual bool getCommand(SmscCommand& cmd)
  {
    MutexGuard guard(mon);
    SchedulerData sd;
    if(queueOrder==qoFirstTime && ftPop(sd))
    {
      queueOrder=qoTimeLine;
    }else
    if(queueOrder==qoTimeLine && tlPop(sd))
    {
      queueOrder=qoFirstTime;
    }else if(!ftPop(sd) && !tlPop(sd))
    {
      return false;
    }
    cmd=SmscCommand::makeForward(sd.id,sd.resched);
    return true;
  }

  virtual int getCommandEx(std::vector<SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    MutexGuard guard(mon);
    int cnt=0;
    bool order=true;
    SchedulerData sd;
    for(int i=0;i<mx;i++)
    {
      if((order && (ftPop(sd) || tlPop(sd))) || (!order && (tlPop(sd) || ftPop(sd))))
      {
        SmscCommand cmd=SmscCommand::makeForward(sd.id,sd.resched);
        cmd.setProxy(prx);
        cmds.push_back(cmd);
        cnt++;
        order=!order;
      }else
      {
        break;
      }
    }
    mx=cnt;
    return 0;
  }


  virtual SmeProxyState getState()const
  {
    return VALID;
  };
  virtual void init(){};
  virtual bool hasInput()const
  {
    MutexGuard guard(mon);
    return timeLine.size()>0 && timeLine.headTime()<=time(NULL);
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

  time_t getScheduleByAddr(const Address& addr)
  {
    MutexGuard g(mon);
    Chain *c=GetChain(addr);
    if(!c)return 0;
    return c->headTime;
  }

public:
  struct StartupItem{
    SMSId id;
    time_t schedTime;
    int attCount;
    Address addr;
    time_t validTime;
    char smeId[32];
  };
  Array<StartupItem> startupCache;

  Smsc* smsc;

  //EventQueue &queue;

  mutable EventMonitor mon;
  Array<SmscCommand> outQueue;

  struct SchedulerData{
    SchedulerData():id(0),resched(false),expDate(0){}
    SchedulerData(const SchedulerData& d)
    {
      id=d.id;
      resched=d.resched;
      expDate=d.expDate;
    }
    SchedulerData(SMSId argId,time_t argExpDate,bool argRes=false):id(argId),expDate(argExpDate),resched(argRes){}
    SMSId id;
    time_t expDate;
    bool resched;

    bool operator<(const SchedulerData& rhs)const
    {
      return expDate<rhs.expDate;
    }
  };

  bool tlPop(SchedulerData& d)
  {
    time_t t=time(NULL);
    if(timeLine.size()!=0)
    {
      static time_t lastUpdate=time(NULL)-1;
      if(lastUpdate!=t)
      {
        debug2(log,"headTime=%d, now=%d",timeLine.headTime(),t);
        lastUpdate=t;
      }
    }else
    {
      //debug(log,"Sc: queue empty");
    }
    if(timeLine.size()==0 || timeLine.headTime()>t)
    {
      return false;
    }
    Chain* c=timeLine.Pop(this);
    if(!c)return false;
    if(!ChainPop(c,d))
    {
      UpdateChainChedule(c);
      debug2(log,"Chain::Pop failed, rescheduled to %d",c->headTime);
      timeLine.Add(c,this);
      return false;
    }

    debug2(log,"makeFwd: id=%lld, addr=%s, c=%p",d.id,c->addr.toString().c_str(),c);
    AddProcessingChain(d.id,c);
    return true;
  }

  bool ftPop(SchedulerData& sd)
  {
    if(firstQueue.Count())
    {
      firstQueue.Pop(sd);
      debug2(log,"firstForward: id=%lld",sd.id);
      return true;
    }
    return false;
  }

  struct Chain{
    Address addr;
    int smeIndex;
    time_t headTime;
    bool inTimeLine;
    bool inProcMap;

    typedef std::multiset<SchedulerData> ScQueue;
    ScQueue queue;
    int queueSize;
    typedef std::multimap<time_t,SchedulerData> ScTimedQueue;
    ScTimedQueue timedQueue;

    struct Iterators{
      ScQueue::iterator li;
      ScTimedQueue::iterator mi;
    };
    typedef std::map<SMSId,Iterators> Id2IteratorsMap;
    Id2IteratorsMap imap;

    Chain(time_t sctime,const Address& argAddr,int argSmeIndex)
    {
      addr=argAddr;
      smeIndex=argSmeIndex;
      headTime=sctime;
      inTimeLine=false;
      inProcMap=false;
      queueSize=0;
    }

    bool AddTimed(time_t sctime,const SchedulerData& d)
    {
      if(imap.find(d.id)!=imap.end())
      {
        debug2(Scheduler::log,"Chain::AddTimed, id=%lld - already in the chain!!!",d.id);
        return false;
      }
      Iterators its;
      debug2(Scheduler::log,"Chain::AddTimed, time=%d, id=%lld",sctime,d.id);
      its.mi=timedQueue.insert(ScTimedQueue::value_type(sctime,d));
      its.li=queue.end();
      imap.insert(Id2IteratorsMap::value_type(d.id,its));
      queueSize++;
      return true;
    }

    time_t Reschedule(time_t newtime)
    {
      if(!timedQueue.empty())
      {
        time_t frontTime=timedQueue.begin()->first;
        if(frontTime<newtime)newtime=frontTime;
      }
      if(!queue.empty())
      {
        time_t frontTime=queue.begin()->expDate;
        if(frontTime<newtime)newtime=frontTime;
      }
      headTime=newtime;
      return headTime;
    }

    bool Push(const SchedulerData& d)
    {
      if(imap.find(d.id)!=imap.end())
      {
        debug2(Scheduler::log,"Chain::Push, id=%lld - already in the chain!!!",d.id);
        return false;
      }
      debug2(Scheduler::log,"Chain::Push, id=%lld",d.id);
      Iterators its;
      its.li=queue.insert(d);
      its.mi=timedQueue.end();
      imap.insert(Id2IteratorsMap::value_type(d.id,its));
      queueSize++;
      return true;
    }

    bool SetHead(time_t sctime,const SchedulerData& d)
    {
      if(imap.find(d.id)!=imap.end())
      {
        debug2(Scheduler::log,"Chain::SetHead, id=%lld - already in the chain!!!",d.id);
        return false;
      }
      debug2(Scheduler::log,"Chain::SetHead, time=%d, id=%lld",sctime,d.id);
      Iterators its;
      its.li=queue.insert(d);
      its.mi=timedQueue.end();
      imap.insert(Id2IteratorsMap::value_type(d.id,its));
      if(!inTimeLine && queue.begin()->expDate>sctime)headTime=sctime;
      queueSize++;
      return true;
    }

    bool Pop(SchedulerData& d)
    {
      if(!timedQueue.empty())
      {
        ScTimedQueue::iterator i=timedQueue.begin();
        if(i->first<=time(NULL))
        {
          d=i->second;
          debug2(Scheduler::log,"Chain::Pop(timed), id=%lld",d.id);
          timedQueue.erase(i);
          imap.erase(imap.find(d.id));
          queueSize--;
          return true;
        }
      }
      if(queue.empty())return false;
      d=*queue.begin();
      debug2(Scheduler::log,"Chain::Pop(queue), id=%lld",d.id);
      queue.erase(queue.begin());
      imap.erase(imap.find(d.id));
      queueSize--;
      return true;
    }

    bool CancelMsgId(SMSId id)
    {
      Id2IteratorsMap::iterator it=imap.find(id);
      if(it==imap.end())
      {
        return false;
      }
      if(it->second.li!=queue.end())
      {
        queue.erase(it->second.li);
      }else
      {
        timedQueue.erase(it->second.mi);
      }
      imap.erase(it);
      return true;
    }

    int Count()
    {
      return queueSize;
    }

    int getTimedHead()
    {
      if(timedQueue.empty())return 0;
      return timedQueue.begin()->first;
    }
  };

  typedef std::map<Address,Chain*> ChainRegistry;
  ChainRegistry chainRegistry;

  Chain* GetChain(const Address& addr)
  {
    ChainRegistry::iterator it=chainRegistry.find(addr);
    if(it==chainRegistry.end())
    {
      return 0;
    }
    return it->second;
  }

  Chain* CreateChain(time_t sctime,const Address& addr,int smeIndex)
  {
    Chain* rv=new Chain(sctime,addr,smeIndex);
    debug2(log,"newChain: %p/%s",rv,addr.toString().c_str());
    chainRegistry.insert(ChainRegistry::value_type(addr,rv));
    return rv;
  }

  void DeleteChain(Chain* c)
  {
    if(c->inTimeLine || c->inProcMap)
    {
      debug2(log,"Do not delete c=%p",c);
      return;
    }
    ChainRegistry::iterator it=chainRegistry.find(c->addr);
    if(it!=chainRegistry.end())
    {
      chainRegistry.erase(it);
    }
    debug2(log,"deleteChain: %p",c);
    delete c;
  }

  struct SmeStat{
    int count;
    typedef std::set<Chain*> ChainSet;
    ChainSet chainSet;

    SmeStat():count(0){}

    void Inc(Chain* c)
    {
      count++;
      chainSet.insert(c);
    }
    void Dec(Chain* c)
    {
      count--;
      if(c->Count()==0)
      {
        ChainSet::iterator it=chainSet.find(c);
        if(it!=chainSet.end())
        {
          chainSet.erase(it);
        }
      }
    }
  };

  typedef std::map<int,SmeStat> SmeStatMap;
  SmeStatMap smeStatMap;

  void IncSme(Chain* c)
  {
    smsCount++;
    SmeStatMap::iterator it=smeStatMap.find(c->smeIndex);
    if(it!=smeStatMap.end())
    {
      it->second.Inc(c);
    }else
    {
      //warn2(log,"IncSme - smeIndex %d not found in stat map!",c->smeIndex);
      it=smeStatMap.insert(SmeStatMap::value_type(c->smeIndex,SmeStat())).first;
      it->second.Inc(c);

    }
  }
  void DecSme(Chain* c)
  {
    smsCount--;
    SmeStatMap::iterator it=smeStatMap.find(c->smeIndex);
    if(it!=smeStatMap.end())
    {
      it->second.Dec(c);
    }else
    {
      warn2(log,"DecSme - smeIndex %d not found in stat map!",c->smeIndex);
    }
  }

  typedef std::map<SMSId,Chain*> ProcessingMap;
  ProcessingMap procMap;

  void AddProcessingChain(SMSId id,Chain* c)
  {
    c->inProcMap=true;
    procMap.insert(ProcessingMap::value_type(id,c));
  }

  Chain* GetProcessingChain(SMSId id)
  {
    ProcessingMap::iterator it=procMap.find(id);
    if(it==procMap.end())return 0;
    Chain* rv=it->second;
    procMap.erase(it);
    rv->inProcMap=false;
    return rv;
  }

  struct MultiChain{
    typedef std::multimap<Address,Chain*> ChainMap;
    ChainMap cmap;
    void Push(Chain* c)
    {
      ChainMap::iterator it=cmap.lower_bound(c->addr);
      if(it==cmap.end() || it->first!=c->addr)
      {
        cmap.insert(it,ChainMap::value_type(c->addr,c));
        c->inTimeLine=true;
        debug2(Scheduler::log,"mc:push: c=%p",c);
      }else
      {
        warn2(Scheduler::log,"FUCK: %p",c);
      }
    }
    Chain* Pop()
    {
      if(cmap.size()==0)return 0;
      ChainMap::iterator it=cmap.begin();
      Chain* rv=it->second;
      cmap.erase(it);
      rv->inTimeLine=false;
      debug2(Scheduler::log,"mc:pop: c=%p",rv);
      return rv;
    }
    void RemoveChain(Chain* c)
    {
      ChainMap::iterator i=cmap.find(c->addr);
      if(i==cmap.end())
      {
        debug2(Scheduler::log,"Chain not found in multichain %p/%s",c,c->addr.toString().c_str());
        return;
      }
      debug2(Scheduler::log,"mc:remove: c=%p",i->second);
      i->second->inTimeLine=false;
      cmap.erase(i);
    }
  };

  std::vector<MultiChain*> mcPool;

  MultiChain* newMultiChain()
  {
    if(mcPool.size()>0)
    {
      MultiChain* rv=mcPool.back();
      mcPool.pop_back();
      return rv;
    }
    return new MultiChain;
  }
  void deleteMultiChain(MultiChain* mc)
  {
    mcPool.push_back(mc);
  }


  struct TimeLine{
    typedef std::multimap<const time_t,MultiChain*> TimeMap;
    TimeMap tmap;


    void Add(Chain* c,Scheduler* sc)
    {
      if(c->inTimeLine || c->inProcMap)
      {
        return;
      }
      debug2(Scheduler::log,"Sc: add to timeLine %p, addr=%s, headTime=%d",c,c->addr.toString().c_str(),c->headTime);
      TimeMap::iterator it=tmap.lower_bound(c->headTime);
      if(it==tmap.end() || it->first!=c->headTime)
      {
        MultiChain* mc=sc->newMultiChain();
        debug1(Scheduler::log,"Sc: create new multichain");
        mc->Push(c);
        tmap.insert(it,TimeMap::value_type(c->headTime,mc));
      }else
      {
        it->second->Push(c);
      }
    }

    Chain* Pop(Scheduler* sc)
    {
      for(;;)
      {
        if(tmap.size()==0)
        {
          debug1(Scheduler::log,"Sc: pop return 0, tmap is empty");
          return 0;
        }
        TimeMap::iterator i=tmap.begin();
        MultiChain* mc=i->second;
        Chain* rv=mc->Pop();
        if(rv)
        {
          return rv;
        }
        debug1(Scheduler::log,"Sc: deleting empty multichain");
        sc->deleteMultiChain(mc);
        tmap.erase(i);
      }
    }

    void RemoveChain(Chain* c)
    {
      TimeMap::iterator i=tmap.find(c->headTime);
      if(i==tmap.end())
      {
        debug2(Scheduler::log,"Multichain not found for chain %p/%s, at time %d",c,c->addr.toString().c_str(),c->headTime);
        return;
      }
      i->second->RemoveChain(c);
    }

    int size()const
    {
      return tmap.size();
    }

    time_t headTime()const
    {
      if(tmap.size()>0)
      {
        return tmap.begin()->first;
      }else
      {
        return 0;
      }
    }

  };

  CyclicQueue<SchedulerData> firstQueue;
  TimeLine timeLine;

  void RescheduleChain(Chain* c,time_t sctime)
  {
    if(c->inProcMap)return;
    if(c->inTimeLine)
    {
      timeLine.RemoveChain(c);
    }
    c->Reschedule(sctime);
    timeLine.Add(c,this);
  }

  void UpdateChainChedule(Chain* c)
  {
    time_t sctime=c->getTimedHead();
    if(sctime!=0)
    {
      if(sctime<c->headTime || (sctime>c->headTime && c->queue.empty()))
      {
        if(!c->inTimeLine)
        {
          c->headTime=sctime;
        }
      }
    }
  }

  void ChainPush(Chain* c,const SchedulerData& d)
  {
    if(c->Push(d))IncSme(c);
  }
  void ChainSetHead(Chain* c,time_t sctime,const SchedulerData& d)
  {
    if(c->SetHead(sctime,d))IncSme(c);
  }
  void ChainAddTimed(Chain* c,time_t sctime,const SchedulerData& d)
  {
    if(c->AddTimed(sctime,d))IncSme(c);
  }
  bool ChainPop(Chain* c,SchedulerData& d)
  {
    if(c->Pop(d))
    {
      DecSme(c);
      return true;
    }else
    {
      return false;
    }

  }

  time_t tlHeadTime()
  {
    MutexGuard mg(mon);
    return timeLine.headTime();
  }

  int smsCount;

  enum QueueOrder{qoTimeLine,qoFirstTime};
  QueueOrder queueOrder;


  Mutex seqMtx;
  int seq;
  ProxyMonitor* prxmon;
};

}//system
}//smsc


#endif
