#include "eyeline/utilx/toLowerCaseString.hpp"
#include "M3uaCommandsInterpreter.hpp"
#include "SccpUsersCommandsInterpreter.hpp"
#include "SgpLinksCommandsInterpreter.hpp"
#include "PointsCommandsInterpreter.hpp"
#include "MTP3RoutingTableCommandsInterpreter.hpp"
#include "TranslationTableCommandsInterpreter.hpp"
#include "lm_commands/LM_M3ua_Show.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"
#include "lm_commands/LM_LocalInterface.hpp"
#include "lm_commands/LM_StateMachineCount.hpp"
#include "lm_commands/LM_SetReassemblyTimer.hpp"
#include "lm_commands/LM_SetReconnectInterval.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
M3uaCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "sccp-users" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.interpreter = create_CommandsInterpreter<SccpUsersCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "sgp-links" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.interpreter = create_CommandsInterpreter<SgpLinksCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "points" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.interpreter = create_CommandsInterpreter<PointsCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "mtp3-routing-table" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.interpreter = create_CommandsInterpreter<MTP3RoutingTableCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "translation-table" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.interpreter = create_CommandsInterpreter<TranslationTableCommandsInterpreter>(string_tokenizer);
    } else if ( tokenValue == "local-interface" )
      parseResult.command = create_LM_LocalInterface(string_tokenizer);
    else if ( tokenValue == "state-machine-count" )
      parseResult.command = create_LM_Command<lm_commands::LM_StateMachineCount>(string_tokenizer, _trnMgr);
    else if ( tokenValue == "set" ) {
      if ( !string_tokenizer.hasNextToken() )
        throw common::lm_subsystem::InvalidCommandLineException("M3uaCommandsInterpreter::interpretCommandLine::: wrong input string format - command 'set' was specified without arguments");
      const std::string& subcommandToken = utilx::toLowerCaseString(string_tokenizer.nextToken());
      if ( subcommandToken == "reassembly_timer" )
        parseResult.command = create_LM_Command<lm_commands::LM_SetReassemblyTimer>(string_tokenizer, _trnMgr);
      else if ( subcommandToken == "reconnect_interval" )
        parseResult.command = create_LM_Command<lm_commands::LM_SetReconnectInterval>(string_tokenizer, _trnMgr);
      else
        throw common::lm_subsystem::InvalidCommandLineException("M3uaCommandsInterpreter::interpretCommandLine::: wrong input string format - invalid argument='%s' for command 'set'",
                                                                subcommandToken.c_str());
    } else if ( tokenValue == "show" )
      parseResult.command  = new lm_commands::LM_M3ua_Show();
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("M3uaCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
    
    return parseResult;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("M3uaCommandsInterpreter::interpretCommandLine::: empty input");
}

common::lm_subsystem::LM_Command*
M3uaCommandsInterpreter::create_LM_LocalInterface(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("M3uaCommandsInterpreter::create_LM_LocalInterface::: wrong input string format - argument 'local_address' is missed");
  const std::string& localAddress = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("M3uaCommandsInterpreter::create_LM_LocalInterface::: wrong input string format - argument 'local_port' is missed");

  const std::string& localPort = string_tokenizer.nextToken();

  return new lm_commands::LM_LocalInterface(localAddress, localPort, _trnMgr);
}

}}}}
