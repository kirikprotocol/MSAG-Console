#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPIOPROCESSOR_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPIOPROCESSOR_HPP__

# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPBinder.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPPIOProcessor : public io_subsystem::IOProcessor {
public:
  SMPPIOProcessor(unsigned io_proc_id,
                  io_subsystem::SwitchCircuitController& switch_circuit_ctrl,
                  unsigned max_events_queue_sz, unsigned reconnect_attempt_period,
                  unsigned max_out_packets_queue_size, unsigned max_num_of_event_processors,
                  unsigned max_links_per_processor)
  : IOProcessor(io_proc_id, switch_circuit_ctrl,
                max_events_queue_sz, reconnect_attempt_period,
                max_out_packets_queue_size, max_num_of_event_processors,
                max_links_per_processor),
    _binder(*this)
  {}

  virtual void removeIncomingLink(const io_subsystem::LinkId& link_id);

  virtual io_subsystem::TimeoutEvent*
  createBindResponseWaitTimeoutEvent(const io_subsystem::LinkId& link_id,
                                     io_subsystem::Message* response_message_for_send);

  virtual io_subsystem::Binder& getBinder()
  {
    return _binder;
  }

private:
  SMPPBinder _binder;
};

}}}}

#endif
