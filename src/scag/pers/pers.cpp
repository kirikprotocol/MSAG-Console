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
#include "PersServer.h"
#include "RegionPersServer.h"

#include "string"
#include "Glossary.h"

using std::string;
using namespace scag::pers;
using namespace smsc::util::config;

static PersServer *ps = NULL;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pers");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        if(ps) ps->Stop();
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

int main(int argc, char* argv[])
{
    std::string storageDir;
    int resultCode = 0;
    std::string host;
    int port = 9988;
    int maxClientCount = 100, recCnt = 1000, timeout = 300;

    Logger::Init();
    Logger* logger = Logger::getInstance("pers");

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
    int glossaryOpened = Glossary::GLOSSARY_CLOSED;

    try{
        smsc_log_info(logger,  "Starting up %s", getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView persConfig(manager, "pers");

        try { storageDir = persConfig.getString("storage_dir"); } catch (...) {};
        int len = storageDir.length();
        if( len > 0 && storageDir[len - 1] != '\\' && storageDir[len - 1] != '/')
            storageDir += '/';
	
        try { recCnt = persConfig.getInt("init_record_count"); } catch (...) { recCnt = 1000; };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Init Abonent's profiles storage 
		ConfigView abntProfStorConfig(manager, "pers.AbntProfStorage");
		
		string storageName;
		try { storageName = abntProfStorConfig.getString("storageName"); } catch (...) { storageName = "abonent";
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.storageName> missed. Defaul value is 'abonent'");
		}
		string storagePath;
		try { storagePath = abntProfStorConfig.getString("storagePath"); } catch (...) { storagePath = "./";
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.storagePath> missed. Defaul value is './'");
		}
		int indexGrowth;
		try { indexGrowth = abntProfStorConfig.getInt("indexGrowth"); } catch (...) { indexGrowth = 0;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.indexGrowth> missed. Defaul value is 0 (automatic size selection)");
		}
		int dataBlockSize;
		try { dataBlockSize = abntProfStorConfig.getInt("dataBlockSize"); } catch (...) { dataBlockSize = 0;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.dataBlockSize> missed. Defaul value is 0 (automatic size selection)");
		}
		int blocksInFile;
		try { blocksInFile = abntProfStorConfig.getInt("blocksInFile"); } catch (...) { blocksInFile = 0;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.blocksInFile> missed. Defaul value is 0 (automatic size selection)");
		}
		int cacheSize;
		try { cacheSize = abntProfStorConfig.getInt("cacheSize"); } catch (...) { cacheSize = 10000;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.blocksInFile> missed. Defaul value is 10000");
		}

        if(argc > 1 && !strcmp(argv[1], "--rebuild-index"))
        {
      		smsc_log_info(logger, "Index rebuilding started");                        
            FSDBProfiles<AbntAddr> store;
            store.RebuildIndex(storageName, storagePath, indexGrowth);
            smsc_log_info(logger, "Index rebuilding finished.");
            return 0;
        }

	    StringProfileStore AbonentStore;
	    IntProfileStore ServiceStore, OperatorStore, ProviderStore;
        
        AbonentStore.init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize, cacheSize,
            Logger::getInstance("pvss.abnt"));
		glossaryOpened = Glossary::Open(storagePath + "/glossary");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Init cache Params 
        ConfigView cacheConfig(manager, "pers.cache_max");
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
        int transactTimeout = 0;
        try {
          heirarchical_mode = persConfig.getBool("hierarchicalMode");
          try {
            central_host = persConfig.getString("centralHost");
          } catch (...) {
            smsc_log_warn(logger, "Parameter <pers.centralHost> missed. heirarchicalMode=false");
            heirarchical_mode = false;
          }
          try {
            central_port = persConfig.getInt("centralPort");
          } catch (...) {
            smsc_log_warn(logger, "Parameter <pers.centralPort> missed. heirarchicalMode=false");
            heirarchical_mode = false;
          }
          try {
            region_psw = persConfig.getString("regionPassword");
          } catch (...) {
            smsc_log_warn(logger, "Parameter <pers.regionPassword> missed. heirarchicalMode=false");
            heirarchical_mode = false;
          }
          try {
            region_id = persConfig.getInt("regionId");
          } catch (...) {
            smsc_log_warn(logger, "Parameter <pers.regionId> missed. heirarchicalMode=false");
            heirarchical_mode = false;
          }
          try {
            transactTimeout = persConfig.getInt("transactTimeout");
          } catch (...) {
            transactTimeout = 200;
            smsc_log_warn(logger, "Parameter <pers.transactTimeout> missed. Default value is %d", transactTimeout);
          }
        } catch (...) {
          smsc_log_warn(logger, "Parameter <pers.hierarchicalMode> missed. Defaul value is false");
          heirarchical_mode = false;
        }

		if (heirarchical_mode) {
          ps = new RegionPersServer(host.c_str(), port, maxClientCount, timeout, transactTimeout, &AbonentStore,
                                    &ServiceStore, &OperatorStore, &ProviderStore, central_host,
                                    central_port, region_id, region_psw);

          smsc_log_info(logger, "PersServer start in hierarchical mode");

        } else {
          ps = new PersServer(host.c_str(), port, maxClientCount, timeout, transactTimeout, &AbonentStore,
                               &ServiceStore, &OperatorStore, &ProviderStore);
        }

        auto_ptr<PersServer> pp(ps);

        ps->InitServer();
        ps->Execute();
        smsc_log_error(logger, "PersServer stopped");
    }
    catch (ConfigException& exc) 
    {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }
	if (glossaryOpened == Glossary::SUCCESS) {
      Glossary::Close();
    }
    return resultCode;
}
