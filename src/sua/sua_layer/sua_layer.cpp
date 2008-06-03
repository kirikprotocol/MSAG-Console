#include <sua/sua_layer/runtime_cfg/Parameter.hpp>
#include <sua/sua_layer/runtime_cfg/CompositeParameter.hpp>
#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

#include <sua/sua_layer/io_dispatcher/IODispatcherSubsystem.hpp>
#include <sua/sua_layer/sua_stack/SuaStackSubsystem.hpp>
#include <sua/sua_layer/sua_user_communication/SuaUserCommunicationSubsystem.hpp>
#include <sua/sua_layer/messages_router/MessagesRouterSubsystem.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>

#include <sua/utilx/SubsystemsManager.hpp>
#include <logger/Logger.h>
#include <exception>
#include <util/config/Manager.h>

#include <iostream>

int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
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

    runtime_cfg::RuntimeConfig::init();
    runtime_cfg::RuntimeConfig& rconfig = runtime_cfg::RuntimeConfig::getInstance();
    rconfig.initialize(&suaConfigView);

    utilx::SubsystemsManager::init();

    smsc_log_info(logger, "main::: try to initialize all subsystems");
    io_dispatcher::IODispatcherSubsystem::init();
    io_dispatcher::IODispatcherSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: IODispatcherSubsystem has been initialized");

    messages_router::MessagesRouterSubsystem::init();
    messages_router::MessagesRouterSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: MessagesRouterSubsystem has been initialized");

    sua_stack::SuaStackSubsystem::init();
    sua_stack::SuaStackSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaStackSubsystem has been initialized");

    sua_user_communication::SuaUserCommunicationSubsystem::init();
    sua_user_communication::SuaUserCommunicationSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaUserCommunicationSubsystem has been initialized");

    utilx::SubsystemsManager::getInstance()->initialize();
    utilx::SubsystemsManager::getInstance()->registerSubsystem(io_dispatcher::IODispatcherSubsystem::getInstance());
    utilx::SubsystemsManager::getInstance()->registerSubsystem(sua_user_communication::SuaUserCommunicationSubsystem::getInstance(), io_dispatcher::IODispatcherSubsystem::getInstance());
    utilx::SubsystemsManager::getInstance()->registerSubsystem(sua_stack::SuaStackSubsystem::getInstance(), io_dispatcher::IODispatcherSubsystem::getInstance());

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
