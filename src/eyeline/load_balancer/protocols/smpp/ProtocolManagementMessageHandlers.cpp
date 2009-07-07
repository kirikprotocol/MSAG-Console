#include "ProtocolManagementMessageHandlers.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
ProtocolManagementMessageHandlers::handle(const AlertNotification& message,
                                          const io_subsystem::LinkId& src_link_id,
                                          io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

void
ProtocolManagementMessageHandlers::handle(const GenericNack& message,
                                          const io_subsystem::LinkId& src_link_id,
                                          io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::LinkId linkSetId, emptyLinkId;
  linkSetId = io_processor.getLinkSetIdOwnerForThisLink(src_link_id);
  if ( linkSetId == emptyLinkId ) {
    if ( message.getSequenceNumber() == 0 )
      throw smsc::util::Exception("ProtocolManagementMessageHandlers::handle::: can't forward GenericNack message to set of smsc with value of sequence number=0 ");
    forwardSmppResponseToSmsc(message, src_link_id, io_processor);
  } else
    forwardMessageToSme(message, src_link_id, io_processor);
}

}}}}
