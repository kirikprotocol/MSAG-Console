#include "eyeline/utilx/toLowerCaseString.hpp"
#include "RootLayerCommandsInterpreter.hpp"
#include "SuaLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
RootLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL), false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "sua-layer" ) {
      parseResult.interpreter = create_CommandsInterpreter<SuaLayerCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw common::lm_subsystem::InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
    else
      parseResult.popUpCurrentInterpreter = true;

    return parseResult;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("RootLayerCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
RootLayerCommandsInterpreter::getPromptString() const
{
  return "sibinco sua# ";
}

}}}}
