#include "eyeline/utilx/toLowerCaseString.hpp"
#include "TranslationTableCommandsInterpreter.hpp"
#include "TranslationRuleCommandsInterpreter.hpp"
#include "lm_commands/LM_TranslationTable_AddTranslationRuleCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
TranslationTableCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" ) {
      if ( string_tokenizer.hasNextToken() &&
           utilx::toLowerCaseString(string_tokenizer.nextToken()) == "translation-rule" &&
           string_tokenizer.hasNextToken() ) {
        const std::string& ruleName = string_tokenizer.nextToken();
        parseResult.command = new lm_commands::LM_TranslationTable_AddTranslationRuleCommand(ruleName);
        parseResult.interpreter = new TranslationRuleCommandsInterpreter(ruleName);
      } else
        throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid 'add' command's arguments");
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid input");
    else
      parseResult.popUpCurrentInterpreter = true;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: empty input");
  return parseResult;
}

std::string
TranslationTableCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config translation-table)# ";
}

}}}}
