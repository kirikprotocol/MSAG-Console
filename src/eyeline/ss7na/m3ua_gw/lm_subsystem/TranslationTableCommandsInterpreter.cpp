#include "eyeline/utilx/toLowerCaseString.hpp"

#include "TranslationTableCommandsInterpreter.hpp"
#include "TranslationRuleCommandsInterpreter.hpp"
#include "lm_commands/LM_TranslationTable_AddTranslationRule.hpp"
#include "lm_commands/LM_TranslationTable_RemoveTranslationRule.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"
#include "lm_commands/LM_TranslationTable_Show.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
TranslationTableCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  bool toMtp3flag;
  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" ) {
      if ( string_tokenizer.hasNextToken() &&
          utilx::toLowerCaseString(string_tokenizer.nextToken()) == "translation-rule" ) {
        std::string ruleName;
        parseResult.command = parseAddTranslationRuleCommand(string_tokenizer, &ruleName, &toMtp3flag);
        parseResult.interpreter = new TranslationRuleCommandsInterpreter(ruleName, toMtp3flag);
      } else
        throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid 'add' command's arguments");
    } if ( tokenValue == "remove" ) {
      if ( string_tokenizer.hasNextToken() &&
           utilx::toLowerCaseString(string_tokenizer.nextToken()) == "translation-rule" ) {
        if ( !string_tokenizer.hasNextToken() )
          throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid 'remove' command's arguments - missed 'rule_name' argument");
        const std::string& ruleName = string_tokenizer.nextToken();
        parseResult.command =
            new lm_commands::LM_TranslationTable_RemoveTranslationRule(ruleName, _trnMgr);
      } else
        throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid 'remove' command's arguments");
    } else if ( tokenValue == "show" )
      parseResult.command = new lm_commands::LM_TranslationTable_Show();
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: invalid input");
  } else
    throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

common::lm_subsystem::LM_Command*
TranslationTableCommandsInterpreter::parseAddTranslationRuleCommand(utilx::StringTokenizer& string_tokenizer,
                                                                    std::string* rule_name,
                                                                    bool* to_mtp3)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::parseAddTranslationRuleCommand::: ruleName is missed in input string");

  *rule_name = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
       utilx::toLowerCaseString(string_tokenizer.nextToken()) != "gt" )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::parseAddTranslationRuleCommand::: gt is missed in input string");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::parseAddTranslationRuleCommand::: gt value is missed in input string");

  const std::string& gt = string_tokenizer.nextToken();

  std::string ssn;
  *to_mtp3 = false;
  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenVal = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenVal == "ssn" ) {
      if ( !string_tokenizer.hasNextToken() )
        throw common::lm_subsystem::InvalidCommandLineException("TranslationTableCommandsInterpreter::parseAddTranslationRuleCommand::: ssn value is expected in input string after ssn keyword");
      ssn = string_tokenizer.nextToken();
    } else if ( tokenVal == "to-mtp3" )
      *to_mtp3 = true;
  }
  if ( ssn != "" )
    return new lm_commands::LM_TranslationTable_AddTranslationRule(*rule_name, gt, ssn, _trnMgr);
  else
    return new lm_commands::LM_TranslationTable_AddTranslationRule(*rule_name, gt, _trnMgr, *to_mtp3);
}

}}}}
