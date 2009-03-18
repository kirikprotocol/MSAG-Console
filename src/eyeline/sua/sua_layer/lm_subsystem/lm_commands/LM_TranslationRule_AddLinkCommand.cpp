#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_TranslationRule_AddLinkCommand.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

LM_TranslationRule_AddLinkCommand::LM_TranslationRule_AddLinkCommand(const std::string& linkId)
  : _linkId(linkId)
{}

std::string
LM_TranslationRule_AddLinkCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationRule_AddLinkCommand::executeCommand::: ruleName=[%s], linkId=[%s]", _ruleName.c_str(), _linkId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName), new utilx::runtime_cfg::Parameter("sgp_link", _linkId));
  return "OK";
}

void
LM_TranslationRule_AddLinkCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}}}}}
