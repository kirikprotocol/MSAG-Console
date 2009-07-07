#include "ApplicationMessageHandlers.hpp"
#include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
ApplicationMessageHandlers::handle(const SubmitMulti& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  if ( message.isSetSarMsgRefNum() &&
       message.isSetSarSegmentSeqNum() &&
       message.isSetSarTotalSegments() )
    forwardMultipartMessageToSmsc(message, src_link_id, io_processor);
  else
    forwardMessageToSmsc(message, src_link_id, io_processor);
}

void
ApplicationMessageHandlers::handle(const SubmitMultiResp& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

void
ApplicationMessageHandlers::handle(const SubmitSm& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  if ( message.isSetSarMsgRefNum() &&
       message.isSetSarSegmentSeqNum() &&
       message.isSetSarTotalSegments() )
    forwardMultipartMessageToSmsc(message, src_link_id, io_processor);
  else
    forwardMessageToSmsc(message, src_link_id, io_processor);
}

void
ApplicationMessageHandlers::handle(const SubmitSmResp& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

void
ApplicationMessageHandlers::handle(const DeliverSm& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

void
ApplicationMessageHandlers::handle(const DeliverSmResp& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  forwardSmppResponseToSmsc(message, src_link_id, io_processor);

}

void
ApplicationMessageHandlers::handle(const DataSm& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::LinkId linkSetId, emptyLinkId;
  linkSetId = io_processor.getLinkSetIdOwnerForThisLink(src_link_id);
  if ( linkSetId == emptyLinkId ) {
    if ( message.isSetSarMsgRefNum() &&
         message.isSetSarSegmentSeqNum() &&
         message.isSetSarTotalSegments() )
      forwardMultipartMessageToSmsc(message, src_link_id, io_processor);
    else
      forwardMessageToSmsc(message, src_link_id, io_processor);
  } else
    forwardMessageToSme(message, src_link_id, io_processor);
}

void
ApplicationMessageHandlers::handle(const DataSmResp& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::LinkId linkSetId, emptyLinkId;
  linkSetId = io_processor.getLinkSetIdOwnerForThisLink(src_link_id);
  if ( linkSetId == emptyLinkId )
    forwardSmppResponseToSmsc(message, src_link_id, io_processor);
  else
    forwardMessageToSme(message, src_link_id, io_processor);
}

}}}}
