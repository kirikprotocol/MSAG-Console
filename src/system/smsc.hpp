#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
#include "util/config/route/RouteConfig.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"
#include "task_container.h"
#include "router/route_manager.h"
#include "system/event_queue.h"
#include "store/MessageStore.h"
#include "util/config/smeman/SmeManConfig.h"
#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include "system/scheduler.hpp"
#include "profiler/profiler.hpp"
#include "system/smscsme.hpp"
#include "system/cancel_agent.hpp"
#include "system/alert_agent.hpp"
#include "system/performance.hpp"
#include "system/mapio/MapIoTask.h"
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"

#include "distrlist/DistrListManager.h"
#include "distrlist/DistrListProcess.h"
#include "stat/StatisticsManager.h"

#include "system/mrcache.hpp"

#include "system/traffic_control.hpp"

#include <sys/types.h>

#include "core/buffers/XHash.hpp"


namespace smsc{
namespace system{

using smsc::sms::SMS;
using smsc::smeman::SmeProxy;
using smsc::alias::AliasManager;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;
using smsc::distrlist::DistrListManager;
using smsc::distrlist::DistrListProcess;
using smsc::util::config::route::RouteConfig;
//class smsc::store::MessageStore;

template<class T>
class Reffer
{
  Mutex sync_;
  T* manager_;
  unsigned refCounter_;
public:
  T* operator->() { return manager_;}
  Reffer(T* manager)
  {
    refCounter_ = 1;
    manager_ = manager;
  }
  void Release()
  {
    unsigned counter;
    {
      MutexGuard g(sync_);
      counter = --refCounter_;
    }
    if ( counter == 0 )
    {
      delete this;
    }
  }
  Reffer* AddRef(){
    MutexGuard g(sync_);
    ++refCounter_;
    return this;
  }
  static Reffer* Create(T* t) {return new Reffer(t);}
protected:
  virtual ~Reffer() {delete manager_;}
private:
  Reffer& operator = (const Reffer&);
  Reffer(const Reffer&);
};

template<class T>
class RefferGuard
{
  mutable Reffer<T>* reffer_;
public:
  RefferGuard(Reffer<T>* reffer) : reffer_(reffer) { reffer_->AddRef(); }
  RefferGuard(const RefferGuard& g) : reffer_(g.reffer_){g.reffer_=0;}
  ~RefferGuard() { if ( reffer_ != 0 ) reffer_->Release(); reffer_ = 0; }
  Reffer<T>& operator->() { return *reffer_; }
private:
  RefferGuard& operator = (const RefferGuard&);
};

namespace StatEvents{
  const int etSubmitOk     =1;
  const int etSubmitErr    =2;
  const int etDeliveredOk  =3;
  const int etDeliverErr   =4;
  const int etUndeliverable=5;
  const int etRescheduled  =6;
};

struct SmscConfigs{
  smsc::util::config::Manager* cfgman;
  smsc::util::config::smeman::SmeManConfig* smemanconfig;
  smsc::util::config::alias::AliasConfig* aliasconfig;
  smsc::util::config::route::RouteConfig* routesconfig;
};

class Smsc
{
public:
  Smsc():ssockman(&tp,&smeman),stopFlag(false),router_(0),aliaser_(0)
  {
    submitOkCounter=0;
    submitErrCounter=0;
    deliverOkCounter=0;
    deliverErrTempCounter=0;
    deliverErrPermCounter=0;
    rescheduleCounter=0;
    startTime=0;
    tcontrol=0;
  };
  ~Smsc();
  void init(const SmscConfigs& cfg);
  void run();
  void stop(){stopFlag=true;}
  void mainLoop();
  void shutdown();
  TaskContainer tasks;
  bool Smsc::routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,smsc::router::RouteInfo* ri,SmeIndex idx=-1);

  bool AliasToAddress(const Address& alias,Address& addr)
  {
    return getAliaserInstance()->AliasToAddress(alias,addr);
  }
  bool AddressToAlias(const Address& addr,Address& alias)
  {
    return getAliaserInstance()->AddressToAlias(addr,alias);
  }

  void cancelSms(SMSId id,const Address& oa,const Address& da)
  {
    eventqueue.enqueue(id,SmscCommand::makeCancel(id,oa,da));
  }

  void notifyScheduler()
  {
    scheduler->notify();
  }

