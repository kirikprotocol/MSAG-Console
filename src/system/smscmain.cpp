#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include "system/smsc.hpp"
//#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <admin/smsc_service/SmscComponent.h>
#include <util/signal.hpp>
#include <util/xml/init.h>
#include <util/findConfigFile.h>
#include <resourcemanager/ResourceManager.hpp>
#include <system/smscsignalhandlers.h>
#include <core/threads/Thread.hpp>
#include <locale.h>

#include "system/version.inc"

#include "util/mirrorfile/mirrorfile.h"

class SmscRunner : public smsc::core::threads::Thread
{
public:
  SmscRunner(smsc::system::Smsc* smsc)
    : _app(smsc)
  {}
  virtual ~SmscRunner()
  {
    _app = 0;
  }

  virtual int Execute()
  {
    try{
      if (_app != 0)
        _app->run();
      else
        fprintf(stderr,"smsc runner not initialized");
    }catch(std::exception& e)
    {
      fprintf(stderr,"top level exception: %s\n",e.what());
      return (-1);
    }
    catch(...)
    {
      fprintf(stderr,"FATAL EXCEPTION!\n");
      return (-0);
    }
    _app->shutdown();
    _app=0;
    fprintf(stderr,"SMSC finished\n");
    return 0;
  }

protected:
  smsc::system::Smsc* _app;
};


extern "C" void atExitHandler(void)
{
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

int main(int argc,char* argv[])
{
  Logger::Init();
  tzset();
  if(setlocale(LC_CTYPE,"")==0)
  {
    fprintf(stderr,"Failed to set LC_CTYPE\n");
  }

  atexit(atExitHandler);
  smsc::system::clearThreadSignalMask();

  try{
    smsc::logger::Logger *logger = Logger::getInstance("smscmain");
    smsc_log_info(logger,"\n==========================\nStarting Smsc.\n==========================");

    smsc::system::SmscConfigs cfgs;
    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    cfgs.cfgman=&cfgs.cfgman->getInstance();


    smsc::system::Smsc::InitLicense();

    smsc_log_info(logger,  "Starting up %s", getStrVersion());
    smsc::resourcemanager::ResourceManager::init(cfgs.cfgman->getString("core.locales"), cfgs.cfgman->getString("core.default_locale"));
    smsc_log_info(logger,  "Locale resources loaded" );
    smsc::util::config::smeman::SmeManConfig smemancfg;
    smemancfg.load(findConfigFile("sme.xml"));
    cfgs.smemanconfig=&smemancfg;
    smsc_log_info(logger,  "SME configuration loaded" );
    //smsc::util::config::alias::AliasConfig aliascfg;
    //aliascfg.load(findConfigFile("aliases.xml"));
    //cfgs.aliasconfig=&aliascfg;
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

      smsc::system::Smsc *app=new smsc::system::Smsc;

      smsc::system::registerSmscSignalHandlers(app);

      const char * node = argc < 2 ? "node=1" : argv[1];

      app->init(cfgs, node);
      app->run();
//      SmscRunner runner(app);
//      runner.Start();
//      runner.WaitFor();

      fprintf(stderr,"quiting smsc\n");
      delete app;
      fprintf(stderr,"app deleted\n");
    } else {
      using namespace smsc::admin::smsc_service;
      using smsc::util::config::Manager;

      const char * node = argc < 2 ? "node=1" : argv[1];

      // init Admin part
      {
        SmscComponent smsc_component(cfgs, node);
        ComponentManager::registerComponent(&smsc_component);

        smsc::admin::service::ServiceSocketListener listener;
        listener.init(admin_host, servicePort);

        smsc::system::registerSmscSignalHandlers(&smsc_component, &listener);
        listener.Start();

        // start
        //fprintf(stderr,"runSmsc\n");
        smsc_component.runSmsc();

        //fprintf(stderr,"smsc started\n");
        //running
        listener.shutdown();
        listener.WaitFor();

        //fprintf(stderr,"smsc stopped, finishing\n");
        // stopped
        if (smsc_component.isSmscRunning() && !smsc_component.isSmscStopping())
        {
          smsc_component.stopSmsc();
        }

        Manager::deinit();
      }
      smsc_log_info(logger,"\n==========================\nSmsc shutdown completed.\n==========================");
      //fprintf(stderr,"smsc finished\n");
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
  //remove("stats.txt");
  return 0;
}
