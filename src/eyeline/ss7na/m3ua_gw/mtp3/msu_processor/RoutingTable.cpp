#include "RoutingTable.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/ss7na/common/types.hpp"
#include "eyeline/ss7na/common/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

void
RoutingTable::addRoute(common::point_code_t dpc, const common::LinkId& dst_link_id)
{
  smsc_log_debug(_logger, "RoutingTable::addRoute::: add route dpc=%u, dst_link_id=%s",
                 dpc, dst_link_id.getValue().c_str());
  MutexGuard synchronize(_lock);
  _routingTable.Insert(dpc, dst_link_id);
}

void
RoutingTable::removeRoute(common::point_code_t dpc)
{
  MutexGuard synchronize(_lock);
  _routingTable.Delete(dpc);
}

const common::LinkId&
RoutingTable::getRoute(common::point_code_t dpc)
{
  MutexGuard synchronize(_lock);

  common::LinkId* dstLinkId = _routingTable.GetPtr(dpc);
  if (!dstLinkId) {
    smsc_log_error(_logger, "RoutingTable::getRoute::: route not found for dpc=[%]", dpc);
    throw common::RouteNotFound(common::MTP_FAILURE, "RoutingTable::getRoute::: routing failure");
  }
  return *dstLinkId;
}

}}}}}