  void ChangeSmsSchedule(SMSId msgId,time_t t,SmeIndex idx)
  {
    scheduler->ChangeSmsSchedule(msgId,t,idx);
  }

  void UpdateSmsSchedule(time_t old,SMSId msgId,time_t t,SmeIndex idx)
  {
    scheduler->UpdateSmsSchedule(old,msgId,t,idx);
  }

  int GetSmeScheduleCount(SmeIndex idx,time_t time)
  {
    return scheduler->getSmeCount(idx,time);
  }

  smsc::profiler::Profiler* getProfiler()
  {
    return profiler;
  }

  smsc::smeman::SmeInfo getSmeInfo(smsc::smeman::SmeIndex idx)
  {
    return smeman.getSmeInfo(idx);
  }

  smsc::smeman::SmeIndex getSmeIndex(const string& sid)
  {
    return smeman.lookup(sid);
  }

  smsc::smeman::SmeInfo getSmeInfo(const string& sid)
  {
    return smeman.getSmeInfo(smeman.lookup(sid));
  }

  SmeProxy* getSmeProxy(const string& sid)
  {
    smsc::smeman::SmeIndex idx=smeman.lookup(sid);
    if(idx==-1)return 0;
    return smeman.getSmeProxy(idx);
  }

  void submitSms(SMS* sms)
  {
    smscsme->putSms(sms);
  }

  void unregisterSmeProxy(const string& sysid)
  {
    smeman.unregisterSmeProxy(sysid);
  }

  SmeAdministrator* getSmeAdmin(){return &smeman;}

  void registerStatisticalEvent(int eventType,const SMS* sms)
  {
    using namespace StatEvents;
    switch(eventType)
    {
      /*
        submit/stored
        submit/error
        delivered - ok
        delivered - failed (undeliverable, expired)
        rescheduled
      */
      case etSubmitOk:
      {
        statMan->updateAccepted(sms->getSourceSmeId(),sms->getRouteId());
        MutexGuard g(perfMutex);
        submitOkCounter++;
      }break;
      case etSubmitErr:
      {
        MutexGuard g(perfMutex);
        submitErrCounter++;
        statMan->updateRejected(sms->getSourceSmeId(),sms->getRouteId(), sms->getLastResult());
      }break;
      case etDeliveredOk:
      {
        statMan->updateChanged(sms->getDestinationSmeId(),sms->getRouteId(),0);
        MutexGuard g(perfMutex);
        deliverOkCounter++;
      }break;
      case etDeliverErr:
      {
        statMan->updateTemporal(sms->getDestinationSmeId(),sms->getRouteId(),sms->getLastResult());
        MutexGuard g(perfMutex);
        deliverErrTempCounter++;
      }break;
      case etUndeliverable:
      {
        statMan->updateChanged(sms->getDestinationSmeId(),sms->getRouteId(),sms->getLastResult());
        MutexGuard g(perfMutex);
        deliverErrPermCounter++;
      }break;
      case etRescheduled:
      {
        statMan->updateScheduled(sms->getDestinationSmeId(),sms->getRouteId());

        MutexGuard g(perfMutex);
        rescheduleCounter++;
      }break;
    }
  }

  void SaveStats()
  {
    FILE *f=fopen("stats.txt","wt");
    if(f)
    {
      fprintf(f,"%d %lld %lld %lld %lld %lld %lld",time(NULL)-startTime,
        submitOkCounter,
        submitErrCounter,
        deliverOkCounter,
        deliverErrTempCounter,
        deliverErrPermCounter,
        rescheduleCounter
      );
      fclose(f);
    }
  }
  void abortSmsc()
  {
    SaveStats();
    statMan->flushStatistics();
    MapDialogContainer::getInstance()->abort();
    kill(getpid(),9);
  }

  void dumpSmsc()
  {
    MapDialogContainer::getInstance()->abort();
    abort();
  }

  void getPerfData(uint64_t *cnt)
  {
    MutexGuard g(perfMutex);
    cnt[0]=submitOkCounter;
    cnt[1]=submitErrCounter;
    cnt[2]=deliverOkCounter;
    cnt[3]=deliverErrTempCounter;
    cnt[4]=deliverErrPermCounter;
    cnt[5]=rescheduleCounter;
  }

