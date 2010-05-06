#include "LM_SGPLinks_RemoveLink.hpp"
#include "LM_SGPLinks_AddLink.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/SGPLinkIdsRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/InactiveMessage.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "eyeline/utilx/runtime_cfg/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SGPLinks_RemoveLink::executeCommand()
{
  utilx::runtime_cfg::CompositeParameter& mtp3Rtable =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.mtp3-routing-tables");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    tableIterator = mtp3Rtable.getIterator<utilx::runtime_cfg::CompositeParameter>("table");
  while ( tableIterator.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* table = tableIterator.getCurrentElement();
    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
        entryIterator = table->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
    while ( entryIterator.hasElement() ) {
      const std::string linkId =
          entryIterator.getCurrentElement()->getParameter<utilx::runtime_cfg::Parameter>("sgp_link")->getValue();
      if ( linkId == _linkId )
        throw utilx::runtime_cfg::InconsistentConfigCommandException("invalid input - there is entry in mtp3 routing table containing this link");
      entryIterator.next();
    }
    tableIterator.next();
  }

  _trnMgr.addOperation(this, LM_SGPLinks_AddLink::composeCommandId(_linkId));
  smsc_log_debug(_logger, "LM_SGPLinks_RemoveLink::executeCommand::: linkId=[%s]", _linkId.c_str());
  return "OK";
}

void
LM_SGPLinks_RemoveLink::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& sgpLinks = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> iterator =
      sgpLinks.getIterator<utilx::runtime_cfg::CompositeParameter>("link");

  smsc_log_debug(_logger, "LM_SGPLinks_RemoveLink::updateConfiguration::: linkId=[%s]", _linkId.c_str());
  bool linkWasFound = false;
  while ( iterator.hasElement() ) {
    const utilx::runtime_cfg::CompositeParameter* parameter = iterator.getCurrentElement();
    if ( parameter->getValue() == _linkId ) {
      iterator.removeElement();
      linkWasFound = true; break;
    }
    iterator.next();
  }

  if ( !linkWasFound )
    return;

  smsc_log_info(_logger, "LM_SGPLinks_RemoveLink::updateConfiguration::: disable link with linkId=[%s]", _linkId.c_str());
  mtp3::SGPLinkIdsRegistry::getInstance().remove(common::LinkId(_linkId));
  io_dispatcher::ConnectMgr::getInstance().send(common::LinkId(_linkId),
                                                mtp3::m3ua_stack::messages::InactiveMessage());

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

std::string
LM_SGPLinks_RemoveLink::composeCommandId(const std::string& link_id)
{
  return "sgpLinks_removeLink" + link_id;
}

}}}}}
