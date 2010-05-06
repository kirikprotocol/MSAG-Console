#include "LM_TranslationTable_AddTranslationRuleCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationTable_AddTranslationRuleCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationTable_AddTranslationRuleCommand::executeCommand::: ruleName=[%s]", _ruleName.c_str());
  return "OK";
}

}}}}}
