#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <typeinfo>
#include <unistd.h>

#include "logger/Logger.h"
#include "core/synchronization/Event.hpp"
#include "util/debug.h"
#include "util/Exception.hpp"
#include "config/route/RouteConfig.h"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"
#include "util/smstext.h"
#include "util/findConfigFile.h"
#include "smppio/SmppAcceptor.hpp"
#include "statemachine/state_machine.hpp"
#include "common/rescheduler.hpp"
#include "abonentinfo/AbonentInfo.hpp"
#include "smscsme/smscsme.hpp"
#ifdef USE_MAP
#include "mapio/MapIoTask.h"
#endif
#include "smsc/mscman/MscManager.h"
#include "smsc/resourcemanager/ResourceManager.hpp"
#include "agents/status_sme.hpp"
#include "profiler/profile-notifier.hpp"
#include "closedgroups/ClosedGroupsManager.hpp"
#include "common/TimeZoneMan.hpp"
#include "smsc/alias/AliasManImpl.hpp"
#include "mapio/FraudControl.hpp"
#include "mapio/MapLimits.hpp"
#include "license/check/license.hpp"
#include "cluster/controller/NetworkDispatcher.hpp"
#ifdef SMSEXTRA
#include "Extra.hpp"
#include "ExtraBits.hpp"
#endif

#include "smsc.hpp"
#include "smsc/cluster/controller/protocol/messages/LockConfig.hpp"
#include "smsc/cluster/controller/protocol/messages/UnlockConfig.hpp"
#include "smsc/cluster/controller/ConfigLockGuard.hpp"
#include "smsc/interconnect/ClusterInterconnect.hpp"
#include "smsc/configregistry/ConfigRegistry.hpp"

namespace smsc{

using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::snmp;
using namespace smsc::core::synchronization;
using namespace smsc::profiler;
using util::Exception;
using smsc::acls::AclAbstractMgr;


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
               PerformanceListener* pl, PerformanceListener* plsme,
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
    uint64_t lastPerfCnt[performanceCounters]={0,};
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
        uint64_t perf[performanceCounters];
        // success, error, reschedule
        smsc->getPerfData(perf);
        PerformanceData d;
        d.countersNumber=performanceCounters;
        for(i=0;i<performanceCounters;i++)
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
          for(i=0;i<performanceCounters;i++)perfCnt[i][curIdx]=0;
        }
        if(mcnt>=60)
        {
          mcnt=59;
          secs=60*60-(60-secs%60);
        }
        for(int j=0;j<performanceCounters;j++)
        {
          d.counters[j].average=0;
        }
        int idx=curIdx;

        for(int j=0;j<performanceCounters;j++)
        {
          perfCnt[j][curIdx]+=d.counters[j].lastSecond;
        }
        for(i=0;i<=mcnt;i++,idx--)
        {
          if(idx<0)idx=59;
          for(int j=0;j<performanceCounters;j++)
          {
            d.counters[j].average+=perfCnt[j][idx];
          }
        }

        if(secs)
        {
          for(i=0;i<performanceCounters;i++)
          {
            d.counters[i].average/=secs;
          }
        }

        d.now=now.tv_sec;
        d.uptime=now.tv_sec-start.tv_sec;

        d.eventQueueSize=equnl;
        d.inProcessingCount=eqhash-equnl;

        scheduler::Scheduler::SchedulerCounts cnts;
        smsc->getScheduler()->getCounts(cnts);
        d.inScheduler=cnts.timeLineCount+cnts.firstTimeCount;
        d.dpfCount=smsc->getScheduler()->getDpfCount();
        perfListener->reportGenPerformance(&d);

        for(i=0;i<performanceCounters;i++)
        {
          lastPerfCnt[i]=perf[i];
        }

        uint32_t smePerfDataSize = 0;
        std::auto_ptr<uint8_t> smePerfData(smsc->getSmePerfData(smePerfDataSize));
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
  int perfCnt[performanceCounters][60];
  timespec start;
  PerformanceListener* perfListener;
  PerformanceListener* perfSmeListener;
  static Smsc* smsc;
};

