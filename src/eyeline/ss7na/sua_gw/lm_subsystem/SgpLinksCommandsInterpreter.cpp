#include <stdlib.h>

#include "eyeline/utilx/toLowerCaseString.hpp"
#include "SgpLinksCommandsInterpreter.hpp"
#include "lm_commands/LM_Applications_RemoveApplicationCommand.hpp"
#include "lm_commands/LM_SGPLinks_Commit.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

LM_CommandsInterpreter::interpretation_result
SgpLinksCommandsInterpreter::interpretCommandLine(utilx::StringTokenizer& stringTokenizer)
{
  interpretation_result parseResult(lm_commands_refptr_t(NULL), lm_commands_interpreter_refptr_t(NULL), false);

  if ( stringTokenizer.hasNextToken() ) {
    const std::string& tokenValue = utilx::toLowerCaseString(stringTokenizer.nextToken());
    if ( tokenValue == "add" ) {
      parseResult.command = create_addLinkCommand(stringTokenizer);
    } else if ( tokenValue == "remove" ) {
      if ( stringTokenizer.hasNextToken() &&
           utilx::toLowerCaseString(stringTokenizer.nextToken()) == "link" &&
           stringTokenizer.hasNextToken() ) {
        const std::string& linkIdValue = stringTokenizer.nextToken();
        if ( stringTokenizer.hasNextToken() )
          throw InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: invalid input");

        parseResult.command = new lm_commands::LM_Applications_RemoveApplicationCommand(linkIdValue);
      }
    } else if ( tokenValue == "exit" || tokenValue == "quit" ) {
      parseResult.command = new lm_commands::LM_SGPLinks_Commit();
      parseResult.popUpCurrentInterpreter = true;
    } else
      throw InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: invalid input=[%s]", tokenValue.c_str());

    return parseResult;
  } else
    throw InvalidCommandLineException("SgpLinksCommandsInterpreter::interpretCommandLine::: empty input");

  return parseResult;
}

std::string
SgpLinksCommandsInterpreter::getPromptString() const
{
  return "sibinco sua(config sgp-links)# ";
}


lm_commands::LM_SGPLinks_AddLinkCommand*
SgpLinksCommandsInterpreter::create_addLinkCommand(utilx::StringTokenizer& stringTokenizer)
{
  if ( stringTokenizer.hasNextToken() ) {
    stringTokenizer.nextToken(); // skip "link" keyword
    if ( !stringTokenizer.hasNextToken() )
      throw InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input");

    const std::string& linkId = stringTokenizer.nextToken();

    if ( !stringTokenizer.hasNextToken() ||
         utilx::toLowerCaseString(stringTokenizer.nextToken()) != "local" ||
         !stringTokenizer.hasNextToken() )
      throw InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input");

    std::vector<std::string> localAddressList, remoteAddressList;
    in_port_t localPort, remotePort;
    parseAddressListParameter(stringTokenizer.nextToken(), &localAddressList, &localPort);

    if ( !stringTokenizer.hasNextToken() ||
         utilx::toLowerCaseString(stringTokenizer.nextToken()) != "remote" ||
         !stringTokenizer.hasNextToken())
      throw InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: invalid input");

    parseAddressListParameter(stringTokenizer.nextToken(), &remoteAddressList, &remotePort);

    return new lm_commands::LM_SGPLinks_AddLinkCommand(linkId, localAddressList, localPort, remoteAddressList, remotePort);
  } else
    throw InvalidCommandLineException("SgpLinksCommandsInterpreter::create_addLinkCommand::: empty input");
}

void
SgpLinksCommandsInterpreter::parseAddressListParameter(const std::string& addressParamaterValue,
                                                       std::vector<std::string>* addressList,
                                                       in_port_t* port)
{
  std::string::size_type idx = addressParamaterValue.find(':');
  if ( idx == std::string::npos )
    throw InvalidCommandLineException("SgpLinksCommandsInterpreter::parseAddressListParameter::: invalid input");

  addressList->push_back(addressParamaterValue.substr(0, idx));
  std::string::size_type new_idx = addressParamaterValue.find(',', idx + 1);
  if ( new_idx == std::string::npos )
    *port = atoi(addressParamaterValue.substr(idx+1).c_str());
  else {
    *port = atoi(addressParamaterValue.substr(idx+1, new_idx - idx -1).c_str());
    idx = new_idx;
    new_idx = addressParamaterValue.find(',', idx + 1);
    while ( new_idx != std::string::npos ) {
      addressList->push_back(addressParamaterValue.substr(idx+1, new_idx - idx - 1));
      idx = new_idx;
      new_idx = addressParamaterValue.find(',', idx + 1);
    }
    addressList->push_back(addressParamaterValue.substr(idx + 1));
  }
}

}}}}
