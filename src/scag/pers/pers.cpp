#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "version.inc"

#include "ProfileStore.h"

using namespace scag::pers;
using namespace smsc::util::config;
using namespace std;

static smsc::logger::Logger *logger;

extern "C" static void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[])
{
	string storageDir;
	int resultCode = 0;

	Logger::Init();
	logger = Logger::getInstance("scag.pers.pers");

	StringProfileStore AbonentStore;
	IntProfileStore ServiceStore, OperatorStore, ProviderStore;

	atexit(atExitHandler);

	try{
		Manager::init("config.xml");
	    Manager& manager = Manager::getInstance();

		ConfigView persConfig(manager, "pers");

        try { storageDir = persConfig.getString("storage_dir"); } catch (...) {};
		int len = storageDir.length();
		if( len > 0 && storageDir[len - 1] != '\\' && storageDir[len - 1] != '/')
			storageDir += '/';

		AbonentStore.init(storageDir + "abonent", 1);
		ServiceStore.init(storageDir + "service", 1);
		OperatorStore.init(storageDir + "operator", 1);
		ProviderStore.init(storageDir + "provider", 1);

		smsc_log_info(logger,  "Starting up %s", getStrVersion());

		Property prop;
		prop.setInt("test_val", 234567, FIXED, -1, 20);
		smsc_log_debug(logger,  "setProperty: %s", prop.toString().c_str());
		StringProfileKey spk("+79232446251");
		AbonentStore.setProperty(spk, &prop);
		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		prop.setString("test_val_string", "test_string", W_ACCESS, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		prop.setDate("test_val_string1", 111111, INFINIT, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		auto_ptr<Property> pp( AbonentStore.getProperty(spk, "test_val"));
		if(pp.get() != NULL)
			smsc_log_debug(logger,  "pers %s", pp.get()->toString().c_str());
		smsc_log_debug(logger,  "end");
/*		AbonentStore.shutdown();
		ServiceStore.shutdown();
		OperatorStore.shutdown();
		ProviderStore.shutdown();*/
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
