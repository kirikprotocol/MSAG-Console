#include <stdio.h>
#include <string.h>

#include "eyeline/utilx/Exception.hpp"

#include "eyeline/ss7na/common/TP.hpp"
#include "BindMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

const uint32_t BindMessage::_MSG_CODE;

BindMessage::BindMessage()
  : LibsccpMessage(_MSG_CODE), _numberOfSSN(0)
{
  _appId[0] = 0;
}

size_t
BindMessage::serialize(common::TP* result_buf) const
{
  size_t offset = LibsccpMessage::serialize(result_buf);
  offset = common::addField(result_buf, offset, uint8_t(PROTOCOL_VERSION));
  uint16_t strLen = static_cast<uint16_t>(strlen(_appId));
  offset = common::addField(result_buf, offset, strLen);
  offset = common::addField(result_buf, offset, (uint8_t*)_appId, strlen(_appId));

  if ( !_numberOfSSN )
    throw utilx::SerializationException("BindMessage::serialize::: mandatory field 'NumberOfSSN' is absent");
  offset = common::addField(result_buf, offset, _numberOfSSN);
  for(unsigned i=0; i < _numberOfSSN; i++)
    offset = common::addField(result_buf, offset, _ssn[i]);
  return offset;
}

size_t
BindMessage::serialize(uint8_t* result_buf, size_t result_buf_max_sz) const
{
  common::TP tp(0, 0, result_buf, result_buf_max_sz);
  return serialize(&tp);
}

size_t
BindMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = LibsccpMessage::deserialize(packet_buf);
  uint8_t protocolVersion;
  offset = common::extractField(packet_buf, offset, &protocolVersion);
  if ( protocolVersion != PROTOCOL_VERSION )
    throw smsc::util::Exception("BindMessage::deserialize::: wrong protocol version=[%d], expected version=[%d]", protocolVersion, PROTOCOL_VERSION);

  uint16_t strLen;
  offset = common::extractField(packet_buf, offset, &strLen);
  if ( strLen + 1 > sizeof(_appId) )
    throw smsc::util::Exception("BindMessage::deserialize::: length of appId field [%d] exceeded max size [%d]", strLen, sizeof(_appId) -1);

  _appId[strLen] = 0;

  offset = common::extractField(packet_buf, offset, (uint8_t*)_appId, strLen);
  offset = common::extractField(packet_buf, offset, &_numberOfSSN);
  if ( !_numberOfSSN )
    throw utilx::SerializationException("BindMessage::deserialize::: mandatory list of 'SSN' is absent");
  for(unsigned i = 0; i < _numberOfSSN; i++)
    offset = common::extractField(packet_buf, offset, &_ssn[i]);

  return offset;
}

size_t
BindMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  common::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  return deserialize(tp);
}

std::string
BindMessage::toString() const
{
  char strBuf[1024];
  int offset = snprintf(strBuf, sizeof(strBuf), ",protocolVersion=[%d],appId=[%s]",
                        getProtocolVersion(), _appId);
  for (unsigned i=0; i < _numberOfSSN; ++i) {
    offset += snprintf(strBuf + offset, sizeof(strBuf) - offset, ",ssn_%d=%u", i, _ssn[i]);
  }
  return LibsccpMessage::toString() + strBuf;
}

void
BindMessage::setAppId(const std::string& appId)
{
  if ( appId.size() + 1 > sizeof(_appId) ) throw smsc::util::Exception("BindMessage::setAppId::: length of appId value [%d] exceeded max size [%d]", appId.size(), sizeof(_appId)-1);
  strcpy(_appId, appId.c_str());
}

uint32_t
BindMessage::getLength() const
{
  return LibsccpMessage::getLength() +
    static_cast<uint32_t>(VERSION_FIELD_SZ +
                          sizeof(uint16_t) /*sizeof of string len prefix*/ + strlen(_appId) +
                          sizeof(_numberOfSSN) + sizeof(_numberOfSSN) * _numberOfSSN);
}

}}}
