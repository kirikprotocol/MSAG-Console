#include "smppgw/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include <memory>
#include "util/debug.h"
#include "smppgw/state_machine.hpp"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "system/common/rescheduler.hpp"
#include "util/config/route/RouteConfig.h"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"
#include <typeinfo>
#include "gwsme.hpp"
#include "smppgw/billing/bill.hpp"

namespace smsc{
namespace system{
using namespace smsc::router;
extern void loadRoutes(RouteManager* rm,const smsc::util::config::route::RouteConfig& rc,bool traceit=false);
}

namespace smppgw{

using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::snmp;
using namespace smsc::core::synchronization;
using util::Exception;


Smsc::~Smsc()
{
  SaveStats();
}

class SpeedMonitor:public smsc::core::threads::ThreadedTask{
public:
  SpeedMonitor(EventQueue& eq,smsc::smppgw::performance::PerformanceListener* pl,Smsc* psmsc):
    queue(eq),perfListener(pl)
  {
    start.tv_sec=0;
    start.tv_nsec=0;
    smsc=psmsc;
  }
  int Execute()
  {
    uint64_t cnt,last=0;
    timespec now={0,0},lasttime={0,0};
    double ut,tm,rate,avg;
    if(start.tv_sec==0)
      clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    __trace__("enter SpeedMonitor");
    timeshift=0;
    time_t perfStart=start.tv_sec;
    for(int i=0;i<60;i++)times[i]=start.tv_sec;
    int lastscnt=0;
    memset(perfCnt,0,sizeof(perfCnt));
    uint64_t lastPerfCnt[smsc::smppgw::performance::performanceCounters]={0,};
    //now.tv_sec=0;
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
      queue.getStats(equnl);
      clock_gettime(CLOCK_REALTIME,&now);
      ut=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (start.tv_sec*1000.0+start.tv_nsec/1000000.0))/1000.0;
      tm=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (lasttime.tv_sec*1000.0+lasttime.tv_nsec/1000000.0))/1000;
      rate=(cnt-last)/tm;
      avg=cnt/ut;
      printf("UT:%.3lf AVG:%.3lf LAST:%.3lf (%llu)[%d]         \r",ut,avg,rate,cnt,equnl);
      fflush(stdout);
      last=cnt;
      lasttime=now;
      if(isStopping)break;
      uint64_t perf[smsc::smppgw::performance::performanceCounters];
      // success, error, reschedule
      smsc->getPerfData(perf);
      smsc::smppgw::performance::PerformanceData d;
      d.countersNumber=smsc::smppgw::performance::performanceCounters;
      for(i=0;i<smsc::smppgw::performance::performanceCounters;i++)
      {
        d.counters[i].lastSecond=(int)(perf[i]-lastPerfCnt[i]);
        d.counters[i].total=perf[i];
      }


      int scnt=(now.tv_sec-perfStart)/60;
      //__trace2__("SpeedMonitor: scnt=%d",scnt);
      if(scnt<0)scnt=0;
      if(scnt>=60)
      {
        timeshift++;
        if(timeshift>=60)timeshift=0;
        perfStart=times[timeshift];
        scnt=59;
        int idx=timeshift-1;
        if(idx<0)idx=59;
        times[idx]=now.tv_sec;
        for(i=0;i<smsc::smppgw::performance::performanceCounters;i++)perfCnt[i][idx]=0;
      }
      if(scnt!=lastscnt)
      {
        times[scnt]=now.tv_sec;
        lastscnt=scnt;
      }
      for(int j=0;j<smsc::smppgw::performance::performanceCounters;j++)
      {
        d.counters[j].average=0;
      }
      int idx=timeshift;
      for(i=0;i<=scnt;i++,idx++)
      {
        if(idx>=60)idx=0;
        if(i==scnt)
        {
          for(int j=0;j<smsc::smppgw::performance::performanceCounters;j++)
          {
            perfCnt[j][idx]+=d.counters[j].lastSecond;
          }
        }
        for(int j=0;j<smsc::smppgw::performance::performanceCounters;j++)
        {
          d.counters[j].average+=perfCnt[j][idx];
        }
      }
      int diff=now.tv_sec-times[timeshift];
      if(diff==0)diff=1;
      //__trace2__("ca=%d,ea=%d,ra=%d, time diff=%u",
      //  d.success.average,d.error.average,d.rescheduled.average,diff);

