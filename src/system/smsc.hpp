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


namespace smsc{
namespace system{

using smsc::sms::SMS;
using smsc::smeman::SmeProxy;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;

//class smsc::store::MessageStore;

namespace StatEvents{
  const int etSubmitOk     =1;
  const int etSubmitErr    =2;
  const int etDeliveredOk  =3;
  const int etUndeliverable=4;
  const int etRescheduled  =5;
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
  Smsc():ssockman(&tp,&smeman),stopFlag(false)
  {
    successCounter=0;
    errorCounter=0;
    rescheduleCounter=0;
    submitCounter=0;
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
    return aliaser.AliasToAddress(alias,addr);
  }
  bool AddressToAlias(const Address& addr,Address& alias)
  {
    return aliaser.AddressToAlias(addr,alias);
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
        MutexGuard g(perfMutex);
        submitCounter++;
      }break;
      case etSubmitErr:
      {
        MutexGuard g(perfMutex);
        errorCounter++;
      }break;
      case etDeliveredOk:
      {
        MutexGuard g(perfMutex);
        successCounter++;
      }break;
      case etUndeliverable:
      {
        MutexGuard g(perfMutex);
        errorCounter++;
      }break;
      case etRescheduled:
      {
        MutexGuard g(perfMutex);
        rescheduleCounter++;
      }break;
    }
  }

  void getPerfData(uint64_t& submit,uint64_t& succ,uint64_t& err,uint64_t& resch)
  {
    MutexGuard g(perfMutex);
    succ=successCounter;
    err=errorCounter;
    resch=rescheduleCounter;
    submit=submitCounter;
  }

protected:
  smsc::core::threads::ThreadPool tp;
  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  smsc::router::RouteManager router;
  EventQueue eventqueue;
  smsc::store::MessageStore *store;
  smsc::alias::AliasManager aliaser;
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


  Mutex perfMutex;
  uint64_t successCounter;
  uint64_t errorCounter;
  uint64_t rescheduleCounter;
  uint64_t submitCounter;

};

};//system
};//smsc


#endif
