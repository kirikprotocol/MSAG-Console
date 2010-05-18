#include "LM_MTP3RoutingTable_AddRoute.hpp"
#include "LM_MTP3RoutingTable_RemoveRoute.hpp"
#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/Router.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/AdjacentDPCRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/RoutingTable.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_MTP3RoutingTable_AddRoute::executeCommand()
{
  try {
    _trnMgr.addOperation(this, LM_MTP3RoutingTable_RemoveRoute::composeCommandId(_routeName));
    smsc_log_debug(_logger, "LM_MTP3RoutingTable_AddRoute::executeCommand::: _routeName=%s, _sgpLinkId=%s, _lpc=%u, _dpc=%u",
                   _routeName.c_str(), _sgpLinkId.c_str(), _lpc, _dpc);
  } catch (...) {
    delete this;
    throw;
  }
  return "OK";
}

void
LM_MTP3RoutingTable_AddRoute::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& mtp3RTables = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.mtp3-routing-tables");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> iterator =
      mtp3RTables.getIterator<utilx::runtime_cfg::CompositeParameter>("table");

  bool lpcDoesntExist = true;
  while ( iterator.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* tableParameter = iterator.getCurrentElement();
    utilx::runtime_cfg::Parameter* lpcParam =
        tableParameter->getParameter<utilx::runtime_cfg::Parameter>("lpc");
    if ( !lpcParam )
      throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_AddRoute::updateConfiguration::: 'lpc' parameter not found");

    if ( lpcParam->getIntValue() == _lpc ) {
      lpcDoesntExist = false;
      if ( !checkAndAddEntry(tableParameter) )
        return;
      else
        break;
    }
    iterator.next();
  }
  if ( lpcDoesntExist ) {
    mtp3::msu_processor::RoutingTable* routingTable = new mtp3::msu_processor::RoutingTable();
    routingTable->addRoute(_dpc, common::LinkId(_sgpLinkId));
    mtp3::msu_processor::Router::getInstance().addRoutingTable(_lpc, routingTable);
  } else {
    mtp3::msu_processor::RoutingTableRefPtr routingTable =
        mtp3::msu_processor::Router::getInstance().getRoutingTable(_lpc);
    if ( !routingTable.Get() )
      throw common::lm_subsystem::InvalidCommandLineException("LM_MTP3RoutingTable_AddRoute::updateConfiguration::: there isn't routingTable for lpc=%u",
                                                              _lpc);
    routingTable->addRoute(_dpc, common::LinkId(_sgpLinkId));
  }
  mtp3::AdjacentDPCRegistry::getInstance().insert(_dpc, common::LinkId(_sgpLinkId));
  RootLayerCommandsInterpreter::configurationWasModified = true;
}

std::string
LM_MTP3RoutingTable_AddRoute::composeCommandId(const std::string& route_name)
{
  return "mtp3routing_addRoute_" + route_name;
}

bool
LM_MTP3RoutingTable_AddRoute::checkAndAddEntry(utilx::runtime_cfg::CompositeParameter* table_param)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> iterator =
      table_param->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
  while ( iterator.hasElement() ) {
    utilx::runtime_cfg::CompositeParameter* entryParameter = iterator.getCurrentElement();
    if ( entryParameter->getValue() == _routeName )
      return false;
  }
  utilx::runtime_cfg::CompositeParameter* entryParam =
      table_param->addParameter(new utilx::runtime_cfg::CompositeParameter("entry", _routeName));
  entryParam->addParameter(new utilx::runtime_cfg::Parameter("dpc", _dpc));
  entryParam->addParameter(new utilx::runtime_cfg::Parameter("sgp_link", _sgpLinkId));

  return true;
}

}}}}}
