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

#include "scag/util/Inst.h"

#include "scag/counter/Manager.h"

#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/core/server/ServerCore.h"
#include "scag/pvss/api/core/server/ServerContext.h"
#include "scag/pvss/api/core/server/ServerConfig.h"
#include "scag/pvss/api/core/server/Worker.h"

#include "scag/pvss/pvss/old/PersServer.h"
#include "scag/pvss/pvss/old/SyncConfig.h"

#include "version.inc"
#include "PvssDispatcher.h"
#include "BackupProcessor.h"
#include "util/config/ConfString.h"

#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/impl/TemplateManagerImpl.h"
#include "scag/counter/impl/ConfigReader.h"

#ifdef SNMP
#include "scag/snmp/SnmpWrapper.h"
#include "scag/snmp/SnmpTrapThread.h"
#endif

using namespace smsc::util::config;
using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::server;
using smsc::logger::Logger;
using smsc::util::Exception;

// EventMonitor waitObj;
std::auto_ptr<PersServer> persServer;
std::auto_ptr<BackupProcessor> backupProcessor;


#ifdef SNMP

MsagCounterTableElement* counterListCtor( MsagCounterTableElement* list )
{
    return scag2::counter::Manager::getInstance().updateSnmpCounterList(list);
}

void counterListDtor( MsagCounterTableElement* list )
{
    while (list) {
        MsagCounterTableElement* next = list->next;
        delete list;
        list = next;
    }
}

#endif

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pvss");
    smsc_log_warn(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        if (persServer.get()) persServer->stop();
        if (backupProcessor.get()) backupProcessor->stop();
        // {
        // MutexGuard mg(waitObj);
        // waitObj.notify();
        // }
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
  try { 
    nodeCfg.expectedSpeed = cfg.getInt("expectedSpeed");
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Parameter <PVSS.expectedSpeed> missed. Default value is %d", nodeCfg.expectedSpeed);
  }
  return nodeCfg;
}

void getAbntStorageConfig(AbonentStorageConfig& abntCfg, ConfigView& locationsConfig, NodeConfig& nodeCfg, Logger* logger) {
  try {
    std::auto_ptr<CStrSet> locations(locationsConfig.getStrParamNames());
    for (CStrSet::iterator i = locations.get()->begin(); i != locations.get()->end(); ++i) {
      string locpath = ConfString(locationsConfig.getString(i->c_str())).c_str();
      AbonentStorageConfig::Location location(locpath, nodeCfg.disksCount);
      smsc_log_debug(logger, "init location: '%s' on disk %d", location.path.c_str(), location.disk);
      abntCfg.locations.push_back(location);
      ++nodeCfg.locationsCount;
    }
  } catch (const Exception& ex) {
    smsc_log_warn(logger, "Section <PVSS.AbonentStorage.Locations> missed.");
  }
  if (abntCfg.locations.empty()) {
    throw Exception("Locations paths is not specified");
  }
}

