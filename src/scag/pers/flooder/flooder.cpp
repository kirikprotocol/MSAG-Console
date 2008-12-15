#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <exception>

#include "logger/Logger.h"
#include "util/Exception.hpp"
#include "scag/config/base/ConfigManager2.h"
#include "scag/config/impl/ConfigManager2.h"

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include "LCPersClient.h"

using std::string;
using namespace scag2::pers::util;
using smsc::logger::Logger;
using scag2::config::ConfigManager;
using scag2::config::ConfigManagerImpl;

int resultCode = 0;
LCPersClient* lcClient = 0;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("lcclient");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        if(lcClient) lcClient->shutdown();
        smsc_log_info(logger, "Stopping ...");
    }
    else if(sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[]) {

  int speed = 1000; //req/sec
  Logger* logger;
  Logger::Init();

  atexit(atExitHandler);

  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGTERM);
  sigdelset(&set, SIGINT);
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGBUS);
  sigdelset(&set, SIGHUP);
  sigprocmask(SIG_SETMASK, &set, NULL);
  sigset(SIGTERM, appSignalHandler);
  sigset(SIGINT, appSignalHandler);
  sigset(SIGHUP, appSignalHandler);   

  try {
    logger = Logger::getInstance("lcclient");
    {
      ConfigManagerImpl *cfg = new ConfigManagerImpl();
      cfg->Init();
    }
  
    ConfigManager& cfgs = ConfigManager::Instance();
    
    PersClient::Init(cfgs.getPersClientConfig());
    if (argc > 1) {
      speed = atoi(argv[1]);
    }

    smsc::util::config::Manager::init("config.xml");
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();
    smsc::util::config::ConfigView flooderConfig(manager, "Flooder");
    
    try { 
      speed = flooderConfig.getInt("speed");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <Flooder.speed> missed. Defaul value is %d", speed);
    };
    int getsetCount = 1;
    try { 
      getsetCount = flooderConfig.getInt("getsetCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <Flooder.getsetCount> missed. Defaul value is %d", getsetCount);
    };
    int addressesCount = 1000000;
    try { 
      addressesCount = flooderConfig.getInt("addressesCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <Flooder.addressesCount> missed. Defaul value is %d", addressesCount);
    };

    lcClient = new LCPersClient(PersClient::Instance(), speed);

    lcClient->execute(addressesCount, getsetCount);
    delete lcClient;
  }
  catch (const PersClientException& exc) 
  {
      smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
      resultCode = -2;
  }
  catch (const smsc::util::Exception& exc) 
  {
      smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
      resultCode = -3;
  }
  catch (const std::exception& exc) 
  {
      smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
      resultCode = -4;
  }
  catch (...) 
  {
      smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
      resultCode = -5;
  }
  return resultCode;
}

