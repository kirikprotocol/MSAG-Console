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
#include "ReaderTaskManager.h"
#include "WriterTaskManager.h"

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
      maxClientCount = persConfig.getInt("maxClientsCount");
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
      maxWaitingCount = persConfig.getInt("storageQueueSize");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.storageQueueSize> missed. Defaul value is %d", maxWaitingCount);
    };
    NodeConfig nodeCfg;
    try { 
      nodeCfg.nodesCount = persConfig.getInt("nodes");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.nodes> missed. Defaul value is %d", nodeCfg.nodesCount);
    };
    try { 
      nodeCfg.nodeNumber = persConfig.getInt("node");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.node> missed. Defaul value is %d", nodeCfg.nodeNumber);
    };
    try { 
      nodeCfg.storagesCount = persConfig.getInt("storages");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <MTPers.storages> missed. Defaul value is %d", nodeCfg.storagesCount);
    };

    ConfigView abntStorageConfig(manager, "MTPers.AbonentStorage");
    AbonentStorageConfig abntCfg(abntStorageConfig, "AbonentStorage", logger);
    abntCfg.dbPath = storagePath;

    try {
      ConfigView locationsConfig(manager, "MTPers.AbonentStorage.Locations");
      std::auto_ptr<CStrSet> locations(locationsConfig.getStrParamNames());
      for (CStrSet::iterator i = locations.get()->begin(); i != locations.get()->end(); ++i) {
        string loc = locationsConfig.getString((*i).c_str());
        abntCfg.locationPath.push_back(loc);
        ++nodeCfg.locationsCount;
      }
    } catch (...) {
      smsc_log_warn(logger, "Section <MTPers.AbonentStorage.Locations> missed.");
    }
    if (abntCfg.locationPath.empty()) {
      throw Exception("Locations paths is not specified");
    }


    StorageManager storageManager(nodeCfg);

    if (nodeCfg.nodeNumber == storageManager.getInfrastructNodeNumber()) {
      ConfigView infStorageConfig(manager, "MTPers.InfrastructStorage");
      std::auto_ptr<InfrastructStorageConfig> infCfg(new InfrastructStorageConfig(infStorageConfig, "InfrastructStorage", logger));
      storageManager.init(maxWaitingCount, abntCfg, infCfg.get());
    } else {
      storageManager.init(maxWaitingCount, abntCfg, NULL);
    }

    ReaderTaskManager readers(ioTasksCount, maxClientCount, timeout, storageManager);
    WriterTaskManager writers(ioTasksCount, maxClientCount, timeout);
    
    ps = new PersServer(readers, writers);
    ps->init(host.c_str(), port);
    ps->Execute();

    smsc_log_error(logger, "PersServer stopped");
  } catch (const ConfigException& exc) {
    smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
    resultCode = -2;
  } catch (const Exception& exc) {
    smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
    resultCode = -3;
  } catch (const std::exception& exc) {
    smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
    resultCode = -4;
  } catch (...) {
    smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
    resultCode = -5;
  }
  return resultCode;
}
