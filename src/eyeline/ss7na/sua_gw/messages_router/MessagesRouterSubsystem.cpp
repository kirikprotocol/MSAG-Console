#include <stdio.h>
#include <stdlib.h>
#include <utility>

#include "util/Exception.hpp"
#include "util/vformat.hpp"

#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/utilx/runtime_cfg/Exception.hpp"
#include "eyeline/utilx/strtol.hpp"

#include "eyeline/ss7na/common/LinkId.hpp"
#include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/common/sig/LinkSetsRegistry.hpp"

#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/sccp_sap/LinkSetInfoRegistry.hpp"

#include "MessagesRouterSubsystem.hpp"
#include "GTTranslator.hpp"
#include "GTTranslationTablesRegistry.hpp"
#include "GTTranslationTable.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

MessagesRouterSubsystem::MessagesRouterSubsystem()
  : common::ApplicationSubsystem("MessagesRouterSubsystem", "msg_rout")
{}

common::io_dispatcher::LinkSet::linkset_mode_t
MessagesRouterSubsystem::convertStringToTrafficModeValue(const std::string& traffic_mode,
                                                         const std::string& where)
{
  if ( traffic_mode == "loadshare" )
    return common::io_dispatcher::LinkSet::LOADSHARE;
  else if ( traffic_mode == "broadcast" )
    return common::io_dispatcher::LinkSet::BROADCAST;
  else if ( traffic_mode == "override" )
    return common::io_dispatcher::LinkSet::OVERRIDE;
  else {
    const std::string fmtStr = where + "convertStringToTrafficModeValue::: wrong traffic-mode parameter value=[%s]";
    throw smsc::util::Exception(fmtStr.c_str(), traffic_mode.c_str());
  }
}

bool
MessagesRouterSubsystem::processApplicationLinkSets(utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter,
                                                    const common::LinkId& linkset_id,
                                                    common::io_dispatcher::LinkSet::linkset_mode_t linkset_mode)
{
  bool result = false;

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> appIdIterator =
      routing_entry_composite_parameter->getIterator<utilx::runtime_cfg::Parameter>("application");

  if ( appIdIterator.hasElement() ) {
    result = true;
    io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkset_id, linkset_mode);
  }

  while (appIdIterator.hasElement()) {
    const std::string& appId = appIdIterator.getCurrentElement()->getValue();
    smsc_log_info(_logger, "MessagesRouterSubsystem::processApplicationLinkSets::: add association for application=[%s] to linkset=[%s]", appId.c_str(), linkset_id.getValue().c_str());
    sccp_sap::LinkSetInfoRegistry::getInstance().addAssociation(linkset_id, appId);
    appIdIterator.next();
  }

  return result;
}

bool
MessagesRouterSubsystem::processSgpLinkSets(utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter,
                                            const common::LinkId& linkset_id,
                                            common::io_dispatcher::LinkSet::linkset_mode_t linkset_mode)
{
  bool result = false;

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> sgpLinksIterator =
      routing_entry_composite_parameter->getIterator<utilx::runtime_cfg::Parameter>("sgp_link");

  if ( sgpLinksIterator.hasElement() ) {
    result = true;
    io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkset_id, linkset_mode);
    common::sig::LinkSetsRegistry::getInstance().registerLinkSet(linkset_id);
  }

  while (sgpLinksIterator.hasElement()) {
    common::LinkId sgpLinkId(sgpLinksIterator.getCurrentElement()->getValue());
    smsc_log_info(_logger, "MessagesRouterSubsystem::processSgpLinkSets::: add link=[%s] to linkset=[%s]", sgpLinkId.getValue().c_str(), linkset_id.getValue().c_str());
    common::sig::LinkSetsRegistry::getInstance().registerLinkInLinkSet(linkset_id, sgpLinkId);
    sgpLinksIterator.next();
  }

  return result;
}

std::string
MessagesRouterSubsystem::makeAddressFamilyPrefix(unsigned int gti,
                                                 const std::string& gt_mask_value) const
{
  std::string gtAddressFamilyPrefix;

  if ( gt_mask_value[0] == '+' && gti == 4 )
    gtAddressFamilyPrefix = ".4.0.1.4";
  else if ( gt_mask_value[0] >= 0x30 && gt_mask_value[0] <= 0x39 && gti == 1 )
    gtAddressFamilyPrefix = ".4.0.1.0";
  else {
    std::string::size_type idx = gt_mask_value.rfind('.');
    if ( idx != std::string::npos )
      gtAddressFamilyPrefix = gt_mask_value.substr(0, idx);
    else
      throw smsc::util::Exception("MessagesRouterSubsystem::makeAddressFamilyPrefix::: can't determine address family prefix for gtMaskValue=[%s]", gt_mask_value.c_str());
  }

  return gtAddressFamilyPrefix;
}

