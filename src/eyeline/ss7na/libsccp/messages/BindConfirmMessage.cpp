#include <stdio.h>
#include "eyeline/ss7na/common/TP.hpp"
#include "BindConfirmMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

const uint32_t BindConfirmMessage::_MSG_CODE;

BindConfirmMessage::BindConfirmMessage()
  : LibsccpMessage(_MSG_CODE), _status(BIND_OK)
{}

BindConfirmMessage::BindConfirmMessage(status_t status)
  : LibsccpMessage(_MSG_CODE), _status(status)
{}

size_t
BindConfirmMessage::serialize(common::TP* result_buf) const
{
  size_t offset = LibsccpMessage::serialize(result_buf);
  return common::addField(result_buf, offset, _status);
}

size_t
BindConfirmMessage::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  common::TP tp(0, 0, resultBuf, resultBufMaxSz);
  return serialize(&tp);
}

size_t
BindConfirmMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = LibsccpMessage::deserialize(packet_buf);
  return offset = common::extractField(packet_buf, offset, &_status);
}

size_t
BindConfirmMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  common::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  return deserialize(tp);
}

std::string
BindConfirmMessage::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), ",status=[%d]", _status);
  return LibsccpMessage::toString() + strBuf;
}

uint32_t
BindConfirmMessage::getLength() const
{
  return LibsccpMessage::getLength() + static_cast<uint32_t>(sizeof(_status));
}

}}}
