#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_TranslationRule_AddApplicationCommand.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

LM_TranslationRule_AddApplicationCommand::LM_TranslationRule_AddApplicationCommand(const std::string& appId)
  : _appId(appId)
{}

std::string
LM_TranslationRule_AddApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_TranslationRule_AddApplicationCommand::executeCommand::: appId=[%s]", _appId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName), new utilx::runtime_cfg::Parameter("application", _appId));

  return "OK";
}

void
LM_TranslationRule_AddApplicationCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}}}}}
