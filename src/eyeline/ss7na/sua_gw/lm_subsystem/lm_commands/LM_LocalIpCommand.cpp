#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"

#include "LM_LocalIpCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_LocalIpCommand::LM_LocalIpCommand(const std::string& local_host,
                                     common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
{
  sockaddr_in addr;
  if ( inet_pton(AF_INET, local_host.c_str(), &addr.sin_addr) < 1 ) {
    struct hostent *hp;

#ifdef _REENTRANT
    struct hostent h_result;
    char addr_list_buffer[1024];
    int h_errno_buf;

    if ( !(hp = gethostbyname_r(local_host.c_str(), &h_result, addr_list_buffer, static_cast<int>(sizeof(addr_list_buffer)), &h_errno_buf)) )
      throw common::lm_subsystem::InvalidCommandLineException("LM_LocalIpCommand::LM_LocalIpCommand::: wrong localHostValue=[%s]", local_host.c_str());
#else
    if ( !(hp = gethostbyname(local_host.c_str())) )
      throw common::lm_subsystem::InvalidCommandLineException("LM_LocalIpCommand::LM_LocalIpCommand::: wrong localHostValue=[%s]", local_host.c_str());
#endif
  }
  _localHost = local_host;
}

std::string
LM_LocalIpCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_LocalIpCommand::executeCommand::: _localHost=[%s]", _localHost.c_str());
  return "OK";
}

}}}}}
