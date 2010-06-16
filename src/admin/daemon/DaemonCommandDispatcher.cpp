#include "DaemonCommandDispatcher.h"

#include <sys/types.h>
#include <sys/wait.h>
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
#include <core/buffers/FastMTQueue.hpp>
#include <logger/Logger.h>

namespace smsc {
namespace admin {
namespace daemon {

using namespace smsc::admin::protocol;
using smsc::core::synchronization::MutexGuard;
using smsc::core::buffers::FastMTQueue;
using namespace smsc::util;
using namespace smsc::util::config;

ServicesList DaemonCommandDispatcher::services;
Mutex DaemonCommandDispatcher::servicesListMutex;
config::Manager *DaemonCommandDispatcher::configManager = 0;
Mutex DaemonCommandDispatcher::configManagerMutex;

class ChildShutdownWaiter;

typedef std::vector<ChildShutdownWaiter*> WAITERS;
///<summary>child shutdown waiter</summary>
class ChildShutdownWaiter : public Thread
{
private:
  static WAITERS startedWaiters;
  static Mutex startedWaitersMutex;

private:
  smsc::logger::Logger *logger;
  bool isStopping;
  bool isStopped_;
  pid_t pid;
  const char * const serviceId;
public:
  ChildShutdownWaiter(const char * const serviceId)
    : logger(Logger::getInstance("smsc.admin.daemon.ChildShutdownWaiter")), isStopping(false), isStopped_(false), pid(-1), serviceId(cStringCopy(serviceId))
  {}
  ~ChildShutdownWaiter()
  {
    Stop();
    delete serviceId;
  }

  void Stop()
  {
    isStopping = true;
  }

  virtual int Execute()
  {
    try {
      MutexGuard servicesGuard(DaemonCommandDispatcher::servicesListMutex);
      MutexGuard configGuard(DaemonCommandDispatcher::configManagerMutex);
      DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
      pid = DaemonCommandDispatcher::services[serviceId]->start();
    } catch (AdminException& e) {
      smsc_log_error(logger, "Couldn't start service \"%s\", nested: %s", serviceId, e.what());
      return -1;
    } catch (...) {
      smsc_log_error(logger, "Couldn't start service \"%s\"", serviceId);
      return -1;
    }

    while (!isStopping)
    {
      __trace2__("ChildShutdownWaiter : waitpid \"%s\" %d", serviceId, pid);
      pid_t chldpid = waitpid(pid, 0, 0);
      __trace2__("ChildShutdownWaiter : waitpid \"%s\" %d finished", serviceId, pid);
      if (chldpid == -1)
      {
        switch (errno)
        {
        case ECHILD:
          break;
        case EINTR:
          smsc_log_debug(logger, "interrupted");
          break;
        case EINVAL:
          smsc_log_error(logger, "invalid arguments");
          break;
        default:
          smsc_log_error(logger, "unknown error");
          break;
        }
      }
      else if (chldpid > 0)
      {
#ifdef SMSC_DEBUG
        __trace2__("CHILD %u is finished", chldpid);
#endif
        MutexGuard a(DaemonCommandDispatcher::servicesListMutex);
        if (const char * const serviceId = DaemonCommandDispatcher::services.markServiceAsStopped(chldpid))
        {
          MutexGuard lock(DaemonCommandDispatcher::configManagerMutex);
          DaemonCommandDispatcher::updateServiceFromConfig(DaemonCommandDispatcher::services[serviceId]);
        }
        isStopping = true;
      }

      sleep(1);
    }
    isStopped_ = true;
    __trace2__("ChildShutdownWaiter : waiter \"%s\" %d FINISHED", serviceId, pid);
    return 0;
  }

  const char * const getServiceId() const {return serviceId;}
  const bool isStopped() const {return isStopped_;}

  static void startService(const char * const serviceId)
  {
    __trace2__("ChildShutdownWaiter : start service \"%s\"", serviceId);
    cleanStoppedWaiters();
    MutexGuard guard(startedWaitersMutex);
    __trace2__("ChildShutdownWaiter : start service \"%s\" continued", serviceId);
    ChildShutdownWaiter* newWaiter = new ChildShutdownWaiter(serviceId);
    startedWaiters.push_back(newWaiter);
    newWaiter->Start();
    __trace2__("ChildShutdownWaiter : start service \"%s\" finished", serviceId);
  }

  static void cleanStoppedWaiters()
  {
    struct __local__ {
      static void CheckRemove(WAITERS::value_type& val) {
        if ( val->isStopped() ) {
          __trace2__("ChildShutdownWaiter::cleanStoppedWaiters : delete waiter for \"%s\"", val->getServiceId());
          val->WaitFor();
          delete val;
          val = 0;
        }
      }
    };
    __trace__("ChildShutdownWaiter : clean stopped waiters");
    MutexGuard guard(startedWaitersMutex);
    std::for_each(startedWaiters.begin(), startedWaiters.end(), &__local__::CheckRemove);
    WAITERS::iterator new_end = std::remove(startedWaiters.begin(), startedWaiters.end(), (ChildShutdownWaiter*)0);
    if ( new_end != startedWaiters.end() ) startedWaiters.erase( new_end, startedWaiters.end() );
    __trace__("ChildShutdownWaiter : clean stopped waiters finished");
  }

