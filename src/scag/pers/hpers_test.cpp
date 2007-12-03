/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "version.inc"
#include "util/mirrorfile/mirrorfile.h"

#include "ProfileStore.h"
#include "CentralPersServer.h"

#include "PersServer.h"
#include "RegionPersServer.h"
#include "string"
#include "Glossary.h"

#include "core/threads/Thread.hpp"

using std::string;
using namespace scag::pers;
using namespace scag::cpers;
using namespace smsc::util::config;
using smsc::core::threads::Thread;

static PersServer *rp2_ps = NULL;
static PersServer *rp1_ps = NULL;
static CentralPersServer *central_ps = NULL;

class PersThread: public Thread {
public:
  PersThread(const char* _task_name) {
    task_name = _task_name;
  }

  void setPersServer(PersSocketServer* pers) {
    pers_server = pers;
  }

  virtual int Execute() {
    if (!pers_server) {
      return 0;
    }
    return pers_server->Execute();
  }
  const char* taskName() {
    return task_name;
  }
private:
  PersSocketServer* pers_server;
  const char* task_name;
};

static PersThread central_thread("CentralPers");
static PersThread rp1_thread("RegionPersRP1");

static int stop_signal = 0;

extern "C" static void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pers");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    stop_signal = sig;
    if (sig==SIGTERM || sig==SIGINT)
    {
      if (rp2_ps) {
        rp2_ps->Stop();
      }
      smsc_log_info(logger, "Stopping ...");
    }
    else if(sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" static void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

PersServer* initRegionPersServer(int argc, char* argv[], Manager& manager, const string& section_name,
 Logger* logger, StringProfileStore& AbonentStore, IntProfileStore& ServiceStore,
                                  IntProfileStore& OperatorStore, IntProfileStore& ProviderStore) {
  string storageDir;
  int resultCode = 0;
  string host;
  int port = 47111;
  int maxClientCount = 100, recCnt = 1000, timeout = 600;
  ConfigView persConfig(manager, section_name.c_str());
  try { storageDir = persConfig.getString("storage_dir"); } catch (...) {};
  int len = storageDir.length();
  if( len > 0 && storageDir[len - 1] != '\\' && storageDir[len - 1] != '/')
      storageDir += '/';

  try { recCnt = persConfig.getInt("init_record_count"); } catch (...) { recCnt = 1000; };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Init Abonent's profiles storage 
  string abnt_section_name = section_name + ".AbntProfStorage"; 
  ConfigView abntProfStorConfig(manager, abnt_section_name.c_str());

  string storageName;
  try { storageName = abntProfStorConfig.getString("storageName"); } catch (...) { storageName = "abonent";
      smsc_log_warn(logger, "Parameter <%s.AbntProfStorage.storageName> missed. Defaul value is 'abonent'",
                     section_name.c_str());
  }
  string storagePath;
  try { storagePath = abntProfStorConfig.getString("storagePath"); } catch (...) { storagePath = "./";
      smsc_log_warn(logger, "Parameter <%s.AbntProfStorage.storagePath> missed. Defaul value is './'",
                    section_name.c_str());
  }
  int indexGrowth;
  try { indexGrowth = abntProfStorConfig.getInt("indexGrowth"); } catch (...) { indexGrowth = 0;
      smsc_log_warn(logger, "Parameter <%s.AbntProfStorage.indexGrowth> missed. Defaul value is 0 (automatic size selection)",
                    section_name.c_str());
  }
  int dataBlockSize;
  try { dataBlockSize = abntProfStorConfig.getInt("dataBlockSize"); } catch (...) { dataBlockSize = 0;
      smsc_log_warn(logger, "Parameter <%s.AbntProfStorage.dataBlockSize> missed. Defaul value is 0 (automatic size selection)",
                    section_name.c_str());
  }
  int blocksInFile;
  try { blocksInFile = abntProfStorConfig.getInt("blocksInFile"); } catch (...) { blocksInFile = 0;
      smsc_log_warn(logger, "Parameter <%s.AbntProfStorage.blocksInFile> missed. Defaul value is 0 (automatic size selection)",
                    section_name.c_str());
  }
  int cacheSize;
  try { cacheSize = abntProfStorConfig.getInt("cacheSize"); } catch (...) { cacheSize = 10000;
      smsc_log_warn(logger, "Parameter <%s.AbntProfStorage.blocksInFile> missed. Defaul value is 10000",
                    section_name.c_str());
  }

  if(argc > 1 && !strcmp(argv[1], "--rebuild-index"))
  {
      smsc_log_info(logger, "Index rebuilding started");                        
      FSDBProfiles<AbntAddr> store;
      store.RebuildIndex(storageName, storagePath, indexGrowth);
      smsc_log_info(logger, "Index rebuilding finished.");
      return 0;
  }

  //StringProfileStore AbonentStore;
  //IntProfileStore ServiceStore, OperatorStore, ProviderStore;

  AbonentStore.init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize, cacheSize,
      Logger::getInstance("pvss.abnt"));
  Glossary::Open(storagePath + "/glossary");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Init cache Params 
  string cache_section_name = section_name + ".cache_max";
  ConfigView cacheConfig(manager, cache_section_name.c_str());
  int cm;
  try { cm = cacheConfig.getInt("service"); } catch (...) { cm = 1000; };
  ServiceStore.init(storageDir + "service", recCnt, cm, Logger::getInstance("pvss.serv"));

  try { cm = cacheConfig.getInt("operator"); } catch (...) { cm = 1000; };
  OperatorStore.init(storageDir + "operator", recCnt, cm, Logger::getInstance("pvss.oper"));

  try { cm = cacheConfig.getInt("provider"); } catch (...) { cm = 1000; };
  ProviderStore.init(storageDir + "provider", recCnt, cm, Logger::getInstance("pvss.prov"));

  try { host = persConfig.getString("host"); } catch (...) {};
  try { port = persConfig.getInt("port"); } catch (...) {};
  try { maxClientCount = persConfig.getInt("connections"); } catch (...) {};

  try { timeout = persConfig.getInt("timeout"); } catch (...) {};

  bool heirarchical_mode = false;
  string central_host;
  int central_port = 0;
  string region_psw;
  uint32_t region_id = 0;
  try {
    heirarchical_mode = persConfig.getBool("hierarchicalMode");
    try {
      central_host = persConfig.getString("centralHost");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <%s.centralHost> missed. heirarchicalMode=false",
                     section_name.c_str());
      heirarchical_mode = false;
    }
    try {
      central_port = persConfig.getInt("centralPort");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <%s.centralPort> missed. heirarchicalMode=false",
                    section_name.c_str());
      heirarchical_mode = false;
    }
    try {
      region_psw = persConfig.getString("regionPassword");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <%s.regionPassword> missed. heirarchicalMode=false",
                    section_name.c_str());
      heirarchical_mode = false;
    }
    try {
      region_id = persConfig.getInt("regionId");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <%s.regionId> missed. heirarchicalMode=false",
                    section_name.c_str());
      heirarchical_mode = false;
    }
  } catch (...) {
    smsc_log_warn(logger, "Parameter <%s.hierarchicalMode> missed. Defaul value is false",
                  section_name.c_str());
    heirarchical_mode = false;
  }

  if (heirarchical_mode) {
    smsc_log_info(logger, "PersServer start in hierarchical mode");
    return new RegionPersServer(host.c_str(), port, maxClientCount, timeout, &AbonentStore,
                              &ServiceStore, &OperatorStore, &ProviderStore, central_host,
                              central_port, region_id, region_psw);


  } else {
    return new PersServer(host.c_str(), port, maxClientCount, timeout, &AbonentStore,
                         &ServiceStore, &OperatorStore, &ProviderStore);
  }
}

