#include <stdio.h>
#include <string.h>
#include <logger/Logger.h>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

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
  uint16_t strLen = strlen(_appId);
  offset = communication::addField(resultBuf, offset, strLen);
  return communication::addField(resultBuf, offset, (uint8_t*)_appId, strlen(_appId));
}

size_t
BindMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  uint16_t strLen;
  offset = extractField(packetBuf, offset, &strLen);
  if ( strLen + 1 > sizeof(_appId) ) throw smsc::util::Exception("BindMessage::deserialize::: length of appId field [%d] exceeded max size [%d]", strLen, sizeof(_appId) -1);
  _appId[strLen] = 0;
  return communication::extractField(packetBuf, offset, (uint8_t*)_appId, strLen);
}

std::string
BindMessage::toString() const
{
  return LibsuaMessage::toString() + ", appId=[" + _appId + "]";
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
  setLength(_MSGCODE_SZ + sizeof(uint16_t) /*sizeof of string len prefix*/ + 
            strlen(_appId));
}

}
