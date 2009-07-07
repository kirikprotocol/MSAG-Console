#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDRESPWAITTIMEOUTEVENT_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDRESPWAITTIMEOUTEVENT_HPP__

# include "eyeline/load_balancer/io_subsystem/Message.hpp"
# include "eyeline/load_balancer/io_subsystem/TimeoutEvent.hpp"
# include "eyeline/load_balancer/protocols/smpp/MessageForwardingHelper.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindRespWaitTimeoutEvent : public io_subsystem::TimeoutEvent,
                                 private MessageForwardingHelper {
public:
  BindRespWaitTimeoutEvent(io_subsystem::IOProcessor& io_processor,
                           const io_subsystem::LinkId& link_id_to_smsc,
                           const io_subsystem::Message* negative_bind_response)
  : TimeoutEvent(io_processor, link_id_to_smsc),
    _negativeBindResponse(negative_bind_response)
  {}

  virtual ~BindRespWaitTimeoutEvent() { delete _negativeBindResponse; }

  virtual void handle();
private:
  const io_subsystem::Message* _negativeBindResponse;
};

}}}}

#endif
