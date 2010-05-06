#include <sstream>

#include "eyeline/ss7na/m3ua_gw/mtp3/MTP3Subsystem.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "LM_SGPLinks_AddLink.hpp"
#include "LM_SGPLinks_RemoveLink.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_SGPLinks_AddLink::LM_SGPLinks_AddLink(const std::string& link_id,
                                         const std::vector<std::string>& local_addr_list,
                                         in_port_t local_port,
                                         const std::vector<std::string>& remote_addr_list,
                                         in_port_t remote_port,
                                         common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _linkId(link_id),
    _localAddressList(local_addr_list), _localPort(local_port),
    _remoteAddressList(remote_addr_list), _remotePort(remote_port)
{
  setId(composeCommandId(_linkId));
}

std::string
LM_SGPLinks_AddLink::composeCommandId(const std::string& link_id)
{
  return "sgpLinks_addLink" + link_id;
}

std::string
LM_SGPLinks_AddLink::executeCommand()
{
  _trnMgr.addOperation(this, LM_SGPLinks_RemoveLink::composeCommandId(_linkId));
  return "OK";
}

void
LM_SGPLinks_AddLink::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& sgpLinks = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> iterator =
      sgpLinks.getIterator<utilx::runtime_cfg::CompositeParameter>("link");
  while ( iterator.hasElement() ) {
    const utilx::runtime_cfg::CompositeParameter* parameter = iterator.getCurrentElement();
    if ( parameter->getValue() == _linkId )
      return;
    iterator.next();
  }
  utilx::runtime_cfg::CompositeParameter* linkParam =
      sgpLinks.addParameter(new utilx::runtime_cfg::CompositeParameter("link", _linkId));

  setAddresses(linkParam, "remote_addresses", "address", _remoteAddressList);
  setAddresses(linkParam, "local_addresses", "address", _localAddressList);
  linkParam->addParameter(new utilx::runtime_cfg::Parameter("remote_port", _remotePort));
  linkParam->addParameter(new utilx::runtime_cfg::Parameter("local_port", _localPort));

  mtp3::MTP3Subsystem::getInstance()->activateLinkToSGP(common::LinkId(_linkId),
                                                        _remoteAddressList, _remotePort,
                                                        _localAddressList, _localPort);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

void
LM_SGPLinks_AddLink::setAddresses(utilx::runtime_cfg::CompositeParameter* link_param,
                                  const std::string& composite_param_name,
                                  const std::string& param_name,
                                  const std::vector<std::string>& values)
{
  if ( values.empty() )
    return;
  utilx::runtime_cfg::CompositeParameter* addresses =
      link_param->addParameter(new utilx::runtime_cfg::CompositeParameter(composite_param_name));
  for ( std::vector<std::string>::const_iterator iter = values.begin(), end_iter = values.end();
      iter != end_iter; ++iter )
    addresses->addParameter(new utilx::runtime_cfg::Parameter(param_name, *iter));
}

}}}}}
