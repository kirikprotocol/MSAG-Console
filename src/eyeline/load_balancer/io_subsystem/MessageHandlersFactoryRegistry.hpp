#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEHANDLERSFACTORYREGISTRY_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEHANDLERSFACTORYREGISTRY_HPP__

# include <sys/types.h>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/load_balancer/io_subsystem/MessageHandlersFactory.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class MessageHandlersFactoryRegistry :
  public utilx::Singleton<MessageHandlersFactoryRegistry> {
public:
  MessageHandlersFactory& getMessageHandlersFactory(uint8_t protocol_family);
  // registry must be filled before using.
  void registerMessageHandlersFactory(MessageHandlersFactory* message_handlers_factory);
private:
  MessageHandlersFactoryRegistry();
  friend class utilx::Singleton<MessageHandlersFactoryRegistry>;

  MessageHandlersFactory* _registeredMessageHandlersFactories[256];
};

}}}

#endif
