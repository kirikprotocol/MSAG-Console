#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <typeinfo>
#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "util/debug.h"
//#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "system/common/rescheduler.hpp"
#include "util/config/route/RouteConfig.h"
#include "system/abonentinfo/AbonentInfo.hpp"
#include "logger/Logger.h"
#include "system/smscsme.hpp"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"
#ifdef USE_MAP
#include "system/mapio/MapIoTask.h"
#endif
#include "util/smstext.h"
#include "system/abonentinfo/AbonentInfo.hpp"
#include "mscman/MscManager.h"
#include "resourcemanager/ResourceManager.hpp"
#include "system/status_sme.hpp"
#include "util/findConfigFile.h"

#include "profiler/profile-notifier.hpp"
#include "cluster/InterconnectManager.h"
#include "cluster/listeners/ApplyCommandListener.h"
#include "cluster/listeners/MscCommandListener.h"
#include "cluster/listeners/DlCommandListener.h"
#include "cluster/listeners/MemCommandListener.h"
#include "cluster/listeners/PrcCommandListener.h"
#include "cluster/listeners/ProfileCommandListener.h"
#include "cluster/listeners/SbmCommandListener.h"
#include "cluster/listeners/SmeCommandListener.h"
#include "cluster/listeners/CgmCommandListener.h"
#include "closedgroups/ClosedGroupsManager.hpp"
#include "system/common/TimeZoneMan.hpp"
#include "alias/AliasManImpl.hpp"
#include "util/TimeSource.h"
#include "util/AutoArrPtr.hpp"

#ifdef USE_MAP
#include "mapio/FraudControl.hpp"
#include "mapio/MapLimits.hpp"
#include "mapio/NetworkProfiles.hpp"
#endif

#include "license/check/license.hpp"

#ifdef SMSEXTRA
#include "Extra.hpp"
#include "ExtraBits.hpp"
#endif

#include <unistd.h>

//#define ENABLE_MAP_SYM

#ifdef ENABLE_MAP_SYM
#include "system/mapsym.hpp"
#endif

typedef struct stat stat_type;

namespace smsc{
namespace system{

using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::snmp;
using namespace smsc::core::synchronization;
using util::Exception;
using smsc::acls::AclAbstractMgr;
#ifdef USE_MAP
using namespace smsc::system::mapio;
#endif

using namespace smsc::util::config;

using smsc::cluster::InterconnectManager;
using smsc::cluster::Interconnect;
using smsc::cluster::ApplyCommandListener;
using smsc::cluster::MscCommandListener;
using smsc::cluster::DlCommandListener;
using smsc::cluster::MemCommandListener;
using smsc::cluster::PrcCommandListener;
using smsc::cluster::ProfileCommandListener;
using smsc::cluster::SbmCommandListener;
using smsc::cluster::SmeCommandListener;
using smsc::cluster::CgmCommandListener;

using smsc::cluster::FakeInterconnect;

Smsc* Smsc::instance=0;

Smsc::~Smsc()
{
  SaveStats();
  delete totalCounter;
  delete statCounter;
  delete schedCounter;
  instance=0;
}

class SpeedMonitor:public smsc::core::threads::ThreadedTask{
public:
  SpeedMonitor(EventQueue& eq,
               performance::PerformanceListener* pl, performance::PerformanceListener* plsme,
               Smsc* psmsc):
    queue(eq),perfListener(pl), perfSmeListener(plsme)
  {
    start.tv_sec=0;
    start.tv_nsec=0;
    smsc=psmsc;
  }
  int Execute()
  {
    smsc::logger::Logger* log=smsc::logger::Logger::getInstance("speedmon");
    uint64_t cnt,last=0;
    timespec now={0,0},lasttime={0,0};
    double ut,tm,rate,avg;
    if(start.tv_sec==0)
      clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    __trace__("enter SpeedMonitor");

    time_t perfStart=time(NULL);
    int lastIdx=0;

    memset(perfCnt,0,sizeof(perfCnt));
    uint64_t lastPerfCnt[performance::performanceCounters]={0,};
    //now.tv_sec=0;
    int i;
    for(;;)
    {
      try{
        //sleep(1);
        //timeval tv;
        //tv.tv_sec=1;
        //tv.tv_usec=0;
        //select(0,0,0,0,&tv);
        while(now.tv_sec==time(NULL))ev.Wait(10);
        cnt=queue.getCounter();
        int eqhash,equnl;
        queue.getStats(eqhash,equnl);
        clock_gettime(CLOCK_REALTIME,&now);
        ut=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
           (start.tv_sec*1000.0+start.tv_nsec/1000000.0))/1000.0;
        tm=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
           (lasttime.tv_sec*1000.0+lasttime.tv_nsec/1000000.0))/1000;
        rate=(cnt-last)/tm;
        avg=cnt/ut;
        printf("UT:%.3lf AVG:%.3lf LAST:%.3lf (%llu)[%d,%d]         \r",ut,avg,rate,cnt,eqhash,equnl);
        fflush(stdout);
        last=cnt;
        lasttime=now;
        if(isStopping)break;
        uint64_t perf[performance::performanceCounters];
        // success, error, reschedule
        smsc->getPerfData(perf);
        performance::PerformanceData d;
        d.countersNumber=performance::performanceCounters;
        for(i=0;i<performance::performanceCounters;i++)
        {
          d.counters[i].lastSecond=(int)(perf[i]-lastPerfCnt[i]);
          d.counters[i].total=perf[i];
        }

        int secs=(int)(now.tv_sec-perfStart);
        int mcnt=secs/60;
        int curIdx=mcnt%60;
        if(lastIdx!=curIdx)
        {
          lastIdx=curIdx;
          for(i=0;i<performance::performanceCounters;i++)perfCnt[i][curIdx]=0;
        }
        if(mcnt>=60)
        {
          mcnt=59;
          secs=60*60-(60-secs%60);
        }
        for(int j=0;j<performance::performanceCounters;j++)
        {
          d.counters[j].average=0;
        }
        int idx=curIdx;

        for(int j=0;j<performance::performanceCounters;j++)
        {
          perfCnt[j][curIdx]+=d.counters[j].lastSecond;
        }
        for(i=0;i<=mcnt;i++,idx--)
        {
          if(idx<0)idx=59;
          for(int j=0;j<performance::performanceCounters;j++)
          {
            d.counters[j].average+=perfCnt[j][idx];
          }
        }

        if(secs)
        {
          for(i=0;i<performance::performanceCounters;i++)
          {
            d.counters[i].average/=secs;
          }
        }

        d.now=now.tv_sec;
        d.uptime=now.tv_sec-start.tv_sec;

        d.eventQueueSize=equnl;
        d.inProcessingCount=eqhash-equnl;

        Scheduler::SchedulerCounts cnts;
        smsc->getScheduler()->getCounts(cnts);
        d.inScheduler=cnts.timeLineCount+cnts.firstTimeCount;
        d.dpfCount=smsc->getScheduler()->getDpfCount();
        perfListener->reportGenPerformance(&d);

        for(i=0;i<performance::performanceCounters;i++)
        {
          lastPerfCnt[i]=perf[i];
        }

        uint32_t smePerfDataSize = 0;
        smsc::util::auto_arr_ptr<uint8_t> smePerfData(smsc->getSmePerfData(smePerfDataSize));
        perfSmeListener->reportSmePerformance(smePerfData.get(), smePerfDataSize);
        info2(log,"ut=%.3lf;avg=%.3lf;last=%.3lf;cnt=%llu;eq=%d;equnl=%d;sched=%d;dpf=%d;sbm=%d;rej=%d;dlv=%d;fwd=%d;tmp=%d;prm=%d",ut,avg,rate,cnt,eqhash,equnl,d.inScheduler,d.dpfCount,
              d.counters[6].lastSecond,d.counters[7].lastSecond,d.counters[8].lastSecond,d.counters[9].lastSecond,d.counters[10].lastSecond,d.counters[11].lastSecond);
      }catch(std::exception& e)
      {
        warn2(log,"Exception in speed monitor:%s",e.what());
      }
    }
    return 0;
  }
  void setStartTime(time_t t)
  {
    start.tv_sec=t;
    start.tv_nsec=0;
  }
  const char* taskName()
  {
    return "SpeedMonitor";
  }
protected:
  EventQueue& queue;
  int perfCnt[performance::performanceCounters][60];
  timespec start;
  performance::PerformanceListener* perfListener;
  performance::PerformanceListener* perfSmeListener;
  static Smsc* smsc;
};

Smsc* SpeedMonitor::smsc=NULL;

extern void loadRoutes(RouteManager* rm,const smsc::util::config::route::RouteConfig& rc,bool traceit=false);

