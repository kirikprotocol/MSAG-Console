#include <stdio.h>
#include <stdlib.h>
#include <util/Exception.hpp>
#include <sua/communication/LinkId.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <sua/sua_layer/sua_user_communication/LinkSetInfoRegistry.hpp>
#include <sua/sua_layer/sua_stack/LinkSetsRegistry.hpp>
#include "MessagesRouter.hpp"
#include "MessagesRouterSubsystem.hpp"

messages_router::MessagesRouterSubsystem*
utilx::Singleton<messages_router::MessagesRouterSubsystem*>::_instance;

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

void
MessagesRouterSubsystem::removeAsteriskFromPrefix(std::string* wildCardStr)
{
  std::string::size_type pos = wildCardStr->find('*');
  if ( pos != std::string::npos )
    wildCardStr->erase(pos);
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
MessagesRouterSubsystem::extractAddrPrefixAndSSN(const std::string& gt, std::string* destinationGTMask, uint8_t* destinationSSN, const char* where)
{
  std::string::size_type pos = gt.find(':');

  if ( pos != std::string::npos ) {
    *destinationGTMask = gt.substr(0, pos);
    removeAsteriskFromPrefix(destinationGTMask);
    std::string destinationSSN_paramVal = gt.substr(pos+1);
    *destinationSSN = static_cast<uint8_t>(strtol(destinationSSN_paramVal.c_str(), (char **)NULL, 10));
    if ( *destinationSSN == 0 && errno == EINVAL )
      throw smsc::util::Exception(std::string(where).append(" wrong SSN value format [%s]").c_str(), destinationSSN_paramVal.c_str());
    return true;
  } else {
    *destinationGTMask = gt;
    removeAsteriskFromPrefix(destinationGTMask);
    return false;
  }
}

void
MessagesRouterSubsystem::initializeIncomingLinkSets(runtime_cfg::CompositeParameter& incomingRoutingKeysParameter)
{
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> incomingGTIterator = incomingRoutingKeysParameter.getIterator<runtime_cfg::CompositeParameter>("GT");

  while(incomingGTIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* gtParams = incomingGTIterator.getCurrentElement();
    const std::string& gt = gtParams->getValue();
    communication::LinkId linkSetId(std::string("IN:") + gt);
    runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> appIdIterator = gtParams->getIterator<runtime_cfg::Parameter>("application_id");
    while (appIdIterator.hasElement()) {
      const std::string& appId=appIdIterator.getCurrentElement()->getValue();
      smsc_log_info(_logger, "MessagesRouterSubsystem::initializeIncomingLinkSets::: add association for application=[%s] to linkset=[%s]", appId.c_str(), linkSetId.getValue().c_str());
      sua_user_communication::LinkSetInfoRegistry::getInstance().addAssociation(linkSetId, appId);
      appIdIterator.next();
    }

    io_dispatcher::LinkSet::linkset_mode_t linkSetMode = convertStringToTrafficModeValue(gtParams->getParameter<runtime_cfg::Parameter>("traffic_mode")->getValue(), "MessagesRouterSubsystem::initializeIncomingLinkSets: ");

    io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkSetId, linkSetMode);

    std::string destinationGTPrefix;
    uint8_t destinationSSN;
    if ( extractAddrPrefixAndSSN(gt, &destinationGTPrefix, &destinationSSN, "MessagesRouterSubsystem::initializeIncomingLinkSets:::") )
      MessagesRouter::getInstance().addIncomingRouteEntry(destinationGTPrefix, destinationSSN, linkSetId);
    else
      MessagesRouter::getInstance().addIncomingRouteEntry(destinationGTPrefix, linkSetId);

    incomingGTIterator.next();
  }

}

void
MessagesRouterSubsystem::initializeOutcomingLinkSets(runtime_cfg::CompositeParameter& outcomingRoutingKeysParameter)
{
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> outcomingGTIterator = outcomingRoutingKeysParameter.getIterator<runtime_cfg::CompositeParameter>("GT");

  while(outcomingGTIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* gtParams = outcomingGTIterator.getCurrentElement();

    io_dispatcher::LinkSet::linkset_mode_t linkSetMode = convertStringToTrafficModeValue(gtParams->getParameter<runtime_cfg::Parameter>("traffic_mode")->getValue(), "MessagesRouterSubsystem::initializeOutcomingLinkSets: ");

    const std::string& gt = gtParams->getValue();
    communication::LinkId linkSetId(std::string("OUT:") + gt);

    io_dispatcher::ConnectMgr::getInstance().registerLinkSet(linkSetId, linkSetMode);
    sua_stack::LinkSetsRegistry::getInstance().registerLinkSet(linkSetId);

    runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> sgpLinkIdIterator = gtParams->getIterator<runtime_cfg::Parameter>("link_id");
    while (sgpLinkIdIterator.hasElement()) {
      communication::LinkId sgpLinkId(sgpLinkIdIterator.getCurrentElement()->getValue());
      smsc_log_info(_logger, "MessagesRouterSubsystem::initializeOutcomingLinkSets::: add link=[%s] to linkset=[%s]", sgpLinkId.getValue().c_str(), linkSetId.getValue().c_str());
      sua_stack::LinkSetsRegistry::getInstance().registerLinkInLinkSet(linkSetId, sgpLinkId);
      sgpLinkIdIterator.next();
    }

    std::string destinationGTPrefix;
    uint8_t destinationSSN;
    if ( extractAddrPrefixAndSSN(gt, &destinationGTPrefix, &destinationSSN, "MessagesRouterSubsystem::initializeOutcomingLinkSets:::") )
      MessagesRouter::getInstance().addOutcomingRouteEntry(destinationGTPrefix, destinationSSN, linkSetId);
    else
      MessagesRouter::getInstance().addOutcomingRouteEntry(destinationGTPrefix, linkSetId);

    outcomingGTIterator.next();
  }
}

void
MessagesRouterSubsystem::initialize(runtime_cfg::RuntimeConfig& rconfig)
{
  MessagesRouter::init();
  sua_user_communication::LinkSetInfoRegistry::init();
  sua_stack::LinkSetsRegistry::init();
  initializeIncomingLinkSets(rconfig.find<runtime_cfg::CompositeParameter>("config.incoming-routing-keys"));
  initializeOutcomingLinkSets(rconfig.find<runtime_cfg::CompositeParameter>("config.outcoming-routing-keys"));
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
