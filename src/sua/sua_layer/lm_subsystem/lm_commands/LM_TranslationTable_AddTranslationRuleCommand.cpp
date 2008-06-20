#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_TranslationTable_AddTranslationRuleCommand.hpp"

namespace lm_commands {

LM_TranslationTable_AddTranslationRuleCommand::LM_TranslationTable_AddTranslationRuleCommand(const std::string& ruleName)
  : _ruleName(ruleName)
{}

std::string
LM_TranslationTable_AddTranslationRuleCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationTable_AddTranslationRuleCommand::executeCommand::: ruleName=[%s]", _ruleName.c_str());

  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(runtime_cfg::CompositeParameter("config.routing-keys"), new runtime_cfg::CompositeParameter("routingEntry", _ruleName));

  return "OK";
}

}