      for(i=0;i<smsc::smppgw::performance::performanceCounters;i++)
      {
        d.counters[i].average/=diff;
      }

      d.now=now.tv_sec;
      d.uptime=now.tv_sec-start.tv_sec;

      d.eventQueueSize=equnl;

      perfListener->reportGenPerformance(&d);

      for(i=0;i<smsc::smppgw::performance::performanceCounters;i++)
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
  int perfCnt[smsc::smppgw::performance::performanceCounters][60];
  int timeshift;
  time_t times[60];
  timespec start;
  smsc::smppgw::performance::PerformanceListener* perfListener;
  static Smsc* smsc;
};

Smsc* SpeedMonitor::smsc=NULL;

void Smsc::init(const SmscConfigs& cfg)
{
  smsc::util::regexp::RegExp::InitLocale();
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smsc.init");
  try{
  InitLicense(*cfg.licconfig);
  tp.preCreateThreads(15);
  //smsc::util::config::Manager::init("config.xml");
  //cfgman=&cfgman->getInstance();

#ifdef SNMP
  {
    snmpAgent = new SnmpAgent(this);
    tp2.startTask(snmpAgent);
    snmpAgent->statusChange(SnmpAgent::INIT);
  }
#endif

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
  smsc_log_info(log, "Aliases loaded" );
  reloadRoutes(cfg);
  smsc_log_info(log, "Routes loaded" );

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

  smsc_log_info(log, "Scheduler initialized" );
  smsc_log_info(log, "Initializing MR cache" );
  //mrCache.assignStore(store);
  smsc_log_info(log, "MR cache inited" );

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


  StateMachine::dataSource=dataSource;

  {
    billing::InitBillingInterface(cfg.cfgman->getString("billing.module"));
  }


  {
    smsc_log_info(log, "Starting statemachines" );
    int cnt=cfg.cfgman->getInt("core.state_machines_count");
    for(int i=0;i<cnt;i++)
    {
      StateMachine *m=new StateMachine(eventqueue,this);
      tp.startTask(m);
    }
    smsc_log_info(log, "Statemachines started" );
  }

  {
    SpeedMonitor *sm=new SpeedMonitor(eventqueue,&perfDataDisp,this);
    FILE *f=fopen("stats.txt","rt");
    if(f)
    {
      time_t ut;
      fscanf(f,"%d %lld %lld %lld %lld %lld %lld",
        &ut,
        &acceptedCounter,
        &rejectedCounter,
        &deliveredCounter,
        &deliverErrCounter,
        &transOkCounter,
        &transFailCounter
      );
      startTime=time(NULL)-ut;
      sm->setStartTime(startTime);
      fclose(f);
      remove("stats.txt");
    }
    tp.startTask(sm);
    smsc_log_info(log, "Speedmonitor started" );
  }

  statMan=new smsc::smppgw::stat::GWStatisticsManager(*dataSource);
  tp2.startTask(statMan);
  smsc_log_info(log, "Statistics manager started" );


  smscHost=cfg.cfgman->getString("smpp.host");
  smscPort=cfg.cfgman->getInt("smpp.port");
  ssockman.setSmppSocketTimeout(cfg.cfgman->getInt("smpp.readTimeout"));
  ssockman.setInactivityTime(cfg.cfgman->getInt("smpp.inactivityTime"));
  ssockman.setInactivityTimeOut(cfg.cfgman->getInt("smpp.inactivityTimeOut"));

  {
    smsc::smppgw::performance::PerformanceServer *perfSrv=new smsc::smppgw::performance::PerformanceServer
    (
      cfg.cfgman->getString("core.performance.host"),
      cfg.cfgman->getInt("core.performance.port"),
      &perfDataDisp
    );
    tp2.startTask(perfSrv);
    smsc_log_info(log, "Performance server started" );
  }

  eventQueueLimit=1000;
  try{
    eventQueueLimit=cfg.cfgman->getInt("core.eventQueueLimit");
  }catch(...)
  {
    __warning__("eventQueueLimit not found, using default(1000)");
  }

  {
    using smsc::util::config::ConfigView;
    using smsc::util::config::CStrSet;
    std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.cfgman,"smsc-connections"));

    std::auto_ptr<CStrSet> conn(cv->getShortSectionNames());

    for(CStrSet::iterator it=conn->begin();it!=conn->end();it++)
    {
      smsc_log_info(log, "Loading smsc connection %s",it->c_str());
      smsc::sme::SmeConfig gwcfg;
      std::auto_ptr<char> val;
      gwcfg.host=(val=std::auto_ptr<char>(cv->getString((*it+".host").c_str()))).get();
      gwcfg.port=cv->getInt((*it+".port").c_str());
      gwcfg.sid=(val=std::auto_ptr<char>(cv->getString((*it+".systemId").c_str()))).get();;
      gwcfg.password=(val=std::auto_ptr<char>(cv->getString((*it+".password").c_str()))).get();;
      gwcfg.smppTimeOut=cv->getInt((*it+".responseTimeout").c_str());
      GatewaySme *gwsme=new GatewaySme(gwcfg,&smeman,cv->getString((*it+".altHost").c_str()),cv->getInt((*it+".altPort").c_str()));
      gwsme->setId(*it,smeman.lookup(*it));
      uint8_t uid=cv->getInt((*it+".uniqueMsgIdPrefix").c_str());
      if(gwSmeMap[uid])
      {
        throw Exception("Duplicate gwsmeid %d!",(int)uid);
      }
      gwSmeMap[uid]=gwsme;
      gwsme->setPrefix(uid);
      smeman.registerInternallSmeProxy(*it,gwsme);
      tp.startTask(gwsme);
    }
  }

