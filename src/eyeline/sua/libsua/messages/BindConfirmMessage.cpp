#include <stdio.h>
#include <eyeline/sua/communication/TP.hpp>
#include "BindConfirmMessage.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

const uint32_t BindConfirmMessage::_MSG_CODE;

BindConfirmMessage::BindConfirmMessage()
  : LibsuaMessage(_MSG_CODE), _status(BIND_OK)
{}

BindConfirmMessage::BindConfirmMessage(status_t status)
  : LibsuaMessage(_MSG_CODE), _status(status)
{}

size_t
BindConfirmMessage::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf, resultBufMaxSz);

  communication::TP tp(0, 0, resultBuf, resultBufMaxSz);

  return communication::addField(&tp, offset, _status);
}

size_t
BindConfirmMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf, packetBufSz);

  communication::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  return communication::extractField(tp, offset, &_status);
}

std::string
BindConfirmMessage::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), ",status=[%d]", _status);
  return LibsuaMessage::toString() + strBuf;
}

const char*
BindConfirmMessage::getMsgCodeTextDescription() const
{
  return "BIND_CONFIRM";
}

uint32_t
BindConfirmMessage::getStatus() const
{
  return _status;
}

void
BindConfirmMessage::setStatus(status_t status)
{
  _status = status;
}

uint32_t
BindConfirmMessage::getLength() const
{
  return LibsuaMessage::getLength() + sizeof(_status);
}

}}}
