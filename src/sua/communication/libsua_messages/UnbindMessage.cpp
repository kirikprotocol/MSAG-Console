#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "UnbindMessage.hpp"

namespace libsua_messages {

const uint32_t UnbindMessage::_MSG_CODE;

UnbindMessage::UnbindMessage()
  : LibsuaMessage(_MSG_CODE) {}

size_t
UnbindMessage::serialize(communication::TP* resultBuf) const
{
  return LibsuaMessage::serialize(resultBuf);
}

size_t
UnbindMessage::deserialize(const communication::TP& packetBuf)
{
  return LibsuaMessage::deserialize(packetBuf);
}

std::string
UnbindMessage::toString() const
{
  return LibsuaMessage::toString();
}

const char*
UnbindMessage::getMsgCodeTextDescription() const
{
  return "UNBIND";
}

}
