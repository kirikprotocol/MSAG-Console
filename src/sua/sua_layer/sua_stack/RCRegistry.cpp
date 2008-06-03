#include "RCRegistry.hpp"
#include <core/synchronization/MutexGuard.hpp>

namespace sua_stack {

RCRegistry::RCRegistry()
  : _logger(smsc::logger::Logger::getInstance("sua_stack"))
{}

sua_messages::TLV_RoutingContext
RCRegistry::getRoutingContext(const communication::LinkId& linkId)
{
  smsc::core::synchronization::MutexGuard lockGuard(_lock);
  smsc_log_info(_logger, "RCRegistry::getRoutingContext::: try get routing context for linkId=[%s]", linkId.getValue().c_str());
  return _registredRoutingContexts.Get(linkId.getValue().c_str());
}

void
RCRegistry::insert(const communication::LinkId& linkId, const sua_messages::TLV_RoutingContext& rc)
{
  smsc::core::synchronization::MutexGuard lockGuard(_lock);
  smsc_log_info(_logger, "RCRegistry::insert::: register received routing context=[%s] for linkId=[%s]", rc.toString().c_str(), linkId.getValue().c_str());
  _registredRoutingContexts.Insert(linkId.getValue().c_str(), rc);
}

void
RCRegistry::remove(const communication::LinkId& linkId)
{
  smsc::core::synchronization::MutexGuard lockGuard(_lock);
  smsc_log_info(_logger, "RCRegistry::remove::: remove routing context for linkId=[%s]", linkId.getValue().c_str());
  _registredRoutingContexts.Delete(linkId.getValue().c_str());
}

}
