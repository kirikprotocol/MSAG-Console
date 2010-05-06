#include "eyeline/utilx/toLowerCaseString.hpp"

#include "SccpUsersCommandsInterpreter.hpp"
#include "lm_commands/LM_SccpUsers_AddUser.hpp"
#include "lm_commands/LM_SccpUsers_Show.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
SccpUsersCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" )
      parseResult.command = createAddUserCommand(string_tokenizer);
    else if ( tokenValue == "show" )
      parseResult.command = new lm_commands::LM_SccpUsers_Show();
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("SccpUsersCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());
  } else
    throw common::lm_subsystem::InvalidCommandLineException("SccpUsersCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

common::lm_subsystem::LM_Command*
SccpUsersCommandsInterpreter::createAddUserCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SccpUsersCommandsInterpreter::createAddUserCommand::: invalid input for 'add user' command");

  const std::string& tokenVal = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( tokenVal != "user" )
    throw common::lm_subsystem::InvalidCommandLineException("SccpUsersCommandsInterpreter::createAddUserCommand::: unknown command 'add %s'",
                                                            tokenVal.c_str());
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SccpUsersCommandsInterpreter::createAddUserCommand::: invalid input for 'add user' command - userId missed");

  const std::string& userId = string_tokenizer.nextToken();

  return new lm_commands::LM_SccpUsers_AddUser(userId, _trnMgr);
}

}}}}
