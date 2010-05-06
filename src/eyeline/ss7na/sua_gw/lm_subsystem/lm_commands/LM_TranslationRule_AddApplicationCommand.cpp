#include "LM_TranslationRule_AddApplicationCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_AddApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_TranslationRule_AddApplicationCommand::executeCommand::: appId=[%s]", _appId.c_str());
  return "OK";
}

void
LM_TranslationRule_AddApplicationCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}}}}}
