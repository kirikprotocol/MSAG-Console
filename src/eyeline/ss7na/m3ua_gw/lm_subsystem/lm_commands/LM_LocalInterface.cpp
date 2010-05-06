#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "util/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/SccpSubsystem.hpp"
#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
#include "eyeline/utilx/strtol.hpp"

#include "LM_LocalInterface.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_LocalInterface::LM_LocalInterface(const std::string& local_host,
                                     const std::string& local_port,
                                     common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
: _trnMgr(trn_mgr)
{
  setId(composeCommandId());
  sockaddr_in addr;
  if ( inet_pton(AF_INET, local_host.c_str(), &addr.sin_addr) < 1 ) {
    struct hostent *hp;

#ifdef _REENTRANT
    struct hostent h_result;
    char addr_list_buffer[1024];
    int h_errno_buf;

    if ( !(hp = gethostbyname_r(local_host.c_str(), &h_result, addr_list_buffer, static_cast<int>(sizeof(addr_list_buffer)), &h_errno_buf)) )
      throw common::lm_subsystem::InvalidCommandLineException("LM_LocalInterface::LM_LocalInterface::: wrong localHostValue=[%s]", local_host.c_str());
#else
    if ( !(hp = gethostbyname(local_host.c_str())) )
      throw common::lm_subsystem::InvalidCommandLineException("LM_LocalInterface::LM_LocalInterface::: wrong localHostValue=[%s]", local_host.c_str());
#endif
  }
  _localHost = local_host;

  _localPort = static_cast<unsigned int>(utilx::strtol(local_port.c_str(), (char **)NULL, 10));
  if ( _localPort == 0 && errno )
    throw common::lm_subsystem::InvalidCommandLineException("LM_LocalInterface::LM_LocalInterface::: invalid localPortValue value format [%s]",
                                                            local_port.c_str());
}

std::string
LM_LocalInterface::executeCommand()
{
  _trnMgr.addOperation(this);
  smsc_log_debug(_logger, "LM_LocalInterface::executeCommand::: _localHost=[%s], _localPort=%u",
                 _localHost.c_str(), _localPort);
  return "OK";
}

void
LM_LocalInterface::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

  utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("local_address");
  if ( !foundParam )
    throw smsc::util::Exception("LM_LocalInterface::updateConfiguration::: parameter 'config.local_address' is not found in the current configuration");

  foundParam->setValue(_localHost);

  foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("local_port");
  if ( !foundParam )
    throw smsc::util::Exception("LM_LocalInterface::updateConfiguration::: parameter 'config.local_port' is not found in the current configuration");

  foundParam->setIntValue(_localPort);
  RootLayerCommandsInterpreter::configurationWasModified = true;

  sccp::SccpSubsystem::getInstance()->reinitConnectAcceptor();
}

std::string
LM_LocalInterface::composeCommandId()
{
  return "localInterface";
}

}}}}}
