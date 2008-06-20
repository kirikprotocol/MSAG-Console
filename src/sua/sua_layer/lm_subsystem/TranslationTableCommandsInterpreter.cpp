#include <sua/utilx/toLowerCaseString.hpp>
#include "TranslationTableCommandsInterpreter.hpp"
#include "TranslationRuleCommandsInterpreter.hpp"
#include "lm_commands/LM_TranslationTable_AddTranslationRuleCommand.hpp"

namespace lm_subsystem {

std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t>
TranslationTableCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL));
  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "add" ) {
      if ( stringTokenizer.hasNextToken() &&
           utilx::toLowerCaseString(stringTokenizer.nextToken()) == "translation-rule" &&
           stringTokenizer.hasNextToken() ) {
        const std::string& ruleName = stringTokenizer.nextToken();
        parseResult.first = new lm_commands::LM_TranslationTable_AddTranslationRuleCommand(ruleName);
        parseResult.second = new TranslationRuleCommandsInterpreter(ruleName);
      } else
        throw InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid 'add' command's arguments");
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid input");
  } else
    throw InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: empty input");
  return parseResult;
}

std::string
TranslationTableCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config translation-table)# ";
}

}
