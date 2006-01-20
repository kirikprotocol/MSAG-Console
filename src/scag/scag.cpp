#include "scag/scag.h"

#include <memory>
#include "util/debug.h"
#include "core/synchronization/Event.hpp"
#include "util/Exception.hpp"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfigView.h"
#include <typeinfo>
#include "util/findConfigFile.h"

#include "scag/config/ConfigManager.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/route/RouteStructures.h"
#include "scag/bill/BillingManager.h"
#include "scag/stat/StatisticsManager.h"
#include "scag/sessions/SessionManager.h"

#include "util/findConfigFile.h"
#include "scag/re/RuleEngine.h"

namespace scag
{

    using namespace scag::transport::smpp::router;
    using scag::config::RouteConfig;
    using scag::config::Route;
    using scag::config::Source;
    using scag::config::Destination;
    using scag::config::DestinationHash;
    using scag::config::SourceHash;
    using scag::config::Mask;
    using scag::config::MaskVector;
    using smsc::sms::Address;

    extern void loadRoutes(RouteManager* rm, const scag::config::RouteConfig& rc,bool traceit=false);

static inline void makeAddress_(Address& addr,const string& mask)
{
  addr=Address(mask.c_str());
}

void loadRoutes(RouteManager* rm,const scag::config::RouteConfig& rc,bool traceit)
{
  try
  {
    Route *route;
    for (RouteConfig::RouteIterator ri = rc.getRouteIterator();
         ri.fetchNext(route) == RouteConfig::success;)
    {
      char * dest_key;
      char* src_key;
      Source src;
      Destination dest;
      RouteInfo rinfo;
      for (DestinationHash::Iterator dest_it = route->getDestinations().getIterator();
           dest_it.Next(dest_key, dest);)
      {
        for (SourceHash::Iterator src_it = route->getSources().getIterator();
             src_it.Next(src_key, src);)
        {
          // masks
          if(dest.isSubject())
          {
            rinfo.dstSubj=( std::string("subj:")+dest.getIdString() ).c_str();
          }
          const MaskVector& dest_masks = dest.getMasks();
          for (MaskVector::const_iterator dest_mask_it = dest_masks.begin();
               dest_mask_it != dest_masks.end();
               ++dest_mask_it)
          {
            makeAddress_(rinfo.dest,*dest_mask_it);
            if(!dest.isSubject())
            {
              rinfo.dstSubj=( std::string("mask:")+*dest_mask_it ).c_str();
            }
            const MaskVector& src_masks = src.getMasks();
            if(src.isSubject())
            {
              rinfo.srcSubj=( std::string("subj:")+src.getIdString() ).c_str();
            }
            for(MaskVector::const_iterator src_mask_it = src_masks.begin();
                src_mask_it != src_masks.end();
                ++src_mask_it)
            {
              if(!src.isSubject())
              {
                rinfo.srcSubj=( std::string("mask:")+*src_mask_it ).c_str();
              }
              makeAddress_(rinfo.source,*src_mask_it);
              rinfo.smeSystemId = dest.getSmeIdString().c_str();//dest.smeId;
              rinfo.srcSmeSystemId = route->getSrcSmeSystemId().c_str();
//              __trace2__("sme sysid: %s",rinfo.smeSystemId.c_str());
              rinfo.archived=route->isArchiving();
              rinfo.enabled = route->isEnabling();
              rinfo.routeId=route->getId();
              rinfo.providerId=route->getProviderId();
              rinfo.ruleId=route->getRuleId();
              rinfo.categoryId=route->getCategoryId();

              try{
                rm->addRoute(rinfo);
              }
              catch(exception& e)
              {
                __warning2__("[route skiped] : %s",e.what());
              }
            }
          }
        }
      }
    }
    rm->commit(traceit);
  }
  catch(...)
  {
    rm->cancel();
    throw;
  }
}

using std::auto_ptr;
using std::string;
using std::exception;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::core::synchronization;
using smsc::util::Exception;

using scag::config::ConfigManager;
using scag::bill::BillingManager;
using scag::stat::StatisticsManager;    
using scag::config::BillingManagerConfig;
using scag::sessions::SessionManager;
using smsc::util::findConfigFile;

Scag::~Scag()
{
  SaveStats();
}

class SpeedMonitor{
public:

