#include <signal.h>
#include <stdlib.h>
#include <utility>

#include "SubsystemsManager.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace utilx {

SubsystemsManager::SubsystemsManager()
  : _logger(smsc::logger::Logger::getInstance("susbsysmgr")), _initialized(false)
{}

void
SubsystemsManager::registerSubsystem(Subsystem* subsystem, Subsystem* dependOnSubsystem)
{
  smsc::core::synchronization::MutexGuard guard(_registryLock);
  subsystems_registry_t::iterator iter = _subsystemsRegistry.find(subsystem->getName());
  if ( iter != _subsystemsRegistry.end() )
    throw smsc::util::Exception("SubsystemsManager::registerSubsystem::: subsystem with name [%s] already registered", subsystem->getName().c_str());
  subsystem_info sInfo(subsystem);
  if ( !dependOnSubsystem )
    _subsystemsRegistry.insert(std::make_pair(subsystem->getName(), new subsystem_info(subsystem)));
  else {
    iter = _subsystemsRegistry.find(dependOnSubsystem->getName());
    if ( iter != _subsystemsRegistry.end() )
      _subsystemsRegistry.insert(std::make_pair(subsystem->getName(), new subsystem_info(subsystem, iter->second)));
    else
      throw smsc::util::Exception("SubsystemsManager::registerSubsystem::: depending on subsystem with name [%s] is not registered", dependOnSubsystem->getName().c_str());
  }
}

Subsystem* 
SubsystemsManager::unregisterSubsystem(const std::string& subsystemName)
{
  smsc::core::synchronization::MutexGuard guard(_registryLock);
  subsystems_registry_t::iterator iter = _subsystemsRegistry.find(subsystemName);
  if ( iter != _subsystemsRegistry.end() ) {
    subsystem_info* subSystemInfo = iter->second;
    _subsystemsRegistry.erase(iter);
    Subsystem* subSystem = subSystemInfo->subsystem;
    delete subSystemInfo;
    return subSystem;
  } else
    return NULL;
}

extern "C" void appSignalHandler_SubsystemsManager(int sig)
{
  return;
}

void
SubsystemsManager::initialize()
{
  if ( !_initialized ) {
    sigfillset(&_blocked_signals);
    sigdelset(&_blocked_signals, SIGKILL);
    sigdelset(&_blocked_signals, SIGALRM);
    sigdelset(&_blocked_signals, SIGSEGV); sigdelset(&_blocked_signals, SIGBUS);
    sigdelset(&_blocked_signals, SIGFPE); sigdelset(&_blocked_signals, SIGILL);
#ifdef SIGWAITING
    sigdelset(&_blocked_signals, SIGWAITING);
#endif
#ifdef SIGLWP
    sigdelset(&_blocked_signals, SIGLWP);
#endif
#ifdef SIGCANCEL
    sigdelset(&_blocked_signals, SIGCANCEL);
#endif

    sigdelset(&_blocked_signals, SIGCHLD);
    sigdelset(&_blocked_signals, SIGUSR1);

    if ( pthread_sigmask(SIG_SETMASK, &_blocked_signals, NULL) )
      throw smsc::util::SystemError("SubsystemsManager::initialize::: call to pthread_sigmask failed");

    sigset(SIGTERM, appSignalHandler_SubsystemsManager);
    _initialized = true;
  }
}

void
SubsystemsManager::startup()
{
  if ( !_initialized ) 
    throw smsc::util::Exception("SubsystemsManager::startup::: manager wasn't initialized");

  for(subsystems_registry_t::iterator iter = _subsystemsRegistry.begin(), end_iter = _subsystemsRegistry.end();
      iter != end_iter; ++iter) {
    iter->second->subsystem->start();
  }
}

void
SubsystemsManager::waitForCompletion()
{
  int ret, out_signal=0;

  do {
    ret = sigwait(&_blocked_signals, &out_signal);
    smsc_log_debug(_logger,"SubsystemsManager::waitForCompletion::: sigwait return: out_signal=%d, ret=%d", out_signal, ret);
  } while (!ret && out_signal != SIGTERM );

  if ( ret ) 
    throw smsc::util::SystemError("SubsystemsManager::waitForCompletion::: call to sigwait failed");

  while( !_subsystemsRegistry.empty() ) {
    {
      smsc::core::synchronization::MutexGuard guard(_registryLock);

      for(shutdown_notifiable_objects_t::iterator iter = _objectsNeedShutdownInProgressNotification.begin(), end_iter = _objectsNeedShutdownInProgressNotification.end();
          iter != end_iter; ++iter) {
        (*iter)->notifyShutdownInProgess();
      }

      for(subsystems_registry_t::iterator iter = _subsystemsRegistry.begin(), end_iter = _subsystemsRegistry.end();
          iter != end_iter;) {
        if ( iter->second->referencedCount == 0 ) {
          iter->second->subsystem->stop();

          if ( iter->second->dependOnSubsystemInfo != NULL )
            iter->second->dependOnSubsystemInfo->referencedCount--;

          _subsystemsBeingShutdowning.push_back(iter->second);
          _subsystemsRegistry.erase(iter++);
        } else ++iter;
      }
    }
    while( !_subsystemsBeingShutdowning.empty() ) {
      subsystem_info* subsysInfo = _subsystemsBeingShutdowning.front();
      subsysInfo->subsystem->waitForCompletion();
      _subsystemsBeingShutdowning.pop_front();
      delete subsysInfo;
    }
  }
}

void
SubsystemsManager::subscribeToShutdownInProgressEvent(ShutdownInProgressNotification* interestedObject)
{
  smsc::core::synchronization::MutexGuard guard(_registryLock);
  _objectsNeedShutdownInProgressNotification.insert(interestedObject);
}

}}
