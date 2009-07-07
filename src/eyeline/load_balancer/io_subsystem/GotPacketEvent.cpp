#include <memory>

#include "GotPacketEvent.hpp"
#include "MessageHandlersFactoryRegistry.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
GotPacketEvent::handle()
{
  try {
    smsc_log_debug(_logger, "GotPacketEvent::handle::: handle packet (protocol_family=%d, packet_type=0x%08X) that got from link with id='%s'",
                   _packet->protocol_family, _packet->packet_type, _linkId.toString().c_str());
    MessageHandlersFactory& messageHandlersFactory =
      MessageHandlersFactoryRegistry::getInstance().getMessageHandlersFactory(_packet->protocol_family);
    MessageHandler* msgHandler = messageHandlersFactory.createMessageHandler(_packet);
    msgHandler->handle(_linkId, _ioProcessor);
  } catch (utilx::RegistryKeyNotFound& ex) {
    smsc_log_error(_logger, "GotPacketEvent::handle::: caught RegistryKeyNotFound exception [%s]", ex.what());
  }
}

}}}
