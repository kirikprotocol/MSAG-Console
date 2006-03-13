/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "version.inc"

#include "ProfileStore.h"
#include "PersServer.h"
#include "CmdDispatcher.h"

using namespace scag::pers;
using namespace smsc::util::config;
using namespace std;

static smsc::logger::Logger *logger;

static PersServer *ps;

/*extern "C" static void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
    }
}*/

extern "C" static void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[])
{
	string storageDir;
	int resultCode = 0;
	string host;
	int port = 9988;
	int maxClientCount = 100;

	Logger::Init();
	logger = Logger::getInstance("pers");

	StringProfileStore AbonentStore;
	IntProfileStore ServiceStore, OperatorStore, ProviderStore;

	atexit(atExitHandler);

/*    sigset_t set, old;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, &old);
	sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
    sigset(SIGINT, appSignalHandler);*/

	try{
		smsc_log_info(logger,  "Starting up %s", getStrVersion());

		Manager::init("config.xml");
	    Manager& manager = Manager::getInstance();

		ConfigView persConfig(manager, "pers");

        try { storageDir = persConfig.getString("storage_dir"); } catch (...) {};
		int len = storageDir.length();
		if( len > 0 && storageDir[len - 1] != '\\' && storageDir[len - 1] != '/')
			storageDir += '/';

		uint32_t cm;
        try { cm = persConfig.getInt("abonent_cache_max"); } catch (...) { cm = 1000; };
		AbonentStore.init(storageDir + "abonent", cm);

        try { cm = persConfig.getInt("service_cache_max"); } catch (...) { cm = 1000; };
		ServiceStore.init(storageDir + "service", cm);

        try { cm = persConfig.getInt("operator_cache_max"); } catch (...) { cm = 1000; };
		OperatorStore.init(storageDir + "operator", cm);

        try { cm = persConfig.getInt("provider_cache_max"); } catch (...) { cm = 1000; };
		ProviderStore.init(storageDir + "provider", cm);

        try { host = persConfig.getString("host"); } catch (...) {};
        try { port = persConfig.getInt("port"); } catch (...) {};
        try { maxClientCount = persConfig.getInt("connections"); } catch (...) {};

		auto_ptr<PersServer> pp(new PersServer(host.c_str(), port, maxClientCount, 
			new CommandDispatcher(&AbonentStore, &ServiceStore, &OperatorStore, &ProviderStore)));

		pp.get()->Execute();

/*		sigemptyset(&set);
		sigaddset(&set, SIGINT);
		sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
		sigprocmask(SIG_SETMASK, &set, &old);*/
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
