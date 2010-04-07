#include "eyeline/utilx/toLowerCaseString.hpp"
#include "SuaLayerCommandsInterpreter.hpp"
#include "SuaApplicationsCommandsInterpreter.hpp"
#include "SgpLinksCommandsInterpreter.hpp"
#include "TranslationTableCommandsInterpreter.hpp"
#include "lm_commands/LM_SuaLayer_Commit.hpp"
#include "lm_commands/LM_LocalIpCommand.hpp"
#include "lm_commands/LM_LocalTcpPortCommand.hpp"
#include "lm_commands/LM_StateMachineCountCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

LM_CommandsInterpreter::interpretation_result
SuaLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  interpretation_result parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL), false);

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "sua-applications" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.interpreter = create_CommandsInterpreter<SuaApplicationsCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue == "sgp-links" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.interpreter = create_CommandsInterpreter<SgpLinksCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue == "translation-table" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.interpreter = create_CommandsInterpreter<TranslationTableCommandsInterpreter>(stringTokenizer);
    } else if ( tokenValue == "local-ip" )
      parseResult.command = create_LM_Command<lm_commands::LM_LocalIpCommand>(stringTokenizer);
    else if ( tokenValue == "local-tcp-port" )
      parseResult.command = create_LM_Command<lm_commands::LM_LocalTcpPortCommand>(stringTokenizer);
    else if ( tokenValue == "state-machine-count" )
      parseResult.command = create_LM_Command<lm_commands::LM_StateMachineCountCommand>(stringTokenizer);
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.popUpCurrentInterpreter = true;
    } else
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

}}}}
