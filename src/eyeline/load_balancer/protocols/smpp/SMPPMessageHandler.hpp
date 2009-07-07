#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPPMESSAGEHANDLER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPMESSAGEHANDLER_HPP__

# include "eyeline/load_balancer/io_subsystem/MessageHandler.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

template<class MESSAGE, class MESSAGE_HANDLERS>
class SMPPMessageHandler : public io_subsystem::MessageHandler {
public:
  SMPPMessageHandler(const MESSAGE& message)
    : _message(message)
  {}

  virtual void handle(const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor) const {
    MESSAGE_HANDLERS msgHandlers;
    msgHandlers.handle(_message, src_link_id, io_processor);
  }
private:
  const MESSAGE& _message;
};

}}}}

#endif