Smsc* SpeedMonitor::smsc=NULL;

namespace common{
extern void loadRoutes(smsc::router::RouteManager* rm,const smsc::config::route::RouteConfig& rc,bool traceit=false);
}

void Smsc::init(const SmscConfigs& cfg, int nodeIdx)
{
  configs=&cfg;


  smsc::util::regexp::RegExp::InitLocale();
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smsc.init");
  try{
    tp.preCreateThreads(15);

    nodeIndex=nodeIdx;
    nodesCount=cfg.cfgman->getInt("cluster.nodesCount");

    smsc::interconnect::ClusterInterconnect::Init(nodeIndex);

    for(int i=0;i<nodesCount;i++)
    {
      char bufHost[64];
      char bufPort[64];
      sprintf(bufHost,"cluster.interconnect.host%d",i+1);
      sprintf(bufPort,"cluster.interconnect.port%d",i+1);
      smsc::interconnect::ClusterInterconnect::getInstance()->addNode(cfg.cfgman->getString(bufHost),cfg.cfgman->getInt(bufPort));
    }
    smsc::interconnect::ClusterInterconnect::getInstance()->Start();

#ifdef SNMP
    {
      snmpAgent = new SnmpAgent(this);
      smsc::snmp::smestattable::SmeStatTableSubagent::Init(&smeman,&smeStats);
      tp2.startTask(snmpAgent);
      snmpAgent->statusChange(SnmpAgent::INIT);
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctSnmp);
      SnmpCounter::Init(findConfigFile("snmp.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctSnmp);
    }
#endif

    {
      smsc::cluster::controller::ConfigLockGuard clgTz(eyeline::clustercontroller::ctTimeZones);
      smsc::cluster::controller::ConfigLockGuard clgRt(eyeline::clustercontroller::ctRoutes);
      std::string tzCfg=findConfigFile("timezones.xml");
      std::string rtCfg=findConfigFile("routes.xml");
      common::TimeZoneManager::Init(tzCfg.c_str(),rtCfg.c_str());
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctTimeZones);
    }

    {
      smsc_log_info(log, "Registering SMEs" );
      smsc::config::smeman::SmeManConfig::RecordIterator i=cfg.smemanconfig->getRecordIterator();
      using namespace smsc::util::regexp;
      RegExp re;
      while(i.hasRecord())
      {
        smsc::config::smeman::SmeRecord *rec;
        i.fetchNext(rec);
        SmeInfo si;
        if(rec->rectype==smsc::config::smeman::SMPP_SME)
        {
          si.priority=rec->priority;

          si.typeOfNumber=rec->recdata.smppSme.typeOfNumber;
          si.numberingPlan=rec->recdata.smppSme.numberingPlan;
          si.interfaceVersion=rec->recdata.smppSme.interfaceVersion;
          si.rangeOfAddress=rec->recdata.smppSme.addrRange;
          si.systemType=rec->recdata.smppSme.systemType;
          si.password=rec->recdata.smppSme.password;
          si.systemId=rec->smeUid;
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
          si.disabled=rec->recdata.smppSme.disabled;
          using namespace smsc::config::smeman;
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
    }catch(std::exception& e)
    {
      smsc_log_warn(log,"noDivertSme init exception:%s",e.what());
    }

    try{
      RouteManager::EnableSmeRouters(cfg.cfgman->getBool("core.srcSmeSeparateRouting"));
    }catch(...)
    {
      __warning__("src sme routing disabled by default");
    }

    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctAliases);
      aliaser=new smsc::alias::AliasManImpl(cfg.cfgman->getString("aliasman.storeFile"));
      aliaser->Load();
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctAliases);
    }
    smsc_log_info(log, "Aliases loaded" );

    reloadRoutes();
    smsc_log_info(log, "Routes loaded" );

    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctFraud);
      mapio::FraudControl::Init(findConfigFile("fraud.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctFraud);
    }
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctMapLimits);
      mapio::MapLimits::Init(findConfigFile("maplimits.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctMapLimits);
    }


    scheduler=new scheduler::Scheduler();

    char dpfStoreCfg[64];
    sprintf(dpfStoreCfg,"dpf.storeDir%d",nodeIndex);
    scheduler->InitDpfTracker(cfg.cfgman->getString(dpfStoreCfg),
                              cfg.cfgman->getInt("dpf.timeOut1179"),
                              cfg.cfgman->getInt("dpf.timeOut1044"),
                              cfg.cfgman->getInt("dpf.maxChanges"),
                              cfg.cfgman->getInt("dpf.maxTime"));

    schedulerSoftLimit=cfg.cfgman->getInt("core.schedulerSoftLimit");
    schedulerHardLimit=cfg.cfgman->getInt("core.schedulerHardLimit");

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


    store=scheduler;
    smsc_log_info(log, "Initializing scheduler" );
    scheduler->DelayInit(this,cfg.cfgman);

    smeman.registerInternallSmeProxy("scheduler",scheduler);

    smeman.registerInternallSmeProxy("CLSTRICON",smsc::interconnect::ClusterInterconnect::getInstance());


    smsc_log_info(log, "Scheduler initialized" );

    inManCom=new inmancomm::INManComm(&smeman);
    char inmanHost[64];
    char inmanPort[64];
    sprintf(inmanHost,"inman.host%d",nodeIndex);
    sprintf(inmanPort,"inman.port%d",nodeIndex);
    inManCom->Init(cfg.cfgman->getString(inmanHost),cfg.cfgman->getInt(inmanPort));
    inmancomm::INManComm::scAddr=cfg.cfgman->getString("core.service_center_address");
    smeman.registerInternallSmeProxy("INMANCOMM",inManCom);
    tp.startTask(inManCom);
    smsc_log_info(log, "IN manager started" );

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

    smsc::closedgroups::ClosedGroupsManager::Init();
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctClosedGroups);
      smsc::closedgroups::ClosedGroupsManager::getInstance()->Load(findConfigFile("ClosedGroups.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctClosedGroups);
    }

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
      int cnt=cfg.cfgman->getInt("core.state_machines_count");
      time_t maxValidTime=cfg.cfgman->getInt("sms.max_valid_time");
      for(int i=0;i<cnt;i++)
      {
        statemachine::StateMachine *m=new statemachine::StateMachine(eventqueue,store,this);
        m->maxValidTime=maxValidTime;
#ifdef SMSEXTRA
        m->createCopyOnNickUsage=createCopyOnNickUsage;
#endif
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

    try{
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctReschedule);
      common::RescheduleCalculator::init(findConfigFile("schedule.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctReschedule);
    }catch(std::exception& e)
    {
      smsc_log_warn(log, "Exception during reading 'schedule.xml':%s" ,e.what());
    }

    SpeedMonitor *sm=new SpeedMonitor(eventqueue,&perfDataDisp,&perfSmeDataDisp,this);
    tp.startTask(sm);
    smsc_log_info(log, "Speedmonitor started" );

    {
      smsc_log_info(log, "Statistics manager starting..." );
      using smsc::util::config::ConfigView;
      std::auto_ptr<ConfigView> msConfig(new smsc::util::config::ConfigView(*cfg.cfgman, "MessageStore"));
      char statDirCfg[64];
      sprintf(statDirCfg,"statisticsDir%d",nodeIndex);
      const char* statisticsLocation = msConfig.get()->getString(statDirCfg);
      statMan=new smsc::stat::StatisticsManager(statisticsLocation);
      tp2.startTask(statMan);
      smsc_log_info(log, "Statistics manager started" );
    }

    aclmgr = AclAbstractMgr::Create2();
    aclmgr->LoadUp(cfg.cfgman->getString("acl.storeDir"),cfg.cfgman->getInt("acl.preCreateSize"));
    smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctAcl);

    /*distlstman=new DistrListManager(*cfg.cfgman);

    distlstsme=new DistrListProcess(distlstman,&smeman);
    tp.startTask(distlstsme);
    smsc_log_info(log, "Distribution list processor started" );

    smeman.registerInternallSmeProxy("DSTRLST",distlstsme);

    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctDistrList);
      distlstman->init();
    }
    */
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctMsc);
      smsc::mscman::MscManager::startup();
      smsc_log_info(log, "MSC manager started" );
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctMsc);
    }

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

      /*
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

      try {
        defProfile.accessMaskIn = cfg.cfgman->getInt("profiler.defaultAccessMaskIn");
      } catch(smsc::util::config::ConfigException& ex){
        smsc_log_warn(log, "profiler.defaultAccessMaskIn not found, using hardcoded default(%d)",defProfile.accessMaskIn);
      }

      try {
        defProfile.accessMaskOut = cfg.cfgman->getInt("profiler.defaultAccessMaskOut");
      } catch(smsc::util::config::ConfigException& ex){
        smsc_log_warn(log, "profiler.defaultAccessMaskOut not found, using hardcoded default(%d)",defProfile.accessMaskOut);
      }
    */
      profiler=new smsc::profiler::Profiler(&smeman,cfg.cfgman->getString("profiler.systemId"));

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
      profiler->setNotifier(pnot);

      try{
        profiler->originatingAddress=cfg.cfgman->getString("profiler.originatingAddress");
      }catch(...)
      {
        ///
      }
    }

    //distlstsme->assignProfiler(profiler);
    //distlstsme->assignAliaser(aliaser);
