#include <logger/Logger.h>
#include <exception>
#include <iostream>
#include <sua/utilx/SubsystemsManager.hpp>
#include <util/config/Manager.h>

#include "runtime_cfg/Parameter.hpp"
#include "runtime_cfg/CompositeParameter.hpp"
#include "runtime_cfg/RuntimeConfig.hpp"

#include "io_dispatcher/ConnectMgr.hpp"
#include "io_dispatcher/IODispatcherSubsystem.hpp"
#include "sua_stack/SuaStackSubsystem.hpp"
#include "sua_user_communication/SuaUserCommunicationSubsystem.hpp"
#include "messages_router/MessagesRouterSubsystem.hpp"
#include "lm_subsystem/LMSubsystem.hpp"
#include "lm_subsystem/InputCommandProcessor.hpp"

static bool
_initialize_singleton_instances()
{
  smsc::logger::Logger::Init();
  utilx::SubsystemsManager::init();
  io_dispatcher::IODispatcherSubsystem::init();
  messages_router::MessagesRouterSubsystem::init();
  sua_stack::SuaStackSubsystem::init();
  lm_subsystem::LMSubsystem::init();
  sua_user_communication::SuaUserCommunicationSubsystem::init();
  runtime_cfg::RuntimeConfig::init();

  return true;
}

static bool all_singleton_was_initialized = _initialize_singleton_instances();;

int main(int argc, char** argv)
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("init");

  const char* cfgFile;
  if ( argc < 2 )
    cfgFile = "config.xml";
  else
    cfgFile = argv[1];

  try {
    smsc::util::config::Manager::init(cfgFile);
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    smsc::util::config::ConfigView suaConfigView(manager, "SuaLayer");

    runtime_cfg::RuntimeConfig& rconfig = runtime_cfg::RuntimeConfig::getInstance();
    rconfig.initialize(&suaConfigView);

    smsc_log_info(logger, "main::: try to initialize all subsystems");

    io_dispatcher::IODispatcherSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: IODispatcherSubsystem has been initialized");

    messages_router::MessagesRouterSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: MessagesRouterSubsystem has been initialized");

    sua_stack::SuaStackSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaStackSubsystem has been initialized");

    sua_user_communication::SuaUserCommunicationSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaUserCommunicationSubsystem has been initialized");

    lm_subsystem::LMSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: LMSubsystem has been initialized");

    utilx::SubsystemsManager::getInstance()->initialize();
    utilx::SubsystemsManager::getInstance()->registerSubsystem(io_dispatcher::IODispatcherSubsystem::getInstance());
    utilx::SubsystemsManager::getInstance()->registerSubsystem(sua_user_communication::SuaUserCommunicationSubsystem::getInstance(), io_dispatcher::IODispatcherSubsystem::getInstance());
    utilx::SubsystemsManager::getInstance()->registerSubsystem(sua_stack::SuaStackSubsystem::getInstance(), io_dispatcher::IODispatcherSubsystem::getInstance());
    utilx::SubsystemsManager::getInstance()->registerSubsystem(lm_subsystem::LMSubsystem::getInstance());

    smsc_log_info(logger, "main::: try start all subsystems");
    utilx::SubsystemsManager::getInstance()->startup();
    smsc_log_info(logger, "main::: all subsystems have been started");

    utilx::SubsystemsManager::getInstance()->waitForCompletion();

    smsc_log_error(logger, "main::: exit");
    //utilx::SubsystemsManager::getInstance()->shutdown();
  } catch (const std::exception& ex) {
    smsc_log_error(logger, "main::: catched exception [%s]. Terminated.", ex.what());
    return 1;
  } catch (...) {
    smsc_log_error(logger, "main::: catched unexpected exception [...]. Terminated.");
    return 1;
  }

  return 0;
}