  RefferGuard<RouteManager> getRouterInstance()
  {
    MutexGuard g(routerSwitchMutex);
    return RefferGuard<RouteManager>(router_);
  }

  RefferGuard<RouteManager> getTestRouterInstance()
  {
    MutexGuard g(routerSwitchMutex);
    return RefferGuard<RouteManager>(testRouter_);
  }

  RefferGuard<AliasManager> getAliaserInstance()
  {
    MutexGuard g(aliasesSwitchMutex);
    return RefferGuard<AliasManager>(aliaser_);
  }

  void ResetRouteManager(RouteManager* manager)
  {
    MutexGuard g(routerSwitchMutex);
    if ( router_ ) router_->Release();
    router_ = new Reffer<RouteManager>(manager);
  }

  void ResetTestRouteManager(RouteManager* manager)
  {
    MutexGuard g(routerSwitchMutex);
    if ( testRouter_ ) testRouter_->Release();
    testRouter_ = new Reffer<RouteManager>(manager);
  }

  void ResetAliases(AliasManager* manager)
  {
    MutexGuard g(aliasesSwitchMutex);
    if ( aliaser_ ) aliaser_->Release();
    aliaser_ = new Reffer<AliasManager>(manager);
  }

  void reloadRoutes(const SmscConfigs& cfg);
  void reloadTestRoutes(const RouteConfig& rcfg);
  void reloadAliases(const SmscConfigs& cfg);

  void flushStatistics()
  {
    statMan->flushStatistics();
  }

  uint8_t getNextMR(const Address& addr)
  {
    return mrCache.getNextMR(addr);
  }

  SmeProxy* getMapProxy() {
    return mapProxy;
  }

  bool allowCommandProcessing(SmscCommand& cmd)
  {
    return tcontrol->processCommand(cmd);
  }


protected:

  void processCommand(SmscCommand& cmd);

  void generateAlert(SMSId id,SMS* sms);

  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  Mutex routerSwitchMutex;
  Mutex aliasesSwitchMutex;
  Reffer<RouteManager>* router_;
  Reffer<RouteManager>* testRouter_;
  Reffer<AliasManager>* aliaser_;
  EventQueue eventqueue;
  smsc::store::MessageStore *store;
  Scheduler *scheduler;
  smsc::profiler::Profiler *profiler;
  bool stopFlag;
  std::string smscHost;
  int smscPort;
  SmscSme *smscsme;
  CancelAgent *cancelAgent;
  AlertAgent *alertAgent;
  performance::PerformanceDataDispatcher perfDataDisp;
  smsc::db::DataSource *dataSource;

  TrafficControl *tcontrol;

  SmeProxy* abonentInfoProxy;

  smsc::stat::StatisticsManager *statMan;

  SmeProxy* mapProxy;

  MessageReferenceCache mrCache;

  DistrListManager *distlstman;
  DistrListProcess *distlstsme;

  Mutex perfMutex;
  uint64_t submitOkCounter;
  uint64_t submitErrCounter;
  uint64_t deliverOkCounter;
  uint64_t deliverErrTempCounter;
  uint64_t deliverErrPermCounter;
  uint64_t rescheduleCounter;
  string scAddr;
  string ussdCenterAddr;
  int    ussdSSN;
  time_t startTime;

  int eventQueueLimit;

  smsc::core::threads::ThreadPool tp,tp2;

  struct MergeCacheItem{
    Address  oa;//originating address
    uint16_t mr;//message reference
    bool operator==(const MergeCacheItem& item)
    {
      return oa==item.oa && mr==item.mr;
    }
  };

  struct MergeCacheHashFunc{
    static unsigned inline int CalcHash(const MergeCacheItem& item)
    {
      unsigned int res=item.mr;
      unsigned int mul=10;
      for(int i=0;i<item.oa.length;i++)
      {
        res+=(item.oa.value[i]-'0')*mul;
        mul*=10;
        if(i==9)break;
      }
      return res;
    }
  };

  smsc::core::buffers::XHash<MergeCacheItem,SMSId,MergeCacheHashFunc> mergeCache;
  smsc::core::buffers::XHash<SMSId,MergeCacheItem> reverseMergeCache;
  std::list<std::pair<time_t,SMSId> > mergeCacheTimeouts;
  time_t mergeConcatTimeout;

  friend class StatusSme;

};

};//system
};//smsc


#endif
