#include "SessionManagementMessageHandlers.hpp"
#include "eyeline/load_balancer/io_subsystem/TimeoutEvent.hpp"
#include "eyeline/load_balancer/io_subsystem/TimeoutMonitor.hpp"
#include "UnbindRespWaitTimeoutEvent.hpp"
#include "SmeRegistry.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
SessionManagementMessageHandlers::handle(const BindReceiver& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  processBindRequest(message, src_link_id, io_processor);
}

void
SessionManagementMessageHandlers::handle(const BindReceiverResp& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  processBindResponse(message, src_link_id, io_processor);
}

void
SessionManagementMessageHandlers::handle(const BindTransceiver& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  processBindRequest(message, src_link_id, io_processor);
}

void
SessionManagementMessageHandlers::handle(const BindTransceiverResp& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  processBindResponse(message, src_link_id, io_processor);
}

void
SessionManagementMessageHandlers::handle(const BindTransmitter& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  processBindRequest(message, src_link_id, io_processor);
}

void
SessionManagementMessageHandlers::handle(const BindTransmitterResp& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  processBindResponse(message, src_link_id, io_processor);
}

void
SessionManagementMessageHandlers::handle(const EnquireLink& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  io_processor.getLink(src_link_id)->send(message.makeResponse(ESME_ROK));
}

void
SessionManagementMessageHandlers::handle(const EnquireLinkResp& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  throw smsc::util::Exception("SessionManagementMessageHandlers::handle::: EnquireLinkResp message isn't accepted");
}

void
SessionManagementMessageHandlers::handle(const Unbind& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::LinkRefPtr link = io_processor.getLink(src_link_id);
  if ( !link.Get() )
    throw smsc::util::Exception("SessionManagementMessageHandlers::handle::: there isn't link for id=%s",
                                src_link_id.toString().c_str());

  io_subsystem::LinkId smscLinksetId;
  io_subsystem::SwitchingTable::getInstance().removeSwitching(src_link_id, &smscLinksetId);
  io_processor.getReconnector().cancelReconnectionAttempts(smscLinksetId);
  io_processor.getBinder().removeBindingInfo(smscLinksetId);

  link->send(message.makeResponse(ESME_ROK));
  SmeRegistry::getInstance().deleteSmeInfo(src_link_id);
  io_processor.removeIncomingLink(src_link_id);

  io_subsystem::LinkSetRefPtr linkSet = io_processor.getLinkSet(smscLinksetId);
  if ( !linkSet.Get() )
    throw smsc::util::Exception("SessionManagementMessageHandlers::handle::: there isn't linkset for id=%s",
                                smscLinksetId.toString().c_str());
  io_subsystem::LinkSet::LinkSetIterator linksetIterator = linkSet->getIterator();
  while ( linksetIterator.hasElement() ) {
    linksetIterator.getCurrentElement().send(message);
    const io_subsystem::LinkId& currentLinkId =
      linksetIterator.getCurrentElement().getLinkId();

    io_subsystem::TimeoutMonitor::timeout_id_t timeoutId = "UNBND:" + currentLinkId.toString();
    io_subsystem::TimeoutMonitor::getInstance().scheduleTimeout(timeoutId,
                                                                linksetIterator.getCurrentElement().getUnbindRespWaitTimeout(),
                                                                new UnbindRespWaitTimeoutEvent(io_processor, currentLinkId)
                                                                );
    linksetIterator.next();
  }
}

void
SessionManagementMessageHandlers::handle(const UnbindResp& message,
                                         const io_subsystem::LinkId& src_link_id,
                                         io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::TimeoutMonitor::timeout_id_t timeoutId = "UNBND:" + src_link_id.toString();
  io_subsystem::TimeoutMonitor::getInstance().cancelTimeout(timeoutId);
  io_subsystem::LinkId linkSetId = io_processor.removeOutcomingLink(src_link_id, true);
  io_subsystem::LinkId emptyLinkId;
  if ( linkSetId != emptyLinkId )
    io_processor.removeLinkSet(linkSetId);
}

}}}}
