#include "CommandDispatcher.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <log4cpp/NDC.hh>

#include <util/config/Config.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace util {

using smsc::admin::AdminException;
using smsc::util::config::Config;

const char * const CommandDispatcher::task_name = "CommandDispatcher";

CommandDispatcher::CommandDispatcher(Shutdownable * parentListener,
																		 int admSocket,
																		 const char * const client_addr,
																		 const char * const loggerCategory = "smsc.admin.util.CommandDispatcher")
	: logger(smsc::util::Logger::getCategory(loggerCategory)),
	  reader(admSocket),
	  writer(admSocket)
{
	sock = admSocket;
	memcpy(cl_addr, client_addr, 15);
	cl_addr[15] = 0;
	logger.debug("Command dispatcher \"%s\" created.", cl_addr);
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

	std::auto_ptr<Command> command(0);
	do
	{
		std::auto_ptr<Response> response(0);
		try {
			command.reset(reader.read());
			response.reset(handle(command.get()));
		}
		catch (AdminException &e)
		{
			logger.warn("Command dispatching failed with exception: %s", e.what());
		}
		catch (char * e)
		{
			logger.warn("Command dispatching failed with exception: %s", e);
		}
		catch (...)
		{
			logger.warn("Command dispatching failed with unknown exception");
		}
	
		// writing response
		try {
			if (response.get() == 0)
			{
				response.reset(new Response(Response::Error, 0));
			}
			writer.write(*response);
		}
		catch (AdminException &e)
		{
			logger.warn("Response writing failed with exception: %s", e.what());
			break;
		}
		catch (char * e)
		{
			logger.warn("Response writing failed with exception: %s", e);
			break;
		}
		catch (...)
		{
			logger.warn("Response writing failed with unknown exception");
			break;
		}
	} while ((command->getId() != Command::undefined) &&
					 (command->getId() != Command::shutdown_service));

	close(sock);
	logger.info("Command dispather stopped");
	log4cpp::NDC::pop();
}

}
}
}


