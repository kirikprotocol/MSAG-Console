#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <util/Exception.hpp>
#include <util/vformat.hpp>

#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/utilx/runtime_cfg/Exception.hpp>
#include <eyeline/utilx/strtol.hpp>

#include <eyeline/sua/communication/LinkId.hpp>
#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <eyeline/sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>
#include <eyeline/sua/sua_layer/sua_stack/LinkSetsRegistry.hpp>

#include "MessagesRouterSubsystem.hpp"
#include "GTTranslator.hpp"
#include "GTTranslationTablesRegistry.hpp"
#include "GTTranslationTable.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace messages_router {

MessagesRouterSubsystem::MessagesRouterSubsystem()
  : sua_layer::ApplicationSubsystem("MessagesRouterSubsystem", "msg_rout")
{}

io_dispatcher::LinkSet::linkset_mode_t
MessagesRouterSubsystem::convertStringToTrafficModeValue(const std::string& trafficMode, const std::string& where)
{
  io_dispatcher::LinkSet::linkset_mode_t linkSetMode;
  if ( trafficMode == "loadshare" )
    return io_dispatcher::LinkSet::LOADSHARE;
  else if ( trafficMode == "broadcast" )
    return io_dispatcher::LinkSet::BROADCAST;
  else if ( trafficMode == "override" )
    return io_dispatcher::LinkSet::OVERRIDE;
  else {
    const std::string fmtStr = where + "convertStringToTrafficModeValue::: wrong traffic-mode parameter value=[%s]";
    throw smsc::util::Exception(fmtStr.c_str(), trafficMode.c_str());
  }
}

bool
MessagesRouterSubsystem::processApplicationLinkSets(utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter, const communication::LinkId& linkSetId, io_dispatcher::LinkSet::linkset_mode_t linkSetMode)
{
  bool result = false;

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> appIdIterator = routingEntryCompositeParameter->getIterator<utilx::runtime_cfg::Parameter>("application");

  if ( appIdIterator.hasElement() ) {
    result = true;
    io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkSetId, linkSetMode);
  }

  while (appIdIterator.hasElement()) {
    const std::string& appId = appIdIterator.getCurrentElement()->getValue();
    smsc_log_info(_logger, "MessagesRouterSubsystem::processApplicationLinkSets::: add association for application=[%s] to linkset=[%s]", appId.c_str(), linkSetId.getValue().c_str());
    sua_user_communication::LinkSetInfoRegistry::getInstance().addAssociation(linkSetId, appId);
    appIdIterator.next();
  }

  return result;
}

bool
MessagesRouterSubsystem::processSgpLinkSets(utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter, const communication::LinkId& linkSetId, io_dispatcher::LinkSet::linkset_mode_t linkSetMode)
{
  bool result = false;

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> sgpLinksIterator = routingEntryCompositeParameter->getIterator<utilx::runtime_cfg::Parameter>("sgp_link");

  if ( sgpLinksIterator.hasElement() ) {
    result = true;
    io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkSetId, linkSetMode);
    sua_stack::LinkSetsRegistry::getInstance().registerLinkSet(linkSetId);
  }

  while (sgpLinksIterator.hasElement()) {
    communication::LinkId sgpLinkId(sgpLinksIterator.getCurrentElement()->getValue());
    smsc_log_info(_logger, "MessagesRouterSubsystem::processSgpLinkSets::: add link=[%s] to linkset=[%s]", sgpLinkId.getValue().c_str(), linkSetId.getValue().c_str());
    sua_stack::LinkSetsRegistry::getInstance().registerLinkInLinkSet(linkSetId, sgpLinkId);
    sgpLinksIterator.next();
  }

  return result;
}

