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

  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.route", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.route.gt", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.route.gti", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.route.ssn", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.application", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.sgp_link", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.routing-keys.routingEntry.traffic-mode", this);

}

void
MessagesRouterSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                  utilx::runtime_cfg::Parameter* added_parameter)
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  if ( context.getFullName() == "config.routing-keys.routingEntry" ) {
    utilx::runtime_cfg::CompositeParameter* routingEntryParam =
        findContextParentParameter(runtimeConfig, context);

    if ( !routingEntryParam ) return;

    smsc_log_debug(_logger, "MessagesRouterSubsystem::addParameterHandler::: handle added parameter '%s'='%s' for context '%s'='%s'", added_parameter->getName().c_str(), added_parameter->getValue().c_str(), context.getFullName().c_str(), context.getValue().c_str());

    if ( added_parameter->getName() != "application" &&
         added_parameter->getName() != "sgp_link" ) return;

    if ( !checkParameterExist(routingEntryParam, added_parameter) ) {
      if ( added_parameter->getName() == "application" ) {
        checkConsistentRuntimeCommand(routingEntryParam, "application", "sgp_link");

        if ( !checkParameterValueIsPresentInConfig<utilx::runtime_cfg::Parameter>("config.sua_applications", "application", added_parameter->getValue()) ) {
          std::string messageToUser("Inconsistent config modification request - application wasn't registered");
          throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - such application wasn't registered", added_parameter->getName().c_str(), added_parameter->getValue().c_str());
        }

      } else {
        checkConsistentRuntimeCommand(routingEntryParam, "sgp_link", "application");

        if ( !checkParameterValueIsPresentInConfig<utilx::runtime_cfg::CompositeParameter>("config.sgp_links", "link", added_parameter->getValue()) ) {
          std::string messageToUser("Inconsistent config modification request - link wasn't registered");
          throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - such link wasn't registered", added_parameter->getName().c_str(), added_parameter->getValue().c_str());
        }
      }
      routingEntryParam->addParameter(added_parameter);
    } else {
      std::string messageToUser("Inconsistent config modification request - the parameter with such value already exist");
      throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", added_parameter->getName().c_str(), added_parameter->getValue().c_str());
    }
  }
}

utilx::runtime_cfg::CompositeParameter*
MessagesRouterSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                  utilx::runtime_cfg::CompositeParameter* added_parameter)
{
  if ( context.getFullName() == "config.routing-keys" ) {
    // this condition is true when called LM_TranslationTable_AddTranslationRuleCommand::executeCommand()
    utilx::runtime_cfg::CompositeParameter& routingKeysParameter = 
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.routing-keys");

    if ( added_parameter->getName() != "routingEntry" )
      generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "MessagesRouterSubsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", added_parameter->getName().c_str(), context.getName().c_str());

    if ( checkParameterExist(&routingKeysParameter, added_parameter) )
      generateExceptionAndForcePopUpCurrentInterpreter("Inconsistent config modification request - translation-rule with such value already exists", "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", added_parameter->getName().c_str(), added_parameter->getValue().c_str());

    routingKeysParameter.addParameter(added_parameter);
    return added_parameter;
  } else if ( context.getFullName() == "config.routing-keys.routingEntry" ) {
    utilx::runtime_cfg::CompositeParameter* routingEntryParam =
        findContextParentParameter(runtime_cfg::RuntimeConfig::getInstance(), context);

    if ( added_parameter->getName() != "route" ) return NULL;

    if ( routingEntryParam ) {
      if ( !checkParameterExist(routingEntryParam, added_parameter) ) {
        routingEntryParam->addParameter(added_parameter);
        common::LinkId linkSetId(routingEntryParam->getValue());
        _uncommitedRoutesCache.addUncommitedRoute(linkSetId, added_parameter);
        return added_parameter;
      } else {
        std::string messageToUser("Inconsistent config modification request - such 'route' already exist");
        throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", added_parameter->getName().c_str(), added_parameter->getValue().c_str());
      }
    } else
      return NULL;
  } else
    generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "MessagesRouterSubsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", added_parameter->getName().c_str(), context.getName().c_str());
}

