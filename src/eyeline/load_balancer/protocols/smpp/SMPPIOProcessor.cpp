#include "SMPPIOProcessor.hpp"
#include "BindRespWaitTimeoutEvent.hpp"
#include "SmeRegistry.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
SMPPIOProcessor::removeIncomingLink(const io_subsystem::LinkId& link_id)
{
  SmeRegistry::getInstance().deleteSmeInfo(link_id);
  IOProcessor::removeIncomingLink(link_id);
}

io_subsystem::TimeoutEvent*
SMPPIOProcessor::createBindResponseWaitTimeoutEvent(const io_subsystem::LinkId& link_id,
                                                    io_subsystem::Message* response_message_for_send)
{
  return
    new BindRespWaitTimeoutEvent(*this, link_id, response_message_for_send);
}

}}}}
