#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_Applications_AddApplicationCommand.hpp"

namespace lm_commands {

LM_Applications_AddApplicationCommand::LM_Applications_AddApplicationCommand(const std::string& appIdValue)
  : _appId(appIdValue)
{}

std::string
LM_Applications_AddApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_Applications_AddApplicationCommand::executeCommand::: _appId=%s", _appId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(runtime_cfg::CompositeParameter("config.sua_applications"), new runtime_cfg::Parameter("application", _appId));
  return "OK";
}

}
