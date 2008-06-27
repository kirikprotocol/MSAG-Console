#include "LM_TranslationRule_Commit.hpp"

#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

namespace lm_commands {

std::string
LM_TranslationRule_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_TranslationRule_Commit::executeCommand::: Enter it");
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName),
                                                                       runtime_cfg::Parameter("commit", "ok"));
  return "";
}

void
LM_TranslationRule_Commit::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}
