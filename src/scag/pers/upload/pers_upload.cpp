#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>
#include <string>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "scag/config/ConfigManager.h"
#include "scag/pers/Glossary.h"
#include "scag/pers/upload/BlocksHSReader.h"
#include "scag/pers/upload/PersClient.h"

using std::string;
using scag::pers::AbntAddr;
using scag::pers::util::PersClient;
using scag::pers::util::PersClientException;
using namespace smsc::util::config;
using scag::pers::util::BlocksHSReader;
using scag::pers::Glossary;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pers");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
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

    int open_result = Glossary::OPEN_ERROR;
    int resultCode = 0;
    try{
        smsc_log_info(logger,  "Starting up Pers Upload");
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView persConfig(manager, "PersUpload");

        ConfigView abntProfStorConfig(manager, "PersUpload.AbntProfStorage");
		
		string storageName = "abonent";
		try { 
          storageName = abntProfStorConfig.getString("storageName"); 
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersUpload.AbntProfStorage.storageName> missed. Defaul value is %s", storageName.c_str());
		}
		string storagePath = "./storage";
		try { 
          storagePath = abntProfStorConfig.getString("storagePath");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersUpload.AbntProfStorage.storagePath> missed. Defaul value is %d", storagePath.c_str());
		}
		int dataBlockSize = 2048;
		try {
           dataBlockSize = abntProfStorConfig.getInt("dataBlockSize");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersUpload.AbntProfStorage.dataBlockSize> missed. Defaul value is %d", dataBlockSize);
		}
		int blocksInFile = 50000;
		try {
          blocksInFile = abntProfStorConfig.getInt("blocksInFile");
        } catch (...) {;
		  smsc_log_warn(logger, "Parameter <PersUpload.AbntProfStorage.blocksInFile> missed. Defaul value is %d", blocksInFile);
		}

        bool sendToPers = false;
        try { sendToPers = persConfig.getBool("sendToPers"); } 
        catch (...) { 
            smsc_log_warn(logger, "Parameter <PersUpload.sendToPers> missed. Defaul value is false");
        }

        smsc_log_warn(logger, "Parameter <PersUpload.AbntProfStorage.storagePath> = '%s'", storagePath.c_str());
        std::string host = "phoenix";
        int port = 47880;

        try { 
          host = persConfig.getString("host");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersUpload.host> missed. Defaul value is %s", host.c_str());
        }

        try { 
          port = persConfig.getInt("port");
        } catch (...) {
          smsc_log_warn(logger, "Parameter <PersUpload.port> missed. Defaul value is %d", port);
        }


        smsc_log_debug(logger, "Client connect to %s:%d", host.c_str(), port);  
        int timeOut = 1000;
        int pingTimeOut = 1000;
        int reconnectTimeout = 100;
        int maxWaitRequestsCount = 100;
        int filesCount = 0;
        if (argc > 1) {
          filesCount = atoi(argv[1]);
        } 

        if ((open_result = Glossary::Open(storagePath + "/glossary")) != Glossary::SUCCESS) {
           throw Exception("Glossary open error");  
        }

        if (sendToPers) {
          PersClient::Init(host.c_str(), port, timeOut, pingTimeOut, reconnectTimeout, maxWaitRequestsCount);
          PersClient& pc = PersClient::Instance();
          BlocksHSReader<AbntAddr> reader(pc, storageName, storagePath, dataBlockSize, blocksInFile);
          smsc_log_debug(logger, "will be read %d files", filesCount);
          reader.readDataFiles(filesCount, sendToPers);
        } else {
          PersClient* pc = 0;
          BlocksHSReader<AbntAddr> reader(*pc, storageName, storagePath, dataBlockSize, blocksInFile);
          smsc_log_debug(logger, "will be read %d files", filesCount);
          reader.readDataFiles(filesCount, sendToPers);
        }
    }

    catch (const PersClientException& exc) 
    {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
        resultCode = -1;
    }
    catch (const ConfigException& exc) 
    {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }   
    catch (const Exception& exc) 
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
    if (open_result == Glossary::SUCCESS) {
      Glossary::Close();
    }
    return resultCode;
}
