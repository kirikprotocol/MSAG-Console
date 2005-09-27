#ifndef __SCAG_SYSTEM_CORE__
#define __SCAG_SYSTEM_CORE__

#include <sys/types.h>

#include "core/threads/ThreadPool.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "system/smppio/SmppSocketsManager.hpp"
#include "smeman/smeman.h"
#include "scag/transport/smpp/router/route_manager.h"
#include "scag/event_queue.h"
#include "util/config/smeman/SmeManConfig.h"
#include "scag/performance.hpp"
#include "sme/SmppBase.hpp"

#include "scag/stat/StatisticsManager.h"

#include "scag/mrcache.hpp"
#include "scag/gwsme.hpp"

#include "core/buffers/XHash.hpp"
#include "logger/Logger.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/stat/Statistics.h"
#include "scag/stat/StatisticsManager.h"
#include "scag/re/RuleEngine.h"

namespace scag
{

using smsc::sms::SMS;
using namespace smsc::smeman;
using smsc::core::threads::ThreadedTask;
using smsc::sme::SmeConfig;
using smsc::smeman::SmeManager;
using scag::GatewaySme;
using namespace smsc::logger;

using scag::config::RouteConfig;
using scag::stat::SmppStatEvent;
using scag::stat::StatisticsManager;
using scag::transport::smpp::router::RouteManager;
using scag::transport::smpp::router::RouteInfo;

// TODO: move to uti    l
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
// TODO: move to util

namespace StatEvents
{
  const int etSubmitOk     =1;
  const int etSubmitErr    =2;
  const int etDeliveredOk  =3;
  const int etDeliverErr   =4;
  const int etUndeliverable=5;
  const int etRescheduled  =6;
}

class GatewaySme;

class Scag
{
public:
  Scag():ssockman(&tp,&smeman),stopFlag(false),router_(0),testRouter_(0)
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
  ~Scag();
  void init();
  void run();
  void stop(){stopFlag=true;}
  void mainLoop();
  void shutdown();

  bool Scag::routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,smsc::router::RouteInfo* ri,SmeIndex idx=-1);

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
    // Wait for SmppManager will be ready
    /*MutexGuard mg(gatewaySwitchMutex);

    uint8_t uid = 0;

    SmeRecord* p = (SmeRecord*)getSmeProxy(sysid);
    GatewaySme* gwsme = dynamic_cast<GatewaySme*>(p->proxy);

    if(gwsme){
        smeman.unregSmsc(sysid);
        uid = gwsme->getPrefix();

        gwsme->Release();

        if(uid){
            gwSmeMap[uid]->Release();
            gwSmeMap[uid] = 0;
        }
    }*/
  }

  SmeAdministrator* getSmeAdmin(){return &smeman;}


  void updatePerformance(int counter)
  {
    MutexGuard g(perfMutex);
    using namespace scag::performance::Counters;
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

  void updateCounter(const SmppStatEvent& si)
  {
    statMan->registerEvent(si);
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
  void abortScag()
  {
    SaveStats();
    //MapDialogContainer::getInstance()->abort();
    kill(getpid(),9);
  }

  void dumpScag()
  {
    //MapDialogContainer::getInstance()->abort();
    abort();
  }

  void getPerfData(uint64_t *cnt)
  {
    MutexGuard g(perfMutex);
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

  void reloadRoutes();
  void reloadTestRoutes(const RouteConfig& rcfg);

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

  // Wait for SmppManager will be ready
  /*GatewaySme* getGwSme(uint8_t uid)
  {
    MutexGuard mg(gatewaySwitchMutex);
    return gwSmeMap[uid];
  }

  void setGwSme(uint8_t uid, GatewaySme* gwSme)
  {
    MutexGuard mg(gatewaySwitchMutex);
    gwSmeMap[uid] = gwSme;
  }*/

  void startTpTask(ThreadedTask *tsk)
  {
    tp.startTask(tsk);
  }

  // wait for SmppManager will be ready
  //bool regSmsc(SmeConfig cfg, std::string altHost, uint8_t altPort, std::string systemId, uint8_t uid);
  //bool modifySmsc(SmeConfig cfg, std::string altHost, uint8_t altPort, std::string systemId, uint8_t uid);

  int ussdTransactionTimeout;

protected:

  void processCommand(SmscCommand& cmd);

  void generateAlert(SMS* sms);

  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  Mutex routerSwitchMutex;
  Mutex gatewaySwitchMutex;
  Reffer<RouteManager>* router_;
  Reffer<RouteManager>* testRouter_;
  EventQueue eventqueue;
  bool stopFlag;
  std::string scagHost;
  int scagPort;
  scag::re::RuleEngine ruleEngine;

  scag::performance::PerformanceDataDispatcher perfDataDisp;

  SmeProxy* abonentInfoProxy;

  scag::stat::StatisticsManager *statMan;

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

}//scag


#endif // __SCAG_SYSTEM_CORE__
