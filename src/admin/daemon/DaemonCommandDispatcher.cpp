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
using smsc::util::config::ConfigException;
using smsc::util::encode;
using smsc::util::encodeDot;
using smsc::util::decodeDot;
using smsc::util::cStringCopy;

ServicesList DaemonCommandDispatcher::services;
Mutex DaemonCommandDispatcher::servicesListMutex;
config::Manager *DaemonCommandDispatcher::configManager = 0;
Mutex DaemonCommandDispatcher::configManagerMutex;


///<summary>child shutdown waiter</summary>
class ChildShutdownWaiter : public Thread
{
private:
	smsc::logger::Logger logger;
	bool isStopping;
public:
	ChildShutdownWaiter() 
		: logger(Logger::getInstance("smsc.admin.daemon.ChildShutdownWaiter")), isStopping(false)
	{};
	~ChildShutdownWaiter()
	{
		Stop();
	}

	void Stop()
	{
		isStopping = true;
	}

	virtual int Execute()
	{
		while (!isStopping)
		{
			pid_t chldpid = waitpid(-1, 0, 0);
			if (chldpid == -1)
			{
				switch (errno)
				{
				case ECHILD:
					break;
				case EINTR:
					logger.debug("interrupted");
					break;
				case EINVAL:
					logger.error("invalid arguments");
					break;
				default:
					logger.error("unknown error");
					break;
				}
			} 
			else if (chldpid > 0)
			{
#ifdef SMSC_DEBUG
				logger.debug("CHILD %u is finished", chldpid);
#endif
				MutexGuard a(DaemonCommandDispatcher::servicesListMutex);
				if (const char * const serviceId = DaemonCommandDispatcher::services.markServiceAsStopped(chldpid))
				{
					MutexGuard lock(DaemonCommandDispatcher::configManagerMutex);
					DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
				}
			}
#ifdef SMSC_DEBUG
/*			else
			{
				logger.debug("waitpid returns %u ");
			}*/
#endif
			sleep(1);
		}
		return 0;
	}
};

static std::auto_ptr<ChildShutdownWaiter> childShutdownWaiter;
unsigned int DaemonCommandDispatcher::shutdownTimeout;


///<summary>daemon command dispatcher implementation</summary>

/// static init
void DaemonCommandDispatcher::init(config::Manager * confManager) throw ()
{
	MutexGuard lock(configManagerMutex);
	configManager = confManager;
	shutdownTimeout = 10;
	try {
		shutdownTimeout = configManager->getInt(CONFIG_SHUTDOWN_TIMEOUT);
	} catch (ConfigException &e) {
		Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").warn("Couldn't get shutdown timeout from config. Shutdown timeout setted to %i seconds. Please define \"%s\" properly in daemon config.\nNested: %s", shutdownTimeout, CONFIG_SHUTDOWN_TIMEOUT, e.what());
	} catch (...) {
		Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").warn("Couldn't get shutdown timeout from config. Shutdown timeout setted to %i seconds. Please define \"%s\" properly in daemon config.\nNested: Unknown exception", shutdownTimeout, CONFIG_SHUTDOWN_TIMEOUT);
	}
	addServicesFromConfig();

	childShutdownWaiter.reset(new ChildShutdownWaiter());
	childShutdownWaiter->Start();
}
void DaemonCommandDispatcher::shutdown()
{
	stopAllServices(shutdownTimeout);
	childShutdownWaiter->Stop();
}


/// constructor
DaemonCommandDispatcher::DaemonCommandDispatcher(Socket * admSocket)
	: CommandDispatcher(admSocket, "smsc.admin.daemon.CommandDispatcher"),
	logger(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"))
{
}

/// main command handler
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

/// commands
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
				services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), command->getServiceId(), command->getPort(), command->getArgs(), command->isAutostart()));
			}
			putServiceToConfig(command->getServiceId(), command->getPort(), command->getArgs(), command->isAutostart());
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

