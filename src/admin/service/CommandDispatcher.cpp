#include "CommandDispatcher.h"

#include <log4cpp/NDC.hh>

#include <admin/protocol/Command.h>
#include <admin/protocol/Response.h>
#include <util/config/Config.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {

Response * CommandDispatcher::handle(const Command * const command)
	throw (AdminException &)
{
	try
	{
		Response * response = 0;
	
		switch (command->getId())
		{
		case Command::get_config:
			{
				const Config & config = handler->getConfig();
				std::auto_ptr<char> data(config.getTextRepresentation());
				response = new Response(Response::Ok, data.get());
			}
			break;
		case Command::set_config:
			{
				Config config(command->getData().getDocumentElement());
				handler->setConfig(config);
				response = new Response(Response::Ok, 0);
				break;
			}
		case Command::get_logs:
			{
				const char * const logs = handler->getLogs(0, 5);
				if (logs != 0)
				{
					response = new Response(Response::Ok, logs);
					delete[] logs;
				}
			}
			break;
		case Command::get_monitoring:
			{
				const MonitoringData& mon = handler->getMonitoring();;
				if ((&mon) != 0)
				{
					response = new Response(Response::Ok, mon.getText());
				}
			}
			break;
		case Command::shutdown_service:
			{
				bool result = handler->shutdown();
				response = new Response(result ? Response::Ok : Response::Error, "");
				if (result && (parent != 0))
				{
					parent->shutdown();
				}
			}
			break;
		default:
			logger.warn("error in parsing: unknown command");
		}
	
		return response;
	}
	catch (AdminException &e)
	{
		throw e;
	}
	catch (...) {
		throw AdminException("Unknown exception in command dispatcher");
	}
}

}
}
}

