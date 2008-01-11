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

#include "string"

using std::string;
using namespace scag::pers;
using namespace scag::cpers;
using namespace smsc::util::config;

static CentralPersServer *ps = NULL;

extern "C" static void appSignalHandler(int sig)
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

extern "C" static void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[])
{
    std::string storageDir;
    int resultCode = 0;
    std::string host;
    int port = 9988;
    int maxClientCount = 100, timeout = 600;
    int transactTimeout = 100;

    Logger::Init();
    Logger* logger = Logger::getInstance("cpers");

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
        smsc_log_info(logger,  "Starting up %s", getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
    
        ConfigView persConfig(manager, "CentralPers");
    
        string storageName;
        try { storageName = persConfig.getString("storageName"); } catch (...) { storageName = "abonent";
            smsc_log_warn(logger, "Parameter <pers.storageName> missed. Defaul value is 'abonent'");
        }
        string storagePath;
        try { storagePath = persConfig.getString("storagePath"); } catch (...) { storagePath = "./";
            smsc_log_warn(logger, "Parameter <pers.storagePath> missed. Defaul value is './'");
        }
        int indexGrowth;
        try { indexGrowth = persConfig.getInt("indexGrowth"); } catch (...) { indexGrowth = 0;
            smsc_log_warn(logger, "Parameter <pers.indexGrowth> missed. Defaul value is 0");
        }
    
        try { host = persConfig.getString("host"); } catch (...) {};
        try { port = persConfig.getInt("port"); } catch (...) {};
        try { maxClientCount = persConfig.getInt("connections"); } catch (...) {};
	    try { timeout = persConfig.getInt("timeout"); } catch (...) {};
        try { 
          transactTimeout = persConfig.getInt("transactTimeout"); 
        } catch (...) {
          smsc_log_warn(logger, "Parameter <pers.transactTimeout> missed. Defaul value is %d",
                         transactTimeout);
        };
        
        std::string regionsFileName;
	    try { regionsFileName = persConfig.getString("regionsConfigFileName"); } catch (...) {};

		ps = new CentralPersServer(host.c_str(), port, maxClientCount, timeout, transactTimeout,
                                   storagePath, storageName, indexGrowth, regionsFileName.c_str());

        auto_ptr<CentralPersServer> pp(ps);

        ps->InitServer();
        ps->Execute();
        smsc_log_error(logger, "CentralPersServer stopped");
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
	
    return resultCode;
}