void Smsc::init(const SmscConfigs& cfg, const char * node)
{
  configs=&cfg;


  smsc::util::regexp::RegExp::InitLocale();
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smsc.init");
  try{
  tp.preCreateThreads(15);
  //smsc::util::config::Manager::init("config.xml");
  //cfgman=&cfgman->getInstance();

  Interconnect *icon = 0;
  try {
      smsc_log_info(log, "InterconnectManager initialization ..." );

      std::auto_ptr<ConfigView> imConfig(new ConfigView(*cfg.cfgman, "cluster"));

      char * mode = imConfig.get()->getString("mode");
      nodesCount = 1;
      if(strcmp(mode, "hs") == 0 || strcmp(mode, "ha") == 0){

          ishs = true;

          const char* nodes[] = { imConfig.get()->getString("host1"), imConfig.get()->getString("host2")};
          smsc_log_info(log, "host1: %s, host2: %s", nodes[0], nodes[1] );
          int port[] = { imConfig.get()->getInt("port1"), imConfig.get()->getInt("port2") };
          smsc_log_info(log, "port1: %d, port2: %d", port[0], port[1] );

          const char * p = strchr(node, '=');

          if( !p )
              throw Exception("Command line parameter is failed");

          int num1 = 0;
          try {
              smsc_log_info(log, "node: %s, num: %s", node, p+ 1 );
              num1 = atoi(p + 1);
          }catch(...){
              throw Exception("Exception atoi");
          }

          nodeIndex=num1;

          --num1;
          if(num1 < 0 || num1 > 1)
              num1 = 0;

          int num2 = num1 == 0 ? 1 : 0;

          smsc_log_info(log, "num1: %d, num2: %d", num1, num2 );

          InterconnectManager::init(nodes[num1], nodes[num2], port[num1], port[num2]);
          icon = InterconnectManager::getInstance();
          smsc_log_info(log, "InterconnectManager is initialized" );

          if(nodes[0])
              delete [] nodes[0];

          if(nodes[1])
              delete [] nodes[1];

      }else if(strcmp(mode, "single") == 0)
      {
          FakeInterconnect::init();
      }else if(strcmp(mode, "scalable") == 0) {
        try{
          nodesCount=imConfig.get()->getInt("nodesCount");
        }catch(...)
        {
          nodesCount = 1;
          __warning2__("nodesCount set to default %d",nodesCount);
        }

        const char * p = strchr(node, '=');
        if( !p )
          throw Exception("Command line parameter is failed");
        int num1 = 0;
        try {
          smsc_log_info(log, "node: %s, num: %s", node, p+ 1 );
          num1 = atoi(p + 1);
        }catch(...){
          throw Exception("Exception atoi");
        }

        nodeIndex=num1;
        FakeInterconnect::init();
      }

      if(mode)
              delete [] mode;

  }catch(std::exception& e){
      throw Exception("InterconnectManager initialization exception:%s",e.what());
  }



#ifdef SNMP
  {
    snmpAgent = new SnmpAgent(this);
    smsc::snmp::smestattable::SmeStatTableSubagent::Init(&smeman,&smeStats);
    tp2.startTask(snmpAgent);
    snmpAgent->statusChange(SnmpAgent::INIT);
    SnmpCounter::Init(smsc::util::findConfigFile("snmp.xml"));
  }
#endif

  {
    std::string tzCfg=smsc::util::findConfigFile("timezones.xml");
    std::string rtCfg=smsc::util::findConfigFile("routes.xml");
    common::TimeZoneManager::Init(tzCfg.c_str(),rtCfg.c_str());
  }

  /*
    register SME's
  */
  //smsc::util::config::smeman::SmeManConfig smemancfg;
  //smemancfg.load("sme.xml");
  {
    smsc_log_info(log, "Registering SMEs" );
    smsc::util::config::smeman::SmeManConfig::RecordIterator i=cfg.smemanconfig->getRecordIterator();
    using namespace smsc::util::regexp;
    RegExp re;
    while(i.hasRecord())
    {
      smsc::util::config::smeman::SmeRecord *rec;
      i.fetchNext(rec);
      SmeInfo si;
      /*
      uint8_t typeOfNumber;
      uint8_t numberingPlan;
      uint8_t interfaceVersion;
      std::string rangeOfAddress;
      std::string systemType;
      std::string password;
      std::string hostname;
      int port;
      SmeSystemId systemId;
      SmeNType SME_N;
      bool  disabled;
      */
      if(rec->rectype==smsc::util::config::smeman::SMPP_SME)
      {
        si.priority=rec->priority;

        si.typeOfNumber=rec->recdata.smppSme.typeOfNumber;
        si.numberingPlan=rec->recdata.smppSme.numberingPlan;
        si.interfaceVersion=rec->recdata.smppSme.interfaceVersion;
        si.rangeOfAddress=rec->recdata.smppSme.addrRange;
        si.systemType=rec->recdata.smppSme.systemType;
        si.password=(const char*)rec->recdata.smppSme.password;
        si.systemId=(const char*)rec->smeUid;
        si.SME_N=rec->recdata.smppSme.smeN;
        si.timeout = rec->recdata.smppSme.timeout;
        si.wantAlias = rec->recdata.smppSme.wantAlias;
        //si.forceDC = rec->recdata.smppSme.forceDC;
        si.proclimit=rec->recdata.smppSme.proclimit;
        si.schedlimit=rec->recdata.smppSme.schedlimit;
        si.receiptSchemeName= rec->recdata.smppSme.receiptSchemeName;
        if(si.rangeOfAddress.length() && !re.Compile(si.rangeOfAddress.c_str(),OP_OPTIMIZE|OP_STRICT))
        {
          smsc_log_error(log, "Failed to compile rangeOfAddress for sme %s",si.systemId.c_str());
        }
        __trace2__("INIT: addSme %s(to=%d,wa=%s)",si.systemId.c_str(),si.timeout,si.wantAlias?"true":"false");
        //si.hostname=rec->recdata->smppSme.
        si.disabled=rec->recdata.smppSme.disabled;
        using namespace smsc::util::config::smeman;
        switch(rec->recdata.smppSme.mode)
        {
          case MODE_TX:si.bindMode=smeTX;break;
          case MODE_RX:si.bindMode=smeRX;break;
          case MODE_TRX:si.bindMode=smeTRX;break;
        };

        si.accessMask=rec->recdata.smppSme.accessMask;
        si.flags=rec->recdata.smppSme.flags;

        try{
          smeman.addSme(si);
        }catch(...)
        {
          smsc_log_warn(log, "UNABLE TO REGISTER SME:%s",si.systemId.c_str());
        }
      }
    }
    smsc_log_info(log, "SME registration done" );
  }

  try{
    const char* ndsmes=cfg.cfgman->getString("core.noDivertSmes");
    if(ndsmes)
    {
      const char* comma=strchr(ndsmes,',');
      const char* last=ndsmes;
      while(comma)
      {
        nodivertSmes.insert(std::string(last,comma-last));
        last=comma+1;
        comma=strchr(last,',');
      }
      nodivertSmes.insert(last);
    }
    smsc_log_info(log,"%d smes added to nodivert list",nodivertSmes.size());
  }catch(...)
  {

  }

  try{
    const char* gtFmt=cfg.cfgman->getString("core.gtformat");
    ansiGt=strcmp(gtFmt,"ansi")==0;
  }catch(...)
  {

  }

  try{
    RouteManager::EnableSmeRouters(cfg.cfgman->getBool("core.srcSmeSeparateRouting"));
  }catch(...)
  {
    __warning__("src sme routing disabled by default");
  }

  aliaser=new smsc::alias::AliasManImpl(cfg.cfgman->getString("aliasman.storeFile"));

  if(!ishs)
  {
    aliaser->Load();
    smsc_log_info(log, "Aliases loaded" );
  }

  reloadRoutes();
  smsc_log_info(log, "Routes loaded" );

#ifdef USE_MAP
  if(!ishs)
  {
    mapio::FraudControl::Init(smsc::util::findConfigFile("fraud.xml"));
    mapio::MapLimits::Init(smsc::util::findConfigFile("maplimits.xml"));
    mapio::NetworkProfiles::init(smsc::util::findConfigFile("network-profiles.xml"));
  }
#endif

  /*auto_ptr<RouteManager> router(new RouteManager());

  // initialize router (all->all)
  router->assign(&smeman);*/
  /*
  auto_ptr<SmeIterator> it(smeman.iterator());
  while (it->next())
  {
    SmeInfo info1 = it->getSmeInfo();
    Address src_addr(info1.rangeOfAddress.length(),
                     info1.typeOfNumber,
                     info1.numberingPlan,
                     info1.rangeOfAddress.c_str());
    auto_ptr<SmeIterator> it2(smeman.iterator());
    while ( it2->next() )
    {
      SmeInfo info2 = it2->getSmeInfo();
      Address dest_addr(info2.rangeOfAddress.length(),
                       info2.typeOfNumber,
                       info2.numberingPlan,
                       info2.rangeOfAddress.c_str());
      RouteInfo rinfo;
      rinfo.smeSystemId = info2.systemId;
      rinfo.source = src_addr;
      rinfo.dest = dest_addr;
      router.addRoute(rinfo);
    }
  }
  //
  try{
    loadRoutes(router.get(),*cfg.routesconfig);
  }catch(...)
  {
    __warning__("Failed to load routes");
  }

  ResetRouteManager(router.release());
  */

  // create scheduler here, and start later in run
  scheduler=new Scheduler();

  try{
    scheduler->setStoresCount(cfg.cfgman->getInt("MessageStore.LocalStore.storeFilesCount"));
  }catch(...)
  {
    smsc_log_warn(log,"MessageStore.LocalStore.storeFilesCount not found, using default %d",32);
    scheduler->setStoresCount(32);
  }

  if(!ishs)
  {
    scheduler->InitDpfTracker(cfg.cfgman->getString("dpf.storeDir"),
                              cfg.cfgman->getInt("dpf.timeOut1179"),
                              cfg.cfgman->getInt("dpf.timeOut1044"),
                              cfg.cfgman->getInt("dpf.maxChanges"),
                              cfg.cfgman->getInt("dpf.maxTime"));
  }

  schedulerSoftLimit=cfg.cfgman->getInt("core.schedulerSoftLimit");
  schedulerHardLimit=cfg.cfgman->getInt("core.schedulerHardLimit");
  try{
    schedulerFreeBandwidthUsage=cfg.cfgman->getInt("core.schedulerFreeBandwidthUsage");
  }catch(...)
  {
    smsc_log_warn(log,"core.schedulerFreeBandwidthUsage not found, using default %d",schedulerFreeBandwidthUsage);
  }

  try
  {
    speedLogFlushPeriod=cfg.cfgman->getInt("core.speedLogFlushPeriodMin");
    if((60%speedLogFlushPeriod)!=0)
    {
      smsc_log_warn(log,"60 isn't multiple of core.speedLogFlushPeriodMin. using default value");
      speedLogFlushPeriod=60;
    }
    time_t now=time(NULL)/60;
    nextSpeedLogFlush=now+(speedLogFlushPeriod-(now%speedLogFlushPeriod));
  } catch(...)
  {
    speedLogFlushPeriod=60;
  }


  //smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance(),0);
  //store=smsc::store::StoreManager::getMessageStore();
  store=scheduler;
  smsc_log_info(log, "Initializing scheduler" );
  scheduler->DelayInit(cfg.cfgman);
  /*
  if(ishs)
  {
    scheduler->DelayInit(this,cfg.cfgman);
  }
  else
  {
    scheduler->Init(this,cfg.cfgman);
  }
  */

  smeman.registerInternallSmeProxy("scheduler",scheduler);
  try{
    smeman.registerInternallSmeProxy("NULLSME",&nullSme);
  }catch(std::exception& e)
  {
    smsc_log_warn(log, "NULLSME not registered:%s",e.what());

  }

  //tp.startTask(scheduler);

  smsc_log_info(log, "Scheduler initialized" );

  inManCom=new INManComm(&smeman);
  inManCom->Init(cfg.cfgman->getString("inman.host"),cfg.cfgman->getInt("inman.port"));
  INManComm::scAddr=cfg.cfgman->getString("core.service_center_address");
  if( !ishs ) {
    smeman.registerInternallSmeProxy("INMANCOMM",inManCom);
    tp.startTask(inManCom);
    smsc_log_info(log, "IN manager started" );
  }

  {
    CStrSet *params=cfg.cfgman->getChildStrParamNames("directives");
    CStrSet::iterator i=params->begin();
    string da;
    for(;i!=params->end();i++)
    {
      da="directives.";
      da+=*i;
      StateMachine::AddDirectiveAlias(cfg.cfgman->getString(da.c_str()),i->c_str());
    }
    delete params;
  }

  smsc::closedgroups::ClosedGroupsManager::Init();
  smsc::closedgroups::ClosedGroupsManager::getInstance()->Load(smsc::util::findConfigFile("ClosedGroups.xml"));

  {
#ifdef SMSEXTRA
    bool createCopyOnNickUsage=false;
    try{
      createCopyOnNickUsage=cfg.cfgman->getBool("smsx.createCopyOnNickUsage");
    }catch(...)
    {
      __warning__("Config parameter smsx.createCopyOnNickUsage not found. Feature disabled by default");
    }
#endif
    smsc_log_info(log, "Starting statemachines" );
    stateMachinesCount=cfg.cfgman->getInt("core.state_machines_count");
    time_t maxValidTime=cfg.cfgman->getInt("sms.max_valid_time");
    for(int i=0;i<stateMachinesCount;i++)
    {
      StateMachine *m=new StateMachine(eventqueue,store,i);
      m->maxValidTime=maxValidTime;
#ifdef SMSEXTRA
      m->createCopyOnNickUsage=createCopyOnNickUsage;
#endif
      Address raddr;
      try{
        raddr=cfg.cfgman->getString("core.receipt_orig_address");
      }catch(...)
      {
        raddr=cfg.cfgman->getString("core.service_center_address");
      }
      m->receiptAddress=raddr;
      m->setReceiptInfo(
        cfg.cfgman->getString("core.service_type"),
        cfg.cfgman->getInt("core.protocol_id"),
        cfg.cfgman->getString("core.systemId")
      );
      tp.startTask(m);
      stateMachines.push_back(m);
    }
    smsc_log_info(log, "Statemachines started" );
  }

  //RescheduleCalculator::InitDefault(cfg.cfgman->getString("core.reschedule_table"));
  try{
      RescheduleCalculator::init(smsc::util::findConfigFile("schedule.xml"));
  }catch(std::exception& e)
  {
      smsc_log_warn(log, "Exception during reading 'schedule.xml':%s" ,e.what());
  }catch(...){
      smsc_log_warn(log, "Exception during reading 'schedule.xml'" );
  }
  /*{
    using smsc::util::config::CStrSet;
    CStrSet *params=cfg.cfgman->getChildStrParamNames("core.reshedule table");
    CStrSet::iterator i=params->begin();
    for(;i!=params->end();i++)
    {
      string pn="core.reshedule table.";
      pn+=*i;
      RescheduleCalculator::AddToTable(i->c_str(),cfg.cfgman->getString(pn.c_str()));
    }
    delete params;
  }*/

  if(!ishs)
  {
    SpeedMonitor *sm=new SpeedMonitor(eventqueue,&perfDataDisp,&perfSmeDataDisp,this);
    /*
    FILE *f=fopen("stats.txt","rt");
    if(f)
    {
      time_t ut;
      fscanf(f,"%d %lld %lld %lld %lld %lld %lld",
        &ut,
        &submitOkCounter,
        &submitErrCounter,
        &deliverOkCounter,
        &deliverErrTempCounter,
        &deliverErrPermCounter,
        &rescheduleCounter
      );
      startTime=time(NULL)-ut;
      sm->setStartTime(startTime);
      fclose(f);
      remove("stats.txt");
    }
    */
    tp.startTask(sm);
    smsc_log_info(log, "Speedmonitor started" );
  }
  /*
  {
    using namespace smsc::db;
    using smsc::util::config::ConfigView;
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";

    std::auto_ptr<ConfigView> dsConfig(new smsc::util::config::ConfigView(*cfg.cfgman, "StartupLoader"));
    DataSourceLoader::loadup(dsConfig.get());

    dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    if (!dataSource) throw Exception("Failed to get DataSource");
    std::auto_ptr<ConfigView> config(new ConfigView(*cfg.cfgman,"DataSource"));

    dataSource->init(config.get());
    smsc_log_info(log, "Datasource configured" );
  }
  */
  {
    smsc_log_info(log, "Statistics manager starting..." );
    std::auto_ptr<ConfigView> msConfig(new ConfigView(*cfg.cfgman, "MessageStore"));
    const char* statisticsLocation = msConfig.get()->getString("statisticsDir");
    statMan=new smsc::stat::StatisticsManager(statisticsLocation);
    if(!ishs)
    {
      tp2.startTask(statMan);
      smsc_log_info(log, "Statistics manager started" );
    }
  }

  aclmgr = AclAbstractMgr::Create2();
  aclmgr->LoadUp(*cfg.cfgman);

  distlstman=new DistrListManager(*cfg.cfgman);

  distlstsme=new DistrListProcess(distlstman,&smeman);
  tp.startTask(distlstsme);
  smsc_log_info(log, "Distribution list processor started" );

  smeman.registerInternallSmeProxy("DSTRLST",distlstsme);

  if(!ishs)
  {
    distlstman->init();

    smsc::mscman::MscManager::startup(*cfg.cfgman);
    smsc_log_info(log, "MSC manager started" );
  }

  /*
  smsc::resourcemanager::ResourceManager::init
  (
    cfg.cfgman->getString("core.locales"),
    cfg.cfgman->getString("core.default_locale")
  );
  */
  smsc_log_info(log, "Resource manager configured" );

  smsc_log_info(log, "Starting profiler" );
  {
    ProfileNotifier* pnot=0;
    try{
      const char* host=cfg.cfgman->getString("profiler.notify.host");
      int port=cfg.cfgman->getInt("profiler.notify.port");
      const char* dir=cfg.cfgman->getString("profiler.notify.dir");
      if(*host && *dir)
      {
        pnot=new ProfileNotifier(host,port,dir);
        tp2.startTask(pnot);
      }else
      {
        __trace__("Profile notifier disabled");
      }
    }catch(std::exception& e)
    {
      __warning2__("Failed to initialize profileNotifier:%s",e.what());
    }

    char *rep=cfg.cfgman->getString("profiler.defaultReport");
    char *dc=cfg.cfgman->getString("profiler.defaultDataCoding");


    char *str=rep;
    while((*str=toupper(*str)))str++;
    str=dc;
    while((*str=toupper(*str)))str++;
    smsc::profiler::Profile defProfile;
    defProfile.locale=(const char*)cfg.cfgman->getString("core.default_locale");

    defProfile.hide=cfg.cfgman->getBool("profiler.defaultHide");
    defProfile.hideModifiable=cfg.cfgman->getBool("profiler.defaultHideModifiable");
    defProfile.divertModifiable=cfg.cfgman->getBool("profiler.defaultDivertModifiable");

    defProfile.udhconcat=cfg.cfgman->getBool("profiler.defaultUdhConcat");

    if(!strcmp(dc,"DEFAULT"))
      defProfile.codepage=profiler::ProfileCharsetOptions::Default;
    else if(!strcmp(dc,"UCS2"))
      defProfile.codepage=profiler::ProfileCharsetOptions::Ucs2;
    else
    {
      smsc_log_warn(log, "Profiler:Unrecognized default data coding");
    }

    if(cfg.cfgman->getBool("profiler.defaultUssdIn7Bit"))
      defProfile.codepage|=profiler::ProfileCharsetOptions::UssdIn7Bit;

    if(!strcmp(rep,"NONE"))
      defProfile.reportoptions=profiler::ProfileReportOptions::ReportNone;
    else if(!strcmp(rep,"FULL"))
      defProfile.reportoptions=profiler::ProfileReportOptions::ReportFull;
    else
    {
      smsc_log_warn(log, "Profiler:Unrecognized default report options");
    }

    try {
      defProfile.accessMaskIn = cfg.cfgman->getInt("profiler.defaultAccessMaskIn");
    } catch(ConfigException& ex){
      smsc_log_warn(log, "profiler.defaultAccessMaskIn not found, using hardcoded default(%d)",defProfile.accessMaskIn);
    }

    try {
      defProfile.accessMaskOut = cfg.cfgman->getInt("profiler.defaultAccessMaskOut");
    } catch(ConfigException& ex){
      smsc_log_warn(log, "profiler.defaultAccessMaskOut not found, using hardcoded default(%d)",defProfile.accessMaskOut);
    }

    profiler=new smsc::profiler::Profiler(defProfile,
               &smeman,
               cfg.cfgman->getString("profiler.systemId"));

#ifdef SMSEXTRA
    try{
      const char* blklstFile=cfg.cfgman->getString("profiler.blackListFile");
      time_t checkPeriod=cfg.cfgman->getInt("profiler.blackListCheckPeriod");
      profiler->InitBlackList(blklstFile,checkPeriod);
    }catch(std::exception& e)
    {
      smsc_log_warn(log,"Failed to init black list:'%s'",e.what());
    }
#endif

    profiler->serviceType=cfg.cfgman->getString("profiler.service_type");
    profiler->protocolId=cfg.cfgman->getInt("profiler.protocol_id");

    profiler->setAliasManager(aliaser);

    CStrSet *params=cfg.cfgman->getChildIntParamNames("profiler.ussdOpsMapping");
    CStrSet::iterator i=params->begin();
    for(;i!=params->end();i++)
    {
      string pn="profiler.ussdOpsMapping.";
      pn+=*i;
      profiler->addToUssdCmdMap(cfg.cfgman->getInt(pn.c_str()),*i);
    }
    delete params;
    profiler->setNotifier(pnot);

    try{
      profiler->originatingAddress=cfg.cfgman->getString("profiler.originatingAddress");
    }catch(...)
    {
      ///
    }
  }

  distlstsme->assignProfiler(profiler);
  distlstsme->assignAliaser(aliaser);

  try{
    distlstsme->originatingAddress=Address(cfg.cfgman->getString("distrList.originatingAddress")).toString().c_str();
  }catch(...)
  {
    //optional parameter
  }

  try{
    distlstsme->originatingAddressForSme=Address(cfg.cfgman->getString("distrList.originatingAddressForSme")).toString().c_str();
  }catch(...)
  {
    //optional parameter
  }

  smsc_log_info(log, "Profiler configured" );
  profiler->load(cfg.cfgman->getString("profiler.storeFile"));
  smsc_log_info(log, "Profiler data loaded" );



  tp2.startTask(profiler);
  smsc_log_info(log, "Profiler started" );

  try{
    smeman.registerInternallSmeProxy(
      cfg.cfgman->getString("profiler.systemId"),
      profiler);
  }catch(...)
  {
    smsc_log_warn(log, "Failed to register profiler");
    __warning__("Failed to register profiler Sme");
  }

  {
    smsc::system::abonentinfo::AbonentInfoSme *abonentInfo=
      new smsc::system::abonentinfo::AbonentInfoSme(this,
        cfg.cfgman->getString("abonentinfo.systemId"));
    abonentInfo->servType=cfg.cfgman->getString("abonentinfo.service_type");
    abonentInfo->protId=cfg.cfgman->getInt("abonentinfo.protocol_id");
    Address hr=Address(cfg.cfgman->getString("abonentinfo.mobile_access_address"));
    Address m=Address(cfg.cfgman->getString("abonentinfo.mobile_access_address"));
    abonentInfo->setSources(hr,m);
    tp.startTask(abonentInfo);
    try{
      smeman.registerInternallSmeProxy(
        cfg.cfgman->getString("abonentinfo.systemId"),
        abonentInfo);
      smsc_log_info(log, "Abonent info started" );
    }catch(exception& e)
    {
      smsc_log_warn(log, "Failed to register abonentinfo");
      __trace2__("Failed to register abonentinfo Sme:%s",e.what());
      __warning__("Failed to register abonentinfo Sme");
    }
    abonentInfoProxy=abonentInfo;
  }

  smscsme=new SmscSme("smscsme",&smeman);
  smscsme->servType=cfg.cfgman->getString("core.service_type");
  smscsme->protId=cfg.cfgman->getInt("core.protocol_id");
  //tp2.startTask(smscsme);
  try{
    smeman.registerInternallSmeProxy(
      cfg.cfgman->getString("core.systemId"),
      smscsme);
    smsc_log_info(log, "SMSC sme registered" );
  }catch(exception& e)
  {
    smsc_log_warn(log, "Failed to register smscsme");
    __trace2__("Failed to register smscsme:%s",e.what());
    __warning__("Failed to register smscsme");
  }

  eventQueueLimit=1000;
  try{
    eventQueueLimit=cfg.cfgman->getInt("core.eventQueueLimit");
  }catch(...)
  {
    __warning2__("eventQueueLimit not found, using default(%d)",eventQueueLimit);
  }

  cancelAgent=new CancelAgent(eventqueue,scheduler,eventQueueLimit);
  tp.startTask(cancelAgent);
  smsc_log_info(log, "Cancel agent started" );

  alertAgent=new AlertAgent(this,store);
  tp.startTask(alertAgent);
  smsc_log_info(log, "Alert agent started" );


  smscHost=cfg.cfgman->getString("smpp.host");
  smscPort=cfg.cfgman->getInt("smpp.port");
  ssockman.setSmppSocketTimeout(cfg.cfgman->getInt("smpp.readTimeout"));
  ssockman.setInactivityTime(cfg.cfgman->getInt("smpp.inactivityTime"));
  ssockman.setInactivityTimeOut(cfg.cfgman->getInt("smpp.inactivityTimeOut"));
  ssockman.setDefaultConnectionsLimit(cfg.cfgman->getInt("smpp.defaultConnectionsLimit"));

  try
  {
    ssockman.setUseAbortiveClose(cfg.cfgman->getBool("smpp.useAbortiveClose"));
  } catch(...)
  {
    __warning__("smpp.useAbortiveClose not found in config");
  }

  try
  {
    SmscCommand::standardErrorCodes=cfg.cfgman->getBool("smpp.standardErrorCodes");
  } catch(...)
  {
    __warning__("smpp.standardErrorCodes not found in config");
  }
  {
    CStrSet *params=cfg.cfgman->getChildIntParamNames("smpp.connectionsLimitsForIps");
    CStrSet::iterator i=params->begin();
    for(;i!=params->end();i++)
    {
      std::string nm="smpp.connectionsLimitsForIps.";
      nm+=*i;
      ssockman.setLimitForIp(i->c_str(),cfg.cfgman->getInt(nm.c_str()));
    }
    delete params;
  }

  try{
    mainLoopsCount=cfg.cfgman->getInt("core.mainLoopsCount");
  }catch(...)
  {
    __warning2__("mainLoopsCount set to default %d",mainLoopsCount);
  }
  try{
    mapIOTasksCount=cfg.cfgman->getInt("map.mapIOTasksCount");
  }catch(...)
  {
    __warning2__("mapIOTasksCount set to default %d",mapIOTasksCount);
  }
#ifdef EIN_HD
    localInst=cfg.cfgman->getString("map.localInstancies");
    remoteInst=cfg.cfgman->getString("map.remoteInstancies");
    CPMgmtAddress=cfg.cfgman->getString("map.cpMgmtAddress");
#endif
    smsc_log_info(log, "MR cache loaded" );

  {
    performance::PerformanceServer *perfSrv=new performance::PerformanceServer
    (
      cfg.cfgman->getString("core.performance.host"),
      cfg.cfgman->getInt("core.performance.port"),
      &perfDataDisp
    );
    tp2.startTask(perfSrv);
    smsc_log_info(log, "Performance server started" );
  }
  {
    performance::PerformanceServer *perfSrv=new performance::PerformanceServer
    (
      cfg.cfgman->getString("core.smeperformance.host"),
      cfg.cfgman->getInt("core.smeperformance.port"),
      &perfSmeDataDisp
    );
    tp2.startTask(perfSrv);
    smsc_log_info(log, "SmePerformance server started" );
  }



  {
    Address addr(cfg.cfgman->getString("core.service_center_address"));
    AddressValue addrval;
    addr.getValue( addrval );
    scAddr = addrval;
  }

  {
    Address addr(cfg.cfgman->getString("core.ussd_center_address"));
    AddressValue addrval;
    addr.getValue( addrval );
    ussdCenterAddr = addrval;
  }

  ussdSSN=cfg.cfgman->getInt("core.ussd_ssn");
  try {
    if( cfg.cfgman->getString("core.add_ussd_ssn") ) {
      addUssdSSN = cfg.cfgman->getString("core.add_ussd_ssn");
    } else {
      addUssdSSN = "";
    }
  } catch (...) {
    addUssdSSN = "";
  }
  try{
    busyMTDelay=cfg.cfgman->getInt("map.busyMTDelay");
  } catch (...) {
    __warning__("map.busyMTDelay not found, using default(20)");
    busyMTDelay = 20;
  }
  try{
    lockedByMODelay=cfg.cfgman->getInt("map.lockedByMODelay");;
  } catch (...) {
    __warning__("map.lockedByMODelay not found, using default(20)");
    lockedByMODelay = 20;
  }
  try{
    MOLockTimeout=cfg.cfgman->getInt("map.MOLockTimeout");
  } catch (...) {
    __warning__("map.MOLockTimeout not found, using default(45)");
    MOLockTimeout = 45;
  }
  try{
    allowCallBarred=cfg.cfgman->getBool("map.allowCallBarred");
  } catch (...) {
    __warning__("map.allowCallBarred not found, disabled");
    allowCallBarred = false;
  }
  try{
    ussdV1Enabled=cfg.cfgman->getBool("map.ussdV1Enabled");
  } catch (...) {
    __warning__("map.ussdV1Enabled not found, disabled");
    ussdV1Enabled = false;
  }
  try{
    ussdV1UseOrigEntityNumber=cfg.cfgman->getBool("map.ussdV1UseOrigEntityNumber");
  } catch (...) {
    __warning__("map.ussdV1UseOrigEntityNumber not found, disabled");
    ussdV1UseOrigEntityNumber = false;
  }

  {
    /*
    TrafficControl::TrafficControlConfig tccfg;
    tccfg.smsc=this;
    tccfg.store=store;
    tccfg.maxSmsPerSecond=cfg.cfgman->getInt("trafficControl.maxSmsPerSecond");
    if(tccfg.maxSmsPerSecond>license.maxsms)
    {
      smsc_log_warn(log, "maxSmsPerSecond in configuration is greater than license limit, adjusting\n");
      tccfg.maxSmsPerSecond=license.maxsms;
    }
    if(tccfg.maxSmsPerSecond==0)tccfg.maxSmsPerSecond=license.maxsms;
    tccfg.shapeTimeFrame=cfg.cfgman->getInt("trafficControl.shapeTimeFrame");
    tccfg.protectTimeFrame=cfg.cfgman->getInt("trafficControl.protectTimeFrame");
    tccfg.protectThreshold=cfg.cfgman->getInt("trafficControl.protectThreshold");
    tccfg.allowedDeliveryFailures=cfg.cfgman->getInt("trafficControl.allowedDeliveryFailures");
    tccfg.lookAheadTime=cfg.cfgman->getInt("trafficControl.lookAheadTime");

    tccfg.smoothTimeFrame=5;

    tcontrol=new TrafficControl(tccfg);
    */

    try{
      smartMultipartForward=cfg.cfgman->getBool("core.smartMultipartForward");
    }catch(...)
    {
      smsc_log_warn(log,"core.smartMultipartForward not found in config and disabled by default");
    }

    maxSmsPerSecond=license.maxsms;//cfg.cfgman->getInt("trafficControl.maxSmsPerSecond");
    /*if(maxSmsPerSecond>license.maxsms)
    {
      smsc_log_warn(log, "maxSmsPerSecond in configuration is greater than license limit, adjusting\n");
      maxSmsPerSecond=license.maxsms;
    }*/
    if(maxSmsPerSecond<=0)maxSmsPerSecond=license.maxsms;
    shapeTimeFrame=cfg.cfgman->getInt("trafficControl.shapeTimeFrame");
    statTimeFrame=cfg.cfgman->getInt("trafficControl.statTimeFrame");
    totalCounter=new IntTimeSlotCounter(shapeTimeFrame,10);
    statCounter=new IntTimeSlotCounter(statTimeFrame,10);
    schedCounter=new IntTimeSlotCounter(shapeTimeFrame,10);
    time_t t=time(NULL);
    tm now;
    localtime_r(&t,&now);
    lastUpdateHour=now.tm_hour;
  }

  try{
    std::string v=cfg.cfgman->getString("inman.chargingPolicy.peer2peer");
    if(v=="submit")p2pChargePolicy=chargeOnSubmit;
    else if(v=="deliver")p2pChargePolicy=chargeOnDelivery;
    else
    {
      smsc_log_warn(log,"Unknown value of config parameter - chargingPolicy.peer2per:'%s'",v.c_str());
    }
    v=cfg.cfgman->getString("inman.chargingPolicy.other");
    if(v=="submit")otherChargePolicy=chargeOnSubmit;
    else if(v=="deliver")otherChargePolicy=chargeOnDelivery;
    else
    {
      smsc_log_warn(log,"Unknown value of config parameter - chargingPolicy.other:'%s'",v.c_str());
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"Exception during charging policy initialization:%s",e.what());
  }


  /*
  try{
    scheduler->setRescheduleLimit(cfg.cfgman->getInt("core.reschedule_limit"));
  }catch(...)
  {
    __warning__("reschedule_limit not found in config, using default");
  }
  */

  {
    StatusSme *ss=new StatusSme(this,"StatusSme");

    bool ok=false;
    try{
      smeman.registerInternallSmeProxy("StatusSme",ss);
      ok=true;
    }catch(...)
    {
      __warning__("status sme not exists.");
    }

    if(ok)tp.startTask(ss);
  }

  mergeConcatTimeout=600;
  try{
    mergeConcatTimeout=cfg.cfgman->getInt("core.mergeTimeout");
  }catch(...)
  {
    smsc_log_warn(log, "core.mergeTimeout not found, using default(%d)",mergeConcatTimeout);
  }

#ifdef SMSEXTRA
  {
    ExtraInfo::Init();
    int bits[]={EXTRA_NICK,EXTRA_FLASH,EXTRA_CALEND,EXTRA_SECRET};
    const char* sections[]={"nick","flash","calendar","secret"};
    std::string cfgParam;
    for(int i=0;i<sizeof(bits)/sizeof(bits[0]);i++)
    {
      cfgParam="smsx.";
      cfgParam+=sections[i];
      const char* prefix=cfg.cfgman->getString((cfgParam+".prefix").c_str());
      const char* divert=0;
      try{
        divert=cfg.cfgman->getString((cfgParam+".divertNumber").c_str());
      }catch(...){}
      ExtraInfo::getInstance().addPrefix(bits[i],prefix,divert);
    }
  }
#endif

#define GETDLOPTIONALCFGPARAM(n,t) \
  try{ distlstsme->n=cfg.cfgman->t("distrList."#n);}\
  catch(...) \
  { smsc_log_warn(log,"Config Parameter distrList."#n" not found in config. Using default value");}

  GETDLOPTIONALCFGPARAM(autoCreatePrincipal,getBool);
  GETDLOPTIONALCFGPARAM(defaultMaxLists,getInt);
  GETDLOPTIONALCFGPARAM(defaultMaxElements,getInt);
  GETDLOPTIONALCFGPARAM(sendSpeed,getInt);

  /*
  try{
    distlstsme->autoCreatePrincipal=cfg.cfgman->getBool("distrList.autocreatePrincipal");
    distlstsme->defaultMaxLists=cfg.cfgman->getInt("distrList.defaultMaxLists");
    distlstsme->defaultMaxElements=cfg.cfgman->getInt("distrList.defaultMaxElements");
  }catch(...)
  {
  }
  */

  Manager::getInstance().registerConfigChangeWatcher(&cfgWatch);


  try {

     if(ishs){
         // Initializes command handlers and registers its;
         smsc_log_info(log, "The command listeners initialization and registration ..." );
         MscCommandListener *mscCommandListener = new MscCommandListener();
         DlCommandListener *dlCommandListener = new DlCommandListener(distlstman);
         MemCommandListener *memCommandListener = new MemCommandListener(distlstman);
         PrcCommandListener *prcCommandListener = new PrcCommandListener(distlstman);
         SbmCommandListener *sbmCommandListener = new SbmCommandListener(distlstman);
         SmeCommandListener *smeCommandListener = new SmeCommandListener(&smeman);
         ProfileCommandListener *proCommandListener = new ProfileCommandListener(profiler);
         ApplyCommandListener *appCommandListener = new ApplyCommandListener(&cfg, this);
         CgmCommandListener* cgmCommandListener=new CgmCommandListener();

         icon->addListener(smsc::cluster::MSCREGISTRATE_CMD,   mscCommandListener);
         icon->addListener(smsc::cluster::DLADD_CMD,           dlCommandListener);
         icon->addListener(smsc::cluster::MEMADDMEMBER_CMD,    memCommandListener);
         icon->addListener(smsc::cluster::PRCADDPRINCIPAL_CMD, prcCommandListener);
         icon->addListener(smsc::cluster::SBMADDSUBMITER_CMD,  sbmCommandListener);
         icon->addListener(smsc::cluster::SMEADD_CMD,          smeCommandListener);
         icon->addListener(smsc::cluster::PROFILEUPDATE_CMD,   proCommandListener);
         icon->addListener(smsc::cluster::APPLYROUTES_CMD,     appCommandListener);
         icon->addListener(smsc::cluster::CGM_COMMANDS,        cgmCommandListener);
         smsc_log_info(log, "The command listeners initialization and registration is completed successful" );
     }
  }catch(...){
      throw Exception("Command listeners initialization exception.");
  }

  try{

      if(icon){
          smsc_log_info(log, "Agent listener is starting..." );

          using smsc::util::config::ConfigView;
          std::auto_ptr<ConfigView> imConfig(new smsc::util::config::ConfigView(*cfg.cfgman, "cluster"));

          auto_ptr<char> host ( imConfig.get()->getString("agentHost") );
          int port = imConfig.get()->getInt("agentPort");

          smsc_log_info(log, "pid: %ld", (long)getpid() );
          agentListener.init(host.get(), port, getpid());
          agentListener.assignSmsc(this);
          agentListener.Start();

          smsc_log_info(log, "Agent listener is started" );
      }

  }catch(...)
  {
      throw Exception("Agent listener initialization exception.");
  }

  smsc_log_info(log, "SMSC init complete" );
  }catch(exception& e)
  {
    __trace2__("Smsc::init exception:%s",e.what());
    smsc_log_warn(log, "Smsc::init exception:%s",e.what());
    throw;
  }catch(...)
  {
    __trace__("Smsc::init exception:unknown");
    smsc_log_warn(log, "Smsc::init exception:unknown");
    throw;
  }
  __trace__("Smsc::init completed");
}


void NotifyMonHandler(smsc::cluster::Role r,void* p)
{
  if(r==smsc::cluster::MASTER)
  {
    EventMonitor* mon=(EventMonitor*)p;
    mon->Lock();
    mon->notify();
    mon->Unlock();
  }
}

class MainLoopRunner:public Thread
{
public:
  MainLoopRunner(){}
  void assignSmsc(Smsc* argSmsc){smsc=argSmsc;}
  void assignIdx(int argIdx)
  {
    idx=argIdx;
  }
  int Execute()
  {
    try{
      smsc->mainLoop(idx);
    }catch(std::exception& e)
    {
      __warning2__("exception in main loop:%s",e.what());
    }
    return 0;
  }
protected:
  Smsc* smsc;
  int idx;
};

void Smsc::run()
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.run");
  //smsc::logger::Logger::getInstance("sms.snmp.alarm").debug("sample alarm");

  if(ishs)
  {
    Interconnect *icon = InterconnectManager::getInstance();
    try {
      icon->activate();
    }catch(std::exception& e)
    {
      throw Exception("InterconnectManager activating exception:%s",e.what());
    }

    {
      MutexGuard mg(idleMon);
      icon->addChangeRoleHandler(NotifyMonHandler,&idleMon);
      smsc_log_info(log,"wait for change role event");
      while(!stopFlag)
      {
         int rv=idleMon.wait(1000);
         //__trace2__("idleMon.wait returned %d(%s)",rv,strerror(rv));
         if(rv!=ETIME && rv!=ETIMEDOUT)break;
         if(icon->getRole()==smsc::cluster::MASTER)break;
      }
    }
    if(stopFlag)
    {
      delete inManCom;
      inManCom=0;
      delete statMan;
      statMan=0;
      return;
    }
    // starting inman
    smeman.registerInternallSmeProxy("INMANCOMM",inManCom);
    tp.startTask(inManCom);
    smsc_log_info(log, "IN manager started" );

    distlstman->init();
    smsc::mscman::MscManager::startup(smsc::util::config::Manager::getInstance());

#ifdef USE_MAP
    mapio::FraudControl::Init(smsc::util::findConfigFile("fraud.xml"));
    mapio::MapLimits::Init(smsc::util::findConfigFile("maplimits.xml"));
    mapio::NetworkProfiles::init(smsc::util::findConfigFile("network-profiles.xml"));
#endif

    aliaser->Load();
    smsc_log_info(log, "Aliases loaded" );

    SpeedMonitor *sm=new SpeedMonitor(eventqueue,&perfDataDisp,&perfSmeDataDisp,this);
    /*
    FILE *f=fopen("stats.txt","rt");
    if(f)
    {
      time_t ut;
      fscanf(f,"%d %lld %lld %lld %lld %lld %lld",
        &ut,
        &submitOkCounter,
        &submitErrCounter,
        &deliverOkCounter,
        &deliverErrTempCounter,
        &deliverErrPermCounter,
        &rescheduleCounter
      );
      startTime=time(NULL)-ut;
      sm->setStartTime(startTime);
      fclose(f);
      remove("stats.txt");
    }
    */
    tp.startTask(sm);
    smsc_log_info(log, "Speedmonitor started" );

    tp2.startTask(statMan);
    smsc_log_info(log, "Statistics manager started" );
  }


  __trace__("Smsc::run");

  try{
  if(startTime==0)startTime=time(NULL);

  {
    __trace__("Starting SMPPIO");
    Event accstarted;
    smppio::SmppAcceptor *acc=new
      smppio::SmppAcceptor(
        smscHost.c_str(),
        smscPort,
        &ssockman,
        &accstarted
      );
    tp.startTask(acc);
    __trace__("Waiting SMPPIO started");
    accstarted.Wait();
    __trace__("SMPPIO started");
#if defined(USE_MAP) && !defined(NOMAPPROXY)
    Event mapiostarted;
    MapIoTask* mapio = new MapIoTask(&mapiostarted,scAddr,ussdCenterAddr,ussdSSN,addUssdSSN,
                                     busyMTDelay,lockedByMODelay,MOLockTimeout,
                                     allowCallBarred,ussdV1Enabled,ussdV1UseOrigEntityNumber,nodeIndex,nodesCount);
    mapioptr=mapio;
    //tp.startTask(mapio);
#ifdef EIN_HD
    mapio->initInstances(localInst,remoteInst);
    mapio->setCPMgmtAddress(CPMgmtAddress);
#endif
    mapio->setMapIoTaskCount(mapIOTasksCount);
    mapio->StartMap();
    mapiostarted.Wait();
    __trace__("MAPIO started");
    if(!acc->isStarted()||!mapio->isStarted())
#else
    if(!acc->isStarted())
#endif
    {
      throw Exception("Failed to start SMPP or MAP acceptor");
    }
//    tp.startTask(new MapTracker());
#ifndef NOMAPPROXY
    MapDialogContainer::getInstance()->registerSelf(&smeman);
//#endif
#ifdef USE_MAP
    mapProxy=MapDialogContainer::getInstance()->getProxy();
    SmeInfo si=smeman.getSmeInfo(smeman.lookup("MAP_PROXY"));
    mapProxy->updateSmeInfo( si );
    MapDialogContainer::getInstance()->getProxy()->setId("MAP_PROXY");
#endif
#endif
  }

  if(ishs)
  {
    smsc::util::config::Config& cfg=smsc::util::config::Manager::getInstance().getConfig();
    scheduler->InitDpfTracker(cfg.getString("dpf.storeDir"),
                              cfg.getInt("dpf.timeOut1179"),
                              cfg.getInt("dpf.timeOut1044"),
                              cfg.getInt("dpf.maxChanges"),
                              cfg.getInt("dpf.maxTime"));
  }

  // start rescheduler created in init
  // start on thread pool 2 to shutdown it after state machines

  scheduler->InitMsgId(&smsc::util::config::Manager::getInstance());
  tp2.startTask(scheduler);

#ifdef ENABLE_MAP_SYM
  MapSymProxy *msprx=new MapSymProxy();

  smeman.registerInternallSmeProxy("MAPSYM",msprx);

  MapSymInputTask *msin=new MapSymInputTask(msprx);
  MapSymOutputTask *msout=new MapSymOutputTask(msprx);
  tp.startTask(msin);
  tp.startTask(msout);
#endif

#ifdef SNMP
    __trace__("Smsc::changing SNMP state to OPER");
  if(snmpAgent) snmpAgent->statusChange(SnmpAgent::OPER);
    __trace__("Smsc::SNMP state to OPER changed");
#endif

  //

  MainLoopRunner *mlr=new MainLoopRunner[mainLoopsCount];
  try{
    for(int i=0;i<mainLoopsCount;i++)
    {
      mlr[i].assignSmsc(this);
      mlr[i].assignIdx(i);
      mlr[i].Start();
    }
    for(int i=0;i<mainLoopsCount;i++)
    {
      mlr[i].WaitFor();
    }
  }catch(std::exception& e)
  {
    __warning2__("exception in main loops start:%s",e.what());
  }
  delete [] mlr;

#ifdef SNMP
    __trace__("Smsc::changing SNMP state to SHUT");
  if(snmpAgent) snmpAgent->statusChange(SnmpAgent::SHUT);
    __trace__("Smsc::SNMP state to SHUT changed");
#endif

  //
  //shutdown();
  }catch(exception& e)
  {

    __trace2__("Smsc::run exception:(%s)%s",typeid(e).name(),e.what());
    smsc_log_warn(log, "Smsc::run exception:%s",e.what());
    throw;
  }catch(...)
  {
    __trace__("Smsc::run exception:unknown");
    smsc_log_warn(log, "Smsc::run exception:unknown");
    throw;
  }
  __trace__("Smsc::run completed");
}

void Smsc::shutdown()
{

  __trace__("shutting down");
  smeman.Dump();

  delete smscsme;

  scheduler->stopDpfTracker();

  tp.shutdown();

#if defined(USE_MAP)
  if(mapioptr)
  {
    MapIoTask *mapio=(MapIoTask*)mapioptr;
    mapio->Stop();
  }
#endif

  smeman.unregisterSmeProxy(scheduler);
  try{
    smeman.unregisterSmeProxy(&nullSme);
  }catch(...)
  {

  }

  tp2.shutdown();

#if defined(USE_MAP)
  if(mapioptr)
  {
    MapIoTask *mapio=(MapIoTask*)mapioptr;
    delete mapio;
  }
  mapio::MapLimits::Shutdown();
  mapio::NetworkProfiles::shutdown();
#endif


  smsc::closedgroups::ClosedGroupsManager::Shutdown();


  delete distlstman;
  delete aclmgr;

  smsc::mscman::MscManager::shutdown();

  //smsc::store::StoreManager::shutdown();
  //if(dataSource)delete dataSource;
  smeman.Dump();
  __trace__("smeman dumped");
  if( ishs )
  {
    __trace__("stopping interconnect");
    InterconnectManager::shutdown();
  }
  common::TimeZoneManager::Shutdown();

  if(aliaser)
  {
    delete aliaser;
    aliaser=0;
  }

#ifdef SMSEXTRA
  ExtraInfo::Shutdown();
#endif

#ifdef SNMP
  SnmpCounter::Shutdown();
#endif
  __trace__("shutdown completed");
}

void Smsc::reloadRoutes()
{
  auto_ptr<RouteManager> router(new RouteManager());
  router->assign(&smeman);
  try{
    loadRoutes(router.get(),*configs->routesconfig);
  }catch(...)
  {
    __warning__("Failed to load routes");
  }
  ResetRouteManager(router.release());
}

void Smsc::reloadTestRoutes(const RouteConfig& rcfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  router->assign(&smeman);
  loadRoutes(router.get(),rcfg,true);
  ResetTestRouteManager(router.release());
}

/*
void Smsc::reloadAliases(const SmscConfigs& cfg)
{
  auto_ptr<AliasManager> aliaser(new AliasManager());
  {
    smsc::util::config::alias::AliasConfig::RecordIterator i =
                                cfg.aliasconfig->getRecordIterator();
    while(i.hasRecord())
    {
      smsc::util::config::alias::AliasRecord *rec;
      i.fetchNext(rec);
      __trace2__("adding %20s %20s",rec->addrValue,rec->aliasValue);
      smsc::alias::AliasInfo ai;
      ai.addr = smsc::sms::Address(
        strlen(rec->addrValue),
        rec->addrTni,
        rec->addrNpi,
        rec->addrValue);
      ai.alias = smsc::sms::Address(
        strlen(rec->aliasValue),
        rec->aliasTni,
        rec->aliasNpi,
        rec->aliasValue);
      ai.hide = rec->hide;
      aliaser->addAlias(ai);
    }
    aliaser->commit();
  }

  ResetAliases(aliaser.release());
}
*/

void Smsc::reloadReschedule(){
    //RescheduleCalculator::reset();
    RescheduleCalculator::init(smsc::util::findConfigFile("schedule.xml"));
}

void Smsc::abortSmsc()
{
  SaveStats();
  statMan->flushStatistics();
#ifdef USE_MAP
  MapDialogContainer::getInstance()->abort();
#endif
  kill(getpid(),9);
}

void Smsc::dumpSmsc()
{
#ifdef USE_MAP
  MapDialogContainer::getInstance()->abort();
#endif
  abort();
}

void Smsc::registerStatisticalEvent(int eventType, const SMS *sms,bool msuOnly)
{
  using namespace smsc::stat;
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
      statMan->updateAccepted(*sms);
      MutexGuard g(perfMutex);
      if(!msuOnly)submitOkCounter++;
      msu_submitOkCounter++;
      smePerfMonitor.incAccepted(sms->getSourceSmeId());
#ifdef SNMP
      int smeIdx=smeman.lookup(sms->getSourceSmeId());
      smeStats.incCounter(smeIdx,smsc::stat::SmeStats::cntAccepted);
#endif
    }break;
    case etSubmitErr:
    {
      statMan->updateRejected(*sms);
      MutexGuard g(perfMutex);
      if(!msuOnly)submitErrCounter++;
      msu_submitErrCounter++;
      smePerfMonitor.incRejected(sms->getSourceSmeId(), sms->getLastResult());
#ifdef SNMP
      int smeIdx=smeman.lookup(sms->getSourceSmeId());
      smeStats.incCounter(smeIdx,smsc::stat::SmeStats::cntRejected);
      smeStats.incError(smeIdx,sms->getLastResult());
#endif
    }break;
    case etDeliveredOk:
    {
      statMan->updateChanged(StatInfo(*sms,false));
      MutexGuard g(perfMutex);
      if(!msuOnly)deliverOkCounter++;
      msu_deliverOkCounter++;
      smePerfMonitor.incDelivered(sms->getDestinationSmeId());
#ifdef SNMP
      int smeIdx=smeman.lookup(sms->getDestinationSmeId());
      smeStats.incCounter(smeIdx,smsc::stat::SmeStats::cntDelivered);
      smeStats.incError(smeIdx,0);
#endif
    }break;
    case etDeliverErr:
    {
      statMan->updateTemporal(StatInfo(*sms,false));

      MutexGuard g(perfMutex);
      if(!msuOnly)deliverErrTempCounter++;
      msu_deliverErrTempCounter++;
      smePerfMonitor.incFailed(sms->getDestinationSmeId(), sms->getLastResult());
#ifdef SNMP
      int smeIdx=smeman.lookup(sms->getDestinationSmeId());
      smeStats.incCounter(smeIdx,smsc::stat::SmeStats::cntTempError);
      smeStats.incError(smeIdx,sms->getLastResult());
#endif
    }break;
    case etUndeliverable:
    {
      statMan->updateChanged(StatInfo(*sms,false));
      MutexGuard g(perfMutex);
      if(!msuOnly)deliverErrPermCounter++;
      msu_deliverErrPermCounter++;
      smePerfMonitor.incFailed(sms->getDestinationSmeId(), sms->getLastResult());
#ifdef SNMP
      int smeIdx=smeman.lookup(sms->getDestinationSmeId());
      smeStats.incCounter(smeIdx,smsc::stat::SmeStats::cntFailed);
      smeStats.incError(smeIdx,sms->getLastResult());
#endif
    }break;
    case etRescheduled:
    {
      statMan->updateScheduled(StatInfo(*sms,false));
      MutexGuard g(perfMutex);
      if(!msuOnly)rescheduleCounter++;
      msu_rescheduleCounter++;
      smePerfMonitor.incRescheduled(sms->getDestinationSmeId());
#ifdef SNMP
      smeStats.incCounter(smeman.lookup(sms->getDestinationSmeId()),smsc::stat::SmeStats::cntRetried);
#endif
    }break;
  }
}



Smsc::LicenseInfo Smsc::license={0,0};
std::string Smsc::licenseFile;
std::string Smsc::licenseSigFile;
time_t Smsc::licenseFileModTime=0;

#ifdef __APPLE__
long local_gethostid()
{
  uuid_t id;
  timespec tv={0,0};
  if(gethostuuid(id,&tv)==0)
  {
    long res=0;
    for(int i=0;i<16;i++)
    {
      res^=((uint32_t)(unsigned char)(id[i]))<<((i%4)*8);
    }
    return res;
  }else
  {
    throw Exception("Failed to get uuid");
  }
}
#else
long local_gethostid()
{
  return gethostid();
}
#endif

void Smsc::InitLicense()
{
  Hash<string> lic;
  if(licenseFile.length()==0)
  {
      licenseFile=smsc::util::findConfigFile("license.ini");
      licenseSigFile=smsc::util::findConfigFile("license.sig");
  }

  stat_type st;
  if(::stat(licenseFile.c_str(),&st)!=0)
  {
    throw Exception("Failed to stat '%s'",licenseFile.c_str());
  }
#ifdef __APPLE__
  if(st.st_mtimespec.tv_sec==licenseFileModTime)
  {
    return;
  }
  licenseFileModTime=st.st_mtimespec.tv_sec;
#else
  if(st.st_mtime==licenseFileModTime)
  {
    return;
  }
  licenseFileModTime=st.st_mtime;
#endif


  static const char *lkeys[]=
  {
  "Organization",
  "Hostids",
  "MaxSmsThroughput",
  "LicenseExpirationDate",
  "LicenseType",
  "Product"
  };

  if(!smsc::license::check::CheckLicense(licenseFile.c_str(),licenseSigFile.c_str(),lic,lkeys,sizeof(lkeys)/sizeof(lkeys[0])))
  {
    throw Exception("Invalid license");
  }



  license.maxsms=atoi(lic["MaxSmsThroughput"].c_str());
  int y,m,d;
  sscanf(lic["LicenseExpirationDate"].c_str(),"%d-%d-%d",&y,&m,&d);
  struct tm t={0,};
  t.tm_year=y-1900;
  t.tm_mon=m-1;
  t.tm_mday=d;
  license.expdate=mktime(&t);
  long hostid;
  std::string ids=lic["Hostids"];
  std::string::size_type pos=0;
  bool ok=false;
  do{
    sscanf(ids.c_str()+pos,"%lx",&hostid);
    if(hostid==local_gethostid())
    {
      ok=true;break;
    }
    pos=ids.find(',',pos);
    if(pos!=std::string::npos)pos++;
  }while(pos!=std::string::npos);
  if(!ok)throw runtime_error("");
  if(smsc::util::crc32(0,lic["Product"].c_str(),lic["Product"].length())!=0x685a3df4)throw runtime_error("");
  if(license.expdate<time(NULL))
  {
    char x[]=
    {
    'L'^0x4c,'i'^0x4c,'c'^0x4c,'e'^0x4c,'n'^0x4c,'s'^0x4c,'e'^0x4c,' '^0x4c,'E'^0x4c,'x'^0x4c,'p'^0x4c,'i'^0x4c,'r'^0x4c,'e'^0x4c,'d'^0x4c,
    };
    std::string s;
    for(int i=0;i<sizeof(x);i++)
    {
      s+=x[i]^0x4c;
    }
    throw runtime_error(s);
  }
}

smsc::util::config::ConfigParamWatchType Smsc::SmscConfigWatcher::getWatchedParams(smsc::util::config::ParamsVector& argParams)
{
  using namespace smsc::util::config;
  argParams.push_back(std::make_pair(cvtInt,"core.schedulerSoftLimit"));
  argParams.push_back(std::make_pair(cvtInt,"core.schedulerHardLimit"));
  argParams.push_back(std::make_pair(cvtInt,"core.schedulerFreeBandwidthUsage"));
  argParams.push_back(std::make_pair(cvtInt,"sms.max_valid_time"));
  argParams.push_back(std::make_pair(cvtInt,"core.eventQueueLimit"));
  argParams.push_back(std::make_pair(cvtInt,"core.speedLogFlushPeriodMin"));
  argParams.push_back(std::make_pair(cvtBool,"core.smartMultipartForward"));
  argParams.push_back(std::make_pair(cvtInt,"core.mergeTimeout"));
  argParams.push_back(std::make_pair(cvtInt,"MessageStore.LocalStore.storeFilesCount"));
  argParams.push_back(std::make_pair(cvtInt,"map.busyMTDelay"));
  argParams.push_back(std::make_pair(cvtInt,"map.lockedByMODelay"));
  return cpwtIndividual;
}
void Smsc::SmscConfigWatcher::paramsChanged()
{
}
void Smsc::SmscConfigWatcher::paramChanged(smsc::util::config::ConfigValueType cvt,const std::string& paramName)
{
  smsc::util::config::Config& cfg=smsc::util::config::Manager::getInstance().getConfig();
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("cfg.reload");
  Smsc& smsc=Smsc::getInstance();
  if(paramName=="core.schedulerSoftLimit")
  {
    smsc.schedulerSoftLimit=cfg.getInt(paramName.c_str());
  }else if(paramName=="core.schedulerHardLimit")
  {
    smsc.schedulerHardLimit=cfg.getInt(paramName.c_str());
  }else if(paramName=="core.schedulerFreeBandwidthUsage")
  {
    smsc.schedulerFreeBandwidthUsage=cfg.getInt(paramName.c_str());
  }else if(paramName=="core.speedLogFlushPeriodMin")
  {
    smsc.speedLogFlushPeriod=cfg.getInt(paramName.c_str());
  }else if(paramName=="sms.max_valid_time")
  {
    time_t maxValidTime=cfg.getInt(paramName.c_str());
    for(std::vector<StateMachine*>::iterator it=smsc.stateMachines.begin(),end=smsc.stateMachines.end();it!=end;++it)
    {
      (*it)->maxValidTime=maxValidTime;
    }
  }else if(paramName=="core.eventQueueLimit")
  {
    smsc.eventQueueLimit=cfg.getInt(paramName.c_str());
  }else if (paramName=="core.speedLogFlushPeriodMin")
  {
    smsc.speedLogFlushPeriod=cfg.getInt(paramName.c_str());
    if((60%smsc.speedLogFlushPeriod)!=0)
    {
      smsc_log_warn(log,"60 isn't multiple of core.speedLogFlushPeriodMin. using default value");
      smsc.speedLogFlushPeriod=60;
    }
    time_t now=time(NULL)/60;
    smsc.nextSpeedLogFlush=now+(smsc.speedLogFlushPeriod-(now%smsc.speedLogFlushPeriod));
  }else if(paramName=="core.smartMultipartForward")
  {
    smsc.smartMultipartForward=cfg.getBool(paramName.c_str());
  }else if(paramName=="core.mergeTimeout")
  {
    smsc.mergeConcatTimeout=cfg.getInt(paramName.c_str());
  }else if(paramName=="MessageStore.LocalStore.storeFilesCount")
  {
    smsc.scheduler->setStoresCount(cfg.getInt(paramName.c_str()));
  }else if(paramName=="map.busyMTDelay")
  {
#ifdef USE_MAP
    MapDialogContainer::setBusyMTDelay(cfg.getInt(paramName.c_str()));
#endif
  }else if(paramName=="map.lockedByMODelay")
  {
#ifdef USE_MAP
    MapDialogContainer::setLockedByMoDelay(cfg.getInt(paramName.c_str()));
#endif
  }
}



}//system
}//smsc
