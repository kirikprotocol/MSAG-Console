#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_SERVICEMESSAGEHANDLERS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_SERVICEMESSAGEHANDLERS_HPP__

# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

# include "eyeline/load_balancer/protocols/smpp/CancelSm.hpp"
# include "eyeline/load_balancer/protocols/smpp/CancelSmResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/QuerySm.hpp"
# include "eyeline/load_balancer/protocols/smpp/QuerySmResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/ReplaceSm.hpp"
# include "eyeline/load_balancer/protocols/smpp/ReplaceSmResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/MessageForwardingHelper.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class ServiceMessageHandlers : private MessageForwardingHelper {
public:
  virtual void handle(const CancelSm& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const CancelSmResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const QuerySm& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const QuerySmResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const ReplaceSm& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const ReplaceSmResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);
};

}}}}

#endif
