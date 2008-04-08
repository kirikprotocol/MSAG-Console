#include <util/Exception.hpp>

#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>
#include <sua/communication/sua_messages/SUAMessage.hpp>
#include <sua/communication/sua_messages/ErrorMessage.hpp>
#include <sua/communication/sua_messages/CLDTMessage.hpp>

#include "EncapsulatedSuaMessage.hpp"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace libsua_messages {

EncapsulatedSuaMessage::EncapsulatedSuaMessage(uint32_t msgCode)
  : LibsuaMessage(msgCode), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")),
    _suaMessage(NULL), _encapsulatedMessagePtr(NULL), _encapsulatedMessageSz(0)
{
  if ( msgCode < _MIN_MSG_CODE ||
       msgCode > _MAX_MSG_CODE )
    throw smsc::util::Exception("EncapsulatedSuaMessage::EncapsulatedSuaMessage::: unexpected msgCode=[%d]", msgCode);
  _encapsulatedMessagePtr = _encapsulatedMessageTP.packetBody + sua_messages::SUAMessage::HEADER_SIZE;
  smsc_log_debug(_logger, "EncapsulatedSuaMessage::EncapsulatedSuaMessage::: created message object, msgCode for created LibsuaMessage=[%d]", getMsgCode());
}

uint32_t
EncapsulatedSuaMessage::produceLibsuaMsgCode(uint32_t suaMsgCode)
{
  uint32_t libSuaMsgCode;
  if ( suaMsgCode == sua_messages::CLDTMessage().getMsgCode() )
    libSuaMsgCode = ENCAPSULATED_SUA_CLDT_MESSAGE_CODE;
  else if ( suaMsgCode == sua_messages::ErrorMessage().getMsgCode() )
    libSuaMsgCode = ENCAPSULATED_SUA_ERROR_MESSAGE_CODE;
  else
    throw smsc::util::Exception("EncapsulatedSuaMessage::produceLibsuaMsgCode::: unexpected encapsulated sua message [message code=%d]", suaMsgCode);

  return libSuaMsgCode;
}

EncapsulatedSuaMessage::EncapsulatedSuaMessage(const sua_messages::SUAMessage* suaMessage)
  : LibsuaMessage(produceLibsuaMsgCode(suaMessage->getMsgCode())),
    _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _suaMessage(suaMessage)
{
  suaMessage->serialize(&_encapsulatedMessageTP);
  _encapsulatedMessagePtr = _encapsulatedMessageTP.packetBody + sua_messages::SUAMessage::HEADER_SIZE;
  _encapsulatedMessageSz = _encapsulatedMessageTP.packetLen - sua_messages::SUAMessage::HEADER_SIZE;
  setLength(_MSGCODE_SZ + _encapsulatedMessageSz);
  smsc_log_debug(_logger, "EncapsulatedSuaMessage::EncapsulatedSuaMessage::: created message object. source suaMessage=[%s], msgCode for created LibsuaMessage=[%d]", suaMessage->toString().c_str(), getMsgCode());
}

size_t
EncapsulatedSuaMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  return communication::addField(resultBuf, offset, _encapsulatedMessagePtr, _encapsulatedMessageSz);
}

uint32_t
EncapsulatedSuaMessage::produceSuaMsgCode(uint32_t libsuaMsgCode)
{
  if ( libsuaMsgCode == 0x01 ) return sua_messages::ErrorMessage().getMsgCode();
  if ( libsuaMsgCode == 0x09 ) return sua_messages::CLDTMessage().getMsgCode();
  throw smsc::util::Exception("EncapsulatedSuaMessage::produceSuaMsgCode::: can't determine corresponding sua message code for libsuaMsgCode=[%d]", libsuaMsgCode);
}

size_t
EncapsulatedSuaMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);

  smsc_log_debug(_logger, "EncapsulatedSuaMessage::deserialize::: LibsuaMessage::deserialize returned offset=%d", offset);

  _encapsulatedMessageSz = getLength() - _MSGCODE_SZ;

  offset = communication::extractField(packetBuf, offset, _encapsulatedMessagePtr, _encapsulatedMessageSz);

  sua_messages::SUAMessage::msg_code_t suaMsgCode = produceSuaMsgCode(getMsgCode());

  _encapsulatedMessageTP.packetLen = sua_messages::SUAMessage::HEADER_SIZE + _encapsulatedMessageSz;
  sua_messages::SUAMessage::makeHeader(_encapsulatedMessageTP.packetBody, suaMsgCode, _encapsulatedMessageTP.packetLen);
  _encapsulatedMessageTP.packetType = sua_messages::SUAMessage::getMessageIndex(suaMsgCode);

  smsc_log_debug(_logger, "EncapsulatedSuaMessage::deserialize::: _encapsulatedMessageTP.packetLen=[%d],_encapsulatedMessageTP.packetType=[%d],sua message dump(header+body)=[%s]", _encapsulatedMessageTP.packetLen, _encapsulatedMessageTP.packetType, hexdmp(_encapsulatedMessageTP.packetBody, _encapsulatedMessageTP.packetLen).c_str());

  sua_messages::SUAMessage* encapsulatedSuaMessage = static_cast<sua_messages::SUAMessage*>(io_dispatcher::SuaLayerMessagesFactory::getInstance().instanceMessage(_encapsulatedMessageTP.packetType));
  if ( encapsulatedSuaMessage ) {
    encapsulatedSuaMessage->deserialize(_encapsulatedMessageTP);
    setEncapsulatedSuaMessage(encapsulatedSuaMessage);
    //    smsc_log_info(_logger, "EncapsulatedSuaMessage::deserialize::: _encapsulatedMessage=[%s]", encapsulatedSuaMessage->toString().c_str());
  } else {
    _encapsulatedMessagePtr = NULL;
    throw smsc::util::Exception("EncapsulatedSuaMessage::deserialize::: can't instantiate message with type=[%d]", _encapsulatedMessageTP.packetType);
  }

  return offset;
}

std::string
EncapsulatedSuaMessage::toString() const
{
  return LibsuaMessage::toString() + ", encapsulated SUA message=[" + getContainedSuaMessage()->toString() + "]";
}

const char*
EncapsulatedSuaMessage::getMsgCodeTextDescription() const
{
  return "ENCAPSULATED-SUA-MESSAGE";
}

const sua_messages::SUAMessage*
EncapsulatedSuaMessage::getContainedSuaMessage() const
{
  return _suaMessage;
}

void
EncapsulatedSuaMessage::setEncapsulatedSuaMessage(const sua_messages::SUAMessage* suaMessage)
{
  _suaMessage = suaMessage;
}

}
