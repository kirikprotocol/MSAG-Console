#include "LibSccpConnectIndicationEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

void
LibSccpConnectIndicationEvent::handle()
{
  smsc_log_debug(_logger, "LibSccpConnectIndicationEvent::handle::: handle new event for new connection with id=[%s]", _acceptedLinkId.getValue().c_str());
}

}}}}
