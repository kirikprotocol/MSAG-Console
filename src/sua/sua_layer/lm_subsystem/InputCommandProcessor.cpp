#include <sua/sua_layer/runtime_cfg/Exception.hpp>
#include "InputCommandProcessor.hpp"
#include "lm_commands/LM_Command.hpp"
#include "RootLayerCommandsInterpreter.hpp"

namespace lm_subsystem {

InputCommandProcessor::InputCommandProcessor()
  : _logger(smsc::logger::Logger::getInstance("lmsubsys"))
{
  _commandInterpreters.push_back(lm_commands_interpreter_refptr_t(new RootLayerCommandsInterpreter()));
}

std::string
InputCommandProcessor::process(const std::string& inputCommandLine) {
  utilx::StringTokenizer stringTokenizer(inputCommandLine);
  stringTokenizer.setDelimeterSymbols(" ");

  lm_commands_interpreter_refptr_t commandInterpreter = _commandInterpreters.back();

  try {
    if ( !commandInterpreter.Get() )
      throw smsc::util::Exception("InputCommandProcessor::process::: command's interpreter wasn't set");

    std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> parseResult = commandInterpreter->interpretCommandLine(stringTokenizer);
    if ( !parseResult.first.Get() && !parseResult.second.Get() ) {
      if ( !_commandInterpreters.empty() ) {
        _commandInterpreters.pop_back();
        if ( !_commandInterpreters.empty() )
          return _commandInterpreters.back()->getPromptString();
      }
      return "";
    } else if ( parseResult.first.Get() && parseResult.second.Get() ) {
      _commandInterpreters.push_back(parseResult.second);
      return parseResult.first->executeCommand() + "\n" + parseResult.second->getPromptString();
    } else if ( parseResult.second.Get() ) {
      _commandInterpreters.push_back(parseResult.second);
      return parseResult.second->getPromptString();
    } else {
      return parseResult.first->executeCommand() + "\n" + commandInterpreter->getPromptString();
    }
  } catch(InvalidCommandLineException& ex) {
    smsc_log_error(_logger, "InputCommandProcessor::process::: catched exception=[%s]", ex.what());
    return std::string("Error: Invalid input\n") + commandInterpreter->getPromptString();
  } catch(runtime_cfg::InconsistentConfigCommandException& ex) {
    smsc_log_error(_logger, "Error: InputCommandProcessor::process::: catched exception=[%s]", ex.what());
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

}
