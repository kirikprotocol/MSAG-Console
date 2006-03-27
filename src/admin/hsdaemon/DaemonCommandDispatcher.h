#ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
#define SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER

#include <admin/AdminException.h>
#include <admin/hsdaemon/ServicesList.h>
#include <core/network/Socket.hpp>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandShutdown.h>
#include <admin/protocol/CommandAddHSService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <admin/protocol/CommandSetHSServiceStartupParameters.h>
#include <admin/protocol/Response.h>
#include <admin/util/CommandDispatcher.h>
#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <core/threads/Thread.hpp>

namespace smsc {
namespace admin {
namespace hsdaemon {

using smsc::admin::AdminException;
using namespace smsc::admin::protocol;
using smsc::admin::util::CommandDispatcher;
using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::logger::Logger;
using namespace smsc::util::config;
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

  static ServicesList& getServicesList(){return services;}

  static int retryTimeout;
  static int retryCount;
  static int stayAliveTimeout;

protected:
  smsc::logger::Logger *logger;
  static ServicesList services;
  static config::Manager *configManager;
  static unsigned int shutdownTimeout;

  static Mutex servicesListMutex;
  static Mutex configManagerMutex;

  friend class ChildShutdownWaiter;
  friend class Interconnect;

  Response * add_hsservice                  (const CommandAddHSService                  * const command) throw (AdminException);
  Response * remove_service                 (const CommandRemoveService               * const command) throw (AdminException);
  Response * set_hsservice_startup_parameters (const CommandSetHSServiceStartupParameters * const command) throw (AdminException);
  Response * list_services                  (const CommandListServices                * const command) throw (AdminException);
  Response * start_service                  (const CommandStartService                * const command) throw (AdminException);
  Response * shutdown_service               (const CommandShutdown                    * const command) throw (AdminException);
  Response * kill_service                   (const CommandKillService                 * const command) throw (AdminException);


  static void addServicesFromConfig() throw ();
  static void updateServiceFromConfig(Service * service) throw (AdminException);
  void putServiceToConfig(const ServiceInfo& info);
  void removeServiceFromConfig(const char * const serviceId);

};

}
}
}

#endif // ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
