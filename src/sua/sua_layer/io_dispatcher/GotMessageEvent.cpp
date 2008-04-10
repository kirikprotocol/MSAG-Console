#include "GotMessageEvent.hpp"

#include <sua/communication/Message.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

namespace io_dispatcher {

GotMessageEvent::GotMessageEvent(communication::TP* transportPacket, const communication::LinkId& curLinkId)
  : _tp(transportPacket), _curLinkId(curLinkId), _logger (smsc::logger::Logger::getInstance("io_dsptch"))
{
}

void
GotMessageEvent::handle()
{
  smsc_log_info(_logger, "GotMessageEvent::handle::: processing transport packet with packet type [=%d]", _tp->packetType);
  communication::Message* message = io_dispatcher::SuaLayerMessagesFactory::getInstance().instanceMessage(_tp->packetType);

  if ( message ) {
    message->deserialize(*_tp);
    smsc_log_info(_logger, "GotMessageEvent::handle::: got new message=[%s]", message->toString().c_str());
    communication::MessageHandlingDispatcherIface* handlingDispatcher = message->getHandlingDispatcher();
    if ( handlingDispatcher )
      handlingDispatcher->dispatch_handle(_curLinkId);
  } else
    throw smsc::util::Exception ("GotMessageEvent::handle::: can't instantiate message from with packet type [=%d]", _tp->packetType);
}

}
