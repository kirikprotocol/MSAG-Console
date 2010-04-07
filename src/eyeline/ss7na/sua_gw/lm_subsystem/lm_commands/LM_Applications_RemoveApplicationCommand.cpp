#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"

#include "LM_Applications_RemoveApplicationCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_Applications_RemoveApplicationCommand::LM_Applications_RemoveApplicationCommand(const std::string& appIdValue)
  : _appId(appIdValue)
{}

std::string
LM_Applications_RemoveApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_Applications_RemoveApplicationCommand::executeCommand::: _appId=%s", _appId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyRemoveParameterEvent(utilx::runtime_cfg::Parameter("config.sua_applications.application", _appId));
  return "OK";
}

}}}}}
