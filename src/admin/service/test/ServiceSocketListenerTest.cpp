#include <iostream>
#include <signal.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/AdminSocketManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <admin/service/ServiceCommandDispatcher.h>
#include <admin/service/test/DumbServiceCommandHandler.h>
#include <util/config/Manager.h>
#include <util/xml/init.h>
#include <util/config/ConfigException.h>
#include <system/smscsignalhandlers.h>

using log4cpp::Category;
using std::cout;
using std::cerr;
using std::endl;
using smsc::admin::AdminException;
using smsc::admin::service::ComponentManager;
using smsc::admin::service::ServiceSocketListener;
using smsc::admin::service::ServiceCommandDispatcher;
using smsc::admin::service::test::DumbServiceCommandHandler;
using smsc::util::config::ConfigException;
using smsc::util::config::Manager;
using smsc::util::Logger;
using smsc::util::xml::initXerces;

void clearThreadSignalMask()
{
	sigset_t set;
	sigemptyset(&set);
	for(int i=1;i<=37;i++)if(i!=SIGQUIT)sigaddset(&set,i);
	if(thr_sigsetmask(SIG_SETMASK,&set,NULL)!=0)
	{
		__warning__("failed to set thread signal mask!");
	};
}

ServiceSocketListener* main_listener = 0;
void sigAbortDispatcher(int signo)
{
	abort();
}

void sigShutdownHandler(int signo)
{
	if (main_listener != 0)
		main_listener->shutdown();
}

void registerSignalHandlers(ServiceSocketListener * listener)
{
	main_listener = listener;
	
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,17);
	sigaddset(&set, SIGBUS);
	sigaddset(&set, SIGFPE);
	sigaddset(&set, SIGILL);
	sigaddset(&set, SIGSEGV);
	sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGALRM);
	sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
//#ifndef SPARC
//	sigaddset(&set,SIGQUIT);
//#endif

	if(thr_sigsetmask(SIG_UNBLOCK,&set,NULL)!=0)
	{
		__warning__("Faield to update signal mask");
	}
	sigset(17,      sigAbortDispatcher);
	sigset(SIGBUS,  sigAbortDispatcher);
	sigset(SIGFPE,  sigAbortDispatcher);
	sigset(SIGILL,  sigAbortDispatcher);
	sigset(SIGSEGV, sigAbortDispatcher);
	sigset(SIGTERM, sigAbortDispatcher);
  sigset(SIGALRM, sigAbortDispatcher);
	sigset(smsc::system::SHUTDOWN_SIGNAL,  sigShutdownHandler);
}

int main (int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("suage: smsc_dumb_service port \n\r");
		return -1;
	}
 	try {
		freopen("dumb.out", "w", stdout);
    freopen("dumb.err", "w", stderr);

		int servicePort = atoi(argv[1]);
		Manager::init("conf/dumbServiceConfig.xml");
		Manager &config = Manager::getInstance();
	
		Logger::Init(config.getString("dumbtest.loggerInitFile"));
		
		Category &logger(Logger::getCategory("smsc.admin.service.test.ServiceSocketListenerTest"));
		
		for (int i=0; i<argc; i++)
			logger.debug("Param[%u]=\"%s\"", i, argv[i]);

		logger.debug("Initializing service");
		DumbServiceCommandHandler main_component;
		ComponentManager::registerComponent(&main_component);
 		logger.debug("Starting service");

    ServiceSocketListener listener("smsc.admin.util.ServiceSocketListener");
		std::auto_ptr<char> host(config.getString("dumbtest.host"));
    listener.init(host.get(), servicePort);
    listener.Start();
    logger.debug("Service started");
		registerSignalHandlers(&listener);

    listener.WaitFor();
		Manager::deinit();
 		logger.debug("Service stopped");
 	} catch (AdminException &e) {
 		Logger::getCategory("smsc.admin.service.test.ServiceSocketListenerTest").debug("Exception occured: \"%s\"", e.what());
	} catch (ConfigException &e) {
 		Logger::getCategory("smsc.admin.service.test.ServiceSocketListenerTest").debug("Exception occured: \"%s\"", e.what());
 	} catch (...) {
 		Logger::getCategory("smsc.admin.service.test.ServiceSocketListenerTest").debug("Unknown Exception occured");
 	}
 	return 0;
}
