#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_TranslationRule_AddApplicationCommand.hpp"

namespace lm_commands {

LM_TranslationRule_AddApplicationCommand::LM_TranslationRule_AddApplicationCommand(const std::string& appId)
  : _appId(appId)
{}

std::string
LM_TranslationRule_AddApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_TranslationRule_AddApplicationCommand::executeCommand::: appId=[%s]", _appId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName), new runtime_cfg::Parameter("application", _appId));

  return "OK";
}

void
LM_TranslationRule_AddApplicationCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}
