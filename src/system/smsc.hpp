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
//#include "store/MessageStore.h"
#include "util/config/smeman/SmeManConfig.h"
#include "alias/AliasMan.hpp"
//#include "util/config/alias/aliasconf.h"
#include "system/scheduler.hpp"
#include "profiler/profiler.hpp"
#include "system/smscsme.hpp"
#include "system/cancel_agent.hpp"
#include "system/alert_agent.hpp"
#include "system/performance.hpp"
#ifdef USE_MAP
#include "system/mapio/MapProxy.h"
#endif
//#include "db/DataSource.h"
//#include "db/DataSourceLoader.h"
#include "snmp/SnmpAgent.hpp"
#ifdef SNMP
#include "system/snmp/SnmpCounter.hpp"
#include "stat/SmeStats.hpp"
#include "snmp/smestattable/SmeStatTableSubAgent.hpp"
#endif
#include "acls/interfaces.h"

#include "distrlist/DistrListManager.h"
#include "distrlist/DistrListProcess.h"
#include "stat/StatisticsManager.h"

#include "system/mrcache.hpp"

//#include "system/traffic_control.hpp"

#include <sys/types.h>

#include "core/buffers/XHash.hpp"
#include "cluster/AgentListener.h"
#include "util/crc32.h"
#include "INManComm.hpp"

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
using smsc::acls::AclAbstractMgr;
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
  //smsc::util::config::alias::AliasConfig* aliasconfig;
  smsc::util::config::route::RouteConfig* routesconfig;
};

class Smsc
{
public:
  Smsc():ssockman(&tp,&smeman),stopFlag(false),router_(0),aliaser(0),testRouter_(0),mergeCacheTimeouts(4096), ishs(false)
  {
    submitOkCounter=0;
    submitErrCounter=0;
    deliverOkCounter=0;
    deliverErrTempCounter=0;
    deliverErrPermCounter=0;
    rescheduleCounter=0;

    msu_submitOkCounter=0;
    msu_submitErrCounter=0;
    msu_deliverOkCounter=0;
    msu_deliverErrTempCounter=0;
    msu_deliverErrPermCounter=0;
    msu_rescheduleCounter=0;

    startTime=time(NULL);
    //tcontrol=0;
    totalCounter=0;
    statCounter=0;
    schedCounter=0;
    maxTotalCounter=0;
    maxStatCounter=0;
    mapProxy=0;
    p2pChargePolicy=chargeOnSubmit;
    otherChargePolicy=chargeOnDelivery;
    smartMultipartForward=false;
    nodeIndex=0;
    mainLoopsCount=1;
    mapIOTasksCount=1;
    mapioptr=0;
    speedLogFlushPeriod=60;
    nextSpeedLogFlush=0;
    if(instance!=0)
    {
      throw smsc::util::Exception("Attempt to init second smsc instance:%p (previous:%p)",this,instance);
    }
    instance=this;
  };
  ~Smsc();

