#include <stdio.h>
#include <stdlib.h>
#include <util/Exception.hpp>
#include <sua/communication/LinkId.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>
#include <sua/sua_layer/sua_stack/LinkSetsRegistry.hpp>
#include "MessagesRouterSubsystem.hpp"
#include "GTTranslator.hpp"
#include "GTTranslationTablesRegistry.hpp"
#include "GTTranslationTable.hpp"

namespace messages_router {

MessagesRouterSubsystem::MessagesRouterSubsystem()
  : _name("MessagesRouterSubsystem"), _logger(smsc::logger::Logger::getInstance("msg_rout"))
{}

void
MessagesRouterSubsystem::start()
{}

void
MessagesRouterSubsystem::stop()
{
}

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
MessagesRouterSubsystem::processApplicationLinkSets(const runtime_cfg::CompositeParameter* routingEntryCompositeParameter, const communication::LinkId& linkSetId, io_dispatcher::LinkSet::linkset_mode_t linkSetMode)
{
  bool result = false;

  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> appIdIterator = routingEntryCompositeParameter->getIterator<runtime_cfg::Parameter>("application");

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
MessagesRouterSubsystem::processSgpLinkSets(const runtime_cfg::CompositeParameter* routingEntryCompositeParameter, const communication::LinkId& linkSetId, io_dispatcher::LinkSet::linkset_mode_t linkSetMode)
{
  bool result = false;

  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> sgpLinksIterator = routingEntryCompositeParameter->getIterator<runtime_cfg::Parameter>("sgp_link");

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
MessagesRouterSubsystem::fillUpRouteTable(const runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                                          const communication::LinkId& linkSetId)
{
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter>& routeInfoIterator = routingEntryCompositeParameter->getIterator<runtime_cfg::CompositeParameter>("route");
  if ( !routeInfoIterator.hasElement() ) { // route to CLOUD - process default route
    GTTranslationTable::addDefaultTranslationEntry(linkSetId);
  } else
    while(routeInfoIterator.hasElement()) {
      const runtime_cfg::CompositeParameter* routeDescriptionCompositeParameter = routeInfoIterator.getCurrentElement();
      runtime_cfg::Parameter* gtiConfigParameter = routeDescriptionCompositeParameter->getParameter<runtime_cfg::Parameter>("gti");
      unsigned int gti = 4;
      if ( gtiConfigParameter )
        gti = atoi(gtiConfigParameter->getValue().c_str());

      std::string gtMaskValue = routeDescriptionCompositeParameter->getParameter<runtime_cfg::Parameter>("gt")->getValue();

      const std::string& gtAddressFamilyPrefix = makeAddressFamilyPrefix(gti, gtMaskValue);

      smsc_log_debug(_logger, "MessagesRouterSubsystem::fillUpRouteTable::: process next route entry, gtMask=[%s]", gtMaskValue.c_str());
      uint8_t destinationSSN = 0;
      const runtime_cfg::Parameter* ssnConfigParam = routeDescriptionCompositeParameter->getParameter<runtime_cfg::Parameter>("ssn");
      if ( ssnConfigParam ) {
        const std::string& ssnValue = ssnConfigParam->getValue();

        destinationSSN = static_cast<uint8_t>(strtol(ssnValue.c_str(), (char **)NULL, 10));
        if ( destinationSSN == 0 && errno == EINVAL )
          throw smsc::util::Exception("MessagesRouterSubsystem::fillUpRouteTable::: wrong SSN value format [%s]", ssnValue.c_str());
      }

      GTTranslationTable* translationTable = GTTranslationTablesRegistry::getInstance().getGTTranslationTable(gtAddressFamilyPrefix);
      if ( !translationTable )
        throw smsc::util::Exception("MessagesRouterSubsystem::fillUpRouteTable::: translation table not found for address family=[%s]", gtAddressFamilyPrefix.c_str());

      translationTable->addTranslationEntry(gtMaskValue, destinationSSN, linkSetId);

      routeInfoIterator.next();
    }
}

void
MessagesRouterSubsystem::initializeLinkSets(runtime_cfg::CompositeParameter& routingKeysParameter)
{
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> routingEntryIterator = routingKeysParameter.getIterator<runtime_cfg::CompositeParameter>("routingEntry");

  while(routingEntryIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* routingEntryCompositeParameter = routingEntryIterator.getCurrentElement();
    io_dispatcher::LinkSet::linkset_mode_t linkSetMode = convertStringToTrafficModeValue(routingEntryCompositeParameter->getParameter<runtime_cfg::Parameter>("traffic-mode")->getValue(), "MessagesRouterSubsystem::initializeIncomingLinkSets: ");
    
    communication::LinkId linkSetId(routingEntryCompositeParameter->getValue());

    smsc_log_debug(_logger, "MessagesRouterSubsystem::initializeLinkSets::: linkSetId=[%s]", linkSetId.getValue().c_str());

    fillUpRouteTable(routingEntryCompositeParameter,linkSetId);

    if ( !processApplicationLinkSets(routingEntryCompositeParameter, linkSetId, linkSetMode) )
      processSgpLinkSets(routingEntryCompositeParameter, linkSetId, linkSetMode);

    routingEntryIterator.next();
  }

}

void
MessagesRouterSubsystem::initialize(runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "try initialize MessagesRouterSubsystem");

  GTTranslator::init();
  GTTranslationTablesRegistry::init();
  sua_user_communication::LinkSetInfoRegistry::init();
  sua_stack::LinkSetsRegistry::init();

  GTTranslationTablesRegistry::getInstance().registerGTTranslationTable(".4.0.1.4", new GTTranslationTable_Prefix_4_0_1_4());
  GTTranslationTablesRegistry::getInstance().registerGTTranslationTable(".4.0.1.0", new GTTranslationTable_Prefix_4_0_1_0());

  initializeLinkSets(rconfig.find<runtime_cfg::CompositeParameter>("config.routing-keys"));
}

const std::string&
MessagesRouterSubsystem::getName() const
{
  return _name;
}

void
MessagesRouterSubsystem::waitForCompletion()
{}

}
