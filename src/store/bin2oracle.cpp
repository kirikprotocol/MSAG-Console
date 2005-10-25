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

  smsc.init(cfgs,"node=1");


  LocalFileStore::IdSeqPairList::iterator it;
  SMS sms,tmp;
  Scheduler& sc=*smsc.getScheduler();
  for(it=sc.currentSnap.begin();it!=sc.currentSnap.end();it++)
  {
    sc.retriveSms(it->first,sms);
    tmp=sms;
    store->createSms(tmp,it->first,CREATE_NEW);
    store->changeSmsStateToEnroute(it->first,sms.getDestinationDescriptor(),sms.lastResult,sms.nextTime,sms.attempts);
  }


  smsc.shutdown();


  return 0;
}
