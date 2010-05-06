#include "LM_SetReconnectInterval.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SetReconnectInterval::composeCommandId()
{
  return "setReconnectInterval";
}

void
LM_SetReconnectInterval::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

  utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("reconnect_interval");
  if ( !foundParam )
    throw smsc::util::Exception("LM_SetReconnectInterval::updateConfiguration::: parameter 'config.reconnect_interval' is not found in the current configuration");

  foundParam->setIntValue(_reconnectInterval);
  common::io_dispatcher::Reconnector::getInstance().setReconnectPeriod(_reconnectInterval);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
