#include <memory>

#include "ApplicationMessageHandlers.hpp"
#include "Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
ApplicationMessageHandlers::handle(const SubmitMulti& message,
                                   const io_subsystem::LinkId& src_link_id,
                                   io_subsystem::IOProcessor& io_processor)
{
  try {
    if ( message.isSetSarMsgRefNum() &&
        message.isSetSarSegmentSeqNum() &&
        message.isSetSarTotalSegments() )
      forwardMultipartMessageToSmsc(message, src_link_id, io_processor);
    else
      forwardMessageToSmsc(message, src_link_id, io_processor);
  } catch (InvalidMessageParamException& ex) {

    //
  } catch (...) {
    // ESME_RSUBMITFAIL
  }
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
  std::auto_ptr<SubmitSmResp> responseMessage;
  try {
    if ( message.isSetSarMsgRefNum() &&
        message.isSetSarSegmentSeqNum() &&
        message.isSetSarTotalSegments() )
      forwardMultipartMessageToSmsc(message, src_link_id, io_processor);
    else
      forwardMessageToSmsc(message, src_link_id, io_processor);
  } catch (InvalidMessageParamException& ex) {
    smsc_log_error(_logger, "ApplicationMessageHandlers::handle::: caught InvalidMessageParamException=[%s], can't handle SubmitSm message",
                   ex.what());
    responseMessage.reset(message.makeResponse(ESME_RINVOPTPARAMVAL));
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "ApplicationMessageHandlers::handle::: caught exception=[%s], can't handle SubmitSm message",
                   ex.what());
    responseMessage.reset(message.makeResponse(ESME_RSUBMITFAIL));
  }
  if ( responseMessage.get() )
    io_processor.getLink(src_link_id)->send(*responseMessage);
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
  std::auto_ptr<DataSmResp> responseMessage;
  try {
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
  } catch (InvalidMessageParamException& ex) {
    smsc_log_error(_logger, "ApplicationMessageHandlers::handle::: caught InvalidMessageParamException=[%s], can't handle DataSm message",
                   ex.what());
    responseMessage.reset(message.makeResponse(ESME_RINVOPTPARAMVAL));
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "ApplicationMessageHandlers::handle::: caught exception=[%s], can't handle DataSm message",
                   ex.what());
    responseMessage.reset(message.makeResponse(ESME_RSYSERR));
  }
  if ( responseMessage.get() )
    io_processor.getLink(src_link_id)->send(*responseMessage);
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
