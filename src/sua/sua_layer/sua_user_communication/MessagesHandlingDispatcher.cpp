#include <sua/communication/libsua_messages/BindMessage.hpp>
#include <sua/communication/libsua_messages/EncapsulatedSuaMessage.hpp>
#include <sua/communication/libsua_messages/MErrorMessage.hpp>
#include <sua/communication/libsua_messages/UnbindMessage.hpp>

#include "MessageHandlers.hpp"

namespace libsua_messages {

void
BindMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

void
EncapsulatedSuaMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

void
MErrorMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

void
UnbindMessage::dispatch_handle(const communication::LinkId& linkId) const
{
  sua_user_communication::MessageHandlers::getInstance().handle(*this, linkId);
}

}
