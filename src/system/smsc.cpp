#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include <memory>
#include "admin/util/SignalHandler.h"
#include "util/debug.h"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "system/rescheduler.hpp"
#include "util/config/route/RouteConfig.h"
#include "system/mapio/MapIoTask.h"
#include "system/abonentinfo/AbonentInfo.hpp"
#include "util/Logger.h"
#include "system/smscsme.hpp"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"

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


class SmscSignalHandler:public smsc::admin::util::SignalHandler{
public:
  SmscSignalHandler(Smsc *app):smsc(app){}
  void handleSignal()throw()
  {
    smsc->stop();
    //trace("got a signal!");
  }
protected:
  Smsc* smsc;
};

Smsc::~Smsc()
{
}

class SpeedMonitor:public smsc::core::threads::ThreadedTask{
public:
  SpeedMonitor(EventQueue& eq,performance::PerformanceListener* pl,Smsc* smsc):
    queue(eq),perfListener(pl),smsc(smsc){}
  int Execute()
  {
    uint64_t cnt,last=0;
    timespec start,now,lasttime;
    double ut,tm,rate,avg;
    clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    __trace__("enter SpeedMonitor");
    timeshift=0;
    time_t perfStart=start.tv_sec;
    times[0]=start.tv_sec;
    int lastscnt=0;
    memset(perfCnt,0,sizeof(perfCnt));
    uint64_t lastPerfCnt[4]={0,0,0};
    now.tv_sec=0;
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
      uint64_t perf[4];
      // success, error, reschedule
      smsc->getPerfData(perf[0],perf[1],perf[2],perf[3]);
      performance::PerformanceData d;
      d.submit.lastSecond=perf[0]-lastPerfCnt[0];
      d.success.lastSecond=perf[1]-lastPerfCnt[1];
      d.error.lastSecond=perf[2]-lastPerfCnt[2];
      d.rescheduled.lastSecond=perf[3]-lastPerfCnt[3];
      d.submit.total=perf[0];
      d.success.total=perf[1];
      d.error.total=perf[2];
      d.rescheduled.total=perf[3];


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
        perfCnt[0][idx]=0;
        perfCnt[1][idx]=0;
        perfCnt[2][idx]=0;
        perfCnt[3][idx]=0;
      }
      if(scnt!=lastscnt)
      {
        times[scnt]=now.tv_sec;
        lastscnt=scnt;
      }
      d.submit.average=0;
      d.success.average=0;
      d.error.average=0;
      d.rescheduled.average=0;
      int idx=timeshift;
      for(int i=0;i<=scnt;i++,idx++)
      {
        if(idx>=60)idx=0;
        if(i==scnt)
        {
          perfCnt[0][idx]+=d.submit.lastSecond;
          perfCnt[1][idx]+=d.success.lastSecond;
          perfCnt[2][idx]+=d.error.lastSecond;
          perfCnt[3][idx]+=d.rescheduled.lastSecond;
        }
        d.submit.average+=perfCnt[0][idx];
        d.success.average+=perfCnt[1][idx];
        d.error.average+=perfCnt[2][idx];
        d.rescheduled.average+=perfCnt[3][idx];
      }
      int diff=now.tv_sec-times[timeshift];
      if(diff==0)diff=1;
      //__trace2__("ca=%d,ea=%d,ra=%d, time diff=%u",
      //  d.success.average,d.error.average,d.rescheduled.average,diff);

      d.submit.average/=diff;
      d.success.average/=diff;
      d.error.average/=diff;
      d.rescheduled.average/=diff;

      d.now=now.tv_sec;
      d.uptime=now.tv_sec-start.tv_sec;

      perfListener->reportPerformance(&d);

      lastPerfCnt[0]=perf[0];
      lastPerfCnt[1]=perf[1];
      lastPerfCnt[2]=perf[2];
      lastPerfCnt[3]=perf[3];
    }
    return 0;
  }
  const char* taskName()
  {
    return "SpeedMonitor";
  }
protected:
  EventQueue& queue;
  int perfCnt[4][60];
  int timeshift;
  time_t times[60];
  performance::PerformanceListener* perfListener;
  Smsc* smsc;
};

extern void loadRoutes(RouteManager* rm,smsc::util::config::route::RouteConfig& rc);