int main(int argc, char* argv[]) {

    try {
        Logger::Init();
        smsc_log_debug(smsc::logger::Logger::getInstance("logger"),"inited");
    } catch ( std::exception& e ) {
        printf("logger initialization failure\n");
        fprintf(stderr,"logger initialization failure\n");
        return -1;
    }

    // counter manager


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

    try {
        smsc_log_info(logger,"creating counter manager...");
        scag2::counter::impl::TemplateManagerImpl* tmgr =
            new scag2::counter::impl::TemplateManagerImpl();
        tmgr->init();
        scag2::counter::impl::HashCountManager* mgr =
            new scag2::counter::impl::HashCountManager(tmgr,10);
        mgr->loadConfigFile();
        mgr->start();
    } catch ( std::exception& e ) {
        smsc_log_error(logger,"exc in counter mgr: %s",e.what());
        std::terminate();
    } catch ( ... ) {
        smsc_log_error(logger,"exc in counter mgr");
        std::terminate();
    }

#ifdef SNMP
    std::auto_ptr< scag2::snmp::SnmpWrapper >    snmp;
    std::auto_ptr< scag2::snmp::SnmpTrapThread > snmpThread;
#endif

    // parsing the command line
    bool recovery = false;
    bool checkIndex = false;
    bool backup = false;
    int dodump = -2;
    for ( int i = 1; i < argc; ++i ) {
        std::string sarg(argv[i]);
        if ( sarg == "--recovery" || sarg == "--rebuild-index" ) {
            recovery = true;
            smsc_log_info(logger,"%s on command line", sarg.c_str());
            continue;
        }
        if ( sarg == "--check-index" ) {
            checkIndex = true;
            smsc_log_info(logger,"%s on command line", sarg.c_str());
            continue;
        }
        if ( sarg == "--backup" ) {
            backup = true;
            smsc_log_info(logger,"%s on command line", sarg.c_str());
            continue;
        }
        if ( sarg == "--dump" ) {
            if ( i >= argc ) {
                smsc_log_error(logger,"--dump requires an argument");
                ::exit(1);
            }
            dodump = atoi(argv[++i]);
            smsc_log_info(logger,"%s on command line", sarg.c_str());
            continue;
        }
        std::string extraMsg;
        if ( sarg != "--help" ) {
            if (extraMsg.empty()) extraMsg = "Unknown option " + sarg + "\n";
        }
        fprintf(stderr,"%sUsage: %s [--recovery] [--check-index] [--backup]\n",
                extraMsg.c_str(), argv[0]);
        smsc_log_error(logger,"%sUsage: %s [--recovery] [--check-index] [--backup]",
                       extraMsg.c_str(), argv[0]);
        ::exit(1);
    }

    try{
        smsc_log_info(logger,"###########################################################################");
        smsc_log_info(logger,"Starting up %s%s", getStrVersion(),
                      ( backup ? " in backup mode" : "" ));

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView persConfig(manager, "PVSS");

        std::string host = "phoenix";
        try { 
            host = smsc::util::config::ConfString(persConfig.getString("host")).c_str();
        } catch (...) {
            smsc_log_warn(logger, "Parameter <PVSS.host> missed. Default value is '%s'", host.c_str());
        }
        uint16_t maxWaitingCount = 20;
        try { 
            maxWaitingCount = persConfig.getInt("storageQueueSize");
        } catch (...) {
            smsc_log_warn(logger, "Parameter <PVSS.storageQueueSize> missed. Default value is %d", maxWaitingCount);
        }

        unsigned maxSpeed = 0;
        try {
            maxSpeed = persConfig.getInt("maxSpeed");
        } catch (...) {
            smsc_log_warn(logger,"Parameter <PVSS.maxSpeed> missed, Default value is %d", maxSpeed );
        }

        unsigned timingInterval = 5000;
        unsigned timingSeriesSize = 5;
        try {
            timingInterval = persConfig.getInt("timingInterval");
        } catch (...) {
            smsc_log_warn(logger,"Parameter <PVSS.timingInterval> is missed, using default %u", timingInterval);
        }
        try {
            timingSeriesSize = persConfig.getInt("timingSeriesSize");
        } catch (...) {
            smsc_log_warn(logger,"Parameter <PVSS.timingSeriesSize> is missed, using default %u", timingSeriesSize);
        }

        std::string abonentBackup, serviceBackup, operatorBackup, providerBackup;
        std::string backupJournalDir;
        size_t propPerSec = 10;
        if ( backup ) {
            ConfigView backupConfig(manager,"PVSS.backup");
            try {
                backupJournalDir = backupConfig.getString("journalDir");
            } catch (...) {
                smsc_log_error(logger,"PVSS.backup.journalDir is required");
                fprintf(stderr,"PVSS.backup.journalDir is required\n");
                exit(-1);
            }
            try {
                abonentBackup = backupConfig.getString("abonent");
            } catch (...) {}
            try {
                serviceBackup = backupConfig.getString("service");
            } catch (...) {}
            try {
                operatorBackup = backupConfig.getString("operator");
            } catch (...) {}
            try {
                providerBackup = backupConfig.getString("provider");
            } catch (...) {}
            try {
                propPerSec = backupConfig.getInt("propertiesPerSecond");
            } catch (...) {}
            if ( abonentBackup.empty() &&
                 serviceBackup.empty() &&
                 operatorBackup.empty() &&
                 providerBackup.empty() ) {
                smsc_log_error(logger,"Section <PVSS.backup> is not properly configured, going to exit" );
                fprintf(stderr,"Section <PVSS.backup> is not properly configured, going to exit\n");
                exit(-1);
            }
        }

        NodeConfig nodeCfg = getNodeConfig(persConfig, logger);

        ConfigView asyncCfg(manager, "PVSS.AsyncTransport");
        ServerConfig serverConfig = getAsyncConfig(asyncCfg, logger);
        serverConfig.setHost(host);
        serverConfig.setQueueSizeLimit(maxWaitingCount);
        serverConfig.setTimingInterval(timingInterval);
        serverConfig.setTimingSeriesSize(timingSeriesSize);
        smsc_log_info(logger, "async config: %s", serverConfig.toString().c_str());

        ConfigView syncTransportCfg(manager, "PVSS.SyncTransport");
        SyncConfig syncConfig = getSyncConfig(syncTransportCfg, logger);
        syncConfig.setHost(host);
        smsc_log_info(logger, "sync config: %s", syncConfig.toString().c_str());

        ConfigView abntStorageConfig(manager, "PVSS.AbonentStorage");
        ConfigView disksConfig(manager, "PVSS.AbonentStorage.disks");

        AbonentStorageConfig abntCfg(abntStorageConfig, "AbonentStorage", logger);
        abntCfg.checkAtStart = checkIndex;

        std::auto_ptr<CStrSet> disks(disksConfig.getSectionNames());
        if (disks->empty()) {
            throw Exception("Subsections not found in <PVSS.AbonentStorage.disks> section");
        }
        for (CStrSet::iterator i = disks->begin(); i != disks->end(); ++i) {
            ConfigView diskConfig(manager, (*i).c_str());
            getAbntStorageConfig(abntCfg, diskConfig, nodeCfg, logger);
            ++nodeCfg.disksCount;
        }

        std::auto_ptr< scag::util::Inst > inst;
        if ( ! backup ) {
            // check instance, only if not backup
            char filename[40];
            sprintf(filename,"/tmp/pvss.%d",serverConfig.getPort());
            inst.reset( new scag::util::Inst(filename) );
            if ( !inst->run()) {
                fprintf( stderr, "Instance is running already.\n");
                exit(-1);
            }
        }

        std::auto_ptr<InfrastructStorageConfig> infCfg;
        if (nodeCfg.nodeNumber == PvssDispatcher::getInfrastructNodeNumber()) {
            ConfigView infStorageConfig(manager, "PVSS.InfrastructStorage");
            infCfg.reset(new InfrastructStorageConfig(infStorageConfig, "InfrastructStorage", logger));
        }

#ifdef SNMP
        // making snmp counters
        try {

            const bool enabled = persConfig.getBool("snmp.enabled");
            if (enabled) {
                std::string socket;
                try {
                    socket = smsc::util::config::ConfString(persConfig.getString("snmp.socket")).str();
                } catch (...) {
                }
                int cacheTimeout = 10;
                try {
                    cacheTimeout = persConfig.getInt("snmp.cacheTimeout");
                } catch (...) {
                    smsc_log_warn(logger,"value <snmp.cacheTimeout> is missed, using %u", cacheTimeout);
                }
                smsc_log_info(logger,"creating snmpwrapper @ '%s'", socket.c_str());
                snmp.reset(new scag2::snmp::SnmpWrapper(socket));
                snmp->initPvss( counterListCtor,
                                counterListDtor,
                                cacheTimeout );
                snmpThread.reset(new scag2::snmp::SnmpTrapThread(snmp.get()));
                snmpThread->Start();
            }
        } catch (std::exception& e) {
            smsc_log_warn(logger, "cannot initialize snmp: %s", e.what());
        } catch (...) {
            smsc_log_warn(logger, "cannot initialize snmp: unknown exception" );
        }
#endif

        PvssDispatcher pvssDispatcher(nodeCfg,abntCfg,infCfg.get());

        try {
            const bool makedirs = !(recovery || (dodump>=-1));
            pvssDispatcher.createLogics( makedirs, abntCfg, infCfg.get() );
            if ( recovery ) {
                pvssDispatcher.rebuildIndex( maxSpeed );
                return 0;
            } else if ( dodump >= -1 ) {
                pvssDispatcher.dumpStorage( dodump );
                return 0;
            } else {
                pvssDispatcher.init();
            }
        } catch ( std::exception& e ) {
            smsc_log_fatal( logger, "Exception in pvss dispatcher createLogics:\n%s", e.what() );
            exit(-1);
        } catch (...) {
            smsc_log_fatal( logger, "unknown exception in pvss dispatcher createLogics" );
            exit(-1);
        }

        if ( backup ) {
            // working in backup mode
            smsc_log_info(logger,"switching to backup mode");
            backupProcessor.reset(new BackupProcessor(pvssDispatcher,
                                                      backupJournalDir,
                                                      propPerSec,
                                                      abonentBackup,
                                                      serviceBackup,
                                                      providerBackup,
                                                      operatorBackup));
            backupProcessor->process();
            backupProcessor.reset(0);
            return 0;            
        }

        std::auto_ptr< ServerCore > server
            ( new ServerCore( new ServerConfig(serverConfig),
                              new scag2::pvss::pvap::PvapProtocol ));

        try {
            // server->init();
            server->startup(pvssDispatcher);
        } catch ( PvssException& e ) {
            smsc_log_error( logger, "exception(%u): %s", __LINE__, e.what() );
            exit(-1);
        }

        PersProtocol persProtocol;
        ReaderTaskManager readers(syncConfig);
        WriterTaskManager writers(syncConfig);

        persServer.reset(new PersServer(static_cast<ServerCore&>(*server.get()), readers, writers, persProtocol,
                                        syncConfig.getPerfCounterOn(), syncConfig.getPerfCounterPeriod()));
        persServer->init(host.c_str(), syncConfig.getPort());
        persServer->Execute();

        smsc_log_info(logger,"going to shutdown");
        server->shutdown();

        readers.shutdown();
        writers.shutdown();

    //EventMonitor waitObj;
    //{
      //  MutexGuard mg(waitObj);
        //waitObj.wait(100000);
      //  waitObj.wait();
    //}
    //smsc_log_info(logger,"going to shutdown");
    //server->shutdown();

        smsc_log_error(logger, "PersServer stopped");
        persServer.reset(0);
        smsc_log_error(logger, "PersServer destroyed");

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

#ifdef SNMP
    if (snmpThread.get()) snmpThread->Stop();
#endif
    return resultCode;
}
