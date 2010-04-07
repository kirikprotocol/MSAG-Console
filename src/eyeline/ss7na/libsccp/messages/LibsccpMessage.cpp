#include "util/Exception.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

LibsccpMessage::LibsccpMessage(uint32_t msgCode)
  : _msgLen(0), _msgCode(msgCode) { setLength(static_cast<uint32_t>(_MSGCODE_SZ)); }

size_t
LibsccpMessage::serialize(common::TP* result_buf) const
{
  result_buf->packetLen = 0;
  size_t offset = common::addField(result_buf, 0, getLength());
  return common::addField(result_buf, offset, getMsgCode());
}

size_t
LibsccpMessage::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  common::TP tp(0, 0, resultBuf, resultBufMaxSz);
  return serialize(&tp);
}

size_t
LibsccpMessage::deserialize(const common::TP& packet_buf)
{
  uint32_t msgCode;

  size_t offset = common::extractField(packet_buf, 0, &_msgLen);
  if ( _msgLen != packet_buf.packetLen - sizeof(_msgLen) )
    throw smsc::util::Exception("LibsuaMessage::deserialize::: wrong value of message length field - expected message length[=0x%08X], has been gotten message length [=0x%08X]", packet_buf.packetLen - sizeof(_msgLen), _msgLen);

  offset = common::extractField(packet_buf, offset, &msgCode);
  if ( msgCode != getMsgCode() )
    throw smsc::util::Exception("LibsuaMessage::deserialize::: wrong message code - expected message code[=0x%08X], has been gotten message code [=0x%08X]", getMsgCode(), msgCode);

  return offset;
}

size_t
LibsccpMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  common::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  return deserialize(tp);
}

std::string
LibsccpMessage::toString() const
{
  char buf[32];
  snprintf(buf, sizeof(buf), "msgcode=[0x%02X]", getMsgCode());
  return std::string(buf);
}

}}}