  //****************************************************************************************
  // Its nesseccary evenqueue to replace on something
  //****************************************************************************************

  SpeedMonitor(/*EventQueue& eq, */ scag::performance::PerformanceListener* pl, Scag* pscag):
    /*queue(eq),*/perfListener(pl)
  {
    start.tv_sec=0;
    start.tv_nsec=0;
    scag = pscag;
  }
  int run()
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
    uint64_t lastPerfCnt[scag::performance::performanceCounters]={0,};
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

      //===========================
      // replace queue
      //cnt=queue.getCounter();

      int eqhash,equnl;

      //=======================
      // replace queue
      //queue.getStats(equnl);

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

      //if(isStopping)break;

      uint64_t perf[scag::performance::performanceCounters];
      // success, error, reschedule
      scag->getPerfData(perf);
      scag::performance::PerformanceData d;
      d.countersNumber=scag::performance::performanceCounters;
      for(i=0;i<scag::performance::performanceCounters;i++)
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
        for(i=0;i<scag::performance::performanceCounters;i++)perfCnt[i][idx]=0;
      }
      if(scnt!=lastscnt)
      {
        times[scnt]=now.tv_sec;
        lastscnt=scnt;
      }
      for(int j=0;j<scag::performance::performanceCounters;j++)
      {
        d.counters[j].average=0;
      }
      int idx=timeshift;
      for(i=0;i<=scnt;i++,idx++)
      {
        if(idx>=60)idx=0;
        if(i==scnt)
        {
          for(int j=0;j<scag::performance::performanceCounters;j++)
          {
            perfCnt[j][idx]+=d.counters[j].lastSecond;
          }
        }
        for(int j=0;j<scag::performance::performanceCounters;j++)
        {
          d.counters[j].average+=perfCnt[j][idx];
        }
      }
      int diff=now.tv_sec-times[timeshift];
      if(diff==0)diff=1;
      //__trace2__("ca=%d,ea=%d,ra=%d, time diff=%u",
      //  d.success.average,d.error.average,d.rescheduled.average,diff);

      for(i=0;i<scag::performance::performanceCounters;i++)
      {
        d.counters[i].average/=diff;
      }

      d.now=now.tv_sec;
      d.uptime=now.tv_sec-start.tv_sec;

      d.eventQueueSize=equnl;

      perfListener->reportGenPerformance(&d);

      for(i=0;i<scag::performance::performanceCounters;i++)
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
protected:
  //EventQueue& queue;
  int perfCnt[scag::performance::performanceCounters][60];
  int timeshift;
  time_t times[60];
  timespec start;
  scag::performance::PerformanceListener* perfListener;
  static Scag* scag;
};

Scag* SpeedMonitor::scag=NULL;

