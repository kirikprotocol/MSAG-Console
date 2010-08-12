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

#include "smsc/smscsignalhandlers.h"
#include "smsc/version.h"
#include "smsc/smsc.hpp"

#include "smsc/cluster/controller/NetworkDispatcher.hpp"
#include "smsc/cluster/controller/ConfigLockGuard.hpp"
#include "smsc/configregistry/ConfigRegistry.hpp"


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

  if(argc<3)
  {
    fprintf(stderr,"invalid number of command line arguments\n");
    return 1;
  }

  int nodeIdx=0;
  {
    std::string node=argv[1];
    std::string::size_type pos=node.find('=');
    if(pos==std::string::npos || pos==0 || node.substr(0,pos+1)!="node=")
    {
      fprintf(stderr,"expected 'node=N' as first argument (found '%s')\n",node.substr(0,pos+1).c_str());
      return 1;
    }
    nodeIdx=atoi(node.substr(pos+1).c_str());
  }
  std::string cchost=argv[2];
  int ccport=0;
  {
    std::string::size_type pos=cchost.find(':');
    if(pos==std::string::npos)
    {
      fprintf(stderr,"expected host:port of cluster controller as 2nd argument\n");
      return 1;
    }
    ccport=atoi(cchost.substr(pos+1).c_str());
    cchost.erase(pos);
    if(ccport==0)
    {
      fprintf(stderr,"invalid port of cluster controller\n");
      return 1;
    }
  }

  atexit(atExitHandler);
  smsc::clearThreadSignalMask();

  try{
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("smscmain");
    smsc_log_info(logger,"\n==========================\nStarting Smsc.\n==========================");

    smsc::configregistry::ConfigRegistry::Init();
    smsc::cluster::controller::NetworkDispatcher::Init(nodeIdx,cchost.c_str(),ccport);



    smsc::SmscConfigs cfgs;
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctMainConfig);
      smsc::util::config::Manager::init(findConfigFile("config.xml"));

      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctMainConfig);
    }
    cfgs.cfgman=&cfgs.cfgman->getInstance();


    smsc::Smsc::InitLicense();

    smsc_log_info(logger,  "Starting up %s", getStrVersion());
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctResources);
      smsc::resourcemanager::ResourceManager::init(cfgs.cfgman->getString("core.locales"), cfgs.cfgman->getString("core.default_locale"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctResources);
    }
    smsc_log_info(logger,  "Locale resources loaded" );
    smsc::config::smeman::SmeManConfig smemancfg;
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctSme);
      smemancfg.load(findConfigFile("sme.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctSme);
    }
    cfgs.smemanconfig=&smemancfg;
    smsc_log_info(logger,  "SME configuration loaded" );
    smsc::config::route::RouteConfig rc;
    {
      smsc::cluster::controller::ConfigLockGuard clg(eyeline::clustercontroller::ctRoutes);
      rc.load(findConfigFile("routes.xml"));
      smsc::configregistry::ConfigRegistry::getInstance()->update(eyeline::clustercontroller::ctRoutes);
    }
    cfgs.routesconfig=&rc;
    smsc_log_info(logger,  "Route configuration loaded" );


    smsc::Smsc *app=new smsc::Smsc;
    smsc::registerSmscSignalHandlers(app);

    app->init(cfgs, nodeIdx);
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
