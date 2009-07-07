#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_PROTOCOLMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_PROTOCOLMANAGEMENTMESSAGEHANDLERS_HPP__

# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

# include "eyeline/load_balancer/protocols/smpp/AlertNotification.hpp"
# include "eyeline/load_balancer/protocols/smpp/GenericNack.hpp"
# include "eyeline/load_balancer/protocols/smpp/MessageForwardingHelper.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class ProtocolManagementMessageHandlers : private MessageForwardingHelper {
public:
  virtual void handle(const AlertNotification& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const GenericNack& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);
};

}}}}

#endif
