#include <util/Exception.hpp>
#include <eyeline/sua/communication/TP.hpp>
#include "LibsuaMessage.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

LibsuaMessage::LibsuaMessage(uint32_t msgCode)
  : _msgLen(0), _msgCode(msgCode) { setLength(_MSGCODE_SZ); }

size_t
LibsuaMessage::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  communication::TP tp(0, 0, resultBuf, resultBufMaxSz);

  size_t offset = communication::addField(&tp, 0, getLength());
  return communication::addField(&tp, offset, getMsgCode());
}

size_t
LibsuaMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  communication::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  size_t offset = communication::extractField(tp, 0, &_msgLen);
  if ( _msgLen != tp.packetLen - sizeof(_msgLen) )
    throw smsc::util::Exception("LibsuaMessage::deserialize::: wrong value of message length field - expected message length[=0x%08X], has been gotten message length [=0x%08X]", tp.packetLen - sizeof(_msgLen), _msgLen);

  uint32_t msgCode;
  offset = communication::extractField(tp, offset, &msgCode);
  if ( msgCode != getMsgCode() )
    throw smsc::util::Exception("LibsuaMessage::deserialize::: wrong message code - expected message code[=0x%08X], has been gotten message code [=0x%08X]", getMsgCode(), msgCode);

  return offset;
}

std::string
LibsuaMessage::toString() const
{
  char buf[32];
  snprintf(buf, sizeof(buf), "msgcode=[0x%02X]", getMsgCode());
  return std::string(buf);
}

uint32_t
LibsuaMessage::getMsgCode() const
{
  return _msgCode;
}

uint32_t
LibsuaMessage::getLength() const
{
  return _msgLen;
}

void
LibsuaMessage::setLength(uint32_t msgLen)
{
  _msgLen = msgLen;
}

}}}
