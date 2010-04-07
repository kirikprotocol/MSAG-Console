#include "MessagesHandlingDispatcher.hpp"
#include "MessageHandlers.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

void
SOGMessage_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlr;
  msgHndlr.handle(*this);
}

void
SORMessage_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlr;
  msgHndlr.handle(*this);
}

void
SSAMessage_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlr;
  msgHndlr.handle(*this);
}

void
SSCMessage_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlr;
  msgHndlr.handle(*this);
}

void
SSPMessage_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlr;
  msgHndlr.handle(*this);
}

void
SSTMessage_HandlingDispatcher::dispatch_handle()
{
  MessageHandlers msgHndlr;
  msgHndlr.handle(*this);
}

}}}}}
