#include <errno.h>
#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/lm_subsystem/Exception.hpp"

#include "LM_LocalTcpPortCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_LocalTcpPortCommand::LM_LocalTcpPortCommand(const std::string& localPortValue)
{
  _localPort = static_cast<unsigned int>(utilx::strtol(localPortValue.c_str(), (char **)NULL, 10));
  if ( _localPort == 0 && errno )
    throw lm_subsystem::InvalidCommandLineException("LM_LocalTcpPortCommand::LM_LocalTcpPortCommand::: invalid localPortValue value format [%s]", localPortValue.c_str());
}

std::string
LM_LocalTcpPortCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_LocalTcpPortCommand::executeCommand::: _localPort=%d", _localPort);
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(utilx::runtime_cfg::CompositeParameter("config"),
                                                                       utilx::runtime_cfg::Parameter("local_port", _localPort));
  return "OK";
}

}}}}}
