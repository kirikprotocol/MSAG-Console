#ifndef __EYELINE_UTILX_SUBSYSTEMSMANAGER_HPP__
# define __EYELINE_UTILX_SUBSYSTEMSMANAGER_HPP__

# include <pthread.h>
# include <string>
# include <map>
# include <list>
# include "eyeline/utilx/Subsystem.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/ShutdownInProgressNotification.hpp"
# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace utilx {

class SubsystemsManager : public Singleton<SubsystemsManager*> {
public:
  SubsystemsManager();

  void subscribeToShutdownInProgressEvent(ShutdownInProgressNotification* interestedObject);

  void registerSubsystem(Subsystem* subsystem, Subsystem* dependOnSubsystem=NULL);
  Subsystem* unregisterSubsystem(const std::string& subsystemName);

  void initialize();
  void startup();
  void waitForCompletion();

private:
  typedef std::set<ShutdownInProgressNotification*> shutdown_notifiable_objects_t;
  shutdown_notifiable_objects_t _objectsNeedShutdownInProgressNotification;

  smsc::core::synchronization::Mutex _registryLock;

  struct subsystem_info {
    explicit subsystem_info(Subsystem* aSubsystem)
      : subsystem(aSubsystem), dependOnSubsystemInfo(NULL), referencedCount(0) {}
    subsystem_info(Subsystem* aSubsystem, subsystem_info* aDependOnSubsystemInfo)
      : subsystem(aSubsystem), dependOnSubsystemInfo(aDependOnSubsystemInfo), referencedCount(0) {
      dependOnSubsystemInfo->referencedCount++;
    }
    Subsystem* subsystem;
    subsystem_info* dependOnSubsystemInfo;
    unsigned int referencedCount;
  };

  typedef std::map<std::string, subsystem_info*> subsystems_registry_t;
  subsystems_registry_t _subsystemsRegistry;

  typedef std::list<subsystem_info*> subsystem_info_list_t;
  subsystem_info_list_t _subsystemsBeingShutdowning;

  smsc::logger::Logger* _logger;
  sigset_t _blocked_signals;
  bool _initialized;
};

}}

#endif
