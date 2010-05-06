#include <exception>
#include <iostream>

#include "logger/Logger.h"
#include "eyeline/utilx/SubsystemsManager.hpp"
#include "util/config/Manager.h"

#include "eyeline/utilx/runtime_cfg/Parameter.hpp"
#include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"

#include "runtime_cfg/RuntimeConfig.hpp"
#include "io_dispatcher/IODispatcherSubsystem.hpp"
#include "lm_subsystem/LMSubsystem.hpp"
#include "mtp3/MTP3Subsystem.hpp"
#include "sccp/SccpSubsystem.hpp"

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

    smsc::util::config::ConfigView m3uaConfigView(manager, "M3uaGW");

    eyeline::utilx::SubsystemsManager::init();
    eyeline::ss7na::m3ua_gw::runtime_cfg::RuntimeConfig::init();
    eyeline::ss7na::m3ua_gw::io_dispatcher::IODispatcherSubsystem::init();
    eyeline::ss7na::m3ua_gw::mtp3::MTP3Subsystem::init();
    eyeline::ss7na::m3ua_gw::sccp::SccpSubsystem::init();
    eyeline::ss7na::m3ua_gw::lm_subsystem::LMSubsystem::init();

    eyeline::utilx::runtime_cfg::RuntimeConfig& rconfig =
        eyeline::ss7na::m3ua_gw::runtime_cfg::RuntimeConfig::getInstance();
    rconfig.initialize(m3uaConfigView, cfgFile);

    smsc_log_info(logger, "main::: try to initialize all subsystems");
    eyeline::ss7na::m3ua_gw::io_dispatcher::IODispatcherSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: IODispatcherSubsystem has been initialized");

    eyeline::ss7na::m3ua_gw::mtp3::MTP3Subsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: M3uaStackSubsystem has been initialized");

    eyeline::ss7na::m3ua_gw::sccp::SccpSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: SccpSapSubsystem has been initialized");

    eyeline::ss7na::m3ua_gw::lm_subsystem::LMSubsystem::getInstance()->initialize(rconfig);
    smsc_log_info(logger, "main::: LMSubsystem has been initialized");

    eyeline::utilx::SubsystemsManager::getInstance()->initialize();
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::m3ua_gw::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::m3ua_gw::sccp::SccpSubsystem::getInstance(),
                                                                        eyeline::ss7na::m3ua_gw::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::m3ua_gw::mtp3::MTP3Subsystem::getInstance(),
                                                                        eyeline::ss7na::m3ua_gw::io_dispatcher::IODispatcherSubsystem::getInstance());
    eyeline::utilx::SubsystemsManager::getInstance()->registerSubsystem(eyeline::ss7na::m3ua_gw::lm_subsystem::LMSubsystem::getInstance());

    smsc_log_info(logger, "main::: try start all subsystems");
    eyeline::utilx::SubsystemsManager::getInstance()->startup();
    smsc_log_info(logger, "main::: all subsystems have been started");

    eyeline::utilx::SubsystemsManager::getInstance()->waitForCompletion();

    smsc_log_error(logger, "main::: exit");
  } catch (const std::exception& ex) {
    smsc_log_error(logger, "main::: caught exception [%s]. Terminated.", ex.what());
    return 1;
  } catch (...) {
    smsc_log_error(logger, "main::: caught unexpected exception [...]. Terminated.");
    return 1;
  }

  return 0;
}
