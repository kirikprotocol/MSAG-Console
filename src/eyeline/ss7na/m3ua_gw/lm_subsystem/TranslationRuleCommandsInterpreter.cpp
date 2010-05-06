#include "TranslationRuleCommandsInterpreter.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"
#include "lm_commands/LM_TranslationRule_AddDpc.hpp"
#include "lm_commands/LM_TranslationRule_AddSccpUser.hpp"
#include "lm_commands/LM_TranslationRule_SetLpc.hpp"
#include "lm_commands/LM_TranslationRule_TrafficMode.hpp"
#include "lm_commands/LM_TranslationRule_OkCmd.hpp"
#include "eyeline/utilx/toLowerCaseString.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
TranslationRuleCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" )
      parseResult.command = parseAddCommand(string_tokenizer);
    else if ( tokenValue == "set" )
      parseResult.command = parseSetLpcCommand(string_tokenizer);
    else if ( tokenValue == "traffic-mode" )
      parseResult.command = parseTrafficModeCommand(string_tokenizer);
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      if ( _addDpcCmd )
        _addDpcCmd->updateConfiguration();
      else if ( _addSccpUserCmd )
        _addSccpUserCmd->updateConfiguration();
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
  } else
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

common::lm_subsystem::LM_Command*
TranslationRuleCommandsInterpreter::parseAddCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: empty argument list for 'add' command");
  const std::string& subcommand = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( subcommand == "dpc" ) {
    if ( !string_tokenizer.hasNextToken() )
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: empty argument list for 'add dpc' command");
    if ( !_toMtp3 )
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: add dpc command is actually only for route to mtp3");

    if ( !_addDpcCmd )
      _addDpcCmd = new lm_commands::LM_TranslationRule_AddDpc(_ruleName,
                                                              string_tokenizer.nextToken(),
                                                              _trnMgr);
    else
      _addDpcCmd->addDpcValue(string_tokenizer.nextToken());
    return new lm_commands::LM_TranslationRule_OkCmd();
  } else if ( subcommand == "sccp_user" ) {
    if ( !string_tokenizer.hasNextToken() )
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: empty argument list for 'add sccp_user' command");
    if ( _toMtp3 )
      throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: add sccp_user command is actually only for terminated route to sccp");
    if ( !_addSccpUserCmd )
      _addSccpUserCmd = new lm_commands::LM_TranslationRule_AddSccpUser(_ruleName,
                                                                        string_tokenizer.nextToken(),
                                                                        _trnMgr);
    else
      _addSccpUserCmd->addSccpUserValue(string_tokenizer.nextToken());
    return new lm_commands::LM_TranslationRule_OkCmd();
  } else
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: invalid subcommand='%s' for 'add command'", subcommand.c_str());
}

common::lm_subsystem::LM_Command*
TranslationRuleCommandsInterpreter::parseSetLpcCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseSetLpcCommand::: empty argument list for 'add' command");
  const std::string& subcommand = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( subcommand != "lpc" )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseSetLpcCommand::: unknown subcommand='%s' for 'set command'", subcommand.c_str());

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: empty argument list for 'set lpc' command");

  if ( !_toMtp3 )
    throw common::lm_subsystem::InvalidCommandLineException("TranslationRuleCommandsInterpreter::parseAddCommand::: set lpc command is actually only for route to mtp3");

  _addDpcCmd->setLpcValue(string_tokenizer.nextToken());
  return new lm_commands::LM_TranslationRule_OkCmd();
}

common::lm_subsystem::LM_Command*
TranslationRuleCommandsInterpreter::parseTrafficModeCommand(utilx::StringTokenizer& string_tokenizer)
{
  std::string tokenValue;
  if ( string_tokenizer.hasNextToken() )
    tokenValue = string_tokenizer.nextToken();
  else
    throw common::lm_subsystem::InvalidCommandLineException("LM_CommandsInterpreter::parseTrafficModeCommand::: missed mandatory keyword");

  if ( _addSccpUserCmd )
    _addSccpUserCmd->setTrafficMode(utilx::toLowerCaseString(tokenValue));
  else
    _addDpcCmd->setTrafficMode(utilx::toLowerCaseString(tokenValue));

  return new lm_commands::LM_TranslationRule_OkCmd();
}

}}}}
