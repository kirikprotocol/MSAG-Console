#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "util/config/smeman/SmeManConfig.h"
#include <memory>
#include "admin/util/SignalHandler.h"
#include "util/debug.h"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"

namespace smsc{
namespace system{

using std::auto_ptr;
using std::string;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;


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

  smsc::admin::util::SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));
}

void Smsc::run()
{
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
