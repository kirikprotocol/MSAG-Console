#include "CommandDispatcher.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <log4cpp/NDC.hh>

#include <admin/protocol/Command.h>
#include <admin/protocol/Response.h>
#include <util/config/Config.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::protocol::Command;
using smsc::admin::protocol::Response;
using smsc::admin::AdminException;
using smsc::util::config::Config;

const char * const CommandDispatcher::task_name = "CommandDispatcher";

CommandDispatcher::CommandDispatcher(ServiceSocketListener * parentListener,
																		 int admSocket,
																		 const char * const client_addr,
																		 ServiceCommandHandler * commandHandler)
	: logger(smsc::util::Logger::getCategory("smsc.admin.service.CommandDispatcher")),
	  reader(admSocket),
	  writer(admSocket)
{
	sock = admSocket;
	memcpy(cl_addr, client_addr, 15);
	cl_addr[15] = 0;
	logger.debug("Command dispatcher \"%s\" created.", cl_addr);
	handler = commandHandler;
	parent = parentListener;
}

CommandDispatcher::~CommandDispatcher()
{
	logger.debug("Command dispatcher \"%s\" destroyed.", task_name);
	sock = 0;
}

void CommandDispatcher::init()
{
	char thr[11];
	logger.debug("Command dispather starting...");
	std::sprintf(thr, "[%.8X]", thr_self());
	char ndc[strlen(thr)+strlen(cl_addr)+1];
	std::strcpy(ndc, thr);
	std::strcat(ndc, cl_addr);
	log4cpp::NDC::push(ndc);
	logger.info("Command dispather started");
}

int CommandDispatcher::Execute()
{
	init();

	Command *command = 0;

	do
	{
		std::auto_ptr<Response> response(0);
		try {
			command = reader.read();
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
			case Command::shutdown:
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
			delete command;
		}
		catch (AdminException &e)
		{
			logger.warn("Command reading failed with exception: %s", e.what());
			break;
		}
		catch (char * e)
		{
			logger.warn("Command reading failed with exception: %s", e);
			break;
		}
		catch (...)
		{
			logger.warn("Command reading failed with unknown exception");
			break;
		}

		try
		{
			if (response.get() == 0)
			{
				response = new Response(Response::Error, 0);
			}
			writer.write(*response);
			response.reset();
		}
		catch (...)
		{
			logger.error("Response writing failed with unknown exception");
			break;
		}

	} while ((command->getId() != Command::undefined) &&
					 (command->getId() != Command::shutdown));
	close(sock);
	logger.info("Command dispather stopped");
	log4cpp::NDC::pop();
}

}
}
}