void
MessagesRouterSubsystem::addRouteEntry(utilx::runtime_cfg::CompositeParameter* route_description_composite_parameter,
                                       const common::LinkId& linkset_id)
{
  utilx::runtime_cfg::Parameter* gtiConfigParameter = route_description_composite_parameter->getParameter<utilx::runtime_cfg::Parameter>("gti");
  unsigned int gti = 4;
  if ( gtiConfigParameter )
    gti = atoi(gtiConfigParameter->getValue().c_str());

  std::string gtMaskValue = route_description_composite_parameter->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue();

  const std::string& gtAddressFamilyPrefix = makeAddressFamilyPrefix(gti, gtMaskValue);

  uint8_t destinationSSN = 0;
  const utilx::runtime_cfg::Parameter* ssnConfigParam = route_description_composite_parameter->getParameter<utilx::runtime_cfg::Parameter>("ssn");
  if ( ssnConfigParam ) {
    const std::string& ssnValue = ssnConfigParam->getValue();

    destinationSSN = static_cast<uint8_t>(utilx::strtol(ssnValue.c_str(), (char **)NULL, 10));
    if ( destinationSSN == 0 && errno )
      throw smsc::util::Exception("MessagesRouterSubsystem::addRouteEntry::: wrong SSN value format [%s]", ssnValue.c_str());
  }

  smsc_log_debug(_logger, "MessagesRouterSubsystem::addRouteEntry::: gtMask=[%s],ssn=[%d]", gtMaskValue.c_str(), destinationSSN);

  GTTranslationTable* translationTable = GTTranslationTablesRegistry::getInstance().getGTTranslationTable(gtAddressFamilyPrefix);
  if ( !translationTable )
    throw smsc::util::Exception("MessagesRouterSubsystem::addRouteEntry::: translation table not found for address family=[%s]", gtAddressFamilyPrefix.c_str());

  translationTable->addTranslationEntry(gtMaskValue, destinationSSN, linkset_id);
}

void
MessagesRouterSubsystem::fillUpRouteTable(utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter,
                                          const common::LinkId& linkset_id)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> routeInfoIterator = routing_entry_composite_parameter->getIterator<utilx::runtime_cfg::CompositeParameter>("route");
  if ( !routeInfoIterator.hasElement() ) { // route to CLOUD - process default route
    GTTranslationTable::addDefaultTranslationEntry(linkset_id);
  } else
    while(routeInfoIterator.hasElement()) {
      addRouteEntry(routeInfoIterator.getCurrentElement(), linkset_id);
      routeInfoIterator.next();
    }
}

void
MessagesRouterSubsystem::initializeLinkSets(utilx::runtime_cfg::CompositeParameter& routing_keys_parameter)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> routingEntryIterator = routing_keys_parameter.getIterator<utilx::runtime_cfg::CompositeParameter>("routingEntry");

  while(routingEntryIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter = routingEntryIterator.getCurrentElement();
    common::io_dispatcher::LinkSet::linkset_mode_t linksetMode = convertStringToTrafficModeValue(routingEntryCompositeParameter->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode")->getValue(), "MessagesRouterSubsystem::initializeIncomingLinkSets: ");
    
    common::LinkId linksetId(routingEntryCompositeParameter->getValue());

    smsc_log_debug(_logger, "MessagesRouterSubsystem::initializeLinkSets::: linkSetId=[%s]", linksetId.getValue().c_str());

    fillUpRouteTable(routingEntryCompositeParameter,linksetId);

    if ( !processApplicationLinkSets(routingEntryCompositeParameter, linksetId, linksetMode) )
      processSgpLinkSets(routingEntryCompositeParameter, linksetId, linksetMode);

    routingEntryIterator.next();
  }
}

void
MessagesRouterSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "try initialize MessagesRouterSubsystem");

  GTTranslator::init();
  GTTranslationTablesRegistry::init();
  sccp_sap::LinkSetInfoRegistry::init();
  common::sig::LinkSetsRegistry::init();
  GTTranslationTablesRegistry::getInstance().registerGTTranslationTable(".4.0.1.4", new GTTranslationTable_Prefix_4_0_1_4());
  GTTranslationTablesRegistry::getInstance().registerGTTranslationTable(".4.0.1.0", new GTTranslationTable_Prefix_4_0_1_0());

  initializeLinkSets(rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.routing-keys"));
}

}}}}
