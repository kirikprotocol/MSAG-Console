#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include "smppgw/smsc.hpp"
#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
//#include <admin/smsc_service/SmscComponent.h>
#include <util/signal.hpp>
#include <util/xml/init.h>
#include <util/findConfigFile.h>
#include <smppgw/smscsignalhandlers.h>
#include <core/threads/Thread.hpp>
#include "license/check/license.hpp"
#include "smppgw/admin/SmppGwCommandDispatcher.h"
#include "smppgw/admin/SmppGwSocketListener.h"

#include "smppgw/version.inc"


extern "C" void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

int main(int argc,char* argv[])
{
  Logger::Init();

  atexit(atExitHandler);
  smsc::smppgw::clearThreadSignalMask();

  try{
    smsc::smppgw::SmscConfigs cfgs;
    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    cfgs.cfgman=&cfgs.cfgman->getInstance();
    smsc::logger::Logger *logger = Logger::getInstance("smscmain");

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
    smsc_log_info(logger,  "Starting up %s", getStrVersion());
    smsc::util::config::smeman::SmeManConfig smemancfg;
    smemancfg.load(findConfigFile("sme.xml"));
    cfgs.smemanconfig=&smemancfg;
    smsc_log_info(logger,  "SME configuration loaded" );
    smsc::util::config::alias::AliasConfig aliascfg;
    //aliascfg.load(findConfigFile("aliases.xml"));
    cfgs.aliasconfig=&aliascfg;
    //smsc_log_info(logger,  "Alias configuration loaded" );
    smsc::util::config::route::RouteConfig rc;
    rc.load(findConfigFile("routes.xml"));
    cfgs.routesconfig=&rc;
    smsc_log_info(logger,  "Route configuration loaded" );


    in_port_t servicePort = 0;
    try {
      servicePort = cfgs.cfgman->getInt("admin.port");
    } catch (std::exception e)
    {
    }

    char * admin_host = 0;
    try
    {
      admin_host = cfgs.cfgman->getString("admin.host");
    }
    catch (smsc::util::config::ConfigException &c)
    {}


    if (servicePort == 0 || admin_host == 0) {
      fprintf(stderr,"WARNING: admin port not specified, admin module disabled - smsc is not administrable\n");

      smsc::smppgw::Smsc *app=new smsc::smppgw::Smsc;

      smsc::smppgw::registerSmscSignalHandlers(app);
      app->init(cfgs);
      app->run();
//      SmscRunner runner(app);
//      runner.Start();
//      runner.WaitFor();

      fprintf(stderr,"quiting smsc\n");
      delete app;
      fprintf(stderr,"app deleted\n");
    }
    else
    {

      //using namespace smsc::smppgw::admin;

      smsc::smppgw::admin::SmppGwCommandDispatcher::setGwConfigs(cfgs);
      smsc::smppgw::admin::SmppGwCommandDispatcher::startGw();
      fprintf(stderr,"SMPP GW started\n");


      smsc::smppgw::admin::SmppGwSocketListener listener;
      listener.init(admin_host, servicePort);


      smsc::smppgw::registerSmscSignalHandlers(&listener);
      listener.Start();

      fprintf(stderr,"SMPP GW admin listener started\n");


      //running...
      fprintf(stderr,"running...\n");

      listener.WaitFor();

      fprintf(stderr,"SMPPGW shutdown...\n");
      smsc::smppgw::admin::SmppGwCommandDispatcher::stopGw();
      fprintf(stderr,"SMPPGW stopped\n");

      smsc::util::config::Manager::deinit();

      fprintf(stderr,"all finished\n");

    }
  }
  catch (AdminException &e)
  {
    fprintf(stderr,"top level exception: %s\n", e.what());
    exit(-2);
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
  //graceful shutdown
  //we don't need to restore stats on next startup in this case
  remove("stats.txt");
  return 0;
}
