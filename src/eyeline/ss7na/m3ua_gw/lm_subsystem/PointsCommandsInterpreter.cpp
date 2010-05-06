#include "PointsCommandsInterpreter.hpp"
#include "eyeline/utilx/toLowerCaseString.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"
#include "lm_commands/LM_Points_AddPoint.hpp"
#include "lm_commands/LM_Points_Show.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
PointsCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" )
      parseResult.command = parseAddPointCommand(string_tokenizer);
    else if ( tokenValue == "show" )
      parseResult.command = new lm_commands::LM_Points_Show();
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::interpretCommandLine::: unknown command=[%s]", tokenValue.c_str());
  } else
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

common::lm_subsystem::LM_Command*
PointsCommandsInterpreter::parseAddPointCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command");

  const std::string& pointToken = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( pointToken != "point" )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: unknown command 'add %s'", pointToken.c_str());

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - point_name missed");
  const std::string& pointName = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
        utilx::toLowerCaseString(string_tokenizer.nextToken()) != "lpc" )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - missed 'lpc' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - missed value for 'lpc'");
  const std::string& pointCode = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
        utilx::toLowerCaseString(string_tokenizer.nextToken()) != "ni" )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - missed 'ni' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - missed value for 'ni'");
  const std::string& ni = string_tokenizer.nextToken();

  std::string standard = "itu";

  if ( string_tokenizer.hasNextToken() ) {
    std::string nextToken = string_tokenizer.nextToken();
    if ( utilx::toLowerCaseString(nextToken) != "standard" )
      throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - unknown keyword '%s'",
                                                              nextToken.c_str());

    if ( !string_tokenizer.hasNextToken() )
      throw common::lm_subsystem::InvalidCommandLineException("PointsCommandsInterpreter::parseAddPointCommand::: invalid input for 'add point' command - missed value for 'standard'");

    standard = utilx::toLowerCaseString(string_tokenizer.nextToken());
  }
  return new lm_commands::LM_Points_AddPoint(pointName, pointCode, ni, standard, _trnMgr);
}

}}}}
