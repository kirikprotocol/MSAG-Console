#include "ConnectionClosedEvent.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"

#include "eyeline/ss7na/m3ua_gw/mtp3/AdjacentDPCRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/SGPLinkIdsRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Pause_Ind.hpp"

#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/MTP3IndicationsProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace io_dispatcher {

void
ConnectionClosedEvent::handle()
{
  smsc_log_info(_logger, "ConnectionClosedEvent::handle::: connection with id=%s has been closed",
                _closedLink->getLinkId().getValue().c_str());

  if ( mtp3::SGPLinkIdsRegistry::getInstance().hasElement(_closedLink->getLinkId()) ) {
    common::io_dispatcher::Reconnector::getInstance().schedule(_closedLink);
    try {
      mtp3::primitives::MTP_Pause_Ind mtpPauseInd(mtp3::AdjacentDPCRegistry::getInstance().lookup(_closedLink->getLinkId()));
      sccp::MTP3IndicationsProcessor::getInstance().handle(mtpPauseInd);
    } catch (utilx::RegistryKeyNotFound& ex) {}
  } else
    common::sccp_sap::ActiveAppsRegistry::getInstance().remove(_closedLink->getLinkId());
}

}}}}
