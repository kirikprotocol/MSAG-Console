#include "LM_TranslationRule_Commit.hpp"

#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_TranslationRule_Commit::executeCommand::: Enter it");
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(utilx::runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName), utilx::runtime_cfg::Parameter("commit", "ok"));
  return "";
}

void
LM_TranslationRule_Commit::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}}}}}
