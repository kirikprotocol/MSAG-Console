#include "DaemonCommandDispatcher.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddHSService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <admin/protocol/CommandSetServiceStartupParameters.h>
#include <admin/hsdaemon/ConfigParameterNames.h>
#include <core/synchronization/Mutex.hpp>
#include <util/signal.hpp>
#include <util/config/Config.h>
#include <core/buffers/FastMTQueue.hpp>
#include "ConfigParameterNames.h"

#include "ChildWaiter.h"

namespace smsc {
namespace admin {
namespace hsdaemon {

using namespace smsc::admin::protocol;
using smsc::core::synchronization::MutexGuard;
using smsc::core::buffers::FastMTQueue;
using namespace smsc::util;
using namespace smsc::util::config;

ServicesList DaemonCommandDispatcher::services;
Mutex DaemonCommandDispatcher::servicesListMutex;
config::Manager *DaemonCommandDispatcher::configManager = 0;
Mutex DaemonCommandDispatcher::configManagerMutex;


Mutex ChildShutdownWaiter::startedWaitersMutex;
WAITERS ChildShutdownWaiter::startedWaiters;
unsigned int DaemonCommandDispatcher::shutdownTimeout;
int DaemonCommandDispatcher::retryTimeout;
int DaemonCommandDispatcher::retryCount;
int DaemonCommandDispatcher::stayAliveTimeout;


///<summary>daemon command dispatcher implementation</summary>

/// static init
void DaemonCommandDispatcher::init(config::Manager * confManager) throw ()
{
  MutexGuard lock(configManagerMutex);
  configManager = confManager;
  shutdownTimeout = 10;
  retryTimeout=5;
  retryCount=3;
  stayAliveTimeout=60;
  Logger* log=Logger::getInstance("hsadmini");
  try {
    shutdownTimeout = configManager->getInt(CONFIG_SHUTDOWN_TIMEOUT);
  } catch (std::exception& e) {
    smsc_log_warn(log, "Couldn't get shutdown timeout from config. Shutdown timeout setted to %i seconds. Please define \"%s\" properly in daemon config.\nNested: %s", shutdownTimeout, CONFIG_SHUTDOWN_TIMEOUT, e.what());
  }
  try{
    retryTimeout=configManager->getInt(CONFIG_RETRY_TIMEOUT);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"Config parameter not found:%s",CONFIG_RETRY_TIMEOUT);
  }
  try{
    retryCount=configManager->getInt(CONFIG_RETRY_COUNT);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"Config parameter not found:%s",CONFIG_RETRY_COUNT);
  }
  try{
    stayAliveTimeout=configManager->getInt(CONFIG_STAY_ALIVE_TIMEOUT);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"Config parameter not found:%s",CONFIG_STAY_ALIVE_TIMEOUT);
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
    case Command::add_hsservice:
      return add_hsservice((CommandAddHSService*)command);
    case Command::remove_service:
      return remove_service((CommandRemoveService*)command);
    case Command::list_services:
      return list_services((CommandListServices*)command);
    case Command::set_hsservice_startup_parameters:
      return set_hsservice_startup_parameters((CommandSetHSServiceStartupParameters*)command);
    default:
      return new Response(Response::Error, "Unknown command");
    }
  }
  catch (AdminException &e)
  {
    return new Response(Response::Error, e.what());
  }
  catch(std::exception& e)
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
Response * DaemonCommandDispatcher::add_hsservice(const CommandAddHSService * const command)
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
        services.add(new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), command->getServiceInfo()));
      }
      putServiceToConfig(command->getServiceInfo());
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

Response * DaemonCommandDispatcher::remove_service(const CommandRemoveService * const command)
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


