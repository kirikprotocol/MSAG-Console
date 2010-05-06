#include "eyeline/utilx/runtime_cfg/Exception.hpp"
#include "InputCommandProcessor.hpp"
#include "LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

std::string
InputCommandProcessor::getUserPrompt()
{
  if ( _commandInterpreters.empty() )
    return "";
  else
    return _commandInterpreters.front()->getPromptString();
}

std::string
InputCommandProcessor::process(const std::string& input_command_line) {
  utilx::StringTokenizer stringTokenizer(input_command_line);
  stringTokenizer.setDelimeterSymbols(" ");

  lm_commands_interpreter_refptr_t commandInterpreter = _commandInterpreters.back();

  try {
    if ( !commandInterpreter.Get() )
      throw smsc::util::Exception("InputCommandProcessor::process::: command's interpreter wasn't set");

    LM_CommandsInterpreter::interpretation_result parseResult = commandInterpreter->interpretCommandLine(stringTokenizer);

    std::string userPromptString;

    // NOTE: destruction of object pointed by parseResult.command made during call to CmdsTransactionMgr::commit
    if ( parseResult.command )
      userPromptString = parseResult.command->executeCommand() + "\n";

    if ( parseResult.popUpCurrentInterpreter &&
         !_commandInterpreters.empty() ) {
      _commandInterpreters.pop_back();
      if ( !_commandInterpreters.empty() )
        commandInterpreter = _commandInterpreters.back();
      else {
        _commandInterpreters.push_back(lm_commands_interpreter_refptr_t(createRootCmdInterpreter()));
        throw UserTerminateSessionException();
      }
    }

    if ( parseResult.interpreter.Get() ) {
      _commandInterpreters.push_back(parseResult.interpreter);
      userPromptString += parseResult.interpreter->getPromptString();
    } else
      userPromptString += commandInterpreter->getPromptString();

    return userPromptString;
  } catch(InvalidCommandLineException& ex) {
    smsc_log_error(_logger, "InputCommandProcessor::process::: caught exception=[%s]", ex.what());
    return std::string("Error: Invalid input\n") + commandInterpreter->getPromptString();
  } catch(utilx::runtime_cfg::InconsistentConfigCommandException& ex) {
    smsc_log_error(_logger, "Error: InputCommandProcessor::process::: caught exception=[%s]", ex.what());
    if ( ex.needPopUpCurrentCommandInterpreter() ) {
      if ( !_commandInterpreters.empty() )
        _commandInterpreters.pop_back();
    }

    std::string commandResultString = ex.getErrorDignostic() + "\n";
    if ( !_commandInterpreters.empty() )
      commandResultString += _commandInterpreters.back()->getPromptString();

    return commandResultString;
  }
}

}}}}
