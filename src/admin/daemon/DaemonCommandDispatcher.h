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
#include <admin/protocol/CommandSetServiceStartupParameters.h>
#include <admin/protocol/Response.h>
#include <admin/util/CommandDispatcher.h>
#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <core/threads/Thread.hpp>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::AdminException;
using namespace smsc::admin::protocol;
using smsc::admin::util::CommandDispatcher;
using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Logger;
using smsc::util::config;
using smsc::core::threads::Thread;

class DaemonCommandDispatcher : public CommandDispatcher
{
public:
	static void startAllServices();
	static void stopAllServices(unsigned int timeout_in_seconds);

	static void init(config::Manager * confManager) throw ();

	DaemonCommandDispatcher(Socket * admSocket);

	virtual Response * handle(const Command * const command) throw (AdminException);

	static void shutdown();

protected:
	log4cpp::Category &logger;
	static ServicesList services;
	static config::Manager *configManager;
	static unsigned int shutdownTimeout;

	static Mutex servicesListMutex;
	static Mutex configManagerMutex;

	friend class ChildShutdownWaiter;

	Response * add_service                    (const CommandAddService                  * const command) throw (AdminException);
	Response * remove_service                 (const CommandRemoveService               * const command) throw (AdminException);
	Response * set_service_startup_parameters (const CommandSetServiceStartupParameters * const command) throw (AdminException);
	Response * list_services                  (const CommandListServices                * const command) throw (AdminException);
	Response * start_service                  (const CommandStartService                * const command) throw (AdminException);
	Response * shutdown_service               (const CommandShutdown                    * const command) throw (AdminException);
	Response * kill_service                   (const CommandKillService                 * const command) throw (AdminException);


	static void addServicesFromConfig() throw ();
	static void updateServiceFromConfig(Service * service) throw (AdminException);
	void putServiceToConfig(const char * const serviceId, const in_port_t servicePort, const char * const serviceArgs, const bool autostart);
	void removeServiceFromConfig(const char * const serviceId);

};

}
}
}

#endif // ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
