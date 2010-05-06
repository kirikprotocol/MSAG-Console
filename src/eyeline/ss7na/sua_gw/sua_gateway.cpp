#include <exception>
#include <iostream>

#include "logger/Logger.h"
#include "eyeline/utilx/SubsystemsManager.hpp"
#include "util/config/Manager.h"

#include "eyeline/utilx/runtime_cfg/Parameter.hpp"
#include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"

#include "runtime_cfg/RuntimeConfig.hpp"
#include "io_dispatcher/IODispatcherSubsystem.hpp"
#include "sua_stack/SuaStackSubsystem.hpp"
#include "sccp_sap/SccpSapSubsystem.hpp"
#include "messages_router/MessagesRouterSubsystem.hpp"
#include "lm_subsystem/LMSubsystem.hpp"

static bool
_initialize_singleton_instances()
{
  smsc::logger::Logger::Init();
  eyeline::utilx::SubsystemsManager::init();
  eyeline::ss7na::sua_gw::runtime_cfg::RuntimeConfig::init();
  eyeline::ss7na::sua_gw::io_dispatcher::IODispatcherSubsystem::init();
  eyeline::ss7na::sua_gw::messages_router::MessagesRouterSubsystem::init();
  eyeline::ss7na::sua_gw::sua_stack::SuaStackSubsystem::init();
  eyeline::ss7na::sua_gw::lm_subsystem::LMSubsystem::init();
  eyeline::ss7na::sua_gw::sccp_sap::SccpSapSubsystem::init();

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

    eyeline::utilx::runtime_cfg::RuntimeConfig& rconfig = eyeline::ss7na::sua_gw::runtime_cfg::RuntimeConfig::getInstance();
    rconfig.initialize(suaConfigView, cfgFile);

    smsc_log_info(logger, "main::: try to initialize all subsystems");

    eyeline::ss7na::sua_gw::io_dispatcher::IODispatcherSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: IODispatcherSubsystem has been initialized");

    eyeline::ss7na::sua_gw::messages_router::MessagesRouterSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: MessagesRouterSubsystem has been initialized");

    eyeline::ss7na::sua_gw::sua_stack::SuaStackSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaStackSubsystem has been initialized");

    eyeline::ss7na::sua_gw::sccp_sap::SccpSapSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SuaUserCommunicationSubsystem has been initialized");

    eyeline::ss7na::sua_gw::lm_subsystem::LMSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: LMSubsystem has been initialized");

    eyeline::utilx::SubsystemsManager::getInstance()->initialize();
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::sua_gw::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::sua_gw::sccp_sap::SccpSapSubsystem::getInstance(),
                                                                        eyeline::ss7na::sua_gw::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::sua_gw::sua_stack::SuaStackSubsystem::getInstance(),
                                                                        eyeline::ss7na::sua_gw::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::sua_gw::lm_subsystem::LMSubsystem::getInstance());

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
