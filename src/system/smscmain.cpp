#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include "system/smsc.hpp"
#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <admin/smsc_service/SmscComponent.h>
#include <admin/smsc_service/SmscShutdownHandler.h>
#include <util/signal.hpp>

bool file_exist(const char * const filename)
{
  struct stat buf;
  return stat(filename, &buf) == 0;
}

char get_filename_result[128];
const char * const get_filename(const char * const file_to_find)
throw (smsc::admin::AdminException)
{
  if (file_exist(file_to_find))
  {
    strcpy(get_filename_result, file_to_find);
    return get_filename_result;
  }
  else
  {
    char buf[strlen(file_to_find)+8+1];
    strcpy(buf, "../conf/");
    strcat(buf, file_to_find);
    if (file_exist(buf))
    {
      strcpy(get_filename_result, buf);
      return get_filename_result;
    }
    else
    {
      char buf[strlen(file_to_find)+8+1];
      strcpy(buf, "./conf/");
      strcat(buf, file_to_find);
      if (file_exist(buf))
      {
        strcpy(get_filename_result, buf);
        return get_filename_result;
      }
    }
  }
  char message[strlen(file_to_find) + 64];
  sprintf(message, "File \"%s\" not found", file_to_find);
  throw smsc::admin::AdminException(message);
}

void alarmHandler(int signo)
{
  abort();
}

int main(int argc,char* argv[])
{
  {
    sigset_t set;
    sigemptyset(&set);
    for(int i=1;i<=37;i++)sigaddset(&set,i);
    if(thr_sigsetmask(SIG_SETMASK,&set,NULL)!=0)
    {
      __warning__("failed to set thread signal mask!");
    };
  }
  smsc::util::setSignalHandler(SIGALRM, alarmHandler);
  try{
    smsc::system::SmscConfigs cfgs;
    smsc::util::config::Manager::init(get_filename("config.xml"));
    cfgs.cfgman=&cfgs.cfgman->getInstance();
    try {
      Logger::Init(cfgs.cfgman->getString("logger.initFile"));
    } catch (smsc::util::config::ConfigException &e)
    {
      fprintf(stderr, "WARNING: parameter \"logger.initFile\" not found in config - logger initialized by default\n");
    }
    smsc::util::config::smeman::SmeManConfig smemancfg;
    smemancfg.load(get_filename("sme.xml"));
    cfgs.smemanconfig=&smemancfg;
    smsc::util::config::alias::AliasConfig aliascfg;
    aliascfg.load(get_filename("aliases.xml"));
    cfgs.aliasconfig=&aliascfg;
    smsc::util::config::route::RouteConfig rc;
    rc.load(get_filename("routes.xml"));
    cfgs.routesconfig=&rc;

    in_port_t servicePort = 0;
    if (argc > 1)
    {
      servicePort = atoi(argv[1]);
    }
    if (servicePort == 0)
    {
      try {
        servicePort = cfgs.cfgman->getInt("admin.port");
      } catch (std::exception e)
      {
      }
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

      app->init(cfgs);

      {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set,SIGINT);
//#ifndef SPARC
        sigaddset(&set,SIGQUIT);
//#endif
        if(thr_sigsetmask(SIG_UNBLOCK,&set,NULL)!=0)
        {
          __warning__("failed to set thread signal mask!");
        };
      }

      app->run();

      fprintf(stderr,"quiting smsc\n");
      delete app;
      fprintf(stderr,"app deleted\n");
    } else {
      using namespace smsc::admin::smsc_service;
      using smsc::util::config::Manager;

      // init Admin part
      SmscComponent smsc_component(cfgs);
      ComponentManager::registerComponent(&smsc_component);

      // start
      smsc_component.runSmsc();
      smsc::admin::service::ServiceSocketListener listener;
      listener.init(admin_host, servicePort);
      listener.Start();

      SmscShutdownHandler shutdown_handler(listener);
      SmscShutdownHandler::registerShutdownHandler(&shutdown_handler);

      {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set,SIGINT);
//#ifndef SPARC
        sigaddset(&set,SIGQUIT);
//#endif
        if(thr_sigsetmask(SIG_UNBLOCK,&set,NULL)!=0)
        {
          __warning__("failed to set thread signal mask!");
        };
      }


      fprintf(stderr,"smsc started\n");
      //running
      listener.WaitFor();

      fprintf(stderr,"smsc stopped, finishing\n");
      // stopped
      smsc_component.stopSmsc();
      Manager::deinit();

      fprintf(stderr,"smsc finished\n");
    }

  }
  catch (AdminException &e)
  {
    fprintf(stderr,"top level exception: %s\n", e.what());
    exit(-3);
  }
  catch (smsc::util::Exception &e)
  {
    fprintf(stderr,"top level exception: %s\n", e.what());
    exit(-2);
  }
  catch(std::exception& e)
  {
    fprintf(stderr,"top level exception: %s\n",e.what());
    exit(-1);
  }
  catch(...)
  {
    fprintf(stderr,"FATAL EXCEPTION!\n");
    exit(-0);
  }
  return 0;
}
