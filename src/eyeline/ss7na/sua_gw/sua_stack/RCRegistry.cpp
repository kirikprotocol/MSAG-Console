#include "RCRegistry.hpp"
#include "core/synchronization/MutexGuard.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

RCRegistry::RCRegistry()
  : _logger(smsc::logger::Logger::getInstance("sua_stack"))
{}

common::TLV_RoutingContext
RCRegistry::getRoutingContext(const common::LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard lockGuard(_lock);
  smsc_log_info(_logger, "RCRegistry::getRoutingContext::: try get routing context for linkId=[%s]", link_id.getValue().c_str());
  return _registredRoutingContexts.Get(link_id.getValue().c_str());
}

void
RCRegistry::insert(const common::LinkId& link_id, const common::TLV_RoutingContext& rc)
{
  smsc::core::synchronization::MutexGuard lockGuard(_lock);
  smsc_log_info(_logger, "RCRegistry::insert::: register received routing context=[%s] for linkId=[%s]", rc.toString().c_str(), link_id.getValue().c_str());
  _registredRoutingContexts.Insert(link_id.getValue().c_str(), rc);
}

void
RCRegistry::remove(const common::LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard lockGuard(_lock);
  smsc_log_info(_logger, "RCRegistry::remove::: remove routing context for linkId=[%s]", link_id.getValue().c_str());
  _registredRoutingContexts.Delete(link_id.getValue().c_str());
}

}}}}
