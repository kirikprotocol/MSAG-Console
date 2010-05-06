#include <stdlib.h>

#include "eyeline/utilx/toLowerCaseString.hpp"
#include "SgpLinksCommandsInterpreter.hpp"
#include "lm_commands/LM_M3ua_Commit.hpp"
#include "lm_commands/LM_SGPLinks_AddLink.hpp"
#include "lm_commands/LM_SGPLinks_RemoveLink.hpp"
#include "lm_commands/LM_SGPLinks_Show.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

common::lm_subsystem::LM_CommandsInterpreter::interpretation_result
SgpLinksCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& string_tokenizer)
{
  interpretation_result parseResult(NULL,
                                    common::lm_subsystem::lm_commands_interpreter_refptr_t(NULL),
                                    false);

  if ( string_tokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(string_tokenizer.nextToken());
    if ( tokenValue == "add" )
      parseResult.command = create_addLinkCommand(string_tokenizer);
    else if ( tokenValue == "remove" )
      parseResult.command = create_removeLinkCommand(string_tokenizer);
    else if ( tokenValue == "show" )
      parseResult.command = new lm_commands::LM_SGPLinks_Show();
    else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_M3ua_Commit(_trnMgr);
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());

    return parseResult;
  } else
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

common::lm_subsystem::LM_Command*
SgpLinksCommandsInterpreter::create_addLinkCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input for 'add link' command");

  const std::string& tokenVal = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( tokenVal != "link" )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: unknown command 'add %s'",
                                                            tokenVal.c_str());

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input for 'add link' command - linkId missed");

  const std::string& linkId = string_tokenizer.nextToken();

  if ( !string_tokenizer.hasNextToken() ||
       utilx::toLowerCaseString(string_tokenizer.nextToken()) != "local" )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input for 'add link' command - missed 'local' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input for 'add link' command - missed value for 'local' keyword");

  std::vector<std::string> localAddressList;
  in_port_t localPort;
  parseAddressListParameter(string_tokenizer.nextToken(), &localAddressList, &localPort);

  if ( !string_tokenizer.hasNextToken() ||
       utilx::toLowerCaseString(string_tokenizer.nextToken()) != "remote" )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input for 'add link' command - missed 'remote' keyword");

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input for 'add link' command - missed value for 'remote' keyword");

  std::vector<std::string> remoteAddressList;
  in_port_t remotePort;
  parseAddressListParameter(string_tokenizer.nextToken(), &remoteAddressList, &remotePort);

  return new lm_commands::LM_SGPLinks_AddLink(linkId, localAddressList, localPort,
                                              remoteAddressList, remotePort, _trnMgr);
}

void
SgpLinksCommandsInterpreter::parseAddressListParameter(const std::string& addr_param_value,
                                                       std::vector<std::string>* addr_list,
                                                       in_port_t* port)
{
  std::string::size_type idx = addr_param_value.find(':');
  if ( idx == std::string::npos )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::parseAddressListParameter::: invalid input");

  addr_list->push_back(trimSpaces(addr_param_value.substr(0, idx)));
  std::string::size_type new_idx = addr_param_value.find(',', idx + 1);
  if ( new_idx == std::string::npos )
    *port = atoi(trimSpaces(addr_param_value.substr(idx+1)).c_str());
  else {
    *port = atoi(trimSpaces(addr_param_value.substr(idx+1, new_idx - idx -1)).c_str());
    idx = new_idx;
    new_idx = addr_param_value.find(',', idx + 1);
    while ( new_idx != std::string::npos ) {
      addr_list->push_back(trimSpaces(addr_param_value.substr(idx+1, new_idx - idx - 1)));
      idx = new_idx;
      new_idx = addr_param_value.find(',', idx + 1);
    }
    addr_list->push_back(trimSpaces(addr_param_value.substr(idx + 1)));
  }
}

common::lm_subsystem::LM_Command*
SgpLinksCommandsInterpreter::create_removeLinkCommand(utilx::StringTokenizer& string_tokenizer)
{
  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_removeLinkCommand::: invalid input for 'remove link' command");

  const std::string& tokenVal = utilx::toLowerCaseString(string_tokenizer.nextToken());
  if ( tokenVal != "link" )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_removeLinkCommand::: unknown command 'remove %s'",
                                                            tokenVal.c_str());

  if ( !string_tokenizer.hasNextToken() )
    throw common::lm_subsystem::InvalidCommandLineException("SgpLinksCommandsInterpreter::create_removeLinkCommand::: invalid input for 'remove link' command - link_id missed");

  const std::string& linkId = string_tokenizer.nextToken();

  return new lm_commands::LM_SGPLinks_RemoveLink(linkId, _trnMgr);
}

std::string
SgpLinksCommandsInterpreter::trimSpaces(const std::string& value)
{
  std::string::size_type i = value.find_first_not_of(0x20);
  std::string::size_type j = value.find_first_of(0x20, i + 1);
  if ( j == std::string::npos )
    return value.substr(i);
  else
    return value.substr(i, j - i);
}

}}}}
