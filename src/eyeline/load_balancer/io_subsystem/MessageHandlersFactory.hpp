#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEHANDLERSFACTORY_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEHANDLERSFACTORY_HPP__

# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/load_balancer/io_subsystem/MessageHandler.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class MessageHandlersFactory {
public:
  virtual ~MessageHandlersFactory() {}

  virtual MessageHandler* createMessageHandler(const Packet* packet) = 0;
  virtual uint8_t getProtocolFamily() const = 0;
};

}}}

#endif
