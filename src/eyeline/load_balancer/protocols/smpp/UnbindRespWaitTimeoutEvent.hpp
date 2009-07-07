#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_UNBINDRESPWAITTIMEOUTEVENT_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_UNBINDRESPWAITTIMEOUTEVENT_HPP__

# include "eyeline/load_balancer/io_subsystem/TimeoutEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class UnbindRespWaitTimeoutEvent : public io_subsystem::TimeoutEvent {
public:
  UnbindRespWaitTimeoutEvent(io_subsystem::IOProcessor& io_processor,
                             const io_subsystem::LinkId& link_id)
  : TimeoutEvent(io_processor, link_id)
  {}

  virtual void handle();
};

}}}}

#endif
