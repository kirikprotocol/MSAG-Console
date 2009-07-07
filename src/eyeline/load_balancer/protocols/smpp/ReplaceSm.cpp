#include <stdio.h>
#include <stdlib.h>

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "ReplaceSm.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

ReplaceSm::ReplaceSm()
  : SMPPMessage(_MSGCODE), _rawMessageBody(NULL), _rawMessageBodyLen(0),
    _messageIdCStringLen(0), _isSetMessageId(false)
{}

size_t
ReplaceSm::serialize(io_subsystem::Packet* packet) const
{
  SMPPMessage::serialize(packet);
  if ( !_rawMessageBodyLen || !_rawMessageBody )
    throw utilx::SerializationException("ReplaceSm::serialize::: message body wasn't set");
  return packet->addValue(_rawMessageBody, _rawMessageBodyLen);
}

size_t
ReplaceSm::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);
  _rawMessageBody = packet->packet_data + offset;
  _rawMessageBodyLen = static_cast<uint32_t>(packet->packet_data_len - offset);

  char message_id[65];
  size_t tmpOffset = offset;
  offset = extractVariableCOctetString(packet, offset, message_id, sizeof(message_id));
  _messageIdCStringLen = static_cast<uint32_t>(tmpOffset - offset);
  _messageId = strtoll(message_id, (char**)NULL, 10);

  if ( !_messageId && errno == ERANGE )
    throw utilx::DeserializationException("ReplaceSm::deserialize::: not numeric message id value='%s'", message_id);

  return packet->packet_data_len;
}

std::string
ReplaceSm::toString() const
{
  char message_dump[32*1024];

  int offset;
  if ( _isSetMessageId )
    offset = sprintf(message_dump, "messageId=%lld, dump of rest part of message body='", _messageId);
  else
    offset = sprintf(message_dump, ", dump of rest part of message body='");

  uint32_t rawMessageBodyLen = _rawMessageBodyLen - _messageIdCStringLen;
  if ( _rawMessageBody && rawMessageBodyLen ) {
    utilx::hexdmp(message_dump + offset, sizeof(message_dump) - offset, _rawMessageBody + _messageIdCStringLen, rawMessageBodyLen);
    offset += static_cast<int>(rawMessageBodyLen << 1);
  }
  snprintf(message_dump + offset, sizeof(message_dump) - offset, "'");
  return SMPPMessage::toString() + message_dump;
}

uint64_t
ReplaceSm::getMessageId() const
{
  if ( !_isSetMessageId )
    throw utilx::FieldNotSetException("ReplaceSm::getMessageId::: field is not set");
  return _messageId;
}

uint32_t
ReplaceSm::calculateCommandBodyLength() const
{
  return _rawMessageBodyLen;
}

}}}}
