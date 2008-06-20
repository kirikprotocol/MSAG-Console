#include <sua/utilx/toLowerCaseString.hpp>
#include "SuaLayerCommandsInterpreter.hpp"
#include "SuaApplicationsCommandsInterpreter.hpp"
#include "SgpLinksCommandsInterpreter.hpp"
#include "TranslationTableCommandsInterpreter.hpp"
#include "lm_commands/LM_LocalIpCommand.hpp"
#include "lm_commands/LM_LocalTcpPortCommand.hpp"
#include "lm_commands/LM_StateMachineCountCommand.hpp"

namespace lm_subsystem {

std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t>
SuaLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL));

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "sua-applications" ) {
      parseResult.second = create_CommandsInterpreter<SuaApplicationsCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue == "sgp-links" ) {
      parseResult.second = create_CommandsInterpreter<SgpLinksCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue == "translation-table" ) {
      parseResult.second = create_CommandsInterpreter<TranslationTableCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue == "local-ip" ) {
      parseResult.first = create_LM_Command<lm_commands::LM_LocalIpCommand>(stringTokenizer);
    } else if ( tokenValue == "local-tcp-port" ) {
      parseResult.first = create_LM_Command<lm_commands::LM_LocalTcpPortCommand>(stringTokenizer);
    } else if ( tokenValue == "state-machine-count" ) {
      parseResult.first = create_LM_Command<lm_commands::LM_StateMachineCountCommand>(stringTokenizer);
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());

    return parseResult;
  } else
    throw InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
SuaLayerCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config)# ";
}

}
