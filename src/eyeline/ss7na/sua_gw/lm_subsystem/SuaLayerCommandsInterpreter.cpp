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

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
SuaLayerCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "sua-applications" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.interpreter = create_CommandsInterpreter<SuaApplicationsCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "sgp-links" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.interpreter = create_CommandsInterpreter<SgpLinksCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "translation-table" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.interpreter = create_CommandsInterpreter<TranslationTableCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "local-ip" )
      parseResult.command = create_LM_Command<lm_commands::LM_LocalIpCommand>(string_tokenizer, _trnMgr);
    else if ( tokenValue == "local-tcp-port" )
      parseResult.command = create_LM_Command<lm_commands::LM_LocalTcpPortCommand>(string_tokenizer, _trnMgr);
    else if ( tokenValue == "state-machine-count" )
      parseResult.command = create_LM_Command<lm_commands::LM_StateMachineCountCommand>(string_tokenizer, _trnMgr);
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_SuaLayer_Commit();
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
    
    return parseResult;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("SuaLayerCommandsInterpreter::interpretCommandLine::: empty input");
}

std::string
SuaLayerCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config)# ";
}

}}}}