int main(int argc, char* argv[])
{
    std::string storageDir;
    int resultCode = 0;
    std::string host;
    int port = 47333;
    int maxClientCount = 100, timeout = 600;

    Logger::Init();
    Logger* log = Logger::getInstance("pers");
    Logger* clog = Logger::getInstance("cpers");
    Logger* rp1_log = Logger::getInstance("rp1_pers");
    Logger* rp2_log = Logger::getInstance("rp2_pers");

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

    try{
        smsc_log_info(log,  "Starting up %s", getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
    
        ConfigView persConfig(manager, "CentralPers");
    
        string storageName;
        try { storageName = persConfig.getString("storageName"); } catch (...) { storageName = "abonent";
            smsc_log_warn(clog, "Parameter <pers.storageName> missed. Defaul value is 'abonent'");
        }
        string storagePath;
        try { storagePath = persConfig.getString("storagePath"); } catch (...) { storagePath = "./";
            smsc_log_warn(clog, "Parameter <pers.storagePath> missed. Defaul value is './'");
        }
        int indexGrowth;
        try { indexGrowth = persConfig.getInt("indexGrowth"); } catch (...) { indexGrowth = 0;
            smsc_log_warn(clog, "Parameter <pers.indexGrowth> missed. Defaul value is 0");
        }
    
        try { host = persConfig.getString("host"); } catch (...) {};
        try { port = persConfig.getInt("port"); } catch (...) {};
        try { maxClientCount = persConfig.getInt("connections"); } catch (...) {};
	    try { timeout = persConfig.getInt("timeout"); } catch (...) {};
        
        std::string regionsFileName;
	    try { regionsFileName = persConfig.getString("regionsConfigFileName"); } catch (...) {};

		central_ps = new CentralPersServer(host.c_str(), port, maxClientCount, timeout, storagePath, storageName, indexGrowth, regionsFileName.c_str());

        auto_ptr<CentralPersServer> pp(central_ps);

        //ps->InitServer();
        //ps->Execute();
        central_ps->InitServer();
        central_thread.setPersServer(central_ps);

        central_thread.Start();

        StringProfileStore AbonentStoreRP1;
        IntProfileStore ServiceStoreRP1, OperatorStoreRP1, ProviderStoreRP1;

        rp1_ps = initRegionPersServer(argc, argv, manager, "RegionPersRP1",
           rp1_log, AbonentStoreRP1, ServiceStoreRP1, OperatorStoreRP1, ProviderStoreRP1);
        auto_ptr<PersServer> rp1_auto_ptr(rp1_ps);
        rp1_ps->InitServer();
        rp1_thread.setPersServer(rp1_ps);
        rp1_thread.Start();

        StringProfileStore AbonentStoreRP2;
        IntProfileStore ServiceStoreRP2, OperatorStoreRP2, ProviderStoreRP2;

        rp2_ps = initRegionPersServer(argc, argv, manager, "RegionPersRP2",
           rp2_log, AbonentStoreRP2, ServiceStoreRP2, OperatorStoreRP2, ProviderStoreRP2);
        auto_ptr<PersServer> rp2_auto_ptr(rp2_ps);
        //PersThread rp2_thread(ps_rp2.get(), "RegionPersRP2");
        //rp2_thread.Start();
        rp2_ps->InitServer();
        rp2_ps->Execute();

        if (rp1_ps) {
          rp1_ps->Stop();
        }
        
        stop_signal ? rp1_thread.Kill(stop_signal) : rp1_thread.Kill(9);

        if (central_ps) {
          central_ps->Stop();
        }
        stop_signal ? central_thread.Kill(stop_signal) : central_thread.Kill(9);

        smsc_log_error(clog, "CentralPersServer stopped");
    }
    catch (ConfigException& exc) 
    {
        smsc_log_error(log, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        smsc_log_error(log, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        smsc_log_error(log, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        smsc_log_error(log, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }
	
    return resultCode;
}
