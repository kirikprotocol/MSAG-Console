#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
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

#include "db/DataSource.h"
#include "db/DataSourceLoader.h"

#include "stat/StatisticsManager.h"


namespace smsc{
namespace system{

using smsc::sms::SMS;
using smsc::smeman::SmeProxy;
using smsc::alias::AliasManager;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;

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
    deliverErrCounter=0;
    rescheduleCounter=0;
  };
  ~Smsc();
  void init(const SmscConfigs& cfg);
  void run();
  void stop(){stopFlag=true;}
  void mainLoop();
  void shutdown();
  TaskContainer tasks;
  bool Smsc::routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,smsc::router::RouteInfo* ri);

  bool AliasToAddress(const Address& alias,Address& addr)
  {
    return getAliaserInstance()->AliasToAddress(alias,addr);
  }
  bool AddressToAlias(const Address& addr,Address& alias)
  {
    return getAliaserInstance()->AddressToAlias(addr,alias);
  }

  void notifyScheduler()
  {
    scheduler->notify();
  }

  smsc::profiler::Profiler* getProfiler()
  {
    return profiler;
  }

  smsc::smeman::SmeInfo getSmeInfo(smsc::smeman::SmeIndex idx)
  {
    return smeman.getSmeInfo(idx);
  }

  void submitSms(SMS* sms)
  {
    smscsme->putSms(sms);
  }

  void unregisterSmeProxy(const string& sysid)
  {
    smeman.unregisterSmeProxy(sysid);
  }

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
        statMan->updateAccepted(sms->getSourceSmeId());
        MutexGuard g(perfMutex);
        submitOkCounter++;
      }break;
      case etSubmitErr:
      {
        MutexGuard g(perfMutex);
        submitErrCounter++;
      }break;
      case etDeliveredOk:
      {
        statMan->updateChanged(sms->getDestinationSmeId(),sms->getRouteId(),0);
        MutexGuard g(perfMutex);
        deliverOkCounter++;
      }break;
      case etDeliverErr:
      {
        MutexGuard g(perfMutex);
        deliverErrCounter++;
      }break;
      case etUndeliverable:
      {
        statMan->updateChanged(sms->getDestinationSmeId(),sms->getRouteId(),sms->getLastResult());
        MutexGuard g(perfMutex);
        deliverErrCounter++;
      }break;
      case etRescheduled:
      {
        statMan->updateScheduled();

        MutexGuard g(perfMutex);
        rescheduleCounter++;
      }break;
    }
  }

  void getPerfData(uint64_t *cnt)
  {
    MutexGuard g(perfMutex);
    cnt[0]=submitOkCounter;
    cnt[1]=submitErrCounter;
    cnt[2]=deliverOkCounter;
    cnt[3]=deliverErrCounter;
    cnt[4]=rescheduleCounter;
  }

  RefferGuard<RouteManager> getRouterInstance()
  {
    MutexGuard g(routerSwitchMutex);
    return RefferGuard<RouteManager>(router_);
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

  void ResetAliases(AliasManager* manager)
  {
    MutexGuard g(aliasesSwitchMutex);
    if ( aliaser_ ) aliaser_->Release();
    aliaser_ = new Reffer<AliasManager>(manager);
  }

  void reloadRoutes(const SmscConfigs& cfg);
  void reloadAliases(const SmscConfigs& cfg);

  void flushStatistics()
  {
    statMan->flushStatistics();
  }

protected:
  smsc::core::threads::ThreadPool tp;
  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  Mutex routerSwitchMutex;
  Mutex aliasesSwitchMutex;
  Reffer<RouteManager>* router_;
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

  SmeProxy* abonentInfoProxy;

  smsc::stat::StatisticsManager *statMan;

  SmeProxy* mapProxy;

  Mutex perfMutex;
  uint64_t submitOkCounter;
  uint64_t submitErrCounter;
  uint64_t deliverOkCounter;
  uint64_t deliverErrCounter;
  uint64_t rescheduleCounter;

};

};//system
};//smsc


#endif
