#include "MessagesHandlingDispatcher.hpp"
#include "MessageHandlers.hpp"

namespace sua_user_communication {

void
BindMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

void
N_UNITDATA_REQ_Message_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

void
UnbindMessage_HandlingDispatcher::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

}
