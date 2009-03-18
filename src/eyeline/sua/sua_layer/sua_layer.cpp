#include <logger/Logger.h>
#include <exception>
#include <iostream>
#include <eyeline/utilx/SubsystemsManager.hpp>
#include <util/config/Manager.h>

#include <eyeline/utilx/runtime_cfg/Parameter.hpp>
#include <eyeline/utilx/runtime_cfg/CompositeParameter.hpp>
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
  eyeline::utilx::SubsystemsManager::init();
  eyeline::sua::sua_layer::runtime_cfg::RuntimeConfig::init();
  eyeline::sua::sua_layer::io_dispatcher::IODispatcherSubsystem::init();
  eyeline::sua::sua_layer::messages_router::MessagesRouterSubsystem::init();
  eyeline::sua::sua_layer::sua_stack::SuaStackSubsystem::init();
  eyeline::sua::sua_layer::lm_subsystem::LMSubsystem::init();
  eyeline::sua::sua_layer::sua_user_communication::SuaUserCommunicationSubsystem::init();

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

    eyeline::utilx::runtime_cfg::RuntimeConfig& rconfig = eyeline::sua::sua_layer::runtime_cfg::RuntimeConfig::getInstance();
    rconfig.initialize(&suaConfigView);

    smsc_log_info(logger, "main::: try to initialize all subsystems");

    eyeline::sua::sua_layer::io_dispatcher::IODispatcherSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: IODispatcherSubsystem has been initialized");

    eyeline::sua::sua_layer::messages_router::MessagesRouterSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: MessagesRouterSubsystem has been initialized");

    eyeline::sua::sua_layer::sua_stack::SuaStackSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaStackSubsystem has been initialized");

    eyeline::sua::sua_layer::sua_user_communication::SuaUserCommunicationSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaUserCommunicationSubsystem has been initialized");

    eyeline::sua::sua_layer::lm_subsystem::LMSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: LMSubsystem has been initialized");

    eyeline::utilx::SubsystemsManager::getInstance()->initialize();
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::sua::sua_layer::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::sua::sua_layer::sua_user_communication::SuaUserCommunicationSubsystem::getInstance(), eyeline::sua::sua_layer::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::sua::sua_layer::sua_stack::SuaStackSubsystem::getInstance(), eyeline::sua::sua_layer::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::sua::sua_layer::lm_subsystem::LMSubsystem::getInstance());

    smsc_log_info(logger, "main::: try start all subsystems");
    eyeline::utilx::SubsystemsManager::getInstance()->startup();
    smsc_log_info(logger, "main::: all subsystems have been started");

    eyeline::utilx::SubsystemsManager::getInstance()->waitForCompletion();

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
