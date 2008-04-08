#include <util/Exception.hpp>
#include <sua/communication/sua_messages/CLDTMessage.hpp>
#include <sua/communication/sua_messages/ErrorMessage.hpp>

#include "LibsuaMessage.hpp"
#include "EncapsulatedSuaMessage.hpp"

namespace libsua_messages {

LibsuaMessage::LibsuaMessage(uint32_t msgCode)
  : _msgLen(0), _msgCode(msgCode) { setLength(_MSGCODE_SZ); }

const sua_messages::SUAMessage*
LibsuaMessage::getContainedSuaMessage() const
{
  return NULL;
}

size_t
LibsuaMessage::serialize(communication::TP* resultBuf) const
{
  resultBuf->packetLen = 0;
  size_t offset = communication::addField(resultBuf, 0, getLength());
  return communication::addField(resultBuf, offset, getMsgCode());
}

size_t
LibsuaMessage::deserialize(const communication::TP& packetBuf)
{
  uint32_t msgCode;

  size_t offset = communication::extractField(packetBuf, 0, &_msgLen);
  if ( _msgLen != packetBuf.packetLen - sizeof(_msgLen) )
    throw smsc::util::Exception("LibsuaMessage::deserialize::: wrong value of message length field - expected message length[=0x%08X], has been gotten message length [=0x%08X]", packetBuf.packetLen - sizeof(_msgLen), _msgLen);

  offset = communication::extractField(packetBuf, offset, &msgCode);
  if ( msgCode != getMsgCode() )
    throw smsc::util::Exception("LibsuaMessage::deserialize::: wrong message code - expected message code[=0x%08X], has been gotten message code [=0x%08X]", getMsgCode(), msgCode);

  return offset;
}

std::string
LibsuaMessage::toString() const
{
  char buf[32];
  snprintf(buf, sizeof(buf), "msgcode=[0x%x]", getMsgCode());
  return std::string(buf);
}

uint32_t
LibsuaMessage::getMsgCode() const
{
  return _msgCode;
}

uint16_t
LibsuaMessage::getStreamNo() const { throw smsc::util::Exception("LibsuaMessage::getStreamNo::: SCTP streams not supported"); }

unsigned int
LibsuaMessage::getMessageIndex(uint32_t msgCode)
{
  if ( msgCode == 0  )
    throw smsc::util::Exception("LibsuaMessage::getMessageIndex::: invalid msgCode value [=0]");

  if ( msgCode > MAX_MESSAGE_CODE_VALUE )
    throw smsc::util::Exception("LibsuaMessage::getMessageIndex::: msgCode value [=0x%x] is greater than max allowable value [=0x%x]", msgCode, MAX_MESSAGE_CODE_VALUE);

  unsigned int idx = _messagesCodeToMessageIdx[msgCode - 1];
  if ( idx == 0 )
    throw smsc::util::Exception("LibsuaMessage::getMessageIndex::: unknown msgCode value [=0x%x]", msgCode);

  return idx;
}

void
LibsuaMessage::registerMessageCode(uint32_t msgCode)
{
  if ( msgCode == 0  )
    throw smsc::util::Exception("LibsuaMessage::registerMessageCode::: invalid msgCode value [=0]");

  if ( msgCode > MAX_MESSAGE_CODE_VALUE )
    throw smsc::util::Exception("LibsuaMessage::registerMessageCode::: msgCode value [=0x%x] is greater than max allowable value [=0x%x]", msgCode, MAX_MESSAGE_CODE_VALUE);

  _messagesCodeToMessageIdx[msgCode-1] = generateMessageIndexValue();
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

unsigned int
LibsuaMessage::_messagesCodeToMessageIdx[MAX_MESSAGE_CODE_VALUE];

}
