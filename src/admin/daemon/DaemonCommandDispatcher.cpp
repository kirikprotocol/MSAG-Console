#include "DaemonCommandDispatcher.h"

#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <core/synchronization/Mutex.hpp>
#include <util/signal.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::protocol::Command;
using smsc::admin::protocol::CommandStartService;
using smsc::admin::protocol::CommandKillService;
using smsc::admin::protocol::CommandAddService;
using smsc::admin::protocol::CommandRemoveService;
using smsc::admin::protocol::CommandListServices;
using smsc::core::synchronization::MutexGuard;
using smsc::util::setExtendedSignalHandler;

ServicesList DaemonCommandDispatcher::services;
Mutex DaemonCommandDispatcher::servicesListMutex;

Response * DaemonCommandDispatcher::handle(const Command * const command)
	throw (AdminException)
{
	try
	{
		switch (command->getId())
		{
		case Command::start_service:
			return start_service((CommandStartService*)command);
		case Command::kill_service:
			return kill_service((CommandKillService*)command);
		case Command::shutdown_service:
			return shutdown_service((CommandShutdown*)command);
		case Command::add_service:
			return add_service((CommandAddService*)command);
		case Command::remove_service:
			return remove_service((CommandRemoveService*)command);
		case Command::list_services:
			return list_services((CommandListServices*)command);
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

Response * DaemonCommandDispatcher::start_service(const CommandStartService * const command)
	throw (AdminException &)
{
	logger.debug("start service");
	if (command != 0)
	{
		if (command->getServiceName() != 0)
		{
			pid_t newPid = 0;
			{
				MutexGuard guard(servicesListMutex);
				newPid = services[command->getServiceName()]->start();
			}
			char text[sizeof(pid_t)*3 +1];
			snprintf(text, sizeof(text),  "%lu", (unsigned long) newPid);
			return new Response(Response::Ok, text);
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

Response * DaemonCommandDispatcher::kill_service(const CommandKillService * const command)
	throw (AdminException &)
{
	logger.debug("kill service");
	if (command != 0)
	{
		if (command->getServiceName() != 0)
		{
			logger.debug("kill service \"%s\"", command->getServiceName());
			{
				MutexGuard guard(servicesListMutex);
				services[command->getServiceName()]->kill();
			}
			return new Response(Response::Ok, 0);
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

Response * DaemonCommandDispatcher::shutdown_service(const CommandShutdown * const command)
	throw (AdminException &)
{
	logger.debug("shutdown service");
	if (command != 0)
	{
		if (command->getServiceName() != 0)
		{
			logger.debug("shutdown service \"%s\"", command->getServiceName());
			{
				MutexGuard guard(servicesListMutex);
				services[command->getServiceName()]->shutdown();
			}
			return new Response(Response::Ok, 0);
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

Response * DaemonCommandDispatcher::add_service(const CommandAddService * const command)
	throw (AdminException &)
{
	logger.debug("add service \"%s\" (%s) %u %s",
							 command->getServiceName(),
							 command->getCmdLine(),
							 command->getPort(),
							 command->getConfigFileName());
	if (command != 0)
	{
		if (command->getServiceName() != 0 && command->getCmdLine() != 0
				&& command->getConfigFileName() != 0)
		{
			{
				MutexGuard guard(servicesListMutex);
				services.add(new Service(command->getServiceName(), command->getCmdLine(), command->getConfigFileName(), command->getPort(), command->getArgs()));
			}
			return new Response(Response::Ok, 0);
		}
		else
		{
			logger.warn("service name or command line or config file name not specified");
			throw AdminException("service name or command line not specified");
		}
	}
	else
	{
		logger.warn("null command received");
		throw AdminException("null command received");
	}
}

Response * DaemonCommandDispatcher::remove_service(const CommandRemoveService * const command)
	throw (AdminException &)
{
	if (command != 0)
	{
		if (command->getServiceName() != 0)
		{
			{
				MutexGuard guard(servicesListMutex);
				Service *s = services[command->getServiceName()];
				logger.debug("remove service \"%s\"", command->getServiceName());
				if (s->getStatus() == Service::running)
				{
					s->kill();
				}
				services.remove(s->getName());
			}
			return new Response(Response::Ok, 0);
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

Response * DaemonCommandDispatcher::list_services(const CommandListServices * const command)
	throw (AdminException &)
{
	logger.debug("list services");
	std::auto_ptr<char> text(0);
	{
		MutexGuard guard(servicesListMutex);
		text.reset(services.getText());
	}
	logger.debug("services list:\n%s\n", text.get());
	return new Response(Response::Ok, text.get());
}

void DaemonCommandDispatcher::shutdown()
{
	smsc::admin::util::CommandDispatcher::shutdown();
	logger.debug("shutdown");
}

void DaemonCommandDispatcher::childSignalListener(int signo,
																									siginfo_t * info,
																									void *some_pointer)
	throw ()
{
	#ifdef SMSC_DEBUG
		log4cpp::Category &log(Logger::getCategory("smsc.admin.daemon.DaemonCommandDispatcher"));
		log.debug("some signal received");
	#endif
	
	if (signo != SIGCHLD || info->si_signo != SIGCHLD)
	{
		return;
	}
	
	#ifdef SMSC_DEBUG
		log.debug("CHILD signal received");
	#endif
	// ACTION: CHILD signal received
	if (info->si_code <= 0)
	{
		{
			#ifdef SMSC_DEBUG
				log.debug("Handmaked signal from child (marking %lu child as dead)", (unsigned long)info->si_pid);
			#endif
			MutexGuard a(servicesListMutex);
			services.markServiceAsStopped(info->si_pid);
			#ifdef SMSC_DEBUG
				log.debug("%lu child marked as dead", (unsigned long)info->si_pid);
			#endif
		}
	}
	else
	{
		switch (info->si_code)
		{
		case CLD_EXITED:
		case CLD_KILLED:
		case CLD_DUMPED:
			{
				#ifdef SMSC_DEBUG
					log.debug("marking %lu child as dead", (unsigned long)info->si_pid);
				#endif
				MutexGuard a(servicesListMutex);
				services.markServiceAsStopped(info->si_pid);
				#ifdef SMSC_DEBUG
					log.debug("%lu child marked as dead", (unsigned long)info->si_pid);
				#endif
			}
			break;
		case CLD_TRAPPED:
		case CLD_STOPPED:
		case CLD_CONTINUED:
		default:
			;// skip these signals
		}
	}
}

void DaemonCommandDispatcher::activateChildSignalHandler()
{
	setExtendedSignalHandler(SIGCHLD, childSignalListener);
}

}
}
}

