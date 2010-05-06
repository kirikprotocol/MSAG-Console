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

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
SuaApplicationsCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" ) {

      if ( string_tokenizer.hasNextToken() &&
           utilx::toLowerCaseString(string_tokenizer.nextToken()) == "application" &&
           string_tokenizer.hasNextToken() ) {
        const std::string& appIdValue = string_tokenizer.nextToken();
        if ( string_tokenizer.hasNextToken() )
          throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");

        parseResult.command = new lm_commands::LM_Applications_AddApplicationCommand(appIdValue);
      } else
        throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( tokenValue == "remove" ) {
      if ( string_tokenizer.hasNextToken() &&
           utilx::toLowerCaseString(string_tokenizer.nextToken()) == "application" &&
           string_tokenizer.hasNextToken() ) {
        const std::string& appIdValue = string_tokenizer.nextToken();
        if ( string_tokenizer.hasNextToken() )
          throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");

        parseResult.command = new lm_commands::LM_Applications_RemoveApplicationCommand(appIdValue);
      } else
        throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");
    } else if ( tokenValue == "show" ) {
      if ( string_tokenizer.hasNextToken() )
        throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input");
      parseResult.command = new lm_commands::LM_Applications_ShowCommand();
    } else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_Applications_Commit();
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
  } else
    throw common::lm_subsystem::InvalidCommandLineException("SuaApplicationsCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

std::string
SuaApplicationsCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config sua_app)# ";
}

}}}}
