#include <errno.h>
#include "eyeline/utilx/strtol.hpp"

#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/io_dispatcher/IODispatcherSubsystem.hpp"
#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"

#include "LM_StateMachineCount.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_StateMachineCount::LM_StateMachineCount(const std::string& state_machine_count,
                                           common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
: _trnMgr(trn_mgr)
{
  setId(composeCommandId());
  _stateMachineCountValue = static_cast<unsigned int>(utilx::strtol(state_machine_count.c_str(), (char **)NULL, 10));
  if ( _stateMachineCountValue == 0 && errno )
    throw common::lm_subsystem::InvalidCommandLineException("LM_StateMachineCount::LM_StateMachineCount::: invalid stateMachineCountValue value format [%s]",
                                                            state_machine_count.c_str());
}

std::string
LM_StateMachineCount::executeCommand()
{
  _trnMgr.addOperation(this);
  smsc_log_debug(_logger, "LM_StateMachineCount::executeCommand::: _stateMachineCountValue=%d", _stateMachineCountValue);
  return "OK";
}

void
LM_StateMachineCount::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

  utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("state_machines_count");
  if ( !foundParam )
    throw smsc::util::Exception("LM_StateMachineCount::updateConfiguration::: parameter 'config.state_machines_count' is not found in the current configuration");

  foundParam->setIntValue(_stateMachineCountValue);

  io_dispatcher::IODispatcherSubsystem::getInstance()->updateStateMachinesCount(_stateMachineCountValue);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
