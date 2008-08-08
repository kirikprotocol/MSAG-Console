#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "PersServer.h"
#include "StorageManager.h"
#include "IOTaskManager.h"

using namespace smsc::util::config;
using namespace scag::mtpers;
using smsc::logger::Logger;
using smsc::core::threads::ThreadedTask;

static PersServer *ps = NULL;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pers");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        if(ps) ps->shutdown();
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
  int resultCode = 0;
  Logger* logger = Logger::getInstance("mtpers");

  try{
    smsc_log_info(logger,  "Starting up ");

    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();

    ConfigView persConfig(manager, "MTPers");

    string storageName;
    string storagePath = "./storage";
    std::string host = "phoenix";
    int port = 47111;
    int maxClientCount = 10, timeout = 600;
    uint16_t storageNumber = 5;
    uint16_t maxWaitingCount = 20;
    uint16_t ioTasksCount = 10;
    uint32_t initRecordCount = 1000;

    try { 
      host = persConfig.getString("host");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.host> missed. Defaul value is '%s'", host.c_str());
    };
    try { 
      port = persConfig.getInt("port");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.port> missed. Defaul value is %d", port);
    };
    try { 
      maxClientCount = persConfig.getInt("maxClientCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.maxClientsCount> missed. Defaul value is %d", maxClientCount);
    };
    try { 
      timeout = persConfig.getInt("timeout");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.timeout> missed. Defaul value is %d", timeout);
    };
    try { 
      ioTasksCount = persConfig.getInt("ioPoolSize");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.ioPoolSize> missed. Defaul value is %d", ioTasksCount);
    };
    try { 
      storageNumber = persConfig.getInt("abonentStorageNumber");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.abonentStorageNumber> missed. Defaul value is %d", storageNumber);
    };
    try { 
      maxWaitingCount = persConfig.getInt("storageQueueSize");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.storageQueueSize> missed. Defaul value is %d", maxWaitingCount);
    };
    try { 
      storagePath = persConfig.getString("storagePath");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.storagePath> missed. Defaul value is '%s'", storagePath.c_str());
    };

    ConfigView abntStorageConfig(manager, "MTPers.AbonentStorage");
    AbonentStorageConfig abntCfg(storageNumber, abntStorageConfig, "AbonentStorage", logger);
    abntCfg.dbPath = storagePath;
    ConfigView infStorageConfig(manager, "MTPers.InfrastructStorage");
    InfrastructStorageConfig infCfg(infStorageConfig, "InfrastructStorage", logger);
    infCfg.dbPath = storagePath;


    StorageManager storageManager;
    storageManager.init(maxWaitingCount, storageNumber, abntCfg, infCfg);

    IOTaskManager ioMananger(storageManager);
    ioMananger.init(ioTasksCount, maxClientCount, "ioman");

    ps = new PersServer(ioMananger);
    ps->init(host.c_str(), port);
    ps->Execute();

    smsc_log_error(logger, "PersServer stopped");
  } catch (ConfigException& exc) {
    smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
    resultCode = -2;
  } catch (Exception& exc) {
    smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
    resultCode = -3;
  } catch (std::exception& exc) {
    smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
    resultCode = -4;
  } catch (...) {
    smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
    resultCode = -5;
  }
  return resultCode;
}
