#include "Router.hpp"
#include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

common::LinkId
Router::route(common::point_code_t lpc, common::point_code_t dpc) const
{
  RoutingTable* routingTable = getRoutingTable(lpc);
  if (!routingTable) {
    smsc_log_error(_logger, "Router::route::: routing table not found for lpc=%u",
                   lpc);
    throw common::MTP3RouteNotFound("Router::route::: can't get route for lpc=[%u]", lpc);
  }

  const common::LinkId& dstLinkId =
    routingTable->getRoute(dpc);

  smsc_log_debug(_logger, "Router::route::: found route to link='%s' for lpc=%u/dpc=%u ",
                 dstLinkId.getValue().c_str(), lpc, dpc);
  return dstLinkId;
}

void
Router::addRoutingTable(common::point_code_t lpc, RoutingTable* routing_table)
{
  _routingTables.Insert(lpc, routing_table);
}

RoutingTable*
Router::getRoutingTable(common::point_code_t lpc) const
{
  RoutingTable** routingTablePtr = _routingTables.GetPtr(lpc);
  if (!routingTablePtr) {
    smsc_log_error(_logger, "Router::getRoutingTable::: can't get routing table for lpc=%u",
                   lpc);
    return NULL;
  } else
    return *routingTablePtr;
}

}}}}}
