#ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_HANDLER
#define SMSC_ADMIN_DAEMON_DAEMON_COMMAND_HANDLER

#include <log4cpp/Category.hh>
#include <admin/daemon/Service.h>
#include <admin/daemon/ServicesList.h>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::protocol::Command;
using smsc::admin::protocol::CommandStartService;
using smsc::admin::protocol::CommandKillService;
using smsc::admin::protocol::CommandAddService;
using smsc::admin::protocol::CommandRemoveService;
using smsc::admin::protocol::CommandListServices;
using smsc::util::Logger;

class DaemonCommandHandler {
public:
	DaemonCommandHandler()
		: logger(Logger::getCategory("smsc.admin.daemon.DaemonCommandHandler"))
	{
	}

	pid_t start_service(const CommandStartService * const command)
		throw (AdminException &)
	{
		logger.debug("start service");
		if (command != 0)
		{
			if (command->getServiceName() != 0)
			{
				return services[command->getServiceName()].start();
			}
			else
			{
				logger.warn("service name not specified");
				throw AdminException("service name not specified");
			}
		}
		else
		{
			logger.warn("null command received");
			throw AdminException("null command received");
		}
	}
	
	void kill_service(const CommandKillService * const command)
		throw (AdminException &)
	{
		logger.debug("kill service");
		if (command != 0)
		{
			if (command->getServiceName() != 0)
			{
				services[command->getServiceName()].kill();
			}
			else
			{
				logger.warn("service name not specified");
			}
		}
		else
		{
			logger.warn("null command received");
		}
	}
	
	void add_service(const CommandAddService * const command)
		throw (AdminException &)
	{
		logger.debug("add service \"%s\" (%s) %u",
								 command->getServiceName(),
								 command->getCmdLine(),
								 command->getPort());
		if (command != 0)
		{
			if (command->getServiceName() != 0 && command->getCmdLine() != 0)
			{
				services.add(Service(command->getServiceName(), command->getCmdLine(), command->getPort(), command->getArgs()));
			}
			else
			{
				logger.warn("service name or command line not specified");
			}
		}
		else
		{
			logger.warn("null command received");
		}
	}
	
	void remove_service(const CommandRemoveService * const command)
		throw (AdminException &)
	{
		logger.debug("remove service \"%s\"", command->getServiceName());
		if (command != 0)
		{
			if (command->getServiceName() != 0)
			{
				Service &s(services[command->getServiceName()]);
				if (s.getStatus() == Service::running)
				{
					s.kill();
				}
				services.remove(s.getName());
			}
			else
			{
				logger.warn("service name not specified");
			}
		}
		else
		{
			logger.warn("null command received");
		}
	}
	
	const ServicesList & list_services(const CommandListServices * const command)
		throw (AdminException &)
	{
		logger.debug("list services");
		return services;
	}

protected:
	log4cpp::Category &logger;
	ServicesList services;
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_HANDLER

