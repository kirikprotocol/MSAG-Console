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

namespace smsc{
namespace system{

using std::auto_ptr;
using std::string;
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
  SpeedMonitor(EventQueue& eq):queue(eq){}
  int Execute()
  {
    uint64_t cnt,last=0;
    timespec start,now,lasttime;
    double ut,tm,rate,avg;
    clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    __trace__("enter SpeedMonitor");
    for(;;)
    {
      //sleep(1);
      //timeval tv;
      //tv.tv_sec=1;
      //tv.tv_usec=0;
      //select(0,0,0,0,&tv);
      ev.Wait(1000);
      cnt=queue.getCounter();
      clock_gettime(CLOCK_REALTIME,&now);
      ut=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (start.tv_sec*1000.0+start.tv_nsec/1000000.0))/1000.0;
      tm=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (lasttime.tv_sec*1000.0+lasttime.tv_nsec/1000000.0))/1000;
      rate=(cnt-last)/tm;
      avg=cnt/ut;
      printf("UT:%.3lf AVG:%.3lf LAST:%.3lf (%llu)         \r",ut,avg,rate,cnt);
      fflush(stdout);
      last=cnt;
      lasttime=now;
      if(isStopping)break;
    }
    return 0;
  }
  const char* taskName()
  {
    return "SpeedMonitor";
  }
protected:
  EventQueue& queue;
};

extern void loadRoutes(RouteManager* rm,smsc::util::config::route::RouteConfig& rc);

void Smsc::init(const SmscConfigs& cfg)
{
  tp.preCreateThreads(15);
  //smsc::util::config::Manager::init("config.xml");
  //cfgman=&cfgman->getInstance();

  /*
    register SME's
  */
  //smsc::util::config::smeman::SmeManConfig smemancfg;
  //smemancfg.load("sme.xml");
  {
    smsc::util::config::smeman::SmeManConfig::RecordIterator i=cfg.smemanconfig->getRecordIterator();
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
        //si.hostname=rec->recdata->smppSme.
        si.disabled=false;
        smeman.addSme(si);
      }
    }
  }
  // initialize aliases
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
      aliaser.addAlias(ai);
    }
    aliaser.commit();
  }
  // initialize router (all->all)
  router.assign(&smeman);
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
  */
  try{
    loadRoutes(&router,*cfg.routesconfig);
  }catch(...)
  {
    __warning__("Failed to load routes");
  }
  smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance());
  store=smsc::store::StoreManager::getMessageStore();

  {
    int cnt=cfg.cfgman->getInt("core.state_machines_count");
    time_t maxValidTime=cfg.cfgman->getInt("sms.max_valid_time");
    for(int i=0;i<cnt;i++)
    {
      StateMachine *m=new StateMachine(eventqueue,store,this);
      m->maxValidTime=maxValidTime;
      tp.startTask(m);
    }
  }

  RescheduleCalculator::Init(cfg.cfgman->getString("core.reschedule_table"));

  //smsc::admin::util::SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));

  tp.startTask(new SpeedMonitor(eventqueue));

  {
    char *rep=cfg.cfgman->getString("profiler.defaultReport");
    char *dc=cfg.cfgman->getString("profiler.defaultDataCoding");
    char *str=rep;
    while((*str=toupper(*str)))str++;
    str=dc;
    while((*str=toupper(*str)))str++;
    smsc::profiler::Profile defProfile;
    if(!strcmp(str,"DEFAULT"))
      defProfile.codepage=profiler::ProfileCharsetOptions::Default;
    else if(!strcmp(str,"UCS2"));
      defProfile.codepage=profiler::ProfileCharsetOptions::Ucs2;
    if(!strcmp(str,"NONE"))
      defProfile.reportoptions=profiler::ProfileReportOptions::ReportNone;
    else if(!strcmp(str,"FULL"))
      defProfile.reportoptions=profiler::ProfileReportOptions::ReportFull;

    profiler=new smsc::profiler::Profiler(defProfile);
  }
  profiler->loadFromDB();

  tp.startTask(profiler);

  try{
    smeman.registerSmeProxy(cfg.cfgman->getString("profiler.systemId"),profiler);
  }catch(...)
  {
    __warning__("Failed to register profiler Sme");
  }

  {
    smsc::system::abonentinfo::AbonentInfoSme *ai=
      new smsc::system::abonentinfo::AbonentInfoSme(profiler);
    tp.startTask(ai);
    try{
      smeman.registerSmeProxy("abonentinfo",ai);
    }catch(...)
    {
      __warning__("Failed to register abonentinfo Sme");
    }
  }

  smscHost=cfg.cfgman->getString("smpp.host");
  smscPort=cfg.cfgman->getInt("smpp.port");

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
      throw Exception("Failed to start smpp acceptor");
    }
    MapDialogContainer::getInstance()->registerSelf(&smeman);
  }

  scheduler=new Scheduler(eventqueue,store);
  tp.startTask(scheduler);


  // некоторые действия до основного цикла
  mainLoop();
  // и после него
  shutdown();
}

void Smsc::shutdown()
{
  __trace__("shutting down");
  tp.shutdown();
  smsc::store::StoreManager::shutdown();
}

};//system
};//smsc
