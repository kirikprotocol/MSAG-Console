#include <iostream>
#include <util/config/Manager.h>
#include <util/xml/init.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/AdminSocketManager.h>
#include <admin/service/ServiceAdminInterfaceStarter.h>
#include <admin/service/ServiceSocketListener.h>
#include <admin/service/ServiceCommandDispatcher.h>
#include <admin/service/test/DumbServiceCommandHandler.h>
#include <admin/service/test/DumbServiceShutdownHandler.h>

using log4cpp::Category;
using std::cout;
using std::cerr;
using std::endl;
using smsc::admin::AdminException;
using smsc::admin::service::ComponentManager;
using smsc::admin::service::AdminSocketManager;
using smsc::admin::service::ServiceAdminInterfaceStarter;
using smsc::admin::service::ServiceSocketListener;
using smsc::admin::service::ServiceCommandDispatcher;
using smsc::admin::service::test::DumbServiceCommandHandler;
using smsc::admin::service::test::DumbServiceShutdownHandler;
using smsc::util::config::Manager;
using smsc::util::Logger;
using smsc::util::xml::initXerces;

int main (int argc, char *argv[])
{
	initXerces();
	char * configFileStr = 0;
	if (argc < 3)
	{
		printf("suage: smsc_dumb_service port <full name of config file>\n\r");
		//return -1;
		Manager::init("config.xml");
	} else {
		Manager::init(argv[2]);
	}
	int servicePort = 0;
	if (argc < 2)
	{
		servicePort = 6677;
	}
	else
	{
		servicePort = atoi(argv[1]);
	}

	Category &logger(Logger::getCategory("smsc.admin.service.test.ServiceSocketListenerTest"));
 	try {
		for (int i=0; i<argc; i++)
			logger.debug("Param[%u]=\"%s\"", i, argv[i]);
 		

		Manager &config = Manager::getInstance();
		logger.debug("Initializing service");
		ComponentManager::registerComponent(new DumbServiceCommandHandler());
 		logger.debug("Starting service");
		DumbServiceShutdownHandler::registerShutdownHandler(new DumbServiceShutdownHandler());
		AdminSocketManager::start(config.getString("dumbtest.host"), servicePort);
 		logger.debug("Service started");
		AdminSocketManager::WaitFor();
 		logger.debug("Service stopped");
 	} catch (AdminException &e) {
 		logger.debug("Exception occured: \"%s\"", e.what());
 	} catch (...) {
 		logger.debug("Unknown Exception occured");
 	}
 	return 0;
}
