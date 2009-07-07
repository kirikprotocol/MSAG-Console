#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPBINDER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPBINDER_HPP__

# include "eyeline/load_balancer/io_subsystem/Binder.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class io_subsystem::IOProcessor;

class SMPPBinder : public io_subsystem::Binder {
public:
  explicit SMPPBinder(io_subsystem::IOProcessor& io_processor)
  : Binder(io_processor)
  {}

  virtual void rebind(const io_subsystem::LinkId& link_set_id_to_smsc);
};

}}}}

#endif /* SMPPBINDER_HPP_ */
