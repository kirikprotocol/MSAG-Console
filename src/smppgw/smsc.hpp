#ifndef __SYSTEM_SMSC_HPP__
#define __SYSTEM_SMSC_HPP__

#include "core/threads/ThreadPool.hpp"
#include "util/config/Manager.h"
#include "util/config/route/RouteConfig.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"
#include "router/route_manager.h"
#include "smppgw/event_queue.h"
#include "util/config/smeman/SmeManConfig.h"
#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include "smppgw/performance.hpp"
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "snmp/SnmpAgent.hpp"

#include "smppgw/stat/StatisticsManager.h"

#include "smppgw/mrcache.hpp"

#include <sys/types.h>

#include "core/buffers/XHash.hpp"


namespace smsc{
namespace smppgw{

using smsc::sms::SMS;
using namespace smsc::smeman;
using smsc::alias::AliasManager;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;
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
}

struct SmscConfigs{
  smsc::util::config::Manager* cfgman;
  smsc::util::config::smeman::SmeManConfig* smemanconfig;
  smsc::util::config::alias::AliasConfig* aliasconfig;
  smsc::util::config::route::RouteConfig* routesconfig;
  Hash<string> *licconfig;
};

class GatewaySme;

class Smsc
{
public:
  Smsc():ssockman(&tp,&smeman),stopFlag(false),router_(0),aliaser_(0)
  {
    acceptedCounter=0;
    rejectedCounter=0;
    deliveredCounter=0;
    deliverErrCounter=0;
    transOkCounter=0;
    transFailCounter=0;

    startTime=0;
    license.maxsms=0;
    license.expdate=0;
    memset(gwSmeMap,0,sizeof(gwSmeMap));
  };
  ~Smsc();
  void init(const SmscConfigs& cfg);
  void run();
  void stop(){stopFlag=true;}
  void mainLoop();
  void shutdown();

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
    eventqueue.enqueue(SmscCommand::makeCancel(id,oa,da));
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

  void unregisterSmeProxy(const string& sysid)
  {
    smeman.unregisterSmeProxy(sysid);
  }

  SmeAdministrator* getSmeAdmin(){return &smeman;}


  void updatePerformance(int counter)
  {
    MutexGuard g(perfMutex);
    using namespace smsc::smppgw::performance::Counters;
    switch(counter)
    {
      case cntAccepted:    acceptedCounter++;break;
      case cntRejected:    rejectedCounter++;break;
      case cntDelivered:   deliveredCounter++;break;
      case cntDeliverErr:  deliverErrCounter++;break;
      case cntTransOk:     transOkCounter++;break;
      case cntTransFail:   transFailCounter++;break;
    }
  }

  void updateCounter(int counter, const char* srcSmeId, const char* routeId,int errorCode=0)
  {
    statMan->updateCounter(counter, srcSmeId,  routeId,errorCode);
  }

  void SaveStats()
  {
    FILE *f=fopen("stats.txt","wt");
    if(f)
    {
      fprintf(f,"%d %lld %lld %lld %lld %lld %lld",time(NULL)-startTime,
        acceptedCounter,
        rejectedCounter,
        deliveredCounter,
        deliverErrCounter,
        transOkCounter,
        transFailCounter
      );
      fclose(f);
    }
  }
  void abortSmsc()
  {
    SaveStats();
    statMan->flushStatistics();
    //MapDialogContainer::getInstance()->abort();
    kill(getpid(),9);
  }

  void dumpSmsc()
  {
    //MapDialogContainer::getInstance()->abort();
    abort();
  }

  void getPerfData(uint64_t *cnt)
  {
    MutexGuard g(perfMutex);
    __trace2__("getPerfData: acceptedCounter=%lld",acceptedCounter);
    cnt[0]=acceptedCounter;
    cnt[1]=rejectedCounter;
    cnt[2]=deliveredCounter;
    cnt[3]=deliverErrCounter;
    cnt[4]=transOkCounter;
    cnt[5]=transFailCounter;
  }

  void getStatData(int& eqsize)
  {
    eventqueue.getStats(eqsize);
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

  void InitLicense(const Hash<string>& lic)
  {
    license.maxsms=atoi(lic["MaxSmsThroughput"].c_str());
    int y,m,d;
    sscanf(lic["LicenseExpirationDate"].c_str(),"%d-%d-%d",&y,&m,&d);
    struct tm t={0,};
    t.tm_year=y;
    t.tm_mon=m;
    t.tm_mday=d;
    license.expdate=mktime(&t);
    long hostid;
    sscanf(lic["Hostid"].c_str(),"%x",&hostid);
    if(hostid!=gethostid())
    {
      throw runtime_error("");
    }
  }

  GatewaySme* getGwSme(uint8_t uid)
  {
    return gwSmeMap[uid];
  }

protected:

  void processCommand(SmscCommand& cmd);

  void generateAlert(SMS* sms);

  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  Mutex routerSwitchMutex;
  Mutex aliasesSwitchMutex;
  Reffer<RouteManager>* router_;
  Reffer<RouteManager>* testRouter_;
  Reffer<AliasManager>* aliaser_;
  EventQueue eventqueue;
  bool stopFlag;
  std::string smscHost;
  int smscPort;

  smsc::smppgw::performance::PerformanceDataDispatcher perfDataDisp;
  smsc::db::DataSource *dataSource;

  SmeProxy* abonentInfoProxy;

  smsc::smppgw::stat::GWStatisticsManager *statMan;
  snmp::SnmpAgent *snmpAgent;

  struct LicenseInfo{
    int maxsms;
    time_t expdate;
  }license;

  MessageReferenceCache mrCache;

  Mutex perfMutex;

  uint64_t acceptedCounter;
  uint64_t rejectedCounter;
  uint64_t deliveredCounter;
  uint64_t deliverErrCounter;
  uint64_t transOkCounter;
  uint64_t transFailCounter;

  string scAddr;
  string ussdCenterAddr;
  int    ussdSSN;
  time_t startTime;

  GatewaySme* gwSmeMap[256];

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

  friend class StatusSme;

};

}//system
}//smsc


#endif