  static Smsc& getInstance()
  {
    return *instance;
  }
  void init(const SmscConfigs& cfg, const char * node);
  void run();
  void stop(){stopFlag=true;}
  bool getStopFlag()
  {
    return stopFlag;
  }
  const SmscConfigs* getConfigs()
  {
    return configs;
  }
  void mainLoop(int);
  void shutdown();
  TaskContainer tasks;
  bool routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,smsc::router::RouteInfo* ri,SmeIndex idx=-1);

  bool AliasToAddress(const Address& alias,Address& addr)
  {
    return aliaser->AliasToAddress(alias,addr);
  }
  bool AddressToAlias(const Address& addr,Address& alias)
  {
    return aliaser->AddressToAlias(addr,alias);
  }

  void cancelSms(SMSId id,const Address& oa,const Address& da)
  {
    eventqueue.enqueue(id,SmscCommand::makeCancel(id,oa,da));
  }

  Scheduler* getScheduler()
  {
    return scheduler;
  }

  smsc::profiler::Profiler* getProfiler()
  {
    return profiler;
  }

  smsc::smeman::SmeInfo getSmeInfo(smsc::smeman::SmeIndex idx)
  {
    return smeman.getSmeInfo(idx);
  }

  smsc::smeman::SmeIndex getSmeIndex(const SmeSystemId& systemId)
  {
    return smeman.lookup(systemId);
  }

  smsc::smeman::SmeInfo getSmeInfo(const SmeSystemId& systemId)
  {
    return smeman.getSmeInfo(smeman.lookup(systemId));
  }

  SmeProxy* getSmeProxy(const SmeSystemId& systemId)
  {
    smsc::smeman::SmeIndex idx=smeman.lookup(systemId);
    if(idx==-1)return 0;
    return smeman.getSmeProxy(idx);
  }

  void submitMrKill(const Address& org,const Address& dst,uint16_t mr)
  {
    //smscsme->putIncomingCommand(SmscCommand::makeKillMrCacheItemCmd(org,dst,mr));
    MergeCacheItem mci;
    mci.mr=mr;
    mci.oa=org;
    mci.da=dst;

    MutexGuard mg(mergeCacheMtx);
    SMSId* pid=mergeCache.GetPtr(mci);
    if(pid)
    {
      //info2(log,"msgId=%lld: kill mr cache item for %s,%s,%d",*pid,ki.org.toString().c_str(),ki.dst.toString().c_str(),(int)ki.mr);
      SMSId killId=*pid;
      reverseMergeCache.Delete(killId);
      mergeCache.Delete(mci);
    }
  }

  void unregisterSmeProxy(SmeProxy* smeProxy)
  {
    smeman.unregisterSmeProxy(smeProxy);
  }

  SmeAdministrator* getSmeAdmin(){return &smeman;}

  void RejectSms(const SmscCommand&,bool isLicenseLimit);

  void incRejected()
  {
    MutexGuard g(perfMutex);
    msu_submitErrCounter++;
    submitErrCounter++;
  }

  void registerMsuStatEvent(int eventType,const SMS* sms)
  {
    using namespace smsc::stat;
    using namespace StatEvents;
    switch(eventType)
    {
      case etSubmitOk:
      {
        MutexGuard g(perfMutex);
        msu_submitOkCounter++;
      }break;
      case etSubmitErr:
      {
        MutexGuard g(perfMutex);
        msu_submitErrCounter++;
      }break;
      case etDeliveredOk:
      {
        MutexGuard g(perfMutex);
        msu_deliverOkCounter++;
      }break;
      case etDeliverErr:
      {
        MutexGuard g(perfMutex);
        msu_deliverErrTempCounter++;
      }break;
      case etUndeliverable:
      {
        MutexGuard g(perfMutex);
        msu_deliverErrPermCounter++;
      }break;
      case etRescheduled:
      {
        MutexGuard g(perfMutex);
        msu_rescheduleCounter++;
      }break;
    }
  }

  void registerStatisticalEvent(int eventType,const SMS* sms);

  void SaveStats()
  {
    /*
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
    */
  }
  void abortSmsc();

  void dumpSmsc();

  void getPerfData(uint64_t *cnt)
  {
    MutexGuard g(perfMutex);
    cnt[0]=submitOkCounter;
    cnt[1]=submitErrCounter;
    cnt[2]=deliverOkCounter;
    cnt[3]=deliverErrTempCounter;
    cnt[4]=deliverErrPermCounter;
    cnt[5]=rescheduleCounter;
    cnt[6]=msu_submitOkCounter;
    cnt[7]=msu_submitErrCounter;
    cnt[8]=msu_deliverOkCounter;
    cnt[9]=msu_deliverErrTempCounter;
    cnt[10]=msu_deliverErrPermCounter;
    cnt[11]=msu_rescheduleCounter;
  }

  uint8_t* getSmePerfData(uint32_t& smePerfDataSize)
  {
    MutexGuard g(perfMutex);
    return smePerfMonitor.dump(smePerfDataSize);
  }

  void getStatData(int& eqsize,int& eqlocked,uint32_t& schedsize)
  {
    int hsize,qsize;
    eventqueue.getStats(hsize,qsize);
    eqsize=qsize;
    eqlocked=hsize-qsize;
    Scheduler::SchedulerCounts scnts;
    scheduler->getCounts(scnts);
    schedsize=scnts.timeLineCount+scnts.firstTimeCount;
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


  AliasManager* getAliaserInstance()
  {
    return aliaser;
  }


  void ResetRouteManager(RouteManager* manager)
  {
    Reffer<RouteManager>* refToKill=router_;
    {
      MutexGuard g(routerSwitchMutex);
      router_ = new Reffer<RouteManager>(manager);
    }
    if ( refToKill ) refToKill->Release();
  }

  void ResetTestRouteManager(RouteManager* manager)
  {
    Reffer<RouteManager>* refToKill=testRouter_;
    {
      MutexGuard g(routerSwitchMutex);
      testRouter_ = new Reffer<RouteManager>(manager);
    }
    if ( refToKill ) refToKill->Release();

  }

  /*
  void ResetAliases(AliasManager* manager)
  {
    MutexGuard g(aliasesSwitchMutex);
    if ( aliaser_ ) aliaser_->Release();
    aliaser_ = new Reffer<AliasManager>(manager);
  }
  */

  void reloadRoutes();
  void reloadTestRoutes(const RouteConfig& rcfg);
  //void reloadAliases(const SmscConfigs& cfg);
  void reloadReschedule();

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

  /*
  bool allowCommandProcessing(SmscCommand& cmd)
  {
    return tcontrol->processCommand(cmd);
  }
  */

  static void InitLicense();


  AclAbstractMgr   *getAclMgr()
  {
    return aclmgr;
  }

  smsc::distrlist::DistrListAdmin* getDlAdmin()
  {
    return distlstman;
  }

  enum{smsWeight=10000};

  typedef smsc::util::TimeSlotCounter<> IntTimeSlotCounter;

  int getTotalCounterRes()
  {
    return totalCounter->getSlotRes();
  }
  int getTotalCounter()
  {
    MutexGuard mg(countersMtx);
    return totalCounter->Get();
  }
  int getSchedCounter()
  {
    MutexGuard mg(countersMtx);
    return schedCounter->Get();
  }
  int getStatCounter()
  {
    MutexGuard mg(countersMtx);
    return statCounter->Get();
  }

  void incTotalCounter(int perslot,bool isForward,int fperslot)
  {
    MutexGuard mg(countersMtx);
    totalCounter->IncDistr(smsWeight,perslot);
    int tcnt=totalCounter->Get();
    int cntVal=(tcnt+smsWeight*shapeTimeFrame/2)/(smsWeight*shapeTimeFrame);
    if(cntVal>maxTotalCounter)maxTotalCounter=cntVal;
    if(isForward)
    {
      schedCounter->IncDistr(smsWeight,fperslot);
    }
  }

  void incStatCounter()
  {
    MutexGuard mg(countersMtx);
    statCounter->Inc(1);
    int cntVal=statCounter->Get()/statTimeFrame;
    if(cntVal>maxStatCounter)maxStatCounter=cntVal;
  }

  enum{
    chargeOnSubmit,chargeOnDelivery,chargeAlways
  };

  int p2pChargePolicy;
  int otherChargePolicy;

  void ChargeSms(SMSId id,const SMS& sms,INSmsChargeResponse::SubmitContext& ctx)
  {
    inManCom->ChargeSms(id,sms,ctx);
  }

  void ChargeSms(SMSId id,const SMS& sms,INFwdSmsChargeResponse::ForwardContext& ctx)
  {
    inManCom->ChargeSms(id,sms,ctx);
  }

  bool ReportDelivery(int dlgId,const SMS& sms,bool final,int policy)
  {
    try{
      if(sms.billingRecord &&
          (
            sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==policy ||
            policy==chargeAlways
          )
        )
      {
        inManCom->Report(dlgId,sms,final);
      }
    }catch(std::exception& e)
    {
      smsc_log_warn(smsc::logger::Logger::getInstance("inmancom"),"ReportDelivery failed:%s",e.what());
      return false;
    }
    return true;
  }

  void FullReportDelivery(SMSId id,const SMS& sms)
  {
    try{
      inManCom->FullReport(id,sms);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsc::logger::Logger::getInstance("inmancom"),"FullReportDelivery failed:%s",e.what());
    }
  }

  bool isHs(){ return ishs;}

  bool getSmartMultipartForward()
  {
    return smartMultipartForward;
  }

  int nodeIndex;
  int nodesCount;

  bool checkSchedulerSoftLimit()
  {
    return scheduler->getStoreSize()>schedulerSoftLimit;
  }

  bool checkSchedulerHardLimit()
  {
    return scheduler->getStoreSize()>schedulerHardLimit;
  }

  bool isNoDivert(const std::string& smeId)
  {
    return nodivertSmes.find(smeId)!=nodivertSmes.end();
  }

