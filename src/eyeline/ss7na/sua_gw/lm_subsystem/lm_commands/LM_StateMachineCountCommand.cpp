#include <errno.h>
#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"

#include "LM_StateMachineCountCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_StateMachineCountCommand::LM_StateMachineCountCommand(const std::string& state_machine_count,
                                                         common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
{
  _stateMachineCountValue = static_cast<unsigned int>(utilx::strtol(state_machine_count.c_str(), (char **)NULL, 10));
  if ( _stateMachineCountValue == 0 && errno )
    throw common::lm_subsystem::InvalidCommandLineException("LM_StateMachineCountCommand::LM_StateMachineCountCommand::: invalid stateMachineCountValue value format [%s]",
                                                            state_machine_count.c_str());
}

std::string
LM_StateMachineCountCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_StateMachineCountCommand::executeCommand::: _stateMachineCountValue=%d", _stateMachineCountValue);
  return "OK";
}

}}}}}
