#include "eyeline/utilx/toLowerCaseString.hpp"
#include "RootLayerCommandsInterpreter.hpp"
#include "SuaLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

LM_CommandsInterpreter::interpretation_result
RootLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  interpretation_result parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL), false);

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "sua-layer" ) {
      parseResult.interpreter = create_CommandsInterpreter<SuaLayerCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
    else
      parseResult.popUpCurrentInterpreter = true;

    return parseResult;
  } else
    throw InvalidCommandLineException("RootLayerCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
RootLayerCommandsInterpreter::getPromptString() const
{
  return "sibinco sua# ";
}

}}}}