Response * DaemonCommandDispatcher::remove_service(const CommandRemoveService * const command) throw (AdminException)
{
	if (command != 0)
	{
		if (command->getServiceId() != 0)
		{
			{
				MutexGuard guard(servicesListMutex);
				Service *s = services[command->getServiceId()];
				logger.debug("remove service \"%s\"", command->getServiceId());
				if (s->getStatus() != Service::stopped)
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


Response * DaemonCommandDispatcher::set_service_startup_parameters(const CommandSetServiceStartupParameters * const command)
	throw (AdminException)
{
	logger.debug("set service startup parameters");
	if (command != 0)
	{
		if (command->getServiceId() != 0)
		{
			MutexGuard guard(servicesListMutex);
			Service *s = services[command->getServiceId()];
			putServiceToConfig(command->getServiceId(), command->getPort(), command->getArgs(), command->isAutostart());
			if (s->getStatus() == Service::stopped)
			{
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

/// global helper methods
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
			const char * const serviceArgs = configManager->getString(tmp.c_str());

			bool autostart = true;
			try {
				autostart = configManager->getBool((prefix+"autostart").c_str());
			} catch (AdminException &e) {
				Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").warn("Could not get autostart flag for service \"%s\", nested: %s", serviceId.get(), e.what());
			} catch (...) 
			{
				//skip
			}

			services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), serviceId.get(), servicePort, serviceArgs, autostart));
		}
	}
	catch (AdminException &e)
	{
		Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").
			error("Exception on adding services, nested: %s", e.what());
	}
	catch (...)
	{
		Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").
			error("Exception on adding services");
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

void DaemonCommandDispatcher::putServiceToConfig(const char * const serviceId, const in_port_t servicePort, const char * const serviceArgs, const bool autostart)
{
	MutexGuard lock(configManagerMutex);
	std::string serviceSectionName = CONFIG_SERVICES_SECTION;
	std::auto_ptr<char> tmpServiceId(encodeDot(cStringCopy(serviceId)));
	serviceSectionName += tmpServiceId.get();

	configManager->setInt((serviceSectionName + ".port").c_str(), servicePort);
	configManager->setString((serviceSectionName + ".args").c_str(), serviceArgs);
	configManager->setBool((serviceSectionName + ".autostart").c_str(), autostart);
	configManager->save();
	logger.debug("new config saved");
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



/// 
void DaemonCommandDispatcher::startAllServices()
{
	MutexGuard guard(servicesListMutex);
	char * serviceId = NULL;
	Service *servicePtr = NULL;
	services.First();
	while (services.Next(serviceId, servicePtr) != 0)
	{
		if (servicePtr != NULL && servicePtr->isAutostart()) {
			try {
				servicePtr->start();
			} catch (...) {
				if (serviceId != NULL)
					Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").error("Couldn't start service \"%s\", skipped", serviceId);
			}
		}
	}
}

void DaemonCommandDispatcher::stopAllServices(unsigned int timeoutInSecs)
{
	MutexGuard guard(servicesListMutex);
	char * serviceId = NULL;
	Service *servicePtr = NULL;
	services.First();
	bool allShutdowned = true;
	while (services.Next(serviceId, servicePtr) != 0)
	{
		if (servicePtr != NULL) {
			try {
				allShutdowned = false;
				servicePtr->shutdown();
			} catch (...) {
				Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").error("Couldn't stop service \"%s\", skipped", serviceId == NULL ? "<unknown>" : serviceId);
			}
		}
	}
	::time_t startTime = ::time(NULL);
	while ((!allShutdowned) && ((::time(NULL) - startTime) > timeoutInSecs)) {
		::sleep(timeoutInSecs);
	}

	services.First();
	while (services.Next(serviceId, servicePtr) != 0)
	{
		if ((servicePtr != NULL) && (servicePtr->getStatus() != Service::stopped)) {
			try {
				servicePtr->kill();
			} catch (AdminException &e) {
				Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").error("Couldn't kill service \"%s\", skipped, nested:\n%s", serviceId == NULL ? "<unknown>" : serviceId, e.what());
			} catch (...) {
				if (serviceId != NULL)
					Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher").error("Couldn't stop service \"%s\", skipped", serviceId);
			}
		}
	}
}

}
}
}

