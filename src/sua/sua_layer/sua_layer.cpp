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

static void
printRuntimeConfig(runtime_cfg::RuntimeConfig& rconfig)
{
  runtime_cfg::Parameter& localIpParam = rconfig.find<runtime_cfg::Parameter>("config.local_ip");
  std::cout << localIpParam.getName() << "=" << localIpParam.getValue() << std::endl;
  runtime_cfg::Parameter& localPortParam = rconfig.find<runtime_cfg::Parameter>("config.local_port");
  std::cout << localPortParam.getName() << "=" << localPortParam.getValue() << std::endl;
  runtime_cfg::Parameter& stateMachinesCountParam = rconfig.find<runtime_cfg::Parameter>("config.state_machines_count");
  std::cout << stateMachinesCountParam.getName() << "=" << stateMachinesCountParam.getValue() << std::endl;

  runtime_cfg::CompositeParameter& suaApplicationsParameter = rconfig.find<runtime_cfg::CompositeParameter>("config.sua_applications");
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> applicationIterator = suaApplicationsParameter.getIterator<runtime_cfg::Parameter>("application");
  while(applicationIterator.hasElement()) {
    runtime_cfg::Parameter* param = applicationIterator.getCurrentElement();
    std::cout << param->getName() << "=" << param->getValue() << std::endl;

    applicationIterator.next(); // pass to next element
  }

  runtime_cfg::CompositeParameter& sgpLinksParameter = rconfig.find<runtime_cfg::CompositeParameter>("config.sgp_links");
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> linksIterator = sgpLinksParameter.getIterator<runtime_cfg::CompositeParameter>("link");
  std::cout << "!!!!! Bypass sgp_links" << std::endl;
  while(linksIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* linkParam = linksIterator.getCurrentElement();
    std::cout << "!!!!! " << linkParam->getName() << "=" << linkParam->getValue() << std::endl;
    runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> rhostsIterator = linkParam->getIterator<runtime_cfg::Parameter>("remote_address");
    while(rhostsIterator.hasElement()) {
      std::cout << "remote_address=" << rhostsIterator.getCurrentElement()->getValue()
                << " remote_port="  << linkParam->getParameter<runtime_cfg::Parameter>("remote_port")->getValue()
                << std::endl;
      rhostsIterator.next(); // pass to next element
    }
    linksIterator.next(); // pass to next element
  }

  runtime_cfg::CompositeParameter& incomingRoutingKeysParameter = rconfig.find<runtime_cfg::CompositeParameter>("config.incoming-routing-keys");
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> incomingGTIterator = incomingRoutingKeysParameter.getIterator<runtime_cfg::CompositeParameter>("GT");
  std::cout << "!!!!! Bypass incoming_routing_keys" << std::endl;
  while(incomingGTIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* gtParams = incomingGTIterator.getCurrentElement();
    std::cout << "GT=" << gtParams->getValue() << std::endl;
    runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> appIdIterator = gtParams->getIterator<runtime_cfg::Parameter>("application_id");
    while (appIdIterator.hasElement()) {
      std::cout << "sua_application_id = " << appIdIterator.getCurrentElement()->getValue() << std::endl;
      appIdIterator.next();
    }
    std::cout << "traffic_mode = " << gtParams->getParameter<runtime_cfg::Parameter>("traffic_mode")->getValue() << std::endl;
    incomingGTIterator.next();
  }

  runtime_cfg::CompositeParameter& outcomingRoutingKeysParameter = rconfig.find<runtime_cfg::CompositeParameter>("config.outcoming-routing-keys");
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> outcomingGTIterator = outcomingRoutingKeysParameter.getIterator<runtime_cfg::CompositeParameter>("GT");
  std::cout << "!!!!! Bypass outcoming_routing_keys" << std::endl;
  while(outcomingGTIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* gtParams = outcomingGTIterator.getCurrentElement();
    std::cout << "GT=" << gtParams->getValue() << std::endl;
    runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> sgpLinkIdIterator = gtParams->getIterator<runtime_cfg::Parameter>("link_id");
    while (sgpLinkIdIterator.hasElement()) {
      std::cout << "sgp_link_id = " << sgpLinkIdIterator.getCurrentElement()->getValue() << std::endl;
      sgpLinkIdIterator.next();
    }
    std::cout << "traffic_mode = " << gtParams->getParameter<runtime_cfg::Parameter>("traffic_mode")->getValue() << std::endl;
    outcomingGTIterator.next();
  }
}

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

    //    printRuntimeConfig(rconfig);
    //    return 0;

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
