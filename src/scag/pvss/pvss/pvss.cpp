#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/core/server/ServerCore.h"
#include "scag/pvss/api/core/server/ServerContext.h"
#include "scag/pvss/api/core/server/ServerConfig.h"
#include "scag/pvss/api/core/server/Worker.h"

#include "scag/pvss/pvss/old/PersServer.h"

#include "version.inc"
#include "PvssDispatcher.h"

using namespace smsc::util::config;
using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::server;
using smsc::logger::Logger;


EventMonitor waitObj;
PersServer *persServer = NULL;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pvss");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
       if (persServer) persServer->stop();
      //{
        //  MutexGuard mg(waitObj);
         // waitObj.notify();
      //}
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
  Logger* logger = Logger::getInstance("pvss");

  try{
    smsc_log_info(logger,  "Starting up %s", getStrVersion());

    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();

    ConfigView persConfig(manager, "PVSS");

    string storageName;
    string storagePath = "./storage";
    std::string host = "phoenix";
    int port = 47111;
    int asyncPort = port + 1;
    int maxClientCount = 10, timeout = 600;
    uint16_t storageNumber = 5;
    uint16_t maxWaitingCount = 20;
    uint16_t ioTasksCount = 10;
    uint32_t initRecordCount = 1000;

    try { 
      host = persConfig.getString("host");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.host> missed. Defaul value is '%s'", host.c_str());
    };
    try { 
      port = persConfig.getInt("port");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.port> missed. Defaul value is %d", port);
    };
    try { 
      asyncPort = persConfig.getInt("asyncPort");
    } catch (...) {
      asyncPort = port + 1;
      smsc_log_warn(logger, "Parameter <PVSS.asyncPort> missed. Defaul value is %d", asyncPort);
    };
    try { 
      maxClientCount = persConfig.getInt("maxClientsCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.maxClientsCount> missed. Defaul value is %d", maxClientCount);
    };
    try { 
      timeout = persConfig.getInt("timeout");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.timeout> missed. Defaul value is %d", timeout);
    };
    try { 
      ioTasksCount = persConfig.getInt("ioPoolSize");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.ioPoolSize> missed. Defaul value is %d", ioTasksCount);
    };
    try { 
      maxWaitingCount = persConfig.getInt("storageQueueSize");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.storageQueueSize> missed. Defaul value is %d", maxWaitingCount);
    };
    NodeConfig nodeCfg;
    try { 
      nodeCfg.nodesCount = persConfig.getInt("nodes");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.nodes> missed. Defaul value is %d", nodeCfg.nodesCount);
    };
    try { 
      nodeCfg.nodeNumber = persConfig.getInt("node");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.node> missed. Defaul value is %d", nodeCfg.nodeNumber);
    };
    try { 
      nodeCfg.storagesCount = persConfig.getInt("storages");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.storages> missed. Defaul value is %d", nodeCfg.storagesCount);
    };
    bool perfCounterOn = false;
    int perfCounterPeriod = 10;
    try { 
      perfCounterOn = persConfig.getBool("perfCounterOn");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.perfCounterOn> missed. Defaul value is false");
    };
    if (perfCounterOn) {
      try { 
        perfCounterPeriod = persConfig.getInt("perfCounterPeriod");
      } catch (...) {
        smsc_log_warn(logger, "Parameter <PVSS.perfCounterPeriod> missed. Defaul value is %d", perfCounterPeriod);
      }
    }

    ConfigView abntStorageConfig(manager, "PVSS.AbonentStorage");
    AbonentStorageConfig abntCfg(abntStorageConfig, "AbonentStorage", logger);
    abntCfg.dbPath = storagePath;

    try {
      ConfigView locationsConfig(manager, "PVSS.AbonentStorage.Locations");
      std::auto_ptr<CStrSet> locations(locationsConfig.getStrParamNames());
      for (CStrSet::iterator i = locations.get()->begin(); i != locations.get()->end(); ++i) {
        string loc = locationsConfig.getString((*i).c_str());
        abntCfg.locationPath.push_back(loc);
        ++nodeCfg.locationsCount;
      }
    } catch (...) {
      smsc_log_warn(logger, "Section <PVSS.AbonentStorage.Locations> missed.");
    }
    if (abntCfg.locationPath.empty()) {
      throw Exception("Locations paths is not specified");
    }


    PvssDispatcher pvssDispatcher(nodeCfg);

    if (nodeCfg.nodeNumber == pvssDispatcher.getInfrastructNodeNumber()) {
      ConfigView infStorageConfig(manager, "PVSS.InfrastructStorage");
      std::auto_ptr<InfrastructStorageConfig> infCfg(new InfrastructStorageConfig(infStorageConfig, "InfrastructStorage", logger));
      pvssDispatcher.init(abntCfg, infCfg.get());
    } else {
      pvssDispatcher.init(abntCfg, NULL);
    }


    ServerConfig serverConfig;
    try {
        serverConfig.setHost(host);
        serverConfig.setPort((short)asyncPort);
        // serverConfig.setConnectionsCount(1);
        serverConfig.setReadersCount(ioTasksCount);
        serverConfig.setWritersCount(ioTasksCount);
        serverConfig.setMaxReaderChannelsCount(maxClientCount/ioTasksCount);
        serverConfig.setMaxWriterChannelsCount(maxClientCount/ioTasksCount);
        serverConfig.setChannelQueueSizeLimit(100);
        serverConfig.setConnectTimeout(1000);
        serverConfig.setInactivityTime(30000);
        serverConfig.setProcessTimeout(1000);
        serverConfig.setIOTimeout(timeout);
        serverConfig.setQueueSizeLimit(maxWaitingCount);
    } catch ( scag2::pvss::core::ConfigException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
        ::abort();
    }

    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Server > server( new ServerCore( serverConfig, *protocol.get() ) );

    try {
        server->startup(pvssDispatcher);
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
    }

    PersProtocol persProtocol;
    ReaderTaskManager readers(ioTasksCount, maxClientCount, timeout, perfCounterOn);
    WriterTaskManager writers(ioTasksCount, maxClientCount, timeout);

    persServer = new PersServer(static_cast<ServerCore&>(*server.get()), readers, writers, persProtocol, perfCounterOn, perfCounterPeriod);
    persServer->init(host.c_str(), port);
    persServer->Execute();

    readers.shutdown();
    writers.shutdown();

    //EventMonitor waitObj;
    //{
      //  MutexGuard mg(waitObj);
        //waitObj.wait(100000);
      //  waitObj.wait();
    //}

    smsc_log_info(logger,"going to shutdown");
    server->shutdown();

    smsc_log_error(logger, "PersServer stopped");
  } catch (const smsc::util::config::ConfigException& exc) {
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
