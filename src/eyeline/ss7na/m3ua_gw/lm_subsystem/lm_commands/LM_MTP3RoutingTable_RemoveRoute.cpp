#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "LM_MTP3RoutingTable_RemoveRoute.hpp"
#include "LM_MTP3RoutingTable_AddRoute.hpp"
#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/Router.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/AdjacentDPCRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_MTP3RoutingTable_RemoveRoute::executeCommand()
{
  try {
    _trnMgr.addOperation(this, LM_MTP3RoutingTable_AddRoute::composeCommandId(_routeName));
    smsc_log_debug(_logger, "LM_MTP3RoutingTable_RemoveRoute::executeCommand::: _routeName=%s",
                   _routeName.c_str());
  } catch (...) {
    delete this;
    throw;
  }
  return "OK";
}

void
LM_MTP3RoutingTable_RemoveRoute::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& mtp3RTables = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.mtp3-routing-tables");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> iterator =
      mtp3RTables.getIterator<utilx::runtime_cfg::CompositeParameter>("table");
  while ( iterator.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* tableParam = iterator.getCurrentElement();
    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> entryIter =
        tableParam->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
    while ( entryIter.hasElement() ) {
      utilx::runtime_cfg::CompositeParameter* entryParam = entryIter.getCurrentElement();
      if ( entryParam->getValue() == _routeName ) {
        utilx::runtime_cfg::Parameter* lpcParam =
            tableParam->getParameter<utilx::runtime_cfg::Parameter>("lpc");
        if ( !lpcParam )
          throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_RemoveRoute::updateConfiguration::: there isn't parameter 'config.mtp3-routing-tables.table.lpc'",
                                                                  _routeName.c_str());
        utilx::runtime_cfg::Parameter* dpc =
            entryParam->getParameter<utilx::runtime_cfg::Parameter>("dpc");
        if ( checkNoTranslationTableRef(static_cast<common::point_code_t>(dpc->getIntValue())) ) {
          removeRoute(entryParam, lpcParam->getIntValue());
          entryIter.removeElement();
        }
        return;
      }
      entryIter.next();
    }
    iterator.next();
  }
  RootLayerCommandsInterpreter::configurationWasModified = true;
}

bool
LM_MTP3RoutingTable_RemoveRoute::checkNoTranslationTableRef(common::point_code_t dpc)
{
  utilx::runtime_cfg::CompositeParameter& translationRtable =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.translation-table");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    entryIterator = translationRtable.getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
  while ( entryIterator.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* entry = entryIterator.getCurrentElement();
    utilx::runtime_cfg::CompositeParameter* dpcs =
        entry->getParameter<utilx::runtime_cfg::CompositeParameter>("dpcs");
    if ( dpcs ) {
      utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
      pcIterator = dpcs->getIterator<utilx::runtime_cfg::Parameter>("pc");
      while ( pcIterator.hasElement() ) {
        if ( pcIterator.getCurrentElement()->getIntValue() == dpc )
          return false;
        pcIterator.next();
      }
    }
    entryIterator.next();
  }
  return true;
}

void
LM_MTP3RoutingTable_RemoveRoute::removeRoute(utilx::runtime_cfg::CompositeParameter* entry_param,
                                             common::point_code_t lpc)
{
  utilx::runtime_cfg::Parameter* sgpLinkParam =
      entry_param->getParameter<utilx::runtime_cfg::Parameter>("sgp_link");
  if ( !sgpLinkParam )
    throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_RemoveRoute::removeRoute::: there isn't parameter 'sgp_link' for entry='%s'",
                                                            _routeName.c_str());
  common::LinkId linkId(sgpLinkParam->getValue());

  mtp3::AdjacentDPCRegistry::getInstance().remove(linkId);

  mtp3::msu_processor::RoutingTable* routingTable =
      mtp3::msu_processor::Router::getInstance().getRoutingTable(lpc);

  if ( !routingTable )
    throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_RemoveRoute::removeRoute::: there isn't routingTable for lpc='%u'",
                                                            lpc);
  utilx::runtime_cfg::Parameter* dpcParam =
      entry_param->getParameter<utilx::runtime_cfg::Parameter>("dpc");
  if ( !dpcParam )
    throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_RemoveRoute::removeRoute::: there isn't parameter 'dpc' for entry='%s'",
                                                            _routeName.c_str());
  routingTable->removeRoute(dpcParam->getIntValue());
}

std::string
LM_MTP3RoutingTable_RemoveRoute::composeCommandId(const std::string& route_name)
{
  return "mtp3routing_removeRoute_" + route_name;
}

}}}}}
