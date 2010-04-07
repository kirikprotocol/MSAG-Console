#include "eyeline/utilx/strtol.hpp"
#include "IODispatcherSubsystem.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace io_dispatcher {

void
IODispatcherSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  common::io_dispatcher::IODispatcherSubsystem::initialize();
  utilx::runtime_cfg::Parameter& stateMachinesCount = rconfig.find<utilx::runtime_cfg::Parameter>("config.state_machines_count");
  _stateMachinesCount = utilx::strtol(stateMachinesCount.getValue().c_str(), (char **)NULL, 10);
  if ( _stateMachinesCount == 0 && errno )
    throw smsc::util::Exception("IODispatcherSubsystem::initialize::: invalid value of config.state_machines_count parameter=[%s]", stateMachinesCount.getValue().c_str());
  unsigned reconnectInterval = 1;
  try {
    utilx::runtime_cfg::Parameter& reconnectIntervalParam = rconfig.find<utilx::runtime_cfg::Parameter>("config.reconnect_interval");
    reconnectInterval = static_cast<unsigned>(utilx::strtol(reconnectIntervalParam.getValue().c_str(), (char **)NULL, 10));
    if ( reconnectInterval == 0 && errno )
      throw smsc::util::Exception("IODispatcherSubsystem::initialize::: invalid value of config.reconnect_interval parameter=[%s]", reconnectIntervalParam.getValue().c_str());
  } catch (std::runtime_error& ex) {}

  ConnectMgr& connMgr = getInitedConnectMgr();
  common::io_dispatcher::Reconnector::getInstance().initialize(&connMgr, reconnectInterval);
}

void
IODispatcherSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                   const utilx::runtime_cfg::Parameter& modified_parameter)
{
  if ( context.getFullName() == "config" ) {
    if ( modified_parameter.getFullName() != "state_machines_count" ) return;

    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

    smsc_log_debug(_logger, "IODispatcherSubsystem::handle::: handle modified parameter 'config.%s'='%s'", modified_parameter.getName().c_str(), modified_parameter.getValue().c_str());

    utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>(modified_parameter.getName());
    if ( foundParam )
      foundParam->setValue(modified_parameter.getValue());
  }
}

}}}}