/*
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
    */

    smsc_log_info(log, "Profiler configured" );
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctProfiles);
      profiler->load(cfg.cfgman->getString("profiler.storeFile"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctProfiles);
    }
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
      smsc::abonentinfo::AbonentInfoSme *abonentInfo=
          new smsc::abonentinfo::AbonentInfoSme(this,
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

    smscsme=new smscsme::SmscSme("smscsme",&smeman);
    smscsme->servType=cfg.cfgman->getString("core.service_type");
    smscsme->protId=cfg.cfgman->getInt("core.protocol_id");
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

    cancelAgent=new agents::CancelAgent(eventqueue,scheduler,eventQueueLimit);
    tp.startTask(cancelAgent);
    smsc_log_info(log, "Cancel agent started" );


    char smppHostCfg[64];
    char smppPortCfg[64];
    sprintf(smppHostCfg,"smpp.host%d",nodeIndex);
    sprintf(smppPortCfg,"smpp.port%d",nodeIndex);
    smscHost=cfg.cfgman->getString(smppHostCfg);
    smscPort=cfg.cfgman->getInt(smppPortCfg);
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
      using smsc::util::config::CStrSet;
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
    char localInstCfg[128];
    sprintf(localInstCfg,"map.localInstancies%d",nodeIndex);
    localInst=cfg.cfgman->getString(localInstCfg);
    remoteInst=cfg.cfgman->getString("map.remoteInstancies");
    CPMgmtAddress=cfg.cfgman->getString("map.cpMgmtAddress");
#endif
    smsc_log_info(log, "MR cache loaded" );

    {
      char perfHost[64];
      char perfPort[64];
      sprintf(perfHost,"core.performance.host%d",nodeIndex);
      sprintf(perfPort,"core.performance.port%d",nodeIndex);
      PerformanceServer *perfSrv=new PerformanceServer
          (
              cfg.cfgman->getString(perfHost),
              cfg.cfgman->getInt(perfPort),
              &perfDataDisp
          );
      tp2.startTask(perfSrv);
      smsc_log_info(log, "Performance server started" );
    }
    {
      char perfHost[64];
      char perfPort[64];
      sprintf(perfHost,"core.smeperformance.host%d",nodeIndex);
      sprintf(perfPort,"core.smeperformance.port%d",nodeIndex);
      PerformanceServer *perfSrv=new PerformanceServer
          (
              cfg.cfgman->getString(perfHost),
              cfg.cfgman->getInt(perfPort),
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

      try{
        smartMultipartForward=cfg.cfgman->getBool("core.smartMultipartForward");
      }catch(...)
      {
        smsc_log_warn(log,"core.smartMultipartForward not found in config and disabled by default");
      }

      maxSmsPerSecond=license.maxsms;
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


    {
      agents::StatusSme *ss=new agents::StatusSme(this,"StatusSme");

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
      using namespace smsc::extra;
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

/*
    #define GETDLOPTIONALCFGPARAM(n,t) \
  try{ distlstsme->n=cfg.cfgman->t("distrList."#n);}\
  catch(...) \
  { smsc_log_warn(log,"Config Parameter distrList."#n" not found in config. Using default value");}
  */

    //GETDLOPTIONALCFGPARAM(autoCreatePrincipal,getBool);
    //GETDLOPTIONALCFGPARAM(defaultMaxLists,getInt);
    //GETDLOPTIONALCFGPARAM(defaultMaxElements,getInt);
    //GETDLOPTIONALCFGPARAM(sendSpeed,getInt);


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


class MainLoopRunner:public smsc::core::threads::Thread
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
#ifndef NOMAPPROXY
      MapDialogContainer::getInstance()->registerSelf(&smeman);
#ifdef USE_MAP
      mapProxy=MapDialogContainer::getInstance()->getProxy();
      SmeInfo si=smeman.getSmeInfo(smeman.lookup("MAP_PROXY"));
      mapProxy->updateSmeInfo( si );
      MapDialogContainer::getInstance()->getProxy()->setId("MAP_PROXY");
#endif
#endif
    }

  // start rescheduler created in init
  // start on thread pool 2 to shutdown it after state machines

    scheduler->InitMsgId(&smsc::util::config::Manager::getInstance(),nodeIndex);
    tp2.startTask(scheduler);

#ifdef SNMP
    __trace__("Smsc::changing SNMP state to OPER");
    if(snmpAgent) snmpAgent->statusChange(SnmpAgent::OPER);
    __trace__("Smsc::SNMP state to OPER changed");
#endif

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

  }catch(exception& e)
  {
    smsc_log_warn(log, "Smsc::run exception:%s",e.what());
    throw;
  }catch(...)
  {
    smsc_log_warn(log, "Smsc::run exception:unknown");
    throw;
  }
  __trace__("Smsc::run completed");
}

void Smsc::shutdown()
{

  smsc::cluster::controller::NetworkDispatcher::getInstance().Stop();
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
  smeman.unregisterSmeProxy(smsc::interconnect::ClusterInterconnect::getInstance());

  tp2.shutdown();

#if defined(USE_MAP)
  if(mapioptr)
  {
    MapIoTask *mapio=(MapIoTask*)mapioptr;
    delete mapio;
  }
#endif

  mapio::MapLimits::Shutdown();


  smsc::closedgroups::ClosedGroupsManager::Shutdown();


  //delete distlstman;
  delete aclmgr;

  smsc::mscman::MscManager::shutdown();

  smeman.Dump();
  __trace__("smeman dumped");
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
  smsc::cluster::controller::NetworkDispatcher::Shutdown();
  __trace__("shutdown completed");
}

void Smsc::reloadRoutes()
{
  auto_ptr<RouteManager> router(new RouteManager());
  router->assign(&smeman);
  try{
    common::loadRoutes(router.get(),*configs->routesconfig);
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
  common::loadRoutes(router.get(),rcfg,true);
  ResetTestRouteManager(router.release());
}


void Smsc::reloadReschedule()
{
    common::RescheduleCalculator::init(findConfigFile("schedule.xml"));
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
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntAccepted,sms->getSourceSmeId());
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
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntRejected,sms->getSourceSmeId());
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


void Smsc::InitLicense()
{
  Hash<string> lic;
  if(licenseFile.length()==0)
  {
    licenseFile=findConfigFile("license.ini");
    licenseSigFile=findConfigFile("license.sig");
  }

  struct ::stat st;
  if(::stat(licenseFile.c_str(),&st)!=0)
  {
    throw Exception("Failed to stat '%s'",licenseFile.c_str());
  }
  if(st.st_mtime==licenseFileModTime)
  {
    return;
  }

  licenseFileModTime=st.st_mtime;

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
    if(hostid==gethostid())
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


}//smsc
