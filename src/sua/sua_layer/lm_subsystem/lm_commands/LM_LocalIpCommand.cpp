#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sua/sua_layer/lm_subsystem/Exception.hpp>
#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

#include "LM_LocalIpCommand.hpp"

namespace lm_commands {

LM_LocalIpCommand::LM_LocalIpCommand(const std::string& localHostValue)
{
  sockaddr_in addr;
  if ( inet_pton(AF_INET, localHostValue.c_str(), &addr.sin_addr) < 1 ) {
    struct hostent *hp;

#ifdef _REENTRANT
    struct hostent h_result;
    char addr_list_buffer[1024];
    int h_errno_buf;

    if ( !(hp = gethostbyname_r(localHostValue.c_str(), &h_result, addr_list_buffer, sizeof(addr_list_buffer), &h_errno_buf)) )
      throw lm_subsystem::InvalidCommandLineException("LM_LocalIpCommand::LM_LocalIpCommand::: wrong localHostValue=[%s]", localHostValue.c_str());
#else
    if ( !(hp = gethostbyname(localHostValue.c_str())) )
      throw lm_subsystem::InvalidCommandLineException("LM_LocalIpCommand::LM_LocalIpCommand::: wrong localHostValue=[%s]", localHostValue.c_str());
#endif
  }
  _localHost = localHostValue;
}

std::string
LM_LocalIpCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_LocalIpCommand::executeCommand::: _localHost=[%s]", _localHost.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(runtime_cfg::CompositeParameter("config"),
                                                                       runtime_cfg::Parameter("local_ip", _localHost));
  return "OK";
}

}
