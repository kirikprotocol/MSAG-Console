#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <locale.h>

#include "core/threads/Thread.hpp"
#include "util/signal.hpp"
#include "util/xml/init.h"
#include "util/findConfigFile.h"
#include "util/mirrorfile/mirrorfile.h"
#include "config/route/RouteConfig.h"
#include "resourcemanager/ResourceManager.hpp"

#include "smscsignalhandlers.h"
#include "version.inc"
#include "smsc.hpp"


extern "C" void atExitHandler(void)
{
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  tzset();
  if(setlocale(LC_CTYPE,"")==0)
  {
    fprintf(stderr,"Failed to set LC_CTYPE\n");
  }

  atexit(atExitHandler);
  smsc::clearThreadSignalMask();

  try{
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("smscmain");
    smsc_log_info(logger,"\n==========================\nStarting Smsc.\n==========================");

    smsc::SmscConfigs cfgs;
    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    cfgs.cfgman=&cfgs.cfgman->getInstance();


    smsc::Smsc::InitLicense();

    smsc_log_info(logger,  "Starting up %s", getStrVersion());
    smsc::resourcemanager::ResourceManager::init(cfgs.cfgman->getString("core.locales"), cfgs.cfgman->getString("core.default_locale"));
    smsc_log_info(logger,  "Locale resources loaded" );
    smsc::config::smeman::SmeManConfig smemancfg;
    smemancfg.load(findConfigFile("sme.xml"));
    cfgs.smemanconfig=&smemancfg;
    smsc_log_info(logger,  "SME configuration loaded" );
    smsc::config::route::RouteConfig rc;
    rc.load(findConfigFile("routes.xml"));
    cfgs.routesconfig=&rc;
    smsc_log_info(logger,  "Route configuration loaded" );


    smsc::Smsc *app=new smsc::Smsc;
    smsc::registerSmscSignalHandlers(app);

    const char * node = argc < 2 ? "node=1" : argv[1];

    app->init(cfgs, node);
    app->run();
    app->shutdown();
    delete app;
    smsc::util::config::Manager::deinit();
    smsc_log_info(logger,"\n==========================\nSmsc shutdown completed.\n==========================");
  }
  catch (smsc::util::Exception &e)
  {
    fprintf(stderr,"top level exception: %s\n", e.what());
    exit(-1);
  }
  catch(std::exception& e)
  {
    fprintf(stderr,"top level exception: %s\n",e.what());
    exit(-1);
  }
  catch(...)
  {
    fprintf(stderr,"FATAL EXCEPTION!\n");
    exit(-1);
  }
  return 0;
}
