#include <stdlib.h>
#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/toLowerCaseString.hpp"
#include "TranslationRuleCommandsInterpreter.hpp"
#include "lm_commands/LM_TranslationRule_TrafficModeCommand.hpp"
#include "lm_commands/LM_TranslationRule_AddApplicationCommand.hpp"
#include "lm_commands/LM_TranslationRule_AddLinkCommand.hpp"
#include "lm_commands/LM_TranslationRule_AddTranslationEntryCommand.hpp"
#include "lm_commands/LM_TranslationRule_Commit.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
TranslationRuleCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& command = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( command == "add" ) {
      if ( string_tokenizer.hasNextToken() ) {
        const std::string& subCommand = utilx::toLowerCaseString(string_tokenizer.nextToken());

        if ( subCommand == "application" )
          parseResult.command = create_addApplicationCommand(string_tokenizer);
        else if ( subCommand == "link" )
          parseResult.command = create_addLinkCommand(string_tokenizer);
        else if ( subCommand == "translation-entry" )
          parseResult.command = create_addTranslationEntryCommand(string_tokenizer);
        else
          throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
      } else
        throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( command == "traffic-mode" ) {
      if ( string_tokenizer.hasNextToken() ) {
        const std::string& trafficModeValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
        lm_commands::LM_TranslationRule_TrafficModeCommand* trafficModeCommand = new lm_commands::LM_TranslationRule_TrafficModeCommand(trafficModeValue);
        trafficModeCommand->setTranslationRuleName(_ruleName);
        parseResult.command = trafficModeCommand;
      } else
        throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( command == "exit" || command == "quit" ) {
      lm_commands::LM_TranslationRule_Commit* commitCommand = new lm_commands::LM_TranslationRule_Commit();
      commitCommand->setTranslationRuleName(_ruleName);
      parseResult.command = commitCommand;
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
  } else
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

lm_commands::LM_TranslationRule_AddApplicationCommand*
TranslationRuleCommandsInterpreter::create_addApplicationCommand(utilx::StringTokenizer& string_tokenizer)
{
  lm_commands::LM_TranslationRule_AddApplicationCommand* addApplicationCommand =
      create_LM_Command<lm_commands::LM_TranslationRule_AddApplicationCommand>(string_tokenizer,
                                                                               _trnMgr);
  addApplicationCommand->setTranslationRuleName(_ruleName);
  return addApplicationCommand;
}

lm_commands::LM_TranslationRule_AddLinkCommand*
TranslationRuleCommandsInterpreter::create_addLinkCommand(utilx::StringTokenizer& string_tokenizer)
{
  lm_commands::LM_TranslationRule_AddLinkCommand* addLinkCommand =
      create_LM_Command<lm_commands::LM_TranslationRule_AddLinkCommand>(string_tokenizer,
                                                                        _trnMgr);
  addLinkCommand->setTranslationRuleName(_ruleName);
  return addLinkCommand;
}

lm_commands::LM_TranslationRule_AddTranslationEntryCommand*
TranslationRuleCommandsInterpreter::create_addTranslationEntryCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( string_tokenizer.hasNextToken() ) {
    const std::string& entryName = string_tokenizer.nextToken();
    if ( string_tokenizer.hasNextToken() &&
         utilx::toLowerCaseString(string_tokenizer.nextToken()) == "gt" &&
         string_tokenizer.hasNextToken() ) {
      const std::string& gt = string_tokenizer.nextToken();
      unsigned ssn = 0;
      if ( string_tokenizer.hasNextToken() &&
           utilx::toLowerCaseString(string_tokenizer.nextToken()) == "ssn" ) {
        if ( string_tokenizer.hasNextToken() ) {
          const std::string& ssnStrValue = string_tokenizer.nextToken();
          ssn = static_cast<unsigned>(utilx::strtol(ssnStrValue.c_str(), (char **)NULL, 10));
          if ( ssn == 0 && errno )
            throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid ssn value=[%s]", ssnStrValue.c_str());
        }
      }
      lm_commands::LM_TranslationRule_AddTranslationEntryCommand* addTranslationEntryCommand = new lm_commands::LM_TranslationRule_AddTranslationEntryCommand(entryName, gt, ssn);
      addTranslationEntryCommand->setTranslationRuleName(_ruleName);
      return addTranslationEntryCommand;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
  } else
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
TranslationRuleCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config translation-table translation-rule)# ";
}

}}}}
