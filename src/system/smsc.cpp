#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include <memory>
#include "util/debug.h"
#include "store/StoreManager.h"
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
#include "system/mapio/MapIoTask.h"
#include "system/abonentinfo/AbonentInfo.hpp"
#include "mscman/MscManager.h"
#include "resourcemanager/ResourceManager.hpp"
#include <typeinfo>
#include "system/status_sme.hpp"

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
using namespace smsc::snmp;
using namespace smsc::core::synchronization;
using util::Exception;


Smsc::~Smsc()
{
  SaveStats();
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
    uint64_t lastPerfCnt[performance::performanceCounters]={0,};
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
        d.counters[i].lastSecond=perf[i]-lastPerfCnt[i];
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

      d.eventQueueSize=equnl;
      d.inProcessingCount=eqhash-equnl;

      d.inScheduler=smsc->GetSchedulerCount();

      perfListener->reportGenPerformance(&d);

      for(i=0;i<performance::performanceCounters;i++)
      {
        lastPerfCnt[i]=perf[i];
      }

      uint32_t smePerfDataSize = 0;
      std::auto_ptr<uint8_t> smePerfData(smsc->getSmePerfData(smePerfDataSize));
      perfSmeListener->reportSmePerformance(smePerfData.get(), smePerfDataSize);
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
  int timeshift;
  time_t times[60];
  timespec start;
  performance::PerformanceListener* perfListener;
  performance::PerformanceListener* perfSmeListener;
  static Smsc* smsc;
};

Smsc* SpeedMonitor::smsc=NULL;

extern void loadRoutes(RouteManager* rm,const smsc::util::config::route::RouteConfig& rc,bool traceit=false);

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
  scheduler=new Scheduler(eventqueue);


  smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance(),0);
  store=smsc::store::StoreManager::getMessageStore();
  smsc_log_info(log, "Initializing scheduler" );
  scheduler->Init(store,this);

  smeman.registerInternallSmeProxy("scheduler",scheduler);

  //tp.startTask(scheduler);

  smsc_log_info(log, "Scheduler initialized" );
  smsc_log_info(log, "Initializing MR cache" );
  mrCache.assignStore(store);
  smsc_log_info(log, "MR cache inited" );

  {
    using smsc::util::config::CStrSet;
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

  {
    smsc_log_info(log, "Starting statemachines" );
    int cnt=cfg.cfgman->getInt("core.state_machines_count");
    time_t maxValidTime=cfg.cfgman->getInt("sms.max_valid_time");
    for(int i=0;i<cnt;i++)
    {
      StateMachine *m=new StateMachine(eventqueue,store,this);
      m->maxValidTime=maxValidTime;
      Address addr(cfg.cfgman->getString("core.service_center_address"));
      m->scAddress=addr;
      m->setReceiptInfo(
        cfg.cfgman->getString("core.service_type"),
        cfg.cfgman->getInt("core.protocol_id"),
        cfg.cfgman->getString("core.systemId")
      );
      tp.startTask(m);
    }
    smsc_log_info(log, "Statemachines started" );
  }

  RescheduleCalculator::InitDefault(cfg.cfgman->getString("core.reschedule_table"));
  {
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
  }

  {
    SpeedMonitor *sm=new SpeedMonitor(eventqueue,&perfDataDisp,&perfSmeDataDisp,this);
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
    tp.startTask(sm);
    smsc_log_info(log, "Speedmonitor started" );
  }
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
  statMan=new smsc::stat::StatisticsManager(*dataSource);
  tp2.startTask(statMan);
  smsc_log_info(log, "Statistics manager started" );

  distlstman=new DistrListManager(*dataSource,*cfg.cfgman);

  distlstsme=new DistrListProcess(distlstman);
  tp.startTask(distlstsme);
  smsc_log_info(log, "Distribution list processor started" );

  smeman.registerInternallSmeProxy("DSTRLST",distlstsme);

  smsc::mscman::MscManager::startup(*dataSource,*cfg.cfgman);
  smsc_log_info(log, "MSC manager started" );

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
    char *rep=cfg.cfgman->getString("profiler.defaultReport");
    char *dc=cfg.cfgman->getString("profiler.defaultDataCoding");


    char *str=rep;
    while((*str=toupper(*str)))str++;
    str=dc;
    while((*str=toupper(*str)))str++;
    smsc::profiler::Profile defProfile;
    defProfile.locale=cfg.cfgman->getString("core.default_locale");

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
  smsc_log_info(log, "Profiler configured" );
  profiler->loadFromDB(dataSource);
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
    smsc_log_info(log, "SMSC sme started" );
  }catch(exception& e)
  {
    smsc_log_warn(log, "Failed to register smscsme");
    __trace2__("Failed to register smscsme:%s",e.what());
    __warning__("Failed to register smscsme");
  }


  cancelAgent=new CancelAgent(eventqueue,store);
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

  {
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
  }

  eventQueueLimit=1000;
  try{
    eventQueueLimit=cfg.cfgman->getInt("core.eventQueueLimit");
  }catch(...)
  {
    __warning__("eventQueueLimit not found, using default(1000)");
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
    Event mapiostarted;
    MapIoTask* mapio = new MapIoTask(&mapiostarted,scAddr,ussdCenterAddr,ussdSSN);
    tp.startTask(mapio);
    mapiostarted.Wait();
    __trace__("MAPIO started");
    if(!acc->isStarted()||!mapio->isStarted())
    {
      throw Exception("Failed to start SMPP or MAP acceptor");
    }
//    tp.startTask(new MapTracker());
#ifndef NOMAPPROXY
    MapDialogContainer::getInstance()->registerSelf(&smeman);
#endif
    mapProxy=MapDialogContainer::getInstance()->getProxy();
    SmeInfo si=smeman.getSmeInfo(smeman.lookup("MAP_PROXY"));
    mapProxy->updateSmeInfo( si );
    MapDialogContainer::getInstance()->getProxy()->setId("MAP_PROXY");
  }


  // start rescheduler created in init
  // start on thread pool 2 to shutdown it after state machines
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

  delete smscsme;
  smeman.unregisterSmeProxy("DSTRLST");

  tp.shutdown();

  smeman.unregisterSmeProxy("scheduler");

  tp2.shutdown();



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

void Smsc::reloadTestRoutes(const RouteConfig& rcfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  router->assign(&smeman);
  loadRoutes(router.get(),rcfg,true);
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

}//system
}//smsc