std::string
MessagesRouterSubsystem::makeAddressFamilyPrefix(unsigned int gti,
                                                 const std::string& gtMaskValue) const
{
  std::string gtAddressFamilyPrefix;

  if ( gtMaskValue[0] == '+' && gti == 4 )
    gtAddressFamilyPrefix = ".4.0.1.4";
  else if ( gtMaskValue[0] >= 0x30 && gtMaskValue[0] <= 0x39 && gti == 1 )
    gtAddressFamilyPrefix = ".4.0.1.0";
  else {
    std::string::size_type idx = gtMaskValue.rfind('.');
    if ( idx != std::string::npos )
      gtAddressFamilyPrefix = gtMaskValue.substr(0, idx);
    else
      throw smsc::util::Exception("MessagesRouterSubsystem::makeAddressFamilyPrefix::: can't determine address family prefix for gtMaskValue=[%s]", gtMaskValue.c_str());
  }

  return gtAddressFamilyPrefix;
}

void
MessagesRouterSubsystem::addRouteEntry(utilx::runtime_cfg::CompositeParameter* routeDescriptionCompositeParameter,
                                       const communication::LinkId& linkSetId)
{
  utilx::runtime_cfg::Parameter* gtiConfigParameter = routeDescriptionCompositeParameter->getParameter<utilx::runtime_cfg::Parameter>("gti");
  unsigned int gti = 4;
  if ( gtiConfigParameter )
    gti = atoi(gtiConfigParameter->getValue().c_str());

  std::string gtMaskValue = routeDescriptionCompositeParameter->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue();

  const std::string& gtAddressFamilyPrefix = makeAddressFamilyPrefix(gti, gtMaskValue);

  uint8_t destinationSSN = 0;
  const utilx::runtime_cfg::Parameter* ssnConfigParam = routeDescriptionCompositeParameter->getParameter<utilx::runtime_cfg::Parameter>("ssn");
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

  translationTable->addTranslationEntry(gtMaskValue, destinationSSN, linkSetId);
}

void
MessagesRouterSubsystem::fillUpRouteTable(utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                                          const communication::LinkId& linkSetId)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> routeInfoIterator = routingEntryCompositeParameter->getIterator<utilx::runtime_cfg::CompositeParameter>("route");
  if ( !routeInfoIterator.hasElement() ) { // route to CLOUD - process default route
    GTTranslationTable::addDefaultTranslationEntry(linkSetId);
  } else
    while(routeInfoIterator.hasElement()) {
      addRouteEntry(routeInfoIterator.getCurrentElement(), linkSetId);
      routeInfoIterator.next();
    }
}

void
MessagesRouterSubsystem::initializeLinkSets(utilx::runtime_cfg::CompositeParameter& routingKeysParameter)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> routingEntryIterator = routingKeysParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("routingEntry");

  while(routingEntryIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter = routingEntryIterator.getCurrentElement();
    io_dispatcher::LinkSet::linkset_mode_t linkSetMode = convertStringToTrafficModeValue(routingEntryCompositeParameter->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode")->getValue(), "MessagesRouterSubsystem::initializeIncomingLinkSets: ");
    
    communication::LinkId linkSetId(routingEntryCompositeParameter->getValue());

    smsc_log_debug(_logger, "MessagesRouterSubsystem::initializeLinkSets::: linkSetId=[%s]", linkSetId.getValue().c_str());

    fillUpRouteTable(routingEntryCompositeParameter,linkSetId);

    if ( !processApplicationLinkSets(routingEntryCompositeParameter, linkSetId, linkSetMode) )
      processSgpLinkSets(routingEntryCompositeParameter, linkSetId, linkSetMode);

    routingEntryIterator.next();
  }

}

