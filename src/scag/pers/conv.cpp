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

#include "string"
#include "Glossary.h"

using std::string;
using namespace scag::pers;
using namespace smsc::util::config;

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[])
{
    int resultCode = 0;

    Logger::Init();
    Logger* logger = Logger::getInstance("pers");

    atexit(atExitHandler);

    try{
	    FSDBProfiles<AbntAddr> AbonentStore, AbonentStore1;
	
        smsc_log_info(logger,  "Starting up %s", getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView persConfig(manager, "pers");

//	Init Abonent's profiles storage 
		ConfigView abntProfStorConfig(manager, "pers.fromAbntProfStorage");
		
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

        AbonentStore.Init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize);
        
//	Init Abonent's profiles storage 
		ConfigView abntProfStorConfig1(manager, "pers.toAbntProfStorage");
		
		try { storageName = abntProfStorConfig1.getString("storageName"); } catch (...) { storageName = "abonent";
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.storageName> missed. Defaul value is 'abonent'");
		}
		try { storagePath = abntProfStorConfig1.getString("storagePath"); } catch (...) { storagePath = "./";
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.storagePath> missed. Defaul value is './'");
		}
		try { indexGrowth = abntProfStorConfig1.getInt("indexGrowth"); } catch (...) { indexGrowth = 0;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.indexGrowth> missed. Defaul value is 0 (automatic size selection)");
		}
		try { dataBlockSize = abntProfStorConfig1.getInt("dataBlockSize"); } catch (...) { dataBlockSize = 0;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.dataBlockSize> missed. Defaul value is 0 (automatic size selection)");
		}
		try { blocksInFile = abntProfStorConfig1.getInt("blocksInFile"); } catch (...) { blocksInFile = 0;
			smsc_log_warn(logger, "Parameter <pers.AbntProfStorage.blocksInFile> missed. Defaul value is 0 (automatic size selection)");
		}
        AbonentStore1.Init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize);
        
        AbonentStore.Reset();
        AbntAddr key;
        DataBlock data;
        int i = 0;
        while(AbonentStore.Next(key, data))
        {
            //AbonentStore1.Set(key, data);
            i++;
        }
        smsc_log_debug(logger, "Records converted: %d", i);
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
