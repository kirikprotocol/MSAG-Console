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

TranslationRuleCommandsInterpreter::TranslationRuleCommandsInterpreter(const std::string& ruleName)
  : _ruleName(ruleName)
{}

LM_CommandsInterpreter::interpretation_result
TranslationRuleCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  interpretation_result parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL), false);

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& command = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( command == "add" ) {
      if ( stringTokenizer.hasNextToken() ) {
        const std::string& subCommand = utilx::toLowerCaseString(stringTokenizer.nextToken());

        if ( subCommand == "application" )
          parseResult.command = create_addApplicationCommand(stringTokenizer);
        else if ( subCommand == "link" )
          parseResult.command = create_addLinkCommand(stringTokenizer);
        else if ( subCommand == "translation-entry" )
          parseResult.command = create_addTranslationEntryCommand(stringTokenizer);
        else
          throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
      } else
        throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( command == "traffic-mode" ) {
      if ( stringTokenizer.hasNextToken() ) {
        const std::string& trafficModeValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
        lm_commands::LM_TranslationRule_TrafficModeCommand* trafficModeCommand = new lm_commands::LM_TranslationRule_TrafficModeCommand(trafficModeValue);
        trafficModeCommand->setTranslationRuleName(_ruleName);
        parseResult.command = trafficModeCommand;
      } else
        throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( command == "exit" || command == "quit" ) {
      lm_commands::LM_TranslationRule_Commit* commitCommand = new lm_commands::LM_TranslationRule_Commit();
      commitCommand->setTranslationRuleName(_ruleName);
      parseResult.command = commitCommand;
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
  } else
    throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

lm_commands::LM_TranslationRule_AddApplicationCommand*
TranslationRuleCommandsInterpreter::create_addApplicationCommand(utilx::StringTokenizer& stringTokenizer)
{
  lm_commands::LM_TranslationRule_AddApplicationCommand* addApplicationCommand = create_LM_Command<lm_commands::LM_TranslationRule_AddApplicationCommand>(stringTokenizer);
  addApplicationCommand->setTranslationRuleName(_ruleName);
  return addApplicationCommand;
}

lm_commands::LM_TranslationRule_AddLinkCommand*
TranslationRuleCommandsInterpreter::create_addLinkCommand(utilx::StringTokenizer& stringTokenizer)
{
  lm_commands::LM_TranslationRule_AddLinkCommand* addLinkCommand = create_LM_Command<lm_commands::LM_TranslationRule_AddLinkCommand>(stringTokenizer);
  addLinkCommand->setTranslationRuleName(_ruleName);
  return addLinkCommand;
}

lm_commands::LM_TranslationRule_AddTranslationEntryCommand*
TranslationRuleCommandsInterpreter::create_addTranslationEntryCommand(utilx::StringTokenizer& stringTokenizer)
{
  if ( stringTokenizer.hasNextToken() ) {
    const std::string& entryName = stringTokenizer.nextToken();
    if ( stringTokenizer.hasNextToken() &&
         utilx::toLowerCaseString(stringTokenizer.nextToken()) == "gt" &&
         stringTokenizer.hasNextToken() ) {
      const std::string& gt = stringTokenizer.nextToken();
      unsigned ssn = 0;
      if ( stringTokenizer.hasNextToken() &&
           utilx::toLowerCaseString(stringTokenizer.nextToken()) == "ssn" ) {
        if ( stringTokenizer.hasNextToken() ) {
          const std::string& ssnStrValue = stringTokenizer.nextToken();
          ssn = static_cast<unsigned>(utilx::strtol(ssnStrValue.c_str(), (char **)NULL, 10));
          if ( ssn == 0 && errno )
            throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid ssn value=[%s]", ssnStrValue.c_str());
        }
      }
      lm_commands::LM_TranslationRule_AddTranslationEntryCommand* addTranslationEntryCommand = new lm_commands::LM_TranslationRule_AddTranslationEntryCommand(entryName, gt, ssn);
      addTranslationEntryCommand->setTranslationRuleName(_ruleName);
      return addTranslationEntryCommand;
    } else
      throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input");
  } else
    throw InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
TranslationRuleCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config translation-table translation-rule)# ";
}

}}}}
