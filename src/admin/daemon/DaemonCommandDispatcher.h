#ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
#define SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER

#include <admin/AdminException.h>
#include <admin/daemon/ServicesList.h>
#include <core/network/Socket.hpp>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandShutdown.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <admin/protocol/Response.h>
#include <admin/util/CommandDispatcher.h>
#include <admin/util/Shutdownable.h>
#include <core/synchronization/Mutex.hpp>
#include <util/Logger.h>
#include <util/config/Manager.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::AdminException;
using smsc::admin::protocol::Command;
using smsc::admin::protocol::CommandStartService;
using smsc::admin::protocol::CommandKillService;
using smsc::admin::protocol::CommandShutdown;
using smsc::admin::protocol::CommandAddService;
using smsc::admin::protocol::CommandRemoveService;
using smsc::admin::protocol::CommandListServices;
using smsc::admin::protocol::Response;
using smsc::admin::util::CommandDispatcher;
using smsc::admin::util::Shutdownable;
using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Logger;
using smsc::util::config;

class DaemonCommandDispatcher : public CommandDispatcher
{
public:
	static void activateChildSignalHandler();

	inline static void init(config::Manager * confManager) throw ()
	{
		MutexGuard lock(configManagerMutex);
		configManager = confManager;
		addServicesFromConfig();
	}

	virtual void shutdown();
	DaemonCommandDispatcher(Socket * admSocket)
		: CommandDispatcher(admSocket, "smsc.admin.daemon.CommandDispatcher"),
		  logger(Logger::getCategory("smsc.admin.daemon.DaemonCommandDispatcher"))
	{
	}

	virtual Response * handle(const Command * const command) throw (AdminException);

protected:
	static void addServicesFromConfig();
	log4cpp::Category &logger;
	static ServicesList services;
	static Mutex servicesListMutex;
	static config::Manager *configManager;
	static Mutex configManagerMutex;

	static void childSignalListener(int signo,
																	siginfo_t * info,
																	void *some_pointer) throw ();
	
	Response * start_service    (const CommandStartService  * const command) throw (AdminException &);
	Response * kill_service     (const CommandKillService   * const command) throw (AdminException &);
	Response * shutdown_service (const CommandShutdown      * const command) throw (AdminException &);
	Response * add_service      (const CommandAddService    * const command) throw (AdminException &);
	Response * remove_service   (const CommandRemoveService * const command) throw (AdminException &);
	Response * list_services    (const CommandListServices  * const command) throw (AdminException &);

	void putServiceToConfig(const char * const serviceName,
													const in_port_t servicePort,
													const char * const serviceCmdLine,
													const char * const serviceConfigFileName,
													const ServiceArguments & serviceArgs);
	void removeServiceFromConfig(const char * const serviceName);
};

}
}
}

#endif // ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
