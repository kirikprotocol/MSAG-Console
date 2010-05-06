#include "MTP3RoutingTableCommandsInterpreter.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"
#include "lm_commands/LM_MTP3RoutingTable_Show.hpp"

#include "eyeline/utilx/toLowerCaseString.hpp"
#include "lm_commands/LM_MTP3RoutingTable_AddRoute.hpp"
#include "lm_commands/LM_MTP3RoutingTable_RemoveRoute.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
MTP3RoutingTableCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" )
      parseResult.command = createAddRouteCommand(string_tokenizer);
    else if ( tokenValue == "remove" )
      parseResult.command = createRemoveRouteCommand(string_tokenizer);
    else if ( tokenValue == "show" )
      parseResult.command = new lm_commands::LM_MTP3RoutingTable_Show();
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::interpretCommandLine::: unknown command=[%s]", tokenValue.c_str());
  } else
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

common::lm_subsystem::LM_Command*
MTP3RoutingTableCommandsInterpreter::createAddRouteCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command");

  const std::string& tokenVal = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( tokenVal != "route" )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: unknown command 'add %s'",
                                                            tokenVal.c_str());

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - route_name missed");

  const std::string& routeName = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
       utilx::toLowerCaseString(string_tokenizer.nextToken()) != "lpc" )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - missed 'lpc' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - missed value for 'lpc' keyword");

  const std::string& lpc = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
       utilx::toLowerCaseString(string_tokenizer.nextToken()) != "dpc" )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - missed 'dpc' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - missed value for 'dpc' keyword");

  const std::string& dpc = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
       utilx::toLowerCaseString(string_tokenizer.nextToken()) != "sgp_link" )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - missed 'sgp_link' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'add route' command - missed value for 'sgp_link' keyword");

  const std::string& sgpLink = string_tokenizer.nextToken();

  return new lm_commands::LM_MTP3RoutingTable_AddRoute(routeName, lpc, dpc, sgpLink, _trnMgr);
}

common::lm_subsystem::LM_Command*
MTP3RoutingTableCommandsInterpreter::createRemoveRouteCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createRemoveRouteCommand::: invalid input for 'remove route' command");

  const std::string& tokenVal = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( tokenVal != "route" )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: unknown command 'remove %s' command",
                                                            tokenVal.c_str());

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("MTP3RoutingTableCommandsInterpreter::createAddRouteCommand::: invalid input for 'remove route' command - route_name missed");

  const std::string& routeName = string_tokenizer.nextToken();

  return new lm_commands::LM_MTP3RoutingTable_RemoveRoute(routeName, _trnMgr);
}

}}}}
