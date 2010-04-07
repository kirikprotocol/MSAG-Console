#include "ConnectionClosedEvent.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"
#include "eyeline/ss7na/common/sig/LinkSetsRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace io_dispatcher {

void
ConnectionClosedEvent::handle()
{
  smsc_log_info(_logger, "ConnectionClosedEvent::handle::: connection with id=%s has been closed",
                _closedLink->getLinkId().getValue().c_str());
  if ( common::sig::LinkSetsRegistry::getInstance().checkLinkIsMemberOfAnyLinkset(_closedLink->getLinkId()) )
    common::io_dispatcher::Reconnector::getInstance().schedule(_closedLink);
  else
    common::sccp_sap::ActiveAppsRegistry::getInstance().remove(_closedLink->getLinkId());
}

}}}}
