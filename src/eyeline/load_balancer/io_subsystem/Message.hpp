#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGE_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGE_HPP__

# include <sys/types.h>
# include <string>
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class Message {
public:
  virtual ~Message() {}

  typedef uint32_t message_code_t;

  virtual message_code_t getMsgCode() const = 0;
  virtual size_t serialize(Packet* packet) const = 0;
  virtual size_t deserialize(const Packet* packet) = 0;
  virtual std::string toString() const = 0;
};

}}}

#endif
