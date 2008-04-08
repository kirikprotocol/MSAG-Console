#include <stdio.h>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "MErrorMessage.hpp"

namespace libsua_messages {

const uint32_t MErrorMessage::_MSG_CODE;

MErrorMessage::MErrorMessage()
  : LibsuaMessage(_MSG_CODE), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")),
    _errorCode(0), _originalMessage(NULL), _originalPacket(NULL)
{
  setLength(_MSGCODE_SZ + 
            sizeof(uint32_t) /*sizeof of error code*/);
}

MErrorMessage::MErrorMessage(uint32_t errCode, const LibsuaMessage* originalMessage)
  : LibsuaMessage(_MSG_CODE), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")),
    _errorCode(errCode), _originalMessage(originalMessage), _originalPacket(NULL)
{
  setLength(_MSGCODE_SZ + 
            sizeof(uint32_t) /*sizeof of error code*/);
}

MErrorMessage::MErrorMessage(uint32_t errCode, const communication::TP* originalPacket)
  : LibsuaMessage(_MSG_CODE), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")),
    _errorCode(errCode), _originalMessage(NULL), _originalPacket(originalPacket)
{
  setLength(_MSGCODE_SZ + 
            sizeof(uint32_t) /*sizeof of error code*/);
}

size_t
MErrorMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  offset = communication::addField(resultBuf, offset, _errorCode);

  const communication::TP *tp;
  communication::TP transportPacketForOriginalMessage;

  if ( _originalMessage ) {
    _originalMessage->serialize(&transportPacketForOriginalMessage);
    tp = &transportPacketForOriginalMessage;
  } else if ( _originalPacket )
    tp = _originalPacket;

  offset = communication::addField(resultBuf, offset, (uint32_t)tp->packetLen);
  offset = communication::addField(resultBuf, offset, tp->packetBody, tp->packetLen);
  smsc_log_info(_logger, "BindMessage::serialize::: message was transformed to packet. packetType=[%d],packetSize=[%d]", resultBuf->packetType, resultBuf->packetLen);
  return offset;
}

size_t
MErrorMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  uint16_t strLen;
  offset = extractField(packetBuf, offset, &_errorCode);
  smsc_log_info(_logger, "BindMessage::deserialize::: message was constructed", offset);
  return offset;
}

std::string
MErrorMessage::toString() const
{
  char errCodeStr[32];
  snprintf(errCodeStr, sizeof(errCodeStr), ", errCode=[%d]", _errorCode);
  return LibsuaMessage::toString() + std::string(errCodeStr);
}

const char*
MErrorMessage::getMsgCodeTextDescription() const
{
  return "M-ERROR";
}

}
