#include "eyeline/utilx/toLowerCaseString.hpp"
#include "TranslationTableCommandsInterpreter.hpp"
#include "TranslationRuleCommandsInterpreter.hpp"
#include "lm_commands/LM_TranslationTable_AddTranslationRuleCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

LM_CommandsInterpreter::interpretation_result
TranslationTableCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  interpretation_result parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL), false);
  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "add" ) {
      if ( stringTokenizer.hasNextToken() &&
           utilx::toLowerCaseString(stringTokenizer.nextToken()) == "translation-rule" &&
           stringTokenizer.hasNextToken() ) {
        const std::string& ruleName = stringTokenizer.nextToken();
        parseResult.command = new lm_commands::LM_TranslationTable_AddTranslationRuleCommand(ruleName);
        parseResult.interpreter = new TranslationRuleCommandsInterpreter(ruleName);
      } else
        throw InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid 'add' command's arguments");
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid input");
    else
      parseResult.popUpCurrentInterpreter = true;
  } else
    throw InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: empty input");
  return parseResult;
}

std::string
TranslationTableCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config translation-table)# ";
}

}}}}
