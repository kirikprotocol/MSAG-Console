#include <iostream>
#include <admin/service/ComponentManager.h>
#include <admin/service/AdminSocketManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <admin/service/ServiceCommandDispatcher.h>
#include <admin/service/test/DumbServiceCommandHandler.h>
#include <admin/service/test/DumbServiceShutdownHandler.h>
#include <util/config/Manager.h>
#include <util/xml/init.h>
#include <util/config/ConfigException.h>

using log4cpp::Category;
using std::cout;
using std::cerr;
using std::endl;
using smsc::admin::AdminException;
using smsc::admin::service::ComponentManager;
using smsc::admin::service::AdminSocketManager;
using smsc::admin::service::ServiceSocketListener;
using smsc::admin::service::ServiceCommandDispatcher;
using smsc::admin::service::test::DumbServiceCommandHandler;
using smsc::admin::service::test::DumbServiceShutdownHandler;
using smsc::util::config::ConfigException;
using smsc::util::config::Manager;
using smsc::util::Logger;
using smsc::util::xml::initXerces;

int main (int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("suage: smsc_dumb_service port \n\r");
		return -1;
	}
 	try {
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
		DumbServiceShutdownHandler shutdown_handler;
		DumbServiceShutdownHandler::registerShutdownHandler(&shutdown_handler);
		std::auto_ptr<char> host(config.getString("dumbtest.host"));
		AdminSocketManager::start(host.get(), servicePort);
 		logger.debug("Service started");
		AdminSocketManager::WaitFor();
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
