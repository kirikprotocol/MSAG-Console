#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "util/config/smeman/SmeManConfig.h"


namespace smsc{
namespace system{

Smsc::~Smsc()
{
}

void Smsc::init()
{
  tp.preCreateThreads(50);
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
    smsc::smeman::SmeInfo si;
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
      //si.hostname=rec->recdata->smppSme.
      si.disabled=false;
      smeman.addSme(si);
    }
  }
}

void Smsc::run()
{
  tp.startTask(
    new smppio::SmppAcceptor(
      cfgman->getString("smpp.host"),
      cfgman->getInt("smpp.port"),
      &ssockman
    )
  );
  for(;;)sleep(1);
}

};//system
};//smsc
