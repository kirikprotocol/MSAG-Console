#include <system/smsc.hpp>
#include <admin/service/ComponentManager.h>
#include <admin/service/AdminSocketManager.h>
#include <admin/smsc_service/SmscComponent.h>
#include <admin/smsc_service/SmscShutdownHandler.h>
#include <util/Logger.h>
#include <util/config/alias/aliasconf.h>

using smsc::system::SmscConfigs;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::util::config::smeman::SmeManConfig;
using smsc::admin::smsc_service::SmscComponent;
using smsc::admin::smsc_service::SmscShutdownHandler;
using smsc::admin::service::ComponentManager;
using smsc::admin::service::AdminSocketManager;
using smsc::admin::AdminException;
using smsc::util::config::alias::AliasConfig;

int main(int argc,char* argv[])
{
	try
	{
		// load configs
		SmscConfigs cfgs;
		Manager::init("../conf/config.xml");
		cfgs.cfgman=&cfgs.cfgman->getInstance();

		Logger::Init(cfgs.cfgman->getString("logger.initFile"));

		SmeManConfig smemancfg;
		smemancfg.load("../conf/sme.xml");
		cfgs.smemanconfig=&smemancfg;

		AliasConfig aliasConfig;
		aliasConfig.load("../conf/aliases.xml");
		cfgs.aliasconfig = &aliasConfig;

		// init Admin part
		SmscComponent smsc_component(cfgs);
		ComponentManager::registerComponent(&smsc_component);

		SmscShutdownHandler shutdown_handler(smsc_component);
		SmscShutdownHandler::registerShutdownHandler(&shutdown_handler);

		in_port_t servicePort = 0;
		if (argc > 1)
		{
			servicePort = atoi(argv[1]);
		}
		if (servicePort == 0)
		{
			servicePort = cfgs.cfgman->getInt("admin.port");
		}

		// start
		smsc_component.runSmsc();
		AdminSocketManager::start(cfgs.cfgman->getString("admin.host"), servicePort);
		fprintf(stderr,"All started");

		//running
		AdminSocketManager::WaitFor();

		// stopped
		smsc_component.stopSmsc();
		Manager::deinit();
		fprintf(stderr,"All stopped");
	}
	catch (AdminException &e)
	{
		fprintf(stderr, "Admin exception occured:%s\n", e.what());
	}
	catch (std::exception &e)
	{
		fprintf(stderr, "exception occured:%s\n", e.what());
	}
	catch (...)
	{
		fprintf(stderr, "some exception occured\n");
	}

	return 0;
}
