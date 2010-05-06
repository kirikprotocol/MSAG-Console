#include <stdlib.h>

#include "eyeline/utilx/toLowerCaseString.hpp"
#include "SgpLinksCommandsInterpreter.hpp"
#include "lm_commands/LM_Applications_RemoveApplicationCommand.hpp"
#include "lm_commands/LM_SGPLinks_Commit.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
SgpLinksCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" ) {
      parseResult.command = create_addLinkCommand(string_tokenizer);
    } else if ( tokenValue == "remove" ) {
      if ( string_tokenizer.hasNextToken() &&
           utilx::toLowerCaseString(string_tokenizer.nextToken()) == "link" &&
           string_tokenizer.hasNextToken() ) {
        const std::string& linkIdValue = string_tokenizer.nextToken();
        if ( string_tokenizer.hasNextToken() )
          throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: invalid input");

        parseResult.command = new lm_commands::LM_Applications_RemoveApplicationCommand(linkIdValue);
      }
    } else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_SGPLinks_Commit();
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());

    return parseResult;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

std::string
SgpLinksCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config sgp-links)# ";
}


lm_commands::LM_SGPLinks_AddLinkCommand*
SgpLinksCommandsInterpreter::create_addLinkCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( string_tokenizer.hasNextToken() ) {
    string_tokenizer.nextToken(); // skip "link" keyword
    if ( !string_tokenizer.hasNextToken() )
      throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input");

    const std::string& linkId = string_tokenizer.nextToken();

    if ( !string_tokenizer.hasNextToken() ||
         utilx::toLowerCaseString(string_tokenizer.nextToken()) != "local" ||
         !string_tokenizer.hasNextToken() )
      throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input");

    std::vector<std::string> localAddressList, remoteAddressList;
    in_port_t localPort, remotePort;
    parseAddressListParameter(string_tokenizer.nextToken(), &localAddressList, &localPort);

    if ( !string_tokenizer.hasNextToken() ||
         utilx::toLowerCaseString(string_tokenizer.nextToken()) != "remote" ||
         !string_tokenizer.hasNextToken())
      throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input");

    parseAddressListParameter(string_tokenizer.nextToken(), &remoteAddressList, &remotePort);

    return new lm_commands::LM_SGPLinks_AddLinkCommand(linkId, localAddressList, localPort, remoteAddressList, remotePort);
  } else
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: empty input");
}

void
SgpLinksCommandsInterpreter::parseAddressListParameter(const std::string& addr_param_value,
                                                       std::vector<std::string>* addr_list,
                                                       in_port_t* port)
{
  std::string::size_type idx = addr_param_value.find(':');
  if ( idx == std::string::npos )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::parseAddressListParameter::: invalid input");

  addr_list->push_back(addr_param_value.substr(0, idx));
  std::string::size_type new_idx = addr_param_value.find(',', idx + 1);
  if ( new_idx == std::string::npos )
    *port = atoi(addr_param_value.substr(idx+1).c_str());
  else {
    *port = atoi(addr_param_value.substr(idx+1, new_idx - idx -1).c_str());
    idx = new_idx;
    new_idx = addr_param_value.find(',', idx + 1);
    while ( new_idx != std::string::npos ) {
      addr_list->push_back(addr_param_value.substr(idx+1, new_idx - idx - 1));
      idx = new_idx;
      new_idx = addr_param_value.find(',', idx + 1);
    }
    addr_list->push_back(addr_param_value.substr(idx + 1));
  }
}

}}}}
