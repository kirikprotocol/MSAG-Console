/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sys/stat.h>

#include <signal.h>

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

static PersServer *ps = NULL;

static smsc::logger::Logger *logger;

extern "C" static void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
		if(ps) ps->Stop();
        smsc_log_info(logger, "Stopping ...");
    }
}

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

    sigset_t set, old;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, &old);
	sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT, appSignalHandler);

	try{
		smsc_log_info(logger,  "Starting up %s", getStrVersion());

		Manager::init("config.xml");
	    Manager& manager = Manager::getInstance();

		ConfigView persConfig(manager, "pers");

        try { storageDir = persConfig.getString("storage_dir"); } catch (...) {};
		int len = storageDir.length();
		if( len > 0 && storageDir[len - 1] != '\\' && storageDir[len - 1] != '/')
			storageDir += '/';

		ConfigView cacheConfig(manager, "pers.cache_max");

		uint32_t cm;
        try { cm = cacheConfig.getInt("abonent"); } catch (...) { cm = 1000; };
		AbonentStore.init(storageDir + "abonent", cm);

        try { cm = cacheConfig.getInt("service"); } catch (...) { cm = 1000; };
		ServiceStore.init(storageDir + "service", cm);

        try { cm = cacheConfig.getInt("operator"); } catch (...) { cm = 1000; };
		OperatorStore.init(storageDir + "operator", cm);

        try { cm = cacheConfig.getInt("provider"); } catch (...) { cm = 1000; };
		ProviderStore.init(storageDir + "provider", cm);

        try { host = persConfig.getString("host"); } catch (...) {};
        try { port = persConfig.getInt("port"); } catch (...) {};
        try { maxClientCount = persConfig.getInt("connections"); } catch (...) {};

		ps = new PersServer(host.c_str(), port, maxClientCount, 
			new CommandDispatcher(&AbonentStore, &ServiceStore, &OperatorStore, &ProviderStore));

		auto_ptr<PersServer> pp(ps);

/*		PersServer pp(host.c_str(), port, maxClientCount, 
			new CommandDispatcher(&AbonentStore, &ServiceStore, &OperatorStore, &ProviderStore));*/

		ps->Execute();

/*		sigemptyset(&set);
		sigaddset(&set, SIGINT);
		sigaddset(&set, SIGTERM);
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