  /*
  try{
    scheduler->setRescheduleLimit(cfg.cfgman->getInt("core.reschedule_limit"));
  }catch(...)
  {
    __warning__("reschedule_limit not found in config, using default");
  }
  */

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


void Smsc::run()
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.run");
  //smsc::logger::Logger::getInstance("sms.snmp.alarm").debug("sample alarm");

  __trace__("Smsc::run");

  try{
  if(startTime==0)startTime=time(NULL);
  {
    __trace__("Starting SMPPIO");
    Event accstarted;
    smsc::system::smppio::SmppAcceptor *acc=new
      smsc::system::smppio::SmppAcceptor(
        smscHost.c_str(),
        smscPort,
        &ssockman,
        &accstarted
      );
    tp.startTask(acc);
    __trace__("Waiting SMPPIO started");
    accstarted.Wait();
    __trace__("SMPPIO started");
    if(!acc->isStarted())
    {
      throw Exception("Failed to start SMPP or MAP eduler created in init");
    }
  }
  // start on thread pool 2 to shutdown it after state machines

#ifdef SNMP
    __trace__("Smsc::changing SNMP state to OPER");
  if(snmpAgent) snmpAgent->statusChange(SnmpAgent::OPER);
    __trace__("Smsc::SNMP state to OPER changed");
#endif

  // некоторые действия до основного цикла
  mainLoop();

#ifdef SNMP
    __trace__("Smsc::changing SNMP state to SHUT");
  if(snmpAgent) snmpAgent->statusChange(SnmpAgent::SHUT);
    __trace__("Smsc::SNMP state to SHUT changed");
#endif

  // и после него
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

  tp.shutdown();

  tp2.shutdown();

  if(dataSource)delete dataSource;
}

void Smsc::reloadRoutes(const SmscConfigs& cfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  router->assign(&smeman);
  try{
    smsc::system::loadRoutes(router.get(),*cfg.routesconfig);
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
  smsc::system::loadRoutes(router.get(),rcfg,true);
  ResetTestRouteManager(router.release());
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

}//smppgw
}//smsc
