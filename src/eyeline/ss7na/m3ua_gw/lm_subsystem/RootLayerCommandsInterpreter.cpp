#include "eyeline/utilx/toLowerCaseString.hpp"
#include "RootLayerCommandsInterpreter.hpp"
#include "M3uaCommandsInterpreter.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

bool RootLayerCommandsInterpreter::configurationWasModified = false;

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
RootLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL), false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "conf" )
      parseResult.interpreter = create_CommandsInterpreter<M3uaCommandsInterpreter>(string_tokenizer);
    else if ( tokenValue != "exit" && tokenValue != "quit" )
      throw common::lm_subsystem::InvalidCommandLineException("RootLayerCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
    else {
      if ( configurationWasModified ) {
        runtime_cfg::RuntimeConfig::getInstance().commit();
        configurationWasModified = false;
      }
      parseResult.popUpCurrentInterpreter = true;
    }
    return parseResult;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("RootLayerCommandsInterpreter::interpretCommandLine::: empty input");
}

}}}}
