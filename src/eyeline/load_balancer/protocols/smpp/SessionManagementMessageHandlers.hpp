#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_SESSIONMANAGEMENTMESSAGEHANDLERS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_SESSIONMANAGEMENTMESSAGEHANDLERS_HPP__

# include <memory>
# include "util/Exception.hpp"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"

# include "eyeline/load_balancer/protocols/smpp/BindReceiver.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindReceiverResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindTransceiver.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindTransceiverResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindTransmitter.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindTransmitterResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/EnquireLink.hpp"
# include "eyeline/load_balancer/protocols/smpp/EnquireLinkResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/Unbind.hpp"
# include "eyeline/load_balancer/protocols/smpp/UnbindResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/MessageForwardingHelper.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SessionManagementMessageHandlers : private MessageForwardingHelper {
public:
  void handle(const BindReceiver& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const BindReceiverResp& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const BindTransceiver& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const BindTransceiverResp& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const BindTransmitter& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const BindTransmitterResp& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const EnquireLink& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const EnquireLinkResp& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const Unbind& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

  void handle(const UnbindResp& message,
              const io_subsystem::LinkId& src_link_id,
              io_subsystem::IOProcessor& io_processor);

protected:
  template<class BIND_REQUEST>
  void processBindRequest(const BIND_REQUEST& message,
                          const io_subsystem::LinkId& src_link_id,
                          io_subsystem::IOProcessor& io_processor);

  template<class BIND_RESPONSE>
  void processBindResponse(const BIND_RESPONSE& message,
                           const io_subsystem::LinkId& src_link_id,
                           io_subsystem::IOProcessor& io_processor);
};

# include "eyeline/load_balancer/protocols/smpp/SessionManagementMessageHandlers_impl.hpp"

}}}}

#endif
