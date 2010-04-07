#include "GotMessageEvent.hpp"

#include "eyeline/ss7na/common/Message.hpp"
#include "MessagesFactory.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

GotMessageEvent::GotMessageEvent(TP* transportPacket, const LinkId& curLinkId)
: _tp(transportPacket), _curLinkId(curLinkId),
  _logger (smsc::logger::Logger::getInstance("io_dsptch"))
{}

void
GotMessageEvent::handle()
{
  smsc_log_debug(_logger, "GotMessageEvent::handle::: processing transport packet with packeType=%u/protocolClass=%u",
                 _tp->packetType, _tp->protocolClass);
  Message* message = MessagesFactory::getInstance().instanceMessage(_tp->packetType, _tp->protocolClass);

  if ( message ) {
    message->deserialize(*_tp);
    smsc_log_debug(_logger, "GotMessageEvent::handle::: got new message=[%s]", message->toString().c_str());
    MessageHandlingDispatcherIface* handlingDispatcher = message->getHandlingDispatcher();
    if ( handlingDispatcher )
      handlingDispatcher->dispatch_handle(_curLinkId);
  } else
    throw smsc::util::Exception ("GotMessageEvent::handle::: can't instantiate message with packet type [=%d]", _tp->packetType);
}

}}}}
