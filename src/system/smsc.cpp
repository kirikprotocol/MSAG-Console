#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "util/config/smeman/SmeManConfig.h"
#include <memory>
#include "admin/util/SignalHandler.h"
#include "util/debug.h"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "core/synchronization/Event.hpp"

namespace smsc{
namespace system{

using std::auto_ptr;
using std::string;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::core::synchronization;


class SmscSignalHandler:public smsc::admin::util::SignalHandler{
public:
  SmscSignalHandler(Smsc *app):smsc(app){}
  void handleSignal()throw()
  {
    smsc->stop();
    trace("got a signal!");
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

void Smsc::init()
{
  tp.preCreateThreads(5);
  smsc::util::config::Manager::init("config.xml");
  cfgman=&cfgman->getInstance();

  /*
    register SME's
  */
  smsc::util::config::smeman::SmeManConfig smemancfg;
  smemancfg.load("sme.xml");
  smsc::util::config::smeman::SmeManConfig::RecordIterator i=smemancfg.getRecordIterator();
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
  tp.startTask(
    new smppio::SmppAcceptor(
      cfgman->getString("smpp.host"),
      cfgman->getInt("smpp.port"),
      &ssockman
    )
  );

  // initialize router (all->all)
  router.assign(&smeman);
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

  smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance());
  store=smsc::store::StoreManager::getMessageStore();

  tp.startTask(new StateMachine(eventqueue,store,this));
  tp.startTask(new StateMachine(eventqueue,store,this));
  tp.startTask(new StateMachine(eventqueue,store,this));
  tp.startTask(new StateMachine(eventqueue,store,this));
  tp.startTask(new StateMachine(eventqueue,store,this));

  smsc::admin::util::SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));

  tp.startTask(new SpeedMonitor(eventqueue));
}

void Smsc::run()
{
  // ��������� �������� �� ��������� �����
  mainLoop();
  // � ����� ����
  shutdown();
}

void Smsc::shutdown()
{
  __trace__("shutting down");
  tp.shutdown();
  smsc::store::StoreManager::shutdown();
  smsc::util::config::Manager::deinit();
}

};//system
};//smsc