  static void stopWaiters()
  {
    __trace__("ChildShutdownWaiter::stopWaiters");
    MutexGuard guard(startedWaitersMutex);
    for (WAITERS::iterator i = startedWaiters.begin(); i != startedWaiters.end(); i++) {
      (*i)->Stop();
      (*i)->WaitFor();
      delete *i;
    }
    startedWaiters.clear();
    __trace__("ChildShutdownWaiter::stopWaiters finished");
  }
};

Mutex ChildShutdownWaiter::startedWaitersMutex;
WAITERS ChildShutdownWaiter::startedWaiters;
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
    smsc_log_warn(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Couldn't get shutdown timeout from config. Shutdown timeout setted to %i seconds. Please define \"%s\" properly in daemon config.\nNested: %s", shutdownTimeout, CONFIG_SHUTDOWN_TIMEOUT, e.what());
  } catch (...) {
    smsc_log_warn(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Couldn't get shutdown timeout from config. Shutdown timeout setted to %i seconds. Please define \"%s\" properly in daemon config.\nNested: Unknown exception", shutdownTimeout, CONFIG_SHUTDOWN_TIMEOUT);
  }
  addServicesFromConfig();
}
void DaemonCommandDispatcher::shutdown()
{
  stopAllServices(shutdownTimeout);
  ChildShutdownWaiter::stopWaiters();
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
  ChildShutdownWaiter::cleanStoppedWaiters();
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
  /*  smsc_log_debug(logger, "add service \"%s\" (%s) %u %s",
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
        services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), command->getServiceId(), command->getArgs(), command->isAutostart()));
      }
      putServiceToConfig(command->getServiceId(), command->getArgs(), command->isAutostart());
      return new Response(Response::Ok, 0);
    }
    else
    {
      smsc_log_warn(logger, "service id not specified");
      throw AdminException("service id not specified");
    }
  }
  else
  {
    smsc_log_warn(logger, "null command received");
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
        smsc_log_debug(logger, "remove service \"%s\"", command->getServiceId());
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
      smsc_log_warn(logger, "service name not specified");
      throw AdminException("service name not specified");
    }
  }
  else
  {
    smsc_log_warn(logger, "null command received");
    throw AdminException("null command received");
  }
}


Response * DaemonCommandDispatcher::set_service_startup_parameters(const CommandSetServiceStartupParameters * const command)
  throw (AdminException)
{
  smsc_log_debug(logger, "set service startup parameters");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      MutexGuard guard(servicesListMutex);
      Service *s = services[command->getServiceId()];
      putServiceToConfig(command->getServiceId(), command->getArgs(), command->isAutostart());
      if (s->getStatus() == Service::stopped)
      {
        s->setArgs(command->getArgs());
      }
      return new Response(Response::Ok, 0);
    }
    else
    {
      smsc_log_warn(logger, "service name or service id not specified");
      throw AdminException("service name or service id not specified");
    }
  }
  else
  {
    smsc_log_warn(logger, "null command received");
    throw AdminException("null command received");
  }
}

Response * DaemonCommandDispatcher::list_services(const CommandListServices * const command)
  throw (AdminException)
{
  smsc_log_debug(logger, "list services");
  std::auto_ptr<char> text(0);
  {
    MutexGuard guard(servicesListMutex);
    text.reset(services.getText());
  }
  smsc_log_debug(logger, "services list:\n%s\n", text.get());
  return new Response(Response::Ok, text.get());
}


Response * DaemonCommandDispatcher::start_service(const CommandStartService * const command)
  throw (AdminException)
{
  smsc_log_debug(logger, "start service");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      ChildShutdownWaiter::startService(command->getServiceId());
      return new Response(Response::Ok, "");
    }
    else
    {
      smsc_log_warn(logger, "service id not specified");
      throw AdminException("service id not specified");
    }
  }
  else
  {
    smsc_log_warn(logger, "null command received");
    throw AdminException("null command received");
  }
}

Response * DaemonCommandDispatcher::shutdown_service(const CommandShutdown * const command)
  throw (AdminException)
{
  smsc_log_debug(logger, "shutdown service");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      smsc_log_debug(logger, "shutdown service \"%s\"", command->getServiceId());
      {
        MutexGuard guard(servicesListMutex);
        services[command->getServiceId()]->shutdown();
      }
      return new Response(Response::Ok, 0);
    }
    else
    {
      smsc_log_warn(logger, "service id not specified");
      throw AdminException("service id not specified");
    }
  }
  else
  {
    smsc_log_warn(logger, "null command received");
    throw AdminException("null command received");
  }
}

Response * DaemonCommandDispatcher::kill_service(const CommandKillService * const command)
  throw (AdminException)
{
  smsc_log_debug(logger, "kill service");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      smsc_log_debug(logger, "kill service \"%s\"", command->getServiceId());
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
      smsc_log_warn(logger, "service id not specified");
      throw AdminException("service id not specified");
    }
  }
  else
  {
    smsc_log_warn(logger, "null command received");
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
      const char * dotpos = strrchr(fullServiceSection, '.');
      //const size_t serviceNameBufLen = strlen(dotpos+1) +1;
      std::auto_ptr<char> serviceId(cStringCopy(decodeDot(dotpos+1).c_str()));

      std::string prefix(fullServiceSection);
      prefix += '.';

      std::string tmp = prefix;
      tmp += "args";
      const char * const serviceArgs = configManager->getString(tmp.c_str());

      bool autostart = true;
      try {
        autostart = configManager->getBool((prefix+"autostart").c_str());
      } catch (AdminException &e) {
        smsc_log_warn(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Could not get autostart flag for service \"%s\", nested: %s", serviceId.get(), e.what());
      } catch (...)
      {
        //skip
      }

      services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), serviceId.get(), serviceArgs, autostart));
    }
  }
  catch (AdminException &e)
  {
    smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Exception on adding services, nested: %s", e.what());
  }
  catch (...)
  {
    smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Exception on adding services");
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
    tmpName += ".args";
    const char * const serviceArgs = configManager->getString(tmpName.c_str());

    service->setArgs(serviceArgs);
  }
  catch (smsc::core::buffers::HashInvalidKeyException &e)
  {
    throw AdminException("Service not found");
  }
}

void DaemonCommandDispatcher::putServiceToConfig(const char * const serviceId, const char * const serviceArgs, const bool autostart)
{
  MutexGuard lock(configManagerMutex);
  std::string serviceSectionName = CONFIG_SERVICES_SECTION;
  std::auto_ptr<char> tmpServiceId(encodeDot(cStringCopy(serviceId)));
  serviceSectionName += tmpServiceId.get();

  configManager->setString((serviceSectionName + ".args").c_str(), serviceArgs);
  configManager->setBool((serviceSectionName + ".autostart").c_str(), autostart);
  configManager->save();
  smsc_log_debug(logger, "new config saved");
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
        ChildShutdownWaiter::startService(servicePtr->getId());
      } catch (...) {
        if (serviceId != NULL)
          smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Couldn't start service \"%s\", skipped", serviceId);
      }
    }
  }
}

void DaemonCommandDispatcher::stopAllServices(unsigned int timeoutInSecs)
{
   bool allShutdowned = true;
  {
    MutexGuard guard(servicesListMutex);
    smsc_log_debug(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Stop all services");
    char * serviceId = NULL;
    Service *servicePtr = NULL;
    services.First();
    while (services.Next(serviceId, servicePtr) != 0)
    {
      if ((servicePtr != NULL) && (servicePtr->getStatus() != Service::stopped)) {
        try {
          allShutdowned = false;
          smsc_log_debug(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "send shutdown signal to service \"%s\"", servicePtr->getId());
          servicePtr->shutdown();
        } catch (...) {
          smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Couldn't stop service \"%s\", skipped", serviceId == NULL ? "<unknown>" : serviceId);
        }
      }
    }
  }

  smsc_log_debug(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Waiting timeout...");
  ::time_t startTime = ::time(NULL);
  while ((!allShutdowned) && ((::time(NULL) - startTime) < timeoutInSecs)) {
    ::sleep(1);
    {
      MutexGuard guard(servicesListMutex);
      allShutdowned = true;
      char * serviceId = NULL;
      Service *servicePtr = NULL;
      services.First();
      while (services.Next(serviceId, servicePtr) != 0)
      {
        if ((servicePtr != NULL) && (servicePtr->getStatus() != Service::stopped))
          allShutdowned = false;
      }
    }
  }

  {
    MutexGuard guard(servicesListMutex);
    char * serviceId = NULL;
    Service *servicePtr = NULL;
    services.First();
    while (services.Next(serviceId, servicePtr) != 0)
    {
      if ((servicePtr != NULL) && (servicePtr->getStatus() != Service::stopped)) {
        try {
          smsc_log_debug(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Kill service \"%s\"", servicePtr->getId());
          servicePtr->kill();
        } catch (AdminException &e) {
          smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Couldn't kill service \"%s\", skipped, nested:\n%s", serviceId == NULL ? "<unknown>" : serviceId, e.what());
        } catch (...) {
          if (serviceId != NULL)
            smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Couldn't stop service \"%s\", skipped", serviceId);
        }
      }
    }
  }
  smsc_log_debug(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Stop all services finished");
}

}
}
}