void
MessagesRouterSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "try initialize MessagesRouterSubsystem");

  GTTranslator::init();
  GTTranslationTablesRegistry::init();
  sua_user_communication::LinkSetInfoRegistry::init();
  sua_stack::LinkSetsRegistry::init();

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
MessagesRouterSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context, utilx::runtime_cfg::Parameter* addedParameter)
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  if ( context.getFullName() == "config.routing-keys.routingEntry" ) {
    utilx::runtime_cfg::CompositeParameter* routingEntryParam = findContexParentParameter(context);

    if ( !routingEntryParam ) return;

    smsc_log_debug(_logger, "MessagesRouterSubsystem::addParameterHandler::: handle added parameter '%s'='%s' for context '%s'='%s'", addedParameter->getName().c_str(), addedParameter->getValue().c_str(), context.getFullName().c_str(), context.getValue().c_str());

    if ( addedParameter->getName() != "application" &&
         addedParameter->getName() != "sgp_link" ) return;

    if ( !checkParameterExist(routingEntryParam, addedParameter) ) {
      if ( addedParameter->getName() == "application" ) {
        checkConsistentRuntimeCommand(routingEntryParam, "application", "sgp_link");

        if ( !checkParameterValueIsPresentInConfig<utilx::runtime_cfg::Parameter>("config.sua_applications", "application", addedParameter->getValue()) ) {
          std::string messageToUser("Inconsistent config modification request - application wasn't registered");
          throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - such application wasn't registered", addedParameter->getName().c_str(), addedParameter->getValue().c_str());
        }

      } else {
        checkConsistentRuntimeCommand(routingEntryParam, "sgp_link", "application");

        if ( !checkParameterValueIsPresentInConfig<utilx::runtime_cfg::CompositeParameter>("config.sgp_links", "link", addedParameter->getValue()) ) {
          std::string messageToUser("Inconsistent config modification request - link wasn't registered");
          throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - such link wasn't registered", addedParameter->getName().c_str(), addedParameter->getValue().c_str());
        }
      }
      routingEntryParam->addParameter(addedParameter);
    } else {
      std::string messageToUser("Inconsistent config modification request - the parameter with such value already exist");
      throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", addedParameter->getName().c_str(), addedParameter->getValue().c_str());
    }
  }
}

utilx::runtime_cfg::CompositeParameter*
MessagesRouterSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context, utilx::runtime_cfg::CompositeParameter* addedParameter)
{
  if ( context.getFullName() == "config.routing-keys" ) {
    // this condition is true when called LM_TranslationTable_AddTranslationRuleCommand::executeCommand()
    utilx::runtime_cfg::CompositeParameter& routingKeysParameter = 
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.routing-keys");

    if ( addedParameter->getName() != "routingEntry" ) 
      generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "MessagesRouterSubsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", addedParameter->getName().c_str(), context.getName().c_str());

    if ( checkParameterExist(&routingKeysParameter, addedParameter) )
      generateExceptionAndForcePopUpCurrentInterpreter("Inconsistent config modification request - translation-rule with such value already exists", "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", addedParameter->getName().c_str(), addedParameter->getValue().c_str());

    routingKeysParameter.addParameter(addedParameter);
    return addedParameter;
  } else if ( context.getFullName() == "config.routing-keys.routingEntry" ) {
    utilx::runtime_cfg::CompositeParameter* routingEntryParam = findContexParentParameter(context);

    if ( addedParameter->getName() != "route" ) return NULL;

    if ( routingEntryParam ) {
      if ( !checkParameterExist(routingEntryParam, addedParameter) ) {
        routingEntryParam->addParameter(addedParameter);
        communication::LinkId linkSetId(routingEntryParam->getValue());
        _uncommitedRoutesCache.addUncommitedRoute(linkSetId, addedParameter);
        return addedParameter;
      } else {
        std::string messageToUser("Inconsistent config modification request - such 'route' already exist");
        throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", addedParameter->getName().c_str(), addedParameter->getValue().c_str());
      }
    } else
      return NULL;
  } else
    generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "MessagesRouterSubsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", addedParameter->getName().c_str(), context.getName().c_str());
}

void
MessagesRouterSubsystem::addParameterEventHandler(utilx::runtime_cfg::CompositeParameter* context,
                                                  utilx::runtime_cfg::Parameter* addedParameter)
{
  if ( !checkParameterExist(context, addedParameter) ) {
    context->addParameter(addedParameter);
  } else {
    std::string messageToUser("Inconsistent config modification request - the parameter with such value already exist");
    throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "MessagesRouterSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", addedParameter->getName().c_str(), addedParameter->getValue().c_str());
  }
}

