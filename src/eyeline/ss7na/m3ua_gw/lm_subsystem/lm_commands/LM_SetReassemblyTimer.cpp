#include "LM_SetReassemblyTimer.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/ReassemblyProcessor.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SetReassemblyTimer::composeCommandId()
{
  return "setReassemblyTimer";
}

void
LM_SetReassemblyTimer::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

  utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("reassembly_timer");
  if ( !foundParam )
    throw smsc::util::Exception("LM_SetReassemblyTimer::updateConfiguration::: parameter 'config.reassembly_timer' is not found in the current configuration");

  foundParam->setIntValue(_timerValue);

  sccp::ReassemblyProcessor::getInstance().reinitialize(_timerValue);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
