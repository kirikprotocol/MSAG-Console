#include <stdio.h>
#include <util/Exception.hpp>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "SUAMessage.hpp"

namespace sua_messages {

unsigned int SUAMessage::_msgClassTypeToMessageIdx[MAX_MSG_CLASS_VALUE+1][MAX_MSG_TYPE_VALUE+1];

SUAMessage::SUAMessage(msg_code_t msgCode)
  : _msgCode(msgCode), _streamNo(0)
{}

size_t
SUAMessage::makeHeader(message_header_t messageHeader, msg_code_t suaMsgCode, uint32_t suaMessageBodySize)
{
  messageHeader[0] = PROTOCOL_VERSION;
  messageHeader[1] = RESERVED_FIELD;
  messageHeader[2] = (suaMsgCode & 0x0000FF00) >> 8; // message class
  messageHeader[3] = suaMsgCode & 0x000000FF; // message type

  uint32_t msgLen;
  msgLen = htonl(suaMessageBodySize);
  memcpy(&messageHeader[4], reinterpret_cast<uint8_t*>(&msgLen), sizeof(msgLen));

  return HEADER_SIZE;
}

size_t
SUAMessage::serialize(communication::TP* resultBuf) const
{
  resultBuf->pkt_sctp_props.streamNo = getStreamNo();
  resultBuf->pkt_sctp_props.orderingTransfer = getOrderDelivering();

  message_header_t messageHeader;
  makeHeader(messageHeader, _msgCode, getLength());
  resultBuf->packetLen = 0;
  return communication::addField(resultBuf, 0, messageHeader, sizeof(messageHeader));
}

size_t
SUAMessage::deserialize(const communication::TP& packetBuf)
{
  uint8_t header[MESSAGE_PREAMBLE_SIZE];
  size_t offset = communication::extractField(packetBuf, 0, header, sizeof(header));
  if ( header[0] != PROTOCOL_VERSION )
    throw smsc::util::Exception("SUAMessage::deserialize::: unsupported protocol version [=%d]", header[0]);

  msg_code_t msgCode = (msg_code_t(header[2]) << 8) | header[3];
  if ( msgCode != getMsgCode() )
    throw smsc::util::Exception("SUAMessage::deserialize::: wrong message code - expected message code[=0x%04X], has been gotten message code [=0x%04X]", getMsgCode(), msgCode);

  uint32_t len;
  offset = communication::extractField(packetBuf, offset, &len);
  if ( packetBuf.packetLen != len )
    throw smsc::util::Exception("SUAMessage::deserialize::: value of length in SUA message header [=%d] is not equal to size of transport packet [=%d]", len, packetBuf.packetLen);

  return offset;
}

std::string
SUAMessage::toString() const
{
  char buf[64];
  msg_code_t msgcode = getMsgCode();
  
  snprintf(buf, sizeof(buf), "version=[%d],msg_class=[%d],msg_type=[%d],len=[%d]", msgcode >> 24, (msgcode >> 8) & 0xFF, msgcode & 0xFF, getLength());
  return std::string(buf);
}

uint16_t
SUAMessage::getStreamNo() const
{ return _streamNo; }

void
SUAMessage::setStreamNo(uint16_t streamNo)
{ _streamNo = streamNo; }

SUAMessage::msg_code_t
SUAMessage::getMsgCode() const
{
  return _msgCode;
}

bool
SUAMessage::getOrderDelivering() const
{
  return false;
}

const uint8_t
SUAMessage::RESERVED_FIELD;

const uint8_t
SUAMessage::PROTOCOL_VERSION;

void
SUAMessage::registerMessageCode(msg_code_t msgCode)
{
  uint8_t msgClass = (msgCode & 0x0000FF00) >> 8, msgType = msgCode & 0x000000FF;

  if ( msgClass > MAX_MSG_CLASS_VALUE ) throw smsc::util::Exception("SUAMessage::registerMessageCode::: msgClass value [=0x%x] is greater than max allowable value [=0x%x]", msgClass, MAX_MSG_CLASS_VALUE);
  if ( msgType > MAX_MSG_TYPE_VALUE) throw smsc::util::Exception("SUAMessage::registerMessageCode::: msgType value [=0x%x] is greater than max allowable value [=0x%x]", msgType, MAX_MSG_TYPE_VALUE);
  _msgClassTypeToMessageIdx[msgClass][msgType] = generateMessageIndexValue();
}

unsigned int
SUAMessage::getMessageIndex(msg_code_t msgCode)
{
  uint8_t msgClass = (msgCode & 0x0000FF00) >> 8, msgType = msgCode & 0x000000FF;

  if ( msgClass > MAX_MSG_CLASS_VALUE ) throw smsc::util::Exception("SUAMessage::getMessageIndex::: msgClass value [=0x%x] is greater than max allowable value [=0x%x]", msgClass, MAX_MSG_CLASS_VALUE);
  if ( msgType > MAX_MSG_TYPE_VALUE) throw smsc::util::Exception("SUAMessage::getMessageIndex::: msgType value [=0x%x] is greater than max allowable value [=0x%x]", msgType, MAX_MSG_TYPE_VALUE);

  unsigned int idx = _msgClassTypeToMessageIdx[msgClass][msgType];
  if ( idx == 0 )
    throw smsc::util::Exception("SUAMessage::getMessageIndex::: unknown msgCode value [=0x%x]", msgCode);

  return idx;
}

const TLV_Address&
SUAMessage::getDestinationAddress() const
{
  return _destinationAddress;
}

const TLV_Address&
SUAMessage::getSourceAddress() const
{
  return _sourceAddress;
}

}

