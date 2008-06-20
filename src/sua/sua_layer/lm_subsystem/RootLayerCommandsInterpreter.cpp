#include <sua/utilx/toLowerCaseString.hpp>
#include "RootLayerCommandsInterpreter.hpp"
#include "SuaLayerCommandsInterpreter.hpp"

namespace lm_subsystem {

std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t>
RootLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL));

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "sua-layer" ) {
      parseResult.second = create_CommandsInterpreter<SuaLayerCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());

    return parseResult;
  } else
    throw InvalidCommandLineException("RootLayerCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
RootLayerCommandsInterpreter::getPromptString() const
{
  return "sibinco sua# ";
}

}
