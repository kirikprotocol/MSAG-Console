#include "ServiceMessageHandlers.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
ServiceMessageHandlers::handle(const CancelSm& message,
                               const io_subsystem::LinkId& src_link_id,
                               io_subsystem::IOProcessor& io_processor)
{
  forwardServiceRequestToConcreteSmsc(message, src_link_id, io_processor);
}

void
ServiceMessageHandlers::handle(const CancelSmResp& message,
                               const io_subsystem::LinkId& src_link_id,
                               io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

void
ServiceMessageHandlers::handle(const QuerySm& message,
                               const io_subsystem::LinkId& src_link_id,
                               io_subsystem::IOProcessor& io_processor)
{
  forwardServiceRequestToConcreteSmsc(message, src_link_id, io_processor);
}

void
ServiceMessageHandlers::handle(const QuerySmResp& message,
                               const io_subsystem::LinkId& src_link_id,
                               io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

void
ServiceMessageHandlers::handle(const ReplaceSm& message,
                               const io_subsystem::LinkId& src_link_id,
                               io_subsystem::IOProcessor& io_processor)
{
  forwardServiceRequestToConcreteSmsc(message, src_link_id, io_processor);
}

void
ServiceMessageHandlers::handle(const ReplaceSmResp& message,
                               const io_subsystem::LinkId& src_link_id,
                               io_subsystem::IOProcessor& io_processor)
{
  forwardMessageToSme(message, src_link_id, io_processor);
}

}}}}
