#include "MessagesRouter.hpp"
#include <util/Exception.hpp>

messages_router::MessagesRouter*
utilx::Singleton<messages_router::MessagesRouter>::_instance;

namespace messages_router {

MessagesRouter::MessagesRouter()
  : _logger(smsc::logger::Logger::getInstance("msg_rout")) {
  for(int i=0; i<RTABLE_SIZE; ++i) {
    _outRoutingTable[i] = new rtable_t();
    _inRoutingTable[i] = new rtable_t();
  }
}

communication::LinkId
MessagesRouter::getOutLink(const RoutingKey& messageRoutingInfo)
{
  smsc_log_debug(_logger, "MessagesRouter::getOutLink::: try find output LinkId for routing key=[%s]", messageRoutingInfo.toString().c_str());

  if ( !messageRoutingInfo.isSetDestinationGT() )
    throw smsc::util::Exception("MessagesRouter::getOutLink::: destination gt wasn' set in routing key");

  communication::LinkId outLinkId;

  uint8_t routeTableIdx = DEFAULT_RTABLE_ENTRY;
  if ( messageRoutingInfo.isSetDestinationSSN() )
    routeTableIdx = messageRoutingInfo.getDestinationSSN();

  if ( !_outRoutingTable[routeTableIdx]->FindPrefix(messageRoutingInfo.getDestinationGT().c_str(), outLinkId) )
    throw smsc::util::Exception("MessagesRouter::getOutLink::: can't find route to SG for routing key=[%s], wrong configuration", messageRoutingInfo.toString().c_str());

  smsc_log_info(_logger, "MessagesRouter::getOutLink::: found route to SG with LinkId=[%s] for routing key=[%s]", outLinkId.getValue().c_str(), messageRoutingInfo.toString().c_str());
  return outLinkId;
}

communication::LinkId
MessagesRouter::getOutLink(const RoutingKey& messageRoutingInfo,
                           bool* callTerminationIndicator)
{
  smsc_log_debug(_logger, "MessagesRouter::getOutLink::: try find output LinkId for routing key=[%s]", messageRoutingInfo.toString().c_str());

  if ( !messageRoutingInfo.isSetDestinationGT() )
    throw smsc::util::Exception("MessagesRouter::getOutLink::: destination gt wasn' set in routing key");

  communication::LinkId outLinkId;

  uint8_t routeTableIdx = DEFAULT_RTABLE_ENTRY;
  if ( messageRoutingInfo.isSetDestinationSSN() )
    routeTableIdx = messageRoutingInfo.getDestinationSSN();

  if ( _inRoutingTable[routeTableIdx]->FindPrefix(messageRoutingInfo.getDestinationGT().c_str(), outLinkId) ) {
    *callTerminationIndicator = true;
    smsc_log_info(_logger, "MessagesRouter::getOutLink::: found LinkId=[%s] for routing key=[%s], callTerminationIndicator=[CALL TERMINATED]", outLinkId.getValue().c_str(), messageRoutingInfo.toString().c_str());
  } else {
    *callTerminationIndicator = false;
    smsc_log_info(_logger, "MessagesRouter::getOutLink::: route not found for routing key=[%s], callTerminationIndicator=[CALL FORWARDING]", outLinkId.getValue().c_str(), messageRoutingInfo.toString().c_str());
  }
  return outLinkId;
}

void
MessagesRouter::addIncomingRouteEntry(const std::string& destinationGtMask,
                                      const communication::LinkId& linkToUser)
{
  smsc_log_info(_logger, "MessagesRouter::addIncomingRouteEntry::: add incoming route entry=[destinationGTPrefix=[%s],linkSetId=[%s]]", destinationGtMask.c_str(), linkToUser.getValue().c_str());
  _inRoutingTable[DEFAULT_RTABLE_ENTRY]->Insert(destinationGtMask.c_str(), linkToUser);
}

void
MessagesRouter::addIncomingRouteEntry(const std::string& destinationGtMask,
                                      uint8_t destinationSSN,
                                      const communication::LinkId& linkToUser)
{
  smsc_log_info(_logger, "MessagesRouter::addIncomingRouteEntry::: add incoming route entry=[destinationGTPrefix=[%s],destinationSSN=[%d],linkSetId=[%s]]", destinationGtMask.c_str(), destinationSSN, linkToUser.getValue().c_str());
  _inRoutingTable[destinationSSN]->Insert(destinationGtMask.c_str(), linkToUser);
}

void
MessagesRouter::addOutcomingRouteEntry(const std::string& destinationGtMask,
                                       const communication::LinkId& linkToSGP)
{
  smsc_log_info(_logger, "MessagesRouterSubsystem::addOutcomingRouteEntry::: add outcoming route entry=[destinationGTPrefix=[%s],linkSetId=[%s]]", destinationGtMask.c_str(), linkToSGP.getValue().c_str());
  _outRoutingTable[DEFAULT_RTABLE_ENTRY]->Insert(destinationGtMask.c_str(), linkToSGP);
}

void
MessagesRouter::addOutcomingRouteEntry(const std::string& destinationGtMask,
                                       uint8_t destinationSSN,
                                       const communication::LinkId& linkToSGP)
{
  smsc_log_info(_logger, "MessagesRouterSubsystem::addOutcomingRouteEntry::: add outcoming route entry=[destinationGTPrefix=[%s],destinationSSN=[%d],linkSetId=[%s]]", destinationGtMask.c_str(), destinationSSN, linkToSGP.getValue().c_str());
  _outRoutingTable[destinationSSN]->Insert(destinationGtMask.c_str(), linkToSGP);
}

}