void
MessagesRouterSubsystem::addParameterEventHandler(utilx::runtime_cfg::CompositeParameter* context,
                                                  utilx::runtime_cfg::Parameter* added_parameter)
{
  if ( !checkParameterExist(context, added_parameter) ) {
    context->addParameter(added_parameter);
  } else {
    std::string messageToUser("Inconsistent config modification request - the parameter with such value already exist");
    throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", added_parameter->getName().c_str(), added_parameter->getValue().c_str());
  }
}

void
MessagesRouterSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                     const utilx::runtime_cfg::Parameter& modified_parameter)
{
  if ( context.getFullName() == "config.routing-keys.routingEntry" ) {
    utilx::runtime_cfg::CompositeParameter* routingEntryParam =
        findContextParentParameter(runtime_cfg::RuntimeConfig::getInstance(), context);
    if ( !routingEntryParam ) return;

    if ( modified_parameter.getName() == "commit" ) {
      utilx::runtime_cfg::Parameter* trafficModeParam = routingEntryParam->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
      common::io_dispatcher::LinkSet::linkset_mode_t linksetMode =
          common::io_dispatcher::LinkSet::OVERRIDE;
      if ( trafficModeParam )
        linksetMode = convertStringToTrafficModeValue(trafficModeParam->getValue(), "MessagesRouterSubsystem::changeParameterEventHandler: ");
      applyParametersChange(common::LinkId(context.getValue()), linksetMode, routingEntryParam);
    } else {
      if ( modified_parameter.getFullName() == "traffic-mode" ) {
        smsc_log_debug(_logger, "MessagesRouterSubsystem::handle::: handle modified parameter 'config.routing-keys.routingEntry.traffic-mode'=[%s] for routingEntry=[%s]", modified_parameter.getValue().c_str(), context.getValue().c_str());
        utilx::runtime_cfg::Parameter* trafficModeParam = routingEntryParam->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
        if ( trafficModeParam )
          trafficModeParam->setValue(modified_parameter.getValue());
        else
          routingEntryParam->addParameter(new utilx::runtime_cfg::Parameter("traffic-mode", modified_parameter.getValue()));
      }
    }
  }
}

void
MessagesRouterSubsystem::applyParametersChange(const common::LinkId& linkset_id,
                                               common::io_dispatcher::LinkSet::linkset_mode_t linkset_mode,
                                               utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter)
{
  smsc_log_debug(_logger, "MessagesRouterSubsystem::applyParametersChange::: Enter it");

  io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkset_id, linkset_mode);

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> appIdIterator = routing_entry_composite_parameter->getIterator<utilx::runtime_cfg::Parameter>("application");

  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();

  utilx::runtime_cfg::CompositeParameter& suaApplicationsParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sua_applications");

  while (appIdIterator.hasElement()) {
    const std::string& appId = appIdIterator.getCurrentElement()->getValue();

    sccp_sap::LinkSetInfoRegistry::getInstance().addAssociation(linkset_id, appId);

    appIdIterator.next();
  }

  processSgpLinkSets(routing_entry_composite_parameter, linkset_id, linkset_mode);

  common::LinkId newRouteLinkSetId;
  utilx::runtime_cfg::CompositeParameter* newRouteEntry;
  while(_uncommitedRoutesCache.fetchNextUncommitedRoute(&newRouteLinkSetId, &newRouteEntry))
    addRouteEntry(newRouteEntry, newRouteLinkSetId);
}

void
MessagesRouterSubsystem::UncommitedRouteEntries::addUncommitedRoute(const common::LinkId& linkset_id,
                                                                    utilx::runtime_cfg::CompositeParameter* route_entry)
{
  _uncommitedRouteModificactionRequests.insert(std::make_pair(linkset_id, route_entry));
}

bool
MessagesRouterSubsystem::UncommitedRouteEntries::fetchNextUncommitedRoute(common::LinkId* linkset_id,
                                                                          utilx::runtime_cfg::CompositeParameter** route_entry)
{
  if ( _uncommitedRouteModificactionRequests.empty() ) return false;
  route_entry_cache_t::iterator iter = _uncommitedRouteModificactionRequests.begin();
  *linkset_id = iter->first;
  *route_entry = iter->second;
  _uncommitedRouteModificactionRequests.erase(iter);
  return true;
}

}}}}
