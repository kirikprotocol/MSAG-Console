#include "CommandDispatcher.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <log4cpp/NDC.hh>

#include <admin/util/ShutdownableList.h>
#include <core/network/Socket.hpp>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace util {

using smsc::admin::AdminException;
//using smsc::util::config::Config;

const char * const CommandDispatcher::task_name = "CommandDispatcher";

CommandDispatcher::CommandDispatcher(Socket* admSocket,
																		 //const char * const client_addr,
																		 const char * const loggerCategory = "smsc.admin.util.CommandDispatcher")
	: logger(smsc::util::Logger::getCategory(loggerCategory)),
	  reader(admSocket),
	  writer(admSocket)
{
	isShutdownSignaled = false;
	sock = admSocket;
	//memcpy(cl_addr, client_addr, 15);
	//cl_addr[15] = 0;
	logger.debug("Command dispatcher created.");
}

CommandDispatcher::~CommandDispatcher()
{
	logger.debug("Command dispatcher \"%s\" destroyed.", task_name);
	sock = 0;
}

void CommandDispatcher::init()
{
	char thr[11];
	snprintf(thr, sizeof(thr), "[%.8X]", thr_self());
	std::string ndc;
	ndc += thr;
	//ndc += cl_addr;
	log4cpp::NDC::push(ndc);
}

int CommandDispatcher::Execute()
{
	init();

	ShutdownableList::addListener(this);
	
	logger.info("Command dispather started");
	
	std::auto_ptr<Command> command(0);
	do
	{
		std::auto_ptr<Response> response(0);
		try {
			while (!reader.canRead())
			{
				if (isShutdownSignaled)
					break;
			}
			if (!isShutdownSignaled)
			{
				command.reset(reader.read());
				response.reset(handle(command.get()));
			}
		}
		catch (AdminException &e)
		{
			response.reset(new Response(Response::Error, e.what()));
			logger.warn("Command dispatching failed with exception: %s", e.what());
		}
		catch (char * e)
		{
			response.reset(new Response(Response::Error, e));
			logger.warn("Command dispatching failed with exception: %s", e);
		}
		catch (...)
		{
			response.reset(new Response(Response::Error, "Command dispatching failed with unknown exception"));
			logger.warn("Command dispatching failed with unknown exception");
		}
	
		if (!isShutdownSignaled)
		{
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
		}
	} while (!isShutdownSignaled
					 && (command->getId() != Command::undefined)
					 && (command->getId() != Command::shutdown_service));
	
	ShutdownableList::removeListener(this);

	sock->Close();
	logger.info("Command dispather stopped");
	log4cpp::NDC::pop();
	return 0;
}

}
}
}

