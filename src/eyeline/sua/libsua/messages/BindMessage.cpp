#include <stdio.h>
#include <string.h>
#include <util/Exception.hpp>
#include <eyeline/sua/communication/TP.hpp>
#include "BindMessage.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

const uint32_t BindMessage::_MSG_CODE;

BindMessage::BindMessage()
  : LibsuaMessage(_MSG_CODE)
{
  memset(_appId, 0, sizeof(_appId));
}

size_t
BindMessage::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf, resultBufMaxSz);

  communication::TP tp(0, 0, resultBuf, resultBufMaxSz);

  offset = communication::addField(&tp, offset, uint8_t(PROTOCOL_VERSION));
  uint16_t strLen = strlen(_appId);
  offset = communication::addField(&tp, offset, strLen);
  return communication::addField(&tp, offset, (uint8_t*)_appId, strlen(_appId));
}

size_t
BindMessage::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf, packetBufSz);

  communication::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  uint8_t protocolVersion;
  offset = communication::extractField(tp, offset, &protocolVersion);
  if ( protocolVersion != PROTOCOL_VERSION )
    throw smsc::util::Exception("BindMessage::deserialize::: wrong protocol version=[%d], expected version=[%d]", protocolVersion, PROTOCOL_VERSION);

  uint16_t strLen;
  offset = communication::extractField(tp, offset, &strLen);
  if ( strLen + 1 > sizeof(_appId) )
    throw smsc::util::Exception("BindMessage::deserialize::: length of appId field [%d] exceeded max size [%d]", strLen, sizeof(_appId) -1);

  _appId[strLen] = 0;
  return communication::extractField(tp, offset, (uint8_t*)_appId, strLen);
}

std::string
BindMessage::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), ",protocolVersion=[%d],appId=[%s]", getProtocolVersion(), _appId);
  return LibsuaMessage::toString() + strBuf;
}

const char*
BindMessage::getMsgCodeTextDescription() const
{
  return "BIND";
}

std::string
BindMessage::getAppId() const
{
  return std::string(_appId);
}

void
BindMessage::setAppId(const std::string& appId)
{
  if ( appId.size() + 1 > sizeof(_appId) ) throw smsc::util::Exception("BindMessage::setAppId::: length of appId value [%d] exceeded max size [%d]", appId.size(), sizeof(_appId)-1);
  strcpy(_appId, appId.c_str());
}

uint8_t
BindMessage::getProtocolVersion() const
{
  return PROTOCOL_VERSION;
}

uint32_t
BindMessage::getLength() const
{
  return LibsuaMessage::getLength() + VERSION_FIELD_SZ + sizeof(uint16_t) /*sizeof of string len prefix*/ + strlen(_appId);
}

}}}
