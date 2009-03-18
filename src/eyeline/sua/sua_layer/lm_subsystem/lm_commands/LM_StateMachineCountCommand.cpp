#include <errno.h>
#include <eyeline/utilx/strtol.hpp>
#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>

#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/sua/sua_layer/lm_subsystem/Exception.hpp>

#include "LM_StateMachineCountCommand.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

LM_StateMachineCountCommand::LM_StateMachineCountCommand(const std::string& stateMachineCountValue)
{
  _stateMachineCountValue = static_cast<unsigned int>(utilx::strtol(stateMachineCountValue.c_str(), (char **)NULL, 10));
  if ( _stateMachineCountValue == 0 && errno )
    throw lm_subsystem::InvalidCommandLineException("LM_StateMachineCountCommand::LM_StateMachineCountCommand::: invalid stateMachineCountValue value format [%s]", stateMachineCountValue.c_str());
}

std::string
LM_StateMachineCountCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_StateMachineCountCommand::executeCommand::: _stateMachineCountValue=%d", _stateMachineCountValue);
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(utilx::runtime_cfg::CompositeParameter("config"),
                                                                       utilx::runtime_cfg::Parameter("state_machines_count", _stateMachineCountValue));
  return "OK";
}

}}}}}
