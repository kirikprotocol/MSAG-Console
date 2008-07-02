#include <stdio.h>
#include <string.h>

#include "BindMessage.hpp"

namespace libsua_messages {

const uint32_t BindMessage::_MSG_CODE;

BindMessage::BindMessage()
  : LibsuaMessage(_MSG_CODE)
{
  memset(_appId, 0, sizeof(_appId));
}

size_t
BindMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  offset = communication::addField(resultBuf, offset, uint8_t(PROTOCOL_VERSION));
  uint16_t strLen = strlen(_appId);
  offset = communication::addField(resultBuf, offset, strLen);
  return communication::addField(resultBuf, offset, (uint8_t*)_appId, strlen(_appId));
}

size_t
BindMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  uint8_t protocolVersion;
  offset = communication::extractField(packetBuf, offset, &protocolVersion);
  if ( protocolVersion != PROTOCOL_VERSION )
    throw smsc::util::Exception("BindMessage::deserialize::: wrong protocol version=[%d], expected version=[%d]", protocolVersion, PROTOCOL_VERSION);

  uint16_t strLen;
  offset = communication::extractField(packetBuf, offset, &strLen);
  if ( strLen + 1 > sizeof(_appId) )
    throw smsc::util::Exception("BindMessage::deserialize::: length of appId field [%d] exceeded max size [%d]", strLen, sizeof(_appId) -1);

  _appId[strLen] = 0;
  return communication::extractField(packetBuf, offset, (uint8_t*)_appId, strLen);
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

}
