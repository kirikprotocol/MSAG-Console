#include "DaemonCommandDispatcher.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <admin/protocol/CommandSetServiceStartupParameters.h>
#include <admin/daemon/config_parameter_names.h>
#include <core/synchronization/Mutex.hpp>
#include <util/signal.hpp>
#include <util/config/Config.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::protocol::Command;
using smsc::admin::protocol::CommandStartService;
using smsc::admin::protocol::CommandKillService;
using smsc::admin::protocol::CommandAddService;
using smsc::admin::protocol::CommandRemoveService;
using smsc::admin::protocol::CommandListServices;
using smsc::admin::protocol::CommandSetServiceStartupParameters;
using smsc::core::synchronization::MutexGuard;
using smsc::util::setExtendedSignalHandler;
using smsc::util::config::CStrSet;
using smsc::util::encode;
using smsc::util::encodeDot;
using smsc::util::decodeDot;
using smsc::util::cStringCopy;

ServicesList DaemonCommandDispatcher::services;
Mutex DaemonCommandDispatcher::servicesListMutex;
config::Manager *DaemonCommandDispatcher::configManager = 0;
Mutex DaemonCommandDispatcher::configManagerMutex;

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
		case Command::set_service_startup_parameters:
			return set_service_startup_parameters((CommandSetServiceStartupParameters*)command);
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

void DaemonCommandDispatcher::updateServiceFromConfig(Service * service)
	throw (AdminException)
{
	const char * const serviceId = service->getId();

	std::string serviceSectionName = CONFIG_SERVICES_SECTION;
	std::auto_ptr<char> tmpServiceId(encodeDot(cStringCopy(serviceId)));
	serviceSectionName += tmpServiceId.get();

	try {
		/*std::string tmpName = serviceSectionName;
		tmpName += ".name";
		const char * const serviceName = configManager->getString(tmpName.c_str());*/
		
		std::string tmpName = serviceSectionName;
		tmpName += ".port";
		const in_port_t servicePort = configManager->getInt(tmpName.c_str());
	
		tmpName = serviceSectionName;
		tmpName += ".args";
		const char * const serviceArgs = configManager->getString(tmpName.c_str());

		//service->setName(serviceName);
		service->setPort(servicePort);
		service->setArgs(serviceArgs);
	}
	catch (smsc::core::buffers::HashInvalidKeyException &e)
	{
		throw AdminException("Service not found");
	}
}

Response * DaemonCommandDispatcher::start_service(const CommandStartService * const command)
	throw (AdminException)
{
	logger.debug("start service");
	if (command != 0)
	{
		if (command->getServiceId() != 0)
		{
			pid_t newPid = 0;
			{
				MutexGuard servicesGuard(servicesListMutex);
				MutexGuard configGuard(configManagerMutex);
				updateServiceFromConfig(services[command->getServiceId()]);
				newPid = services[command->getServiceId()]->start();
			}
			char text[sizeof(pid_t)*3 +1];
			snprintf(text, sizeof(text),  "%lu", (unsigned long) newPid);
			return new Response(Response::Ok, text);
		}
		else
		{
			logger.warn("service id not specified");
			throw AdminException("service id not specified");
		}
	}
	else
	{
		logger.warn("null command received");
		throw AdminException("null command received");
	}
}

Response * DaemonCommandDispatcher::kill_service(const CommandKillService * const command)
	throw (AdminException)
{
	logger.debug("kill service");
	if (command != 0)
	{
		if (command->getServiceId() != 0)
		{
			logger.debug("kill service \"%s\"", command->getServiceId());
			{
				MutexGuard servicesGuard(servicesListMutex);
				services[command->getServiceId()]->kill();
				MutexGuard configGuard(configManagerMutex);
				updateServiceFromConfig(services[command->getServiceId()]);
			}
			return new Response(Response::Ok, 0);
		}
		else
		{
			logger.warn("service id not specified");
			throw AdminException("service id not specified");
		}
	}
	else
	{
		logger.warn("null command received");
		throw AdminException("null command received");
	}
}