Response * DaemonCommandDispatcher::set_hsservice_startup_parameters(const CommandSetHSServiceStartupParameters * const command)
{
  smsc_log_debug(logger, "set service startup parameters");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      MutexGuard guard(servicesListMutex);
      Service *s = services[command->getServiceId()];

      putServiceToConfig(command->getServiceInfo());
      if (s->getStatus() == Service::stopped)
      {
        s->setArgs(command->getServiceInfo().args.c_str());
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
{
  smsc_log_debug(logger, "start service");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      MutexGuard guard(servicesListMutex);
      Service* svc=services[command->getServiceId()];
      if(!svc)throw AdminException("Unknown serviceId='%s'",command->getServiceId());
      if(svc->getType()!=ServiceInfo::standalone)
      {
        if(svc->getInfo().preferedNode.length()>0 && svc->getInfo().preferedNode!=icon->getLocalNode())
        {
          try{
            if(icon->remoteStartService(command->getServiceId()))
            {
              return new Response(Response::Ok,"");
            }else
            {
              smsc_log_warn(logger,"failed to start service '%s' on prefered node",command->getServiceId());
            }
          }catch(std::exception& e)
          {
            smsc_log_warn(logger,"failed to start service '%s' on prefered node '%s'",command->getServiceId(),e.what());
          }
        }
      }
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
{
  smsc_log_debug(logger, "shutdown service");
  if (command != 0)
  {
    if (command->getServiceId() != 0)
    {
      smsc_log_debug(logger, "shutdown service \"%s\"", command->getServiceId());
      {
        MutexGuard guard(servicesListMutex);
        Service* svc=services[command->getServiceId()];
        if(!svc)throw AdminException("Unknown serviceId='%s'",command->getServiceId());
        if(svc->getType()==ServiceInfo::standalone)
        {
          svc->shutdown();
        }else
        {
          if(svc->getStatus()==Service::stopped)
          {
            icon->remoteShutdownService(command->getServiceId());
          }
        }
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
{
  Logger* log=Logger::getInstance("hsadm.add");
  try
  {
    std::auto_ptr<CStrSet> childs(configManager->getChildSectionNames(CONFIG_SERVICES_SECTION));
    for (CStrSet::iterator i = childs.get()->begin(); i != childs.get()->end(); i++)
    {
      ServiceInfo info;
      const char * fullServiceSection = i->c_str();
      const char * dotpos = strrchr(fullServiceSection, '.');
      info.id=decodeDot(dotpos+1);
      smsc_log_info(log,"Loading service '%s'",info.id.c_str());


      std::string prefix(fullServiceSection);
      prefix += '.';

      std::string tmp = prefix;
      tmp += "args";
      info.args = configManager->getString(tmp.c_str());

      info.autoStart = true;
      try {
        info.autoStart = configManager->getBool((prefix+"autostart").c_str());
      } catch (AdminException &e) {
        smsc_log_warn(log, "Could not get autostart flag for service \"%s\", nested: %s", info.id.c_str(), e.what());
      } catch (...)
      {
        //skip
      }

      tmp=prefix+"serviceType";

      if(strcmp(configManager->getString(tmp.c_str()),"failover")==0)
      {
        info.serviceType=ServiceInfo::failover;
      }else if(strcmp(configManager->getString(tmp.c_str()),"standalone")==0)
      {
        info.serviceType=ServiceInfo::standalone;
      }else
      {
        throw AdminException("Invalid service type:%s",configManager->getString(tmp.c_str()));
      }
      try{
        tmp=prefix+"hostName";
        const char* val=configManager->getString(tmp.c_str());
        if(val && *val)info.hostName=val;
      }catch(std::exception& e)
      {
        smsc_log_warn(Logger::getInstance("admdmn.Ini"),"hostName not found for service %s, hostname switching disabled",info.id.c_str());
      }
      catch(HashInvalidKeyException& e)
      {
        smsc_log_warn(Logger::getInstance("admdmn.Ini"),"hostName not found for service %s, hostname switching disabled",info.id.c_str());
      }
      try{
        info.autostartDelay=configManager->getInt((prefix+"autostartDelay").c_str());
      }catch(...)
      {
        //ignore exception if optional parameter not present
      }

      Service* svc=new Service(configManager->getString(CONFIG_SERVICES_FOLDER_PARAMETER), info);
      services.add(svc);
    }
  }
  catch (AdminException &e)
  {
    smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Exception on adding services, nested: %s", e.what());
  }
  catch(std::exception& e)
  {
    smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Exception on adding services, nested: %s", e.what());
  }
  catch (...)
  {
    smsc_log_error(Logger::getInstance("smsc.admin.daemon.DaemonCommandDispatcher"), "Exception on adding services");
  }
}

void DaemonCommandDispatcher::updateServiceFromConfig(Service * service)
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
    tmpName=serviceSectionName;
    tmpName+=".hostName";
    try{
      service->setHost(configManager->getString(tmpName.c_str()));
    }catch(...)
    {
      //host not specified
    }
  }
  catch (smsc::core::buffers::HashInvalidKeyException &e)
  {
    throw AdminException("Service not found");
  }
}

void DaemonCommandDispatcher::putServiceToConfig(const ServiceInfo& info)
{
  MutexGuard lock(configManagerMutex);
  std::string serviceSectionName = CONFIG_SERVICES_SECTION;
  std::auto_ptr<char> tmpServiceId(encodeDot(cStringCopy(info.id.c_str())));
  serviceSectionName += tmpServiceId.get();

  configManager->setString((serviceSectionName + ".args").c_str(), info.args.c_str());
  configManager->setBool((serviceSectionName + ".autostart").c_str(), info.autoStart);
  configManager->setString((serviceSectionName + ".hostName").c_str(), info.hostName.c_str());
  configManager->setString((serviceSectionName + ".preferedNode").c_str(),info.preferedNode.c_str());
  configManager->setString((serviceSectionName + ".serviceType").c_str(),info.serviceType==ServiceInfo::failover?"failover":"standalone");
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
  Logger* log=Logger::getInstance("hsa.start");
  while (services.Next(serviceId, servicePtr) != 0)
  {
    if (servicePtr != NULL && servicePtr->isAutostart()) {
      try {
        smsc_log_info(log,"Autostarting service:%s",servicePtr->getId());
        servicePtr->autostartNotify();
        ChildShutdownWaiter::startService(servicePtr->getId());
      } catch (...) {
        if (serviceId != NULL)
          smsc_log_error(log, "Couldn't start service \"%s\", skipped", serviceId);
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
