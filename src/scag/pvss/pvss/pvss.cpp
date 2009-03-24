#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

#include "logger/Logger.h"
#include "util/Exception.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/core/server/ServerCore.h"
#include "scag/pvss/api/core/server/ServerContext.h"
#include "scag/pvss/api/core/server/ServerConfig.h"
#include "scag/pvss/api/core/server/Worker.h"

#include "scag/pvss/pvss/old/PersServer.h"
#include "scag/pvss/pvss/old/SyncConfig.h"

#include "version.inc"
#include "PvssDispatcher.h"


using namespace smsc::util::config;
using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::server;
using smsc::logger::Logger;
using smsc::util::Exception;


EventMonitor waitObj;
PersServer *persServer = NULL;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pvss");
    smsc_log_warn(logger, "Signal %d handled !", sig);
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

SyncConfig getSyncConfig(ConfigView& cfg, Logger* logger) {
  SyncConfig syncConfig;
  try { 
    syncConfig.setPort(cfg.getInt("port"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.SyncTransport.port> missed. Default value is %d", syncConfig.getPort());
  }
  try { 
    syncConfig.setIoTasksCount(cfg.getInt("ioTasksCount"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.SyncTransport.ioTasksCount> missed. Default value is %d", syncConfig.getIoTasksCount());
  }
  try { 
    syncConfig.setMaxClientsCount(cfg.getInt("maxClientsCount"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.SyncTransport.maxClientsCount> missed. Default value is %d", syncConfig.getMaxClientsCount());
  }
  try { 
    syncConfig.setConnectTimeout(cfg.getInt("inactivityTime"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.SyncTransport.inactivityTime> missed. Default value is %d", syncConfig.getConnectTimeout());
  }
  try { 
    syncConfig.setIoTimeout(cfg.getInt("ioTimeout"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.SyncTransport.ioTimeout> missed. Default value is %d", syncConfig.getIoTimeout());
  }
  try { 
    syncConfig.setPerfCounterPeriod(cfg.getInt("statisticsInterval"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.SyncTransport.statisticsInterval> missed. Default value is %d", syncConfig.getPerfCounterPeriod());
  }
  return syncConfig;
}

ServerConfig getAsyncConfig(ConfigView& cfg, Logger* logger) {
  ServerConfig serverConfig;
  try { 
    serverConfig.setPort(cfg.getInt("port"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.port> missed. Default value is %d", serverConfig.getPort());
  }
  try { 
    serverConfig.setWritersCount(cfg.getInt("writersCount"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.writersCount> missed. Default value is %d", serverConfig.getWritersCount());
  }
  try { 
    serverConfig.setReadersCount(cfg.getInt("readersCount"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.readersCount> missed. Default value is %d", serverConfig.getReadersCount());
  }
  try { 
    serverConfig.setMaxWriterChannelsCount(cfg.getInt("channelsPerWriter"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.chanelsPerWriter> missed. Default value is %d", serverConfig.getMaxWriterChannelsCount());
  }
  try { 
    serverConfig.setMaxReaderChannelsCount(cfg.getInt("channelsPerReader"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.channelsPerReader> missed. Default value is %d", serverConfig.getMaxReaderChannelsCount());
  }
  try { 
    serverConfig.setChannelQueueSizeLimit(cfg.getInt("channelQueueSizeLimit"));
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.channelQueueSizeLimit> missed. Default value is %d", serverConfig.getChannelQueueSizeLimit());
  }
  try { 
    serverConfig.setConnectTimeout(cfg.getInt("connectTimeout"));
  } catch ( scag2::pvss::core::ConfigException& e ) {
    smsc_log_error( logger, "exception: %s", e.what() );
    ::abort();
  }  catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.connectTimeout> missed. Default value is %d", serverConfig.getConnectTimeout());
  }
  try { 
    serverConfig.setInactivityTime(cfg.getInt("inactivityTime"));
  } catch ( scag2::pvss::core::ConfigException& e ) {
    smsc_log_error( logger, "exception: %s", e.what() );
    ::abort();
  }  catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.inactivityTime> missed. Default value is %d", serverConfig.getInactivityTime());
  }
  try { 
    serverConfig.setProcessTimeout(cfg.getInt("processTimeout"));
  } catch ( scag2::pvss::core::ConfigException& e ) {
    smsc_log_error( logger, "exception: %s", e.what() );
    ::abort();
  }  catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.processTimeout> missed. Default value is %d", serverConfig.getProcessTimeout());
  }
  try { 
    serverConfig.setIOTimeout(cfg.getInt("ioTimeout"));
  } catch ( scag2::pvss::core::ConfigException& e ) {
    smsc_log_error( logger, "exception: %s", e.what() );
    ::abort();
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.ioTimeout> missed. Default value is %d", serverConfig.getIOTimeout());
  }
  try { 
    serverConfig.setStatisticsInterval(cfg.getInt("statisticsInterval")*1000ULL);
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.AsyncTransport.statisticsInterval> missed. Default value is %d", int(serverConfig.getStatisticsInterval()/1000));
  }

  return serverConfig;
}

NodeConfig getNodeConfig(ConfigView& cfg, Logger* logger) {
  NodeConfig nodeCfg;
  try { 
    nodeCfg.nodesCount = cfg.getInt("nodes");
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.nodes> missed. Default value is %d", nodeCfg.nodesCount);
  }
  try { 
    nodeCfg.nodeNumber = cfg.getInt("node");
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.node> missed. Default value is %d", nodeCfg.nodeNumber);
  }
  try { 
    nodeCfg.storagesCount = cfg.getInt("storages");
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.storages> missed. Default value is %d", nodeCfg.storagesCount);
  }
  return nodeCfg;
}

AbonentStorageConfig getAbntStorageConfig(ConfigView& cfg, Manager& manager, NodeConfig& nodeCfg, Logger* logger) {
  AbonentStorageConfig abntCfg(cfg, "AbonentStorage", logger);
  try {
    ConfigView locationsConfig(manager, "PVSS.AbonentStorage.Locations");
    std::auto_ptr<CStrSet> locations(locationsConfig.getStrParamNames());
    for (CStrSet::iterator i = locations.get()->begin(); i != locations.get()->end(); ++i) {
      string loc = locationsConfig.getString((*i).c_str());
      abntCfg.locationPath.push_back(loc);
      ++nodeCfg.locationsCount;
    }
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Section <PVSS.AbonentStorage.Locations> missed.");
  }
  if (abntCfg.locationPath.empty()) {
    throw Exception("Locations paths is not specified");
  }
  return abntCfg;
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
  Logger* logger = Logger::getInstance("pvss.main");

  try{
    smsc_log_info(logger,  "Starting up %s", getStrVersion());

    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();

    ConfigView persConfig(manager, "PVSS");

    std::string host = "phoenix";
    try { 
      host = persConfig.getString("host");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.host> missed. Default value is '%s'", host.c_str());
    }
    uint16_t maxWaitingCount = 20;
    try { 
      maxWaitingCount = persConfig.getInt("storageQueueSize");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PVSS.storageQueueSize> missed. Default value is %d", maxWaitingCount);
    }

    NodeConfig nodeCfg = getNodeConfig(persConfig, logger);

    ConfigView asyncCfg(manager, "PVSS.AsyncTransport");
    ServerConfig serverConfig = getAsyncConfig(asyncCfg, logger);
    serverConfig.setHost(host);
    serverConfig.setQueueSizeLimit(maxWaitingCount);
    smsc_log_info(logger, "async config: %s", serverConfig.toString().c_str());

    ConfigView syncTransportCfg(manager, "PVSS.SyncTransport");
    SyncConfig syncConfig = getSyncConfig(syncTransportCfg, logger);
    syncConfig.setHost(host);
    smsc_log_info(logger, "sync config: %s", syncConfig.toString().c_str());

    ConfigView abntStorageConfig(manager, "PVSS.AbonentStorage");

    AbonentStorageConfig abntCfg = getAbntStorageConfig(abntStorageConfig, manager, nodeCfg, logger);

    PvssDispatcher pvssDispatcher(nodeCfg);

    if (nodeCfg.nodeNumber == pvssDispatcher.getInfrastructNodeNumber()) {
      ConfigView infStorageConfig(manager, "PVSS.InfrastructStorage");
      std::auto_ptr<InfrastructStorageConfig> infCfg(new InfrastructStorageConfig(infStorageConfig, "InfrastructStorage", logger));
      pvssDispatcher.init(abntCfg, infCfg.get());
    } else {
      pvssDispatcher.init(abntCfg, NULL);
    }

    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Server > server( new ServerCore( serverConfig, *protocol.get() ) );

    try {
        server->startup(pvssDispatcher);
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception(%u):", __LINE__, e.what() );
    }

    PersProtocol persProtocol;
    ReaderTaskManager readers(syncConfig);
    WriterTaskManager writers(syncConfig);

    persServer = new PersServer(static_cast<ServerCore&>(*server.get()), readers, writers, persProtocol,
                                syncConfig.getPerfCounterOn(), syncConfig.getPerfCounterPeriod());
    persServer->init(host.c_str(), syncConfig.getPort());
    persServer->Execute();

    server->shutdown();

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
