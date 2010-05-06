#include "LM_TranslationRule_AddTranslationEntryCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_AddTranslationEntryCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationRule_AddTranslationEntryCommand::executeCommand:::entryName=[%s],gt=[%s],ssn=[%d]", _entryName.c_str(), _gt.c_str(), _ssn);
  return "OK";
}

}}}}}
