#include <string.h>
#include "MessageHandlersFactoryRegistry.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

MessageHandlersFactoryRegistry::MessageHandlersFactoryRegistry()
{
  memset(reinterpret_cast<uint8_t*>(_registeredMessageHandlersFactories), 0,
         sizeof(_registeredMessageHandlersFactories));
}

MessageHandlersFactory&
MessageHandlersFactoryRegistry::getMessageHandlersFactory(uint8_t protocol_family)
{
  if ( !_registeredMessageHandlersFactories[protocol_family] )
    throw utilx::RegistryKeyNotFound("MessageHandlersFactoryRegistry::getMessageHandlersFactory::: MessageHandlersFactory isn't registered for protocol family value = %d",
                                     protocol_family);
  return *_registeredMessageHandlersFactories[protocol_family];
}

void
MessageHandlersFactoryRegistry::registerMessageHandlersFactory(MessageHandlersFactory* message_handlers_factory)
{
  if ( _registeredMessageHandlersFactories[message_handlers_factory->getProtocolFamily()] )
    throw utilx::DuplicatedRegistryKeyException("MessageHandlersFactoryRegistry::registerMessageHandlersFactory::: MessageHandlersFactory has already registered for protocol family value = %d",
                                                message_handlers_factory->getProtocolFamily());
  _registeredMessageHandlersFactories[message_handlers_factory->getProtocolFamily()] = message_handlers_factory;
}

}}}
