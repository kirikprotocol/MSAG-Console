#include "MessagesHandlingDispatcher.hpp"
#include "MessageHandlers.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

void
UDT_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlrs;
  msgHndlrs.setSequenceControl(getSLS());
  msgHndlrs.handle(*this);
}

void
UDTS_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlrs;
  msgHndlrs.handle(*this);
}

void
XUDT_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlrs;
  msgHndlrs.setSequenceControl(getSLS());
  msgHndlrs.handle(*this);
}

void
XUDTS_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlrs;
  msgHndlrs.handle(*this);
}

}}}}
