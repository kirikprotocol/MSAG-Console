#include "UnbindMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

const uint32_t UnbindMessage::_MSG_CODE;

UnbindMessage::UnbindMessage()
  : LibsccpMessage(_MSG_CODE) {}

size_t
UnbindMessage::serialize(common::TP* result_buf) const
{
  return LibsccpMessage::serialize(result_buf);
}

size_t
UnbindMessage::serialize(uint8_t* result_buf, size_t result_buf_max_sz) const
{
  common::TP tp(0, 0, result_buf, result_buf_max_sz);
  return serialize(&tp);
}

size_t
UnbindMessage::deserialize(const common::TP& packet_buf)
{
  return LibsccpMessage::deserialize(packet_buf);
}

size_t
UnbindMessage::deserialize(const uint8_t* packet_buf, size_t packet_buf_sz)
{
  common::TP tp(0, packet_buf_sz, const_cast<uint8_t*>(packet_buf), packet_buf_sz);
  return deserialize(tp);
}

std::string
UnbindMessage::toString() const
{
  return LibsccpMessage::toString();
}

const char*
UnbindMessage::getMsgCodeTextDescription() const
{
  return "UNBIND";
}

}}}