void Scag::init()
{
  ConfigManager & cfg = ConfigManager::Instance();

  smsc::util::regexp::RegExp::InitLocale();
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("scag.init");

  try{
  InitLicense(*cfg.getLicConfig());
  //tp.preCreateThreads(15);
    
    //********************************************************
    //************** SmppManager initialization **************
    try {
        smsc_log_info(log, "Smpp Manager is starting");
        smppMan.Init(findConfigFile("../conf/smpp.xml"));
        smsc_log_info(log, "Smpp Manager started");
    }catch(...)
    {
        throw Exception("Exception during initialization of SmppManager");
    }
  //********************************************************
    

 
    try {
        BillingManager::Init(cfg.getBillManConfig());
    }catch(...)
    {
        throw Exception("Exception during initialization of BillingManager");
    }
  

  //********************************************************
  //*********** SessionManager initialization **************
  try{
      smsc_log_info(log, "Session Manager is starting..." );

      SessionManager::Init(cfg.getSessionManConfig());

      smsc_log_info(log, "Session Manager started" );
  }catch(exception& e){
      smsc_log_warn(log, "Scag.init exception: %s", e.what());
      __warning__("Sessioan Manager is not started.");
  }catch(...){
      __warning__("Session Manager is not started.");
  }
  //********************************************************


  //********************************************************
  //************** RuleEngine initialization ***************
  try{
      smsc_log_info(log, "Rule Engine is starting..." );

      using scag::config::ConfigView;
      std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.getConfig(),"RuleEngine"));

      auto_ptr <char> loc(cv->getString("location", 0, false));
      if(!loc.get())
          throw Exception("RuleEngine.location not found");

      std::string location = loc.get();

      scag::re::RuleEngine & re = scag::re::RuleEngine::Instance();
      re.Init(location);

      smsc_log_info(log, "Rule Engine started" );
  }catch(exception& e){
      smsc_log_warn(log, "Scag.init exception: %s", e.what());
      __warning__("Rule Engine is not started.");
  }catch(...){
      __warning__("Rule Engine is not started.");
  }
  //********************************************************


  //********************************************************
  //********** Statistics manager initialization ***********
  try{
      using scag::config::ConfigView;
      std::auto_ptr<ConfigView> cv(new ConfigView(*cfg.getConfig(),"MessageStorage"));

      auto_ptr <char> loc(cv->getString("statisticsDir", 0, false));
      if(!loc.get())
          throw Exception("MessageStorage.statisticsDir not found");

      std::string location = loc.get();
      //StatisticsManager::init(location);
      StatisticsManager::init(cfg.getStatManConfig());

      smsc_log_info(log, "Statistics manager started" );
  }catch(exception& e){
      smsc_log_warn(log, "Smsc.init exception: %s", e.what());
      __warning__("Statistics manager is not started.");
  }catch(...){
      __warning__("Statistics manager is not started.");
  }
  //********************************************************


  scagHost=cfg.getConfig()->getString("smpp.host");
  scagPort=cfg.getConfig()->getInt("smpp.port");


  eventQueueLimit=1000;
  try{
    eventQueueLimit=cfg.getConfig()->getInt("core.eventQueueLimit");
  }catch(...)
  {
    __warning__("eventQueueLimit not found, using default(1000)");
  }

  //*****************************************************
  // test route instance initialization
  //*****************************************************
  try {
      RouteConfig cfg;
      if (cfg.load("conf/routes__.xml") == RouteConfig::fail){
          smsc_log_warn(log, "Load routes config file failed.");
      }else{
          reloadTestRoutes(cfg);
          getTestRouterInstance()->enableTrace(true);
      }
  }catch(...)
  {
      throw Exception("Exception during test route instance initialization");
  }
  //*****************************************************

  smsc_log_info(log, "SCAG init complete" );

  }
  catch(exception& e)
  {
    smsc_log_warn(log, "Scag::init exception:%s",e.what());
    throw;
  }
  catch(...)
  {
    smsc_log_warn(log, "Scag::init exception:unknown");
    throw;
  }
  __trace__("Smsc::init completed");
}

void Scag::run()
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.run");

  //TODO: report performance on Speed Monitor
    /*SpeedMonitor *sm=new SpeedMonitor(&perfDataDisp,this);
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
    sm->run();*/
  while(!stopFlag)
  {
      sleep(1);
  }
    smsc_log_info(log, "Scag stoped" );
}

void Scag::shutdown()
{
  __trace__("shutting down");

  //tp.shutdown();
  //tp2.shutdown();
}

void Scag::reloadTestRoutes(const RouteConfig& rcfg)
{
  auto_ptr<RouteManager> router(new RouteManager());
  //router->assign(&smeman);
  loadRoutes(router.get(),rcfg,true);
  ResetTestRouteManager(router.release());
}

} //scag
