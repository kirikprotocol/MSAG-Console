#include "DaemonCommandDispatcher.h"

#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::protocol::Command;
using smsc::admin::protocol::CommandStartService;
using smsc::admin::protocol::CommandKillService;
using smsc::admin::protocol::CommandAddService;
using smsc::admin::protocol::CommandRemoveService;
using smsc::admin::protocol::CommandListServices;

Response * DaemonCommandDispatcher::handle(const Command * const command)
	throw (AdminException &)
{
	try
	{
		switch (command->getId())
		{
		case Command::start_service:
			return start_service(command);
		case Command::kill_service:
			return kill_service(command);
		case Command::add_service:
			return add_service(command);
		case Command::remove_service:
			return remove_service(command);
		case Command::list_services:
			return list_services(command);
		default:
			return new Response(Response::Error, "Unknown command");
		}
	}
	catch (AdminException &e)
	{
		return new Response(Response::Error, e.what());
	}
	catch (const char * const e)
	{
		return new Response(Response::Error, e);
	}
	catch (...)
	{
		return new Response(Response::Error, "Unknown exception");
	}
}

Response * DaemonCommandDispatcher::start_service(const Command * const command)
	throw (AdminException &)
{
	logger.debug("start service");
	pid_t newPid = handler->start_service(dynamic_cast<const CommandStartService * const>(command));
	std::auto_ptr<char> text(new char[sizeof(pid_t)*3 +1]);
	sprintf(text.get(),  "%lu", (unsigned long) newPid);
	return new Response(Response::Ok, text.get());
}

Response * DaemonCommandDispatcher::kill_service(const Command * const command)
	throw (AdminException &)
{
	logger.debug("kill service");
	handler->kill_service(dynamic_cast<const CommandKillService * const>(command));
	return new Response(Response::Ok, 0);
}

Response * DaemonCommandDispatcher::add_service(const Command * const command)
	throw (AdminException &)
{
	logger.debug("add service");
	handler->add_service(dynamic_cast<const CommandAddService * const>(command));
	return new Response(Response::Ok, 0);
}

Response * DaemonCommandDispatcher::remove_service(const Command * const command)
	throw (AdminException &)
{
	logger.debug("remove service");
	handler->remove_service(dynamic_cast<const CommandRemoveService * const>(command));
	return new Response(Response::Ok, 0);
}

Response * DaemonCommandDispatcher::list_services(const Command * const command)
	throw (AdminException &)
{
	logger.debug("list services");
	const CommandListServices * const list_Services_command = dynamic_cast<const CommandListServices * const>(command);
	logger.debug("list_Services_command = %p", list_Services_command);
	const ServicesList & list = handler->list_services(list_Services_command);
	std::auto_ptr<char> text(list.getText());
	logger.debug("services list:\n%s\n", text.get());
	return new Response(Response::Ok, text.get());
}

}
}
}
