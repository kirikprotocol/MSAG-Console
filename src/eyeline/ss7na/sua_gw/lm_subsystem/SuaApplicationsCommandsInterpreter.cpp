#include "eyeline/utilx/toLowerCaseString.hpp"

#include "SuaApplicationsCommandsInterpreter.hpp"
#include "lm_commands/LM_Applications_AddApplicationCommand.hpp"
#include "lm_commands/LM_Applications_RemoveApplicationCommand.hpp"
#include "lm_commands/LM_Applications_ShowCommand.hpp"
#include "lm_commands/LM_Applications_Commit.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

LM_CommandsInterpreter::interpretation_result
SuaApplicationsCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  interpretation_result parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL), false);

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "add" ) {

      if ( stringTokenizer.hasNextToken() &&
           utilx::toLowerCaseString(stringTokenizer.nextToken()) == "application" &&
           stringTokenizer.hasNextToken() ) {
        const std::string& appIdValue = stringTokenizer.nextToken();
        if ( stringTokenizer.hasNextToken() )
          throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");

        parseResult.command = new lm_commands::LM_Applications_AddApplicationCommand(appIdValue);
      } else
        throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( tokenValue == "remove" ) {
      if ( stringTokenizer.hasNextToken() &&
           utilx::toLowerCaseString(stringTokenizer.nextToken()) == "application" &&
           stringTokenizer.hasNextToken() ) {
        const std::string& appIdValue = stringTokenizer.nextToken();
        if ( stringTokenizer.hasNextToken() )
          throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");

        parseResult.command = new lm_commands::LM_Applications_RemoveApplicationCommand(appIdValue);
      } else
        throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( tokenValue == "show" ) {
      if ( stringTokenizer.hasNextToken() )
        throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");
      parseResult.command = new lm_commands::LM_Applications_ShowCommand();
    } else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_Applications_Commit();
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
  } else
    throw InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

std::string
SuaApplicationsCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config sua_app)# ";
}

}}}}
