#include "UnbindMessage.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

const uint32_t UnbindMessage::_MSG_CODE;

UnbindMessage::UnbindMessage()
  : LibsuaMessage(_MSG_CODE) {}

size_t
UnbindMessage::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  return LibsuaMessage::serialize(resultBuf, resultBufMaxSz);
}

size_t
UnbindMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  return LibsuaMessage::deserialize(packetBuf, packetBufSz);
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

}}}