void
MessagesRouterSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                     const utilx::runtime_cfg::Parameter& modifiedParameter)
{
  if ( context.getFullName() == "config.routing-keys.routingEntry" ) {
    utilx::runtime_cfg::CompositeParameter* routingEntryParam = findContexParentParameter(context);
    if ( !routingEntryParam ) return;

    if ( modifiedParameter.getName() == "commit" ) {
      utilx::runtime_cfg::Parameter* trafficModeParam = routingEntryParam->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
      io_dispatcher::LinkSet::linkset_mode_t linkSetMode = io_dispatcher::LinkSet::OVERRIDE;
      if ( trafficModeParam )
        linkSetMode = convertStringToTrafficModeValue(trafficModeParam->getValue(), "MessagesRouterSubsystem::changeParameterEventHandler: ");
      applyParametersChange(communication::LinkId(context.getValue()), linkSetMode, routingEntryParam);
    } else {
      if ( modifiedParameter.getFullName() == "traffic-mode" ) {
        smsc_log_debug(_logger, "MessagesRouterSubsystem::handle::: handle modified parameter 'config.routing-keys.routingEntry.traffic-mode'=[%s] for routingEntry=[%s]", modifiedParameter.getValue().c_str(), context.getValue().c_str());
        utilx::runtime_cfg::Parameter* trafficModeParam = routingEntryParam->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
        if ( trafficModeParam )
          trafficModeParam->setValue(modifiedParameter.getValue());
        else
          routingEntryParam->addParameter(new utilx::runtime_cfg::Parameter("traffic-mode", modifiedParameter.getValue()));
      }
    }
  }
}

void
MessagesRouterSubsystem::applyParametersChange(const communication::LinkId& linkSetId,
                                               io_dispatcher::LinkSet::linkset_mode_t linkSetMode,
                                               utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter)
{
  smsc_log_debug(_logger, "MessagesRouterSubsystem::applyParametersChange::: Enter it");

  io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkSetId, linkSetMode);

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> appIdIterator = routingEntryCompositeParameter->getIterator<utilx::runtime_cfg::Parameter>("application");

  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();

  utilx::runtime_cfg::CompositeParameter& suaApplicationsParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sua_applications");

  while (appIdIterator.hasElement()) {
    const std::string& appId = appIdIterator.getCurrentElement()->getValue();

    sua_user_communication::LinkSetInfoRegistry::getInstance().addAssociation(linkSetId, appId);

    appIdIterator.next();
  }

  processSgpLinkSets(routingEntryCompositeParameter, linkSetId, linkSetMode);

  communication::LinkId newRouteLinkSetId;
  utilx::runtime_cfg::CompositeParameter* newRouteEntry;
  while(_uncommitedRoutesCache.fetchNextUncommitedRoute(&newRouteLinkSetId, &newRouteEntry))
    addRouteEntry(newRouteEntry, newRouteLinkSetId);
}

void
MessagesRouterSubsystem::UncommitedRouteEntries::addUncommitedRoute(const communication::LinkId& linkSetId,
                                                                    utilx::runtime_cfg::CompositeParameter* routeEntry)
{
  _uncommitedRouteModificactionRequests.insert(std::make_pair(linkSetId, routeEntry));
}

bool
MessagesRouterSubsystem::UncommitedRouteEntries::fetchNextUncommitedRoute(communication::LinkId* linkSetId,
                                                                          utilx::runtime_cfg::CompositeParameter** routeEntry)
{
  if ( _uncommitedRouteModificactionRequests.empty() ) return false;
  route_entry_cache_t::iterator iter = _uncommitedRouteModificactionRequests.begin();
  *linkSetId = iter->first;
  *routeEntry = iter->second;
  _uncommitedRouteModificactionRequests.erase(iter);
  return true;
}

}}}}
