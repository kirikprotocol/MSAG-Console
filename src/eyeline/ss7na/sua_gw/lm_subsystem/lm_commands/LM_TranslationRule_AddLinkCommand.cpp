#include "LM_TranslationRule_AddLinkCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_AddLinkCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationRule_AddLinkCommand::executeCommand::: ruleName=[%s], linkId=[%s]", _ruleName.c_str(), _linkId.c_str());
  return "OK";
}

void
LM_TranslationRule_AddLinkCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}}}}}
