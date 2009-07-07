#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOEVENT_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOEVENT_HPP__

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOEvent {
public:
  virtual ~IOEvent() {}
  virtual void handle() = 0;
};

}}}

#endif
