#include <stdio.h>
#include "smpp/smpp_optional.h"
#include "smpp/smpp_stream.h"
#include "util/config/Manager.h"
#include "util/findConfigFile.h"
#include "sms/sms.h"
#include "system/scheduler.hpp"
#include "system/smsc.hpp"
#include "license/check/license.hpp"
#include "store/StoreManager.h"

extern "C"
const char* getStrVersion(){return"";}


using namespace smsc::util;
using namespace smsc::store;
using namespace smsc::sms;
using namespace smsc::system;

int main(int argc,char* argv[])
{
  Logger::Init();
  smsc::logger::Logger *logger = Logger::getInstance("smscmain");
  smsc::util::config::Manager::init(findConfigFile("config.xml"));
  StoreManager::startup(smsc::util::config::Manager::getInstance(),0);
  MessageStore *store=StoreManager::getMessageStore();
  TimeIdIterator* it=store->getReadyForRetry(time(NULL)+30*24*60*60);
  smsc::system::SmscConfigs cfgs;
  Smsc smsc;
  cfgs.cfgman=&cfgs.cfgman->getInstance();
  Hash<string> lic;
  {
    string lf=findConfigFile("license.ini");
    string sig=findConfigFile("license.sig");
    if(!smsc::license::check::CheckLicense(lf.c_str(),sig.c_str(),lic))
    {
      smsc_log_error(logger, "Invalid license\n");
      return -1;
    }
  }

  cfgs.licconfig=&lic;

  smsc::util::config::smeman::SmeManConfig smemancfg;
  smemancfg.load(findConfigFile("sme.xml"));
  cfgs.smemanconfig=&smemancfg;

  smsc::util::config::alias::AliasConfig aliascfg;
  aliascfg.load(findConfigFile("aliases.xml"));
  cfgs.aliasconfig=&aliascfg;

  smsc::util::config::route::RouteConfig rc;
  rc.load(findConfigFile("routes.xml"));
  cfgs.routesconfig=&rc;

  smsc.init(cfgs,"");

  int count=0;
  if(it)
  {
    SMSId id;
    SMS sms;
    while(it->getNextId(id))
    {
      store->retriveSms(id,sms);
      smsc.getScheduler()->LocalFileStoreSave(id,0,sms,false);
      count++;
    }
    delete it;
  }else
  {
    printf("Failed to create timeid iterator!\n");
  }
  smsc.getScheduler()->idSeq=store->getNextId();
  smsc.getScheduler()->lastIdSeqFlush=Scheduler::MessageIdSequenceExtent;
  smsc.getScheduler()->getNextId();


  smsc.shutdown();
  printf("\n\n%d records imported ok\n",count);


  return 0;
}
