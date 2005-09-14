#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include "scag/scag.h"

#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <util/signal.hpp>
#include <util/xml/init.h>
#include <util/findConfigFile.h>
#include <scag/smscsignalhandlers.h>
#include <core/threads/Thread.hpp>

#include "license/check/license.hpp"
#include "scag/admin/SCAGCommandDispatcher.h"
#include "scag/admin/SCAGSocketListener.h"
#include "scag/config/ConfigManager.h"
#include "Inst.h"

#include "scag/version.inc"

extern "C" void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

int main(int argc,char* argv[])
{
  Logger::Init();

  atexit(atExitHandler);
  scag::clearThreadSignalMask();

  try{
    smsc::logger::Logger *logger = Logger::getInstance("scagmain");

    smsc_log_info(logger,  "SCAG configuration loading..." );
    scag::config::ConfigManager::Init();
    scag::config::ConfigManager& cfgs = scag::config::ConfigManager::Instance();
    smsc_log_info(logger,  "SCAG configuration is loaded" );

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

    cfgs.getLicConfig() = &lic;
    smsc_log_info(logger,  "Starting up %s", getStrVersion());

    in_port_t servicePort = 0;
    try {
      servicePort = cfgs.getConfig()->getInt("admin.port");
    } catch (std::exception e)
    {
    }

    char * admin_host = 0;
    try
    {
      admin_host = cfgs.getConfig()->getString("admin.host");
    }
    catch (scag::config::ConfigException &c)
    {}

    // For instance control
    char filename[20];
    sprintf(filename, "/tmp/scag.%d", servicePort);
    Inst inst(filename);
    // Shutdown if there is instance allready.
    if(!inst.run()) {
        exit(-1);
        fprintf(stderr, "Instance is runing allready.\n");
    }


    if (servicePort == 0 || admin_host == 0) {
      fprintf(stderr,"WARNING: admin port not specified, admin module disabled - smsc is not administrable\n");

      scag::Scag *app=new scag::Scag;

      scag::registerScagSignalHandlers(app);
      app->init();
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

      //using namespace smsc::scag::admin;

      scag::admin::SCAGCommandDispatcher::startGw();
      fprintf(stderr,"SMPP GW started\n");


      scag::admin::SCAGSocketListener listener;
      listener.init(admin_host, servicePort);


      scag::registerScagSignalHandlers(&listener);
      listener.Start();

      fprintf(stderr,"SMPP GW admin listener started\n");


      //running...
      fprintf(stderr,"running...\n");

      listener.WaitFor();

      fprintf(stderr,"SCAG shutdown...\n");
      scag::admin::SCAGCommandDispatcher::stopGw();
      fprintf(stderr,"SCAG stopped\n");

      //smsc::util::config::Manager::deinit();

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