Response * DaemonCommandDispatcher::shutdown_service(const CommandShutdown * const command)
	throw (AdminException)
{
	logger.debug("shutdown service");
	if (command != 0)
	{
		if (command->getServiceId() != 0)
		{
			logger.debug("shutdown service \"%s\"", command->getServiceId());
			{
				MutexGuard guard(servicesListMutex);
				services[command->getServiceId()]->shutdown();
			}
			return new Response(Response::Ok, 0);
		}
		else
		{
			logger.warn("service id not specified");
			throw AdminException("service id not specified");
		}
	}
	else
	{
		logger.warn("null command received");
		throw AdminException("null command received");
	}
}

Response * DaemonCommandDispatcher::add_service(const CommandAddService * const command)
	throw (AdminException)
{
/*	logger.debug("add service \"%s\" (%s) %u %s",
							 command->getServiceName(),
							 command->getCmdLine(),
							 command->getPort(),
							 command->getConfigFileName());*/
	if (command != 0)
	{
		if (command->getServiceId() != 0/* && command->getServiceName() != 0*/)
		{
			{
				MutexGuard guard(servicesListMutex);
				services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), command->getServiceId(), /*command->getServiceName(), */command->getPort(), command->getArgs()));
			}
			putServiceToConfig(command->getServiceId(), /*command->getServiceName(), */command->getPort(), command->getArgs());
			return new Response(Response::Ok, 0);
		}
		else
		{
			logger.warn("service id not specified");
			throw AdminException("service id not specified");
		}
	}
	else
	{
		logger.warn("null command received");
		throw AdminException("null command received");
	}
}

void DaemonCommandDispatcher::putServiceToConfig(const char * const serviceId,
																								 //const char * const serviceName,
																								 const in_port_t servicePort,
																								 const char * const serviceArgs)
{
	MutexGuard lock(configManagerMutex);
	std::string serviceSectionName = CONFIG_SERVICES_SECTION;
	std::auto_ptr<char> tmpServiceId(encodeDot(cStringCopy(serviceId)));
	serviceSectionName += tmpServiceId.get();

	/*std::string tmpName = serviceSectionName;
	tmpName += ".name";
	configManager->setString(tmpName.c_str(), serviceName);*/
	
	std::string tmpName = serviceSectionName;
	tmpName += ".port";
	configManager->setInt(tmpName.c_str(), servicePort);

	tmpName = serviceSectionName;
	tmpName += ".args";
	configManager->setString(tmpName.c_str(), serviceArgs);
	configManager->save();
  logger.debug("new config saved");
}

