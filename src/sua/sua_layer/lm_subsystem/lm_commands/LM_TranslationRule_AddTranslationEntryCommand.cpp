#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_TranslationRule_AddTranslationEntryCommand.hpp"

namespace lm_commands {

LM_TranslationRule_AddTranslationEntryCommand::LM_TranslationRule_AddTranslationEntryCommand(const std::string& entryName, const std::string& gt, unsigned int ssn)
  : _entryName(entryName), _gt(gt), _ssn(ssn)
{}

std::string
LM_TranslationRule_AddTranslationEntryCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationRule_AddTranslationEntryCommand::executeCommand:::entryName=[%s],gt=[%s],ssn=[%d]", _entryName.c_str(), _gt.c_str(), _ssn);
  runtime_cfg::CompositeParameter* addedRouteParameter =
    runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName), new runtime_cfg::CompositeParameter("route", _entryName));

  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(addedRouteParameter, new runtime_cfg::Parameter("gt", _gt));
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(addedRouteParameter, new runtime_cfg::Parameter("ssn", _ssn));
  return "OK";
}

void
LM_TranslationRule_AddTranslationEntryCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}
