#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include <memory>
#include "util/debug.h"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "system/rescheduler.hpp"
#include "util/config/route/RouteConfig.h"
#include "system/abonentinfo/AbonentInfo.hpp"
#include "util/Logger.h"
#include "system/smscsme.hpp"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"
#include "system/mapio/MapIoTask.h"
#include "system/abonentinfo/AbonentInfo.hpp"
#include "mscman/MscManager.h"
#include "resourcemanager/ResourceManager.hpp"

//#define ENABLE_MAP_SYM

#ifdef ENABLE_MAP_SYM
#include "system/mapsym.hpp"
#endif

namespace smsc{
namespace system{

using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::core::synchronization;
using util::Exception;


Smsc::~Smsc()
{
}

class SpeedMonitor:public smsc::core::threads::ThreadedTask{
public:
  SpeedMonitor(EventQueue& eq,performance::PerformanceListener* pl,Smsc* psmsc):
    queue(eq),perfListener(pl)
  {
    start.tv_sec=0;
    start.tv_nsec=0;
    smsc=psmsc;
  }
  int Execute()
  {
    uint64_t cnt,last=0;
    timespec now,lasttime;
    double ut,tm,rate,avg;
    if(start.tv_sec==0)
      clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    __trace__("enter SpeedMonitor");
    timeshift=0;
    time_t perfStart=start.tv_sec;
    times[0]=start.tv_sec;
    int lastscnt=0;
    memset(perfCnt,0,sizeof(perfCnt));
    uint64_t lastPerfCnt[5]={0,0,0,0,0};
    now.tv_sec=0;
    int i;
    for(;;)
    {
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
      uint64_t perf[5];
      // success, error, reschedule
      smsc->getPerfData(perf);
      performance::PerformanceData d;
      for(i=0;i<performance::performanceCounters;i++)
      {
        d.counters[i].lastSecond=perf[i]-lastPerfCnt[i];
        d.counters[i].total=perf[i];
      }


      int scnt=(now.tv_sec-perfStart)/60;
      //__trace2__("SpeedMonitor: scnt=%d",scnt);
      if(scnt>=60)
      {
        timeshift++;
        if(timeshift>=60)timeshift=0;
        perfStart=times[timeshift];
        scnt=59;
        int idx=timeshift-1;
        if(idx<0)idx=59;
        times[idx]=now.tv_sec;
        for(i=0;i<performance::performanceCounters;i++)perfCnt[i][idx]=0;
      }
      if(scnt!=lastscnt)
      {
        times[scnt]=now.tv_sec;
        lastscnt=scnt;
      }
      for(int j=0;j<performance::performanceCounters;j++)
      {
        d.counters[j].average=0;
      }
      int idx=timeshift;
      for(i=0;i<=scnt;i++,idx++)
      {
        if(idx>=60)idx=0;
        if(i==scnt)
        {
          for(int j=0;j<performance::performanceCounters;j++)
          {
            perfCnt[j][idx]+=d.counters[j].lastSecond;
          }
        }
        for(int j=0;j<performance::performanceCounters;j++)
        {
          d.counters[j].average+=perfCnt[j][idx];
        }
      }
      int diff=now.tv_sec-times[timeshift];
      if(diff==0)diff=1;
      //__trace2__("ca=%d,ea=%d,ra=%d, time diff=%u",
      //  d.success.average,d.error.average,d.rescheduled.average,diff);

      for(i=0;i<performance::performanceCounters;i++)
      {
        d.counters[i].average/=diff;
      }

      d.now=now.tv_sec;
      d.uptime=now.tv_sec-start.tv_sec;

      perfListener->reportPerformance(&d);

      for(i=0;i<performance::performanceCounters;i++)
      {
        lastPerfCnt[i]=perf[i];
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
  int perfCnt[5][60];
  int timeshift;
  time_t times[60];
  timespec start;
  performance::PerformanceListener* perfListener;
  static Smsc* smsc;
};

Smsc* SpeedMonitor::smsc=NULL;

extern void loadRoutes(RouteManager* rm,smsc::util::config::route::RouteConfig& rc);

void Smsc::init(const SmscConfigs& cfg)
{
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.init");
  try{
  tp.preCreateThreads(15);
  //smsc::util::config::Manager::init("config.xml");
  //cfgman=&cfgman->getInstance();

  /*
    register SME's
  */
  //smsc::util::config::smeman::SmeManConfig smemancfg;
  //smemancfg.load("sme.xml");
  {
    log.info( "Registering SMEs" );
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
        si.typeOfNumber=rec->recdata.smppSme.typeOfNumber;
        si.numberingPlan=rec->recdata.smppSme.numberingPlan;
        si.interfaceVersion=rec->recdata.smppSme.interfaceVersion;
        si.rangeOfAddress=rec->recdata.smppSme.addrRange;
        si.systemType=rec->recdata.smppSme.systemType;
        si.password=rec->recdata.smppSme.password;
        si.systemId=rec->smeUid;
        si.timeout = rec->recdata.smppSme.timeout;
        si.wantAlias = rec->recdata.smppSme.wantAlias;
        si.forceDC = rec->recdata.smppSme.forceDC;
        si.receiptSchemeName= rec->recdata.smppSme.receiptSchemeName;
        if(si.rangeOfAddress.length() && !re.Compile(si.rangeOfAddress.c_str(),OP_OPTIMIZE|OP_STRICT))
        {
          log.error("Failed to compile rangeOfAddress for sme %s",si.systemId.c_str());
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

        try{
          smeman.addSme(si);
        }catch(...)
        {
          log.warn("UNABLE TO REGISTER SME:%s",si.systemId.c_str());
        }
      }
    }
    log.info( "SME registration done" );
  }
  // initialize aliases
  /*{
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
      aliaser.addAlias(ai);
    }
    aliaser.commit();
  }*/

  reloadAliases(cfg);
  log.info( "Aliases loaded" );
  reloadRoutes(cfg);
  log.info( "Routes loaded" );

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
  scheduler=new Scheduler(eventqueue,store);

  smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance(),scheduler);
  store=smsc::store::StoreManager::getMessageStore();

  mrCache.assignStore(store);

  {
    log.info( "Starting statemachines" );
    int cnt=cfg.cfgman->getInt("core.state_machines_count");
    time_t maxValidTime=cfg.cfgman->getInt("sms.max_valid_time");
    for(int i=0;i<cnt;i++)
    {
      StateMachine *m=new StateMachine(eventqueue,store,this);
      m->maxValidTime=maxValidTime;
      Address addr(cfg.cfgman->getString("core.service_center_address"));
      m->scAddress=addr;
      tp.startTask(m);
    }
    log.info( "Statemachines started" );
  }

  RescheduleCalculator::Init(cfg.cfgman->getString("core.reschedule_table"));

  {
    SpeedMonitor *sm=new SpeedMonitor(eventqueue,&perfDataDisp,this);
    FILE *f=fopen("stats.txt","rt");
    if(f)
    {
      time_t ut;
      fscanf(f,"%d %lld %lld %lld %lld %lld",
        &ut,
        &submitOkCounter,
        &submitErrCounter,
        &deliverOkCounter,
        &deliverErrCounter,
        &rescheduleCounter
      );
      startTime=time(NULL)-ut;
      sm->setStartTime(startTime);
      fclose(f);
      remove("stats.txt");
    }
    tp.startTask(sm);
    log.info( "Speedmonitor started" );
 }

  {
    using namespace smsc::db;
    using smsc::util::config::ConfigView;
    ConfigView *dsConfig;
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";


    dsConfig = new smsc::util::config::ConfigView(*cfg.cfgman, "StartupLoader");
    DataSourceLoader::loadup(dsConfig);

    dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    if (!dataSource) throw Exception("Failed to get DataSource");
    ConfigView* config =
        new ConfigView(*cfg.cfgman,"DataSource");

    dataSource->init(config);
    log.info( "Datasource configured" );
  }
  statMan=new smsc::stat::StatisticsManager(*dataSource);
  tp2.startTask(statMan);
  log.info( "Statistics manager started" );

  distlstman=new DistrListManager(*dataSource,*cfg.cfgman);

  distlstsme=new DistrListProcess(distlstman);
  tp.startTask(distlstsme);
  log.info( "Distribution list processor started" );

  smeman.registerInternallSmeProxy("DSTRLST",distlstsme);

  smsc::mscman::MscManager::startup(*dataSource,*cfg.cfgman);
  log.info( "MSC manager started" );

  smsc::resourcemanager::ResourceManager::init
  (
    cfg.cfgman->getString("core.locales"),
    cfg.cfgman->getString("core.default_locale")
  );
  log.info( "Resource manager configured" );

  log.info( "Starting profiler" );
  {
    char *rep=cfg.cfgman->getString("profiler.defaultReport");
    char *dc=cfg.cfgman->getString("profiler.defaultDataCoding");


    char *str=rep;
    while((*str=toupper(*str)))str++;
    str=dc;
    while((*str=toupper(*str)))str++;
    smsc::profiler::Profile defProfile={0,0,"",0,false};

    defProfile.hide=cfg.cfgman->getBool("profiler.defaultHide");
    defProfile.hideModifiable=cfg.cfgman->getBool("profiler.defaultHideModifiable");

    if(!strcmp(dc,"DEFAULT"))
      defProfile.codepage=profiler::ProfileCharsetOptions::Default;
    else if(!strcmp(dc,"UCS2"))
      defProfile.codepage=profiler::ProfileCharsetOptions::Ucs2;
    else
    {
      log.warn("Profiler:Unrecognized default data coding");
    }
    if(!strcmp(rep,"NONE"))
      defProfile.reportoptions=profiler::ProfileReportOptions::ReportNone;
    else if(!strcmp(rep,"FULL"))
      defProfile.reportoptions=profiler::ProfileReportOptions::ReportFull;
    else
    {
      log.warn("Profiler:Unrecognized default report options");
    }
    profiler=new smsc::profiler::Profiler(defProfile,
               &smeman,
               cfg.cfgman->getString("profiler.systemId"));

    profiler->serviceType=cfg.cfgman->getString("profiler.service_type");
    profiler->protocolId=cfg.cfgman->getInt("profiler.protocol_id");

    using smsc::util::config::CStrSet;
    CStrSet *params=cfg.cfgman->getChildIntParamNames("profiler.ussdOpsMapping");
    CStrSet::iterator i=params->begin();
    for(;i!=params->end();i++)
    {
      string pn="profiler.ussdOpsMapping.";
      pn+=*i;
      profiler->addToUssdCmdMap(cfg.cfgman->getInt(pn.c_str()),*i);
    }
    delete params;
  }
  log.info( "Profiler configured" );
  profiler->loadFromDB(dataSource);
  log.info( "Profiler data loaded" );

  tp2.startTask(profiler);
  log.info( "Profiler started" );

  try{
    smeman.registerInternallSmeProxy(
      cfg.cfgman->getString("profiler.systemId"),
      profiler);
  }catch(...)
  {
    log.warn("Failed to register profiler");
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
      log.info( "Abonent info started" );
    }catch(exception& e)
    {
      log.warn("Failed to register abonentinfo");
      __trace2__("Failed to register abonentinfo Sme:%s",e.what());
      __warning__("Failed to register abonentinfo Sme");
    }
    abonentInfoProxy=abonentInfo;
  }

  smscsme=new SmscSme("smscsme",&smeman);
  smscsme->servType=cfg.cfgman->getString("core.service_type");
  smscsme->protId=cfg.cfgman->getInt("core.protocol_id");
  tp2.startTask(smscsme);
  try{
    smeman.registerInternallSmeProxy(
      cfg.cfgman->getString("core.systemId"),
      smscsme);
    log.info( "SMSC sme started" );
  }catch(exception& e)
  {
    log.warn("Failed to register smscsme");
    __trace2__("Failed to register smscsme:%s",e.what());
    __warning__("Failed to register smscsme");
  }


  cancelAgent=new CancelAgent(eventqueue,store);
  tp.startTask(cancelAgent);
  log.info( "Cancel agent started" );

  alertAgent=new AlertAgent(eventqueue,store);
  tp.startTask(alertAgent);
  log.info( "Alert agent started" );


  smscHost=cfg.cfgman->getString("smpp.host");
  smscPort=cfg.cfgman->getInt("smpp.port");
  ssockman.setSmppSocketTimeout(cfg.cfgman->getInt("smpp.readTimeout"));
  ssockman.setInactivityTime(cfg.cfgman->getInt("smpp.inactivityTime"));
  ssockman.setInactivityTimeOut(cfg.cfgman->getInt("smpp.inactivityTimeOut"));

  log.info( "MR cache loaded" );

  {
    performance::PerformanceServer *perfSrv=new performance::PerformanceServer
    (
      cfg.cfgman->getString("core.performance.host"),
      cfg.cfgman->getInt("core.performance.port"),
      &perfDataDisp
    );
    tp2.startTask(perfSrv);
    log.info( "Performance server started" );
  }

  smsc::util::regexp::RegExp::InitLocale();
  Address addr(cfg.cfgman->getString("core.service_center_address"));
  AddressValue addrval;
  addr.getValue( addrval );
  scAddr = addrval;

  try{
    scheduler->setRescheduleLimit(cfg.cfgman->getInt("core.reschedule_limit"));
  }catch(...)
  {
    __warning__("reschedule_limit not found in config, using default");
  }

  log.info( "SMSC init complete" );

  }catch(exception& e)
  {
    __trace2__("Smsc::init exception:%s",e.what());
    log.warn("Smsc::init exception:%s",e.what());
    throw;
  }catch(...)
  {
    __trace2__("Smsc::init exception:unknown");
    log.warn("Smsc::init exception:unknown");
    throw;
  }
  __trace__("Smsc::init completed");
}

void Smsc::run()
{
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.run");
  __trace__("Smsc::run");
  try{
  if(startTime==0)startTime=time(NULL);
  {
    Event accstarted;
    smppio::SmppAcceptor *acc=new
      smppio::SmppAcceptor(
        smscHost.c_str(),
        smscPort,
        &ssockman,
        &accstarted
      );
    tp.startTask(acc);
    Event mapiostarted;
    MapIoTask* mapio = new MapIoTask(&mapiostarted,scAddr);
    tp.startTask(mapio);
    accstarted.Wait();
    mapiostarted.Wait();
    if(!acc->isStarted()||!mapio->isStarted())
    {
      throw Exception("Failed to start SMPP or MAP acceptor");
    }
//    tp.startTask(new MapTracker());
    MapDialogContainer::getInstance()->registerSelf(&smeman);
    mapProxy=MapDialogContainer::getInstance()->getProxy();
    MapDialogContainer::getInstance()->getProxy()->setId("MAP_PROXY");
  }


  // start rescheduler created in init
  tp.startTask(scheduler);

#ifdef ENABLE_MAP_SYM
  MapSymProxy *msprx=new MapSymProxy();

  smeman.registerInternallSmeProxy("MAPSYM",msprx);

  MapSymInputTask *msin=new MapSymInputTask(msprx);
  MapSymOutputTask *msout=new MapSymOutputTask(msprx);
  tp.startTask(msin);
  tp.startTask(msout);
#endif

  // некоторые действия до основного цикла
  mainLoop();
  // и после него
  //shutdown();
  }catch(exception& e)
  {
    __trace2__("Smsc::run exception:%s",e.what());
    log.warn("Smsc::run exception:%s",e.what());
    throw;
  }catch(...)
  {
    __trace2__("Smsc::run exception:unknown");
    log.warn("Smsc::run exception:unknown");
    throw;
  }
  __trace__("Smsc::run completed");
}

void Smsc::shutdown()
{
  __trace__("shutting down");

  smeman.unregisterSmeProxy("DSTRLST");

  tp.shutdown();
  tp2.shutdown();

  if(mapProxy)
  {
    MapDialogContainer::getInstance()->unregisterSelf(&smeman);
    MapDialogContainer::dropInstance();
  }

  delete distlstman;

  smsc::mscman::MscManager::shutdown();

  smsc::store::StoreManager::shutdown();
  if(dataSource)delete dataSource;
}

void Smsc::reloadRoutes(const SmscConfigs& cfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  router->assign(&smeman);
  try{
    loadRoutes(router.get(),*cfg.routesconfig);
  }catch(...)
  {
    __warning__("Failed to load routes");
  }
  ResetRouteManager(router.release());
}

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

};//system
};//smsc