Response * DaemonCommandDispatcher::remove_service(const CommandRemoveService * const command)
	throw (AdminException)
{
	if (command != 0)
	{
		if (command->getServiceId() != 0)
		{
			{
				MutexGuard guard(servicesListMutex);
				Service *s = services[command->getServiceId()];
				logger.debug("remove service \"%s\"", command->getServiceId());
				if (s->isRunning())
				{
					s->kill();
				}
				services.remove(s->getId());
			}
			removeServiceFromConfig(command->getServiceId());
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

void DaemonCommandDispatcher::removeServiceFromConfig(const char * const serviceId)
{
	MutexGuard lock(configManagerMutex);
	std::string serviceSectionName = CONFIG_SERVICES_SECTION;

	std::auto_ptr<char> tmpServiceName(encodeDot(cStringCopy(serviceId)));
	serviceSectionName += tmpServiceName.get();
	configManager->removeSection(serviceSectionName.c_str());
	configManager->save();
}

Response * DaemonCommandDispatcher::list_services(const CommandListServices * const command)
	throw (AdminException)
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

/*void DaemonCommandDispatcher::shutdown()
{
	smsc::admin::util::CommandDispatcher::shutdown();
	logger.debug("shutdown");
}*/

void DaemonCommandDispatcher::childSignalListener(int signo,
																									siginfo_t * info,
																									void *some_pointer)
	throw ()
{
	#ifdef SMSC_DEBUG
		log4cpp::Category &log(Logger::getCategory("smsc.admin.daemon.DaemonCommandDispatcher"));
		//log.debug("some signal received");
	#endif
	
	if (signo != SIGCHLD || info->si_signo != SIGCHLD)
	{
		return;
	}
	
	#ifdef SMSC_DEBUG
		log.debug("some CHILD signal received");
	#endif
	// ACTION: CHILD signal received
	if (info->si_code <= 0)
	{
		{
			#ifdef SMSC_DEBUG
				log.debug("Handmaked signal from child (marking %lu child as dead)", (unsigned long)info->si_pid);
			#endif
			MutexGuard a(servicesListMutex);
			if (const char * const serviceId = services.markServiceAsStopped(info->si_pid))
			{
				MutexGuard lock(configManagerMutex);
				updateServiceFromConfig(services[serviceId]);
			}

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
				if (const char * const serviceId = services.markServiceAsStopped(info->si_pid))
				{
					MutexGuard lock(configManagerMutex);
					updateServiceFromConfig(services[serviceId]);
				}
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

void DaemonCommandDispatcher::addServicesFromConfig()
	throw ()
{
	try
	{
		std::auto_ptr<CStrSet> childs(configManager->getChildSectionNames(CONFIG_SERVICES_SECTION));
		for (CStrSet::iterator i = childs.get()->begin(); i != childs.get()->end(); i++)
		{
			const char * fullServiceSection = i->c_str();
			char * dotpos = strrchr(fullServiceSection, '.');
			//const size_t serviceNameBufLen = strlen(dotpos+1) +1;
			std::auto_ptr<char> serviceId(decodeDot(cStringCopy(dotpos+1)));
		
			std::string prefix(fullServiceSection);
			prefix += '.';
		
			/*std::string tmp = prefix;
			tmp += "name";
			std::auto_ptr<char> serviceName(configManager->getString(tmp.c_str()));*/
		
			std::string tmp = prefix;
			tmp += "port";
			in_port_t servicePort = configManager->getInt(tmp.c_str());
		
			tmp = prefix;
			tmp += "args";
			std::auto_ptr<char> serviceArgs(configManager->getString(tmp.c_str()));
			
			services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), serviceId.get(), /*serviceName.get(), */servicePort, serviceArgs.get()));
		}
	}
	catch (AdminException &e)
	{
		Logger::getCategory("smsc.admin.daemon.DaemonCommandDispatcher").
			error("Exception on adding services, nested: %s", e.what());
	}
	catch (...)
	{
		Logger::getCategory("smsc.admin.daemon.DaemonCommandDispatcher").
			error("Exception on adding services");
	}
}

Response * DaemonCommandDispatcher::set_service_startup_parameters(const CommandSetServiceStartupParameters * const command)
	throw (AdminException)
{
	logger.debug("set service startup parameters");
	if (command != 0)
	{
		if (command->getServiceId() != 0/* && command->getServiceName() != 0*/)
		{
			MutexGuard guard(servicesListMutex);
			Service *s = services[command->getServiceId()];
			putServiceToConfig(command->getServiceId(), /*command->getServiceName(), */command->getPort(), command->getArgs());
			if (!s->isRunning())
			{
				//s->setName(command->getServiceName());
				s->setPort(command->getPort());
				s->setArgs(command->getArgs());
			}
			return new Response(Response::Ok, 0);
		}
		else
		{
			logger.warn("service name or service id not specified");
			throw AdminException("service name or service id not specified");
		}
	}
	else
	{
		logger.warn("null command received");
		throw AdminException("null command received");
	}
}

}
}
}

