#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEHANDLER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEHANDLER_HPP__

# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class MessageHandler {
public:
  virtual ~MessageHandler() {}
  virtual void handle(const LinkId& src_link_id, IOProcessor& io_processor) const = 0;
};

}}}

#endif