protected:

  void processCommand(SmscCommand& cmd,EventQueue::EnqueueVector& ev,FindTaskVector& ftv);

  void generateAlert(SMSId id,SMS* sms,int);

  smsc::system::smppio::SmppSocketsManager ssockman;
  smsc::smeman::SmeManager smeman;
  Mutex routerSwitchMutex;
  //Mutex aliasesSwitchMutex;
  Reffer<RouteManager>* router_;
  Reffer<RouteManager>* testRouter_;

  AliasManager* aliaser;

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
  performance::PerformanceDataDispatcher perfSmeDataDisp;
  //smsc::db::DataSource *dataSource;

  //TrafficControl *tcontrol;

  IntTimeSlotCounter* totalCounter;
  IntTimeSlotCounter* statCounter;
  IntTimeSlotCounter* schedCounter;
  Mutex countersMtx;

  int shapeTimeFrame;
  int statTimeFrame;
  int maxSmsPerSecond;
  int maxTotalCounter;
  int maxStatCounter;
  int lastUpdateHour;


  SmeProxy* abonentInfoProxy;

  smsc::stat::StatisticsManager *statMan;
  snmp::SnmpAgent *snmpAgent;

#ifdef SNMP
  smsc::stat::SmeStats smeStats;
#endif

  SmeProxy* mapProxy;

  struct LicenseInfo{
    int maxsms;
    time_t expdate;
  };
  static LicenseInfo license;
  static std::string licenseFile;
  static std::string licenseSigFile;
  static time_t licenseFileModTime;

  MessageReferenceCache mrCache;


  AclAbstractMgr   *aclmgr;
  DistrListManager *distlstman;
  DistrListProcess *distlstsme;

  Mutex perfMutex;
  uint64_t submitOkCounter;
  uint64_t submitErrCounter;
  uint64_t deliverOkCounter;
  uint64_t deliverErrTempCounter;
  uint64_t deliverErrPermCounter;
  uint64_t rescheduleCounter;

  uint64_t msu_submitOkCounter;
  uint64_t msu_submitErrCounter;
  uint64_t msu_deliverOkCounter;
  uint64_t msu_deliverErrTempCounter;
  uint64_t msu_deliverErrPermCounter;
  uint64_t msu_rescheduleCounter;

  string scAddr;
  string ussdCenterAddr;
  int    ussdSSN;
  string addUssdSSN;
  int    busyMTDelay;
  int    lockedByMODelay;
  int    MOLockTimeout;
  bool   allowCallBarred;
  bool   ussdV1Enabled;
  bool   ussdV1UseOrigEntityNumber;
  time_t startTime;
  std::string localInst;
  std::string remoteInst;
  std::string CPMgmtAddress;

  int schedulerSoftLimit;
  int schedulerHardLimit;

  performance::SmePerformanceMonitor smePerfMonitor;

  int eventQueueLimit;

  smsc::core::threads::ThreadPool tp,tp2;

  struct MergeCacheItem{
    Address  oa;//originating address
    Address  da;//originating address
    uint16_t mr;//message reference
    bool operator==(const MergeCacheItem& item)
    {
      return mr==item.mr && oa==item.oa && da==item.da;
    }
  };

  struct MergeCacheHashFunc;
  friend struct smsc::system::Smsc::MergeCacheHashFunc;
  struct MergeCacheHashFunc{
    static unsigned inline int CalcHash(const MergeCacheItem& item)
    {
      unsigned int res=item.mr;
      unsigned int mul=10;
      for(int i=0;i<item.oa.length;i++)
      {
        res+=(item.oa.value[i]-'0')*mul;
        mul*=10;
      }
      for(int i=0;i<item.da.length;i++)
      {
        res+=(item.da.value[i]-'0')*mul;
        mul*=10;
      }
      return res;
    }
  };

  struct SMSIdHashFunc{
    static inline unsigned int CalcHash(SMSId key)
    {
      return (unsigned int)key;
    }
  };

  smsc::core::buffers::XHash<MergeCacheItem,SMSId,MergeCacheHashFunc> mergeCache;
  smsc::core::buffers::XHash<SMSId,MergeCacheItem,SMSIdHashFunc> reverseMergeCache;
  smsc::core::buffers::CyclicQueue<std::pair<time_t,SMSId> > mergeCacheTimeouts;
  time_t mergeConcatTimeout;
  Mutex mergeCacheMtx;


  smsc::cluster::AgentListener agentListener;
  bool ishs;

  friend class StatusSme;

  EventMonitor idleMon;

  INManComm* inManCom;

  std::set<std::string> nodivertSmes;

  bool smartMultipartForward;
  int speedLogFlushPeriod;
  time_t nextSpeedLogFlush;

  int mainLoopsCount;
  int mapIOTasksCount;
  void* mapioptr;

  const SmscConfigs* configs;

  static Smsc* instance;
};

}//system
}//smsc


#endif