void Smsc::init(const SmscConfigs& cfg)
{
  tp.preCreateThreads(15);
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.init");

  //smsc::util::config::Manager::init("config.xml");
  //cfgman=&cfgman->getInstance();

  /*
    register SME's
  */
  //smsc::util::config::smeman::SmeManConfig smemancfg;
  //smemancfg.load("sme.xml");
  {
    smsc::util::config::smeman::SmeManConfig::RecordIterator i=cfg.smemanconfig->getRecordIterator();
    smsc::util::regexp::RegExp re;
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
        if(si.rangeOfAddress.length() && !re.Compile(si.rangeOfAddress.c_str(),OP_OPTIMIZE|OP_STRICT))
        {
          log.error("Failed to compile rangeOfAddress for sme %s",si.systemId.c_str());
        }
        __trace2__("INIT: addSme %s(to=%d,wa=%s)",si.systemId.c_str(),si.timeout,si.wantAlias?"true":"false");
        //si.hostname=rec->recdata->smppSme.
        si.disabled=false;
        try{
          smeman.addSme(si);
        }catch(...)
        {
          log.warn("UNABLE TO REGISTER SME:%s",si.systemId.c_str());
        }
      }
    }
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
  reloadRoutes(cfg);

  /*auto_ptr<RouteManager> router(new RouteManager());

  // initialize router (all->all)
  router->assign(&smeman);
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

  smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance());
  store=smsc::store::StoreManager::getMessageStore();

  {
    int cnt=cfg.cfgman->getInt("core.state_machines_count");
    time_t maxValidTime=cfg.cfgman->getInt("sms.max_valid_time");
    for(int i=0;i<cnt;i++)
    {
      StateMachine *m=new StateMachine(eventqueue,store,this);
      m->maxValidTime=maxValidTime;
      try{
        m->initFormatters(
          cfg.cfgman->getString("core.receipt_delivered"),
          cfg.cfgman->getString("core.receipt_failed"),
          cfg.cfgman->getString("core.receipt_notify"));
      }catch(exception& e)
      {
        log.warn("INIT: Delivery receipts init failed:%s",e.what());
        __warning__("INIT: Delivery receipts init failed");
      }
      Address addr(cfg.cfgman->getString("core.service_center_address"));
      m->scAddress=addr;
      tp.startTask(m);
    }
  }

  RescheduleCalculator::Init(cfg.cfgman->getString("core.reschedule_table"));

  //smsc::admin::util::SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));

  tp.startTask(new SpeedMonitor(eventqueue,&perfDataDisp,this));

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
  }
  statMan=new smsc::stat::StatisticsManager(*dataSource);
  tp.startTask(statMan);



  {
    char *rep=cfg.cfgman->getString("profiler.defaultReport");
    char *dc=cfg.cfgman->getString("profiler.defaultDataCoding");
    char *str=rep;
    while((*str=toupper(*str)))str++;
    str=dc;
    while((*str=toupper(*str)))str++;
    smsc::profiler::Profile defProfile={0,0};
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

    profiler->msgRepNone=cfg.cfgman->getString("profiler.msgReportNone");
    profiler->msgRepFull=cfg.cfgman->getString("profiler.msgReportFull");
    profiler->msgDCDef=cfg.cfgman->getString("profiler.msgDataCodingDefault");
    profiler->msgDCUCS2=cfg.cfgman->getString("profiler.msgDataCodingUCS2");
    profiler->msgError=cfg.cfgman->getString("profiler.msgError");
    profiler->serviceType=cfg.cfgman->getString("profiler.service_type");
    profiler->protocolId=cfg.cfgman->getInt("profiler.protocol_id");
  }
  profiler->loadFromDB(dataSource);

  tp.startTask(profiler);

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
    smsc::system::abonentinfo::AbonentInfoSme *ai=
      new smsc::system::abonentinfo::AbonentInfoSme(this,
        cfg.cfgman->getString("abonentinfo.systemId"));
    ai->servType=cfg.cfgman->getString("abonentinfo.service_type");
    ai->protId=cfg.cfgman->getInt("abonentinfo.protocol_id");
    tp.startTask(ai);
    try{
      smeman.registerInternallSmeProxy(
        cfg.cfgman->getString("abonentinfo.systemId"),
        ai);
    }catch(exception& e)
    {
      log.warn("Failed to register abonentinfo");
      __trace2__("Failed to register abonentinfo Sme:%s",e.what());
      __warning__("Failed to register abonentinfo Sme");
    }
  }

  smscsme=new SmscSme("smscsme",&smeman);
  smscsme->servType=cfg.cfgman->getString("core.service_type");
  smscsme->protId=cfg.cfgman->getInt("core.protocol_id");
  tp.startTask(smscsme);
  try{
    smeman.registerInternallSmeProxy(
      cfg.cfgman->getString("core.systemId"),
      smscsme);
  }catch(exception& e)
  {
    log.warn("Failed to register smscsme");
    __trace2__("Failed to register smscsme:%s",e.what());
    __warning__("Failed to register smscsme");
  }

  cancelAgent=new CancelAgent(eventqueue,store);
  tp.startTask(cancelAgent);

  alertAgent=new AlertAgent(eventqueue,store);
  tp.startTask(alertAgent);


  smscHost=cfg.cfgman->getString("smpp.host");
  smscPort=cfg.cfgman->getInt("smpp.port");
  ssockman.setSmppSocketTimeout(cfg.cfgman->getInt("smpp.readTimeout"));
  ssockman.setInactivityTime(cfg.cfgman->getInt("smpp.inactivityTime"));

  {
    performance::PerformanceServer *perfSrv=new performance::PerformanceServer
    (
      cfg.cfgman->getString("core.performance.host"),
      cfg.cfgman->getInt("core.performance.port"),
      &perfDataDisp
    );
    tp.startTask(perfSrv);
  }

  smsc::util::regexp::RegExp::InitLocale();
}

void Smsc::run()
{
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
    MapIoTask* mapio = new MapIoTask(&mapiostarted);
    tp.startTask(mapio);
    accstarted.Wait();
    mapiostarted.Wait();
    if(!acc->isStarted()||!mapio->isStarted())
    {
      throw Exception("Failed to start SMPP or MAP acceptor");
    }
    tp.startTask(new MapTracker());
    MapDialogContainer::getInstance()->registerSelf(&smeman);
  }

  scheduler=new Scheduler(eventqueue,store);
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
}

void Smsc::shutdown()
{
  __trace__("shutting down");
  tp.shutdown();
  smsc::store::StoreManager::shutdown();
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
