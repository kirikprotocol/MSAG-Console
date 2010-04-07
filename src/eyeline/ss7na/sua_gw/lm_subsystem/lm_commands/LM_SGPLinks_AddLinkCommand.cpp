#include <sstream>
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "LM_SGPLinks_AddLinkCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_SGPLinks_AddLinkCommand::LM_SGPLinks_AddLinkCommand(const std::string& linkId,
                                                       const std::vector<std::string>& localAddressList,
                                                       in_port_t localPort,
                                                       const std::vector<std::string>& remoteAddressList,
                                                       in_port_t remotePort)
  : _linkId(linkId),
    _localAddressList(localAddressList), _localPort(localPort),
    _remoteAddressList(remoteAddressList), _remotePort(remotePort)
{}

std::string
LM_SGPLinks_AddLinkCommand::executeCommand()
{
  std::ostringstream oBuf;

  oBuf << "linkId=[" << _linkId << "],";

  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sgp_links"),
                                                                    new utilx::runtime_cfg::CompositeParameter("link", _linkId));

  for(std::vector<std::string>::const_iterator iter = _localAddressList.begin(), end_iter = _localAddressList.end();
      iter != end_iter; ++iter) {
    oBuf << "local_address=[" << *iter << "],";
    runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sgp_links.link", _linkId),
                                                                             new utilx::runtime_cfg::Parameter("local_address", *iter));
  }

  oBuf << "local_port=[" << _localPort << "],";
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sgp_links.link", _linkId), new utilx::runtime_cfg::Parameter("local_port", _localPort));

  for(std::vector<std::string>::const_iterator iter = _remoteAddressList.begin(), end_iter = _remoteAddressList.end();
      iter != end_iter; ++iter) {
    oBuf << "remote_address=[" << *iter << "],";
    runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sgp_links.link", _linkId), new utilx::runtime_cfg::Parameter("remote_address", *iter));
  }
  oBuf << "remote_port=[" << _remotePort << "]";
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sgp_links.link", _linkId), new utilx::runtime_cfg::Parameter("remote_port", _localPort));

  smsc_log_debug(_logger, "called LM_SGPLinks_AddLinkCommand::executeCommand::: %s", oBuf.str().c_str());

  return "OK";
}

}}}}}
