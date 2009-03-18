#include "LibSuaConnectIndicationEvent.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

LibSuaConnectIndicationEvent::LibSuaConnectIndicationEvent(const communication::LinkId& linkId)
  : _acceptedLinkId(linkId), _logger(smsc::logger::Logger::getInstance("io_dsptch"))
{}

void
LibSuaConnectIndicationEvent::handle()
{
  smsc_log_info(_logger, "LibSuaConnectIndicationEvent::handle::: handle new event for new connection with id=[%s]", _acceptedLinkId.getValue().c_str());
  // TODO: realize adding of linkId into routing table
}

}}}}
