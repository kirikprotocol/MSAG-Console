#include <stdio.h>
#include <stdlib.h>

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "CancelSm.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

CancelSm::CancelSm()
  : SMPPMessage(_MSGCODE), _rawMessageBody(NULL), _rawMessageBodyLen(0),
    _messageIdCStringLen(0), _isSetMessageId(false)
{}

size_t
CancelSm::serialize(io_subsystem::Packet* packet) const
{
  SMPPMessage::serialize(packet);
  if ( !_rawMessageBodyLen || !_rawMessageBody )
    throw utilx::SerializationException("CancelSm::serialize::: message body wasn't set");
  return packet->addValue(_rawMessageBody, _rawMessageBodyLen);
}

size_t
CancelSm::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);
  _rawMessageBody = packet->packet_data + offset;
  _rawMessageBodyLen = static_cast<uint32_t>(packet->packet_data_len - offset);

  char serviceType[6];
  offset = extractVariableCOctetString(packet, offset, serviceType, sizeof(serviceType)); //skip service type

  char message_id[65];
  size_t tmpOffset = offset;
  offset = extractVariableCOctetString(packet, offset, message_id, sizeof(message_id));
  _messageIdCStringLen = static_cast<uint32_t>(tmpOffset - offset);
  _messageId = strtoll(message_id, (char**)NULL, 10);

  if ( !_messageId && errno == ERANGE )
    throw utilx::DeserializationException("CancelSm::deserialize::: not numeric message id value='%s'", message_id);

  return packet->packet_data_len;
}

std::string
CancelSm::toString() const
{
  char message_dump[32*1024];

  int offset;
  if ( _isSetMessageId )
    offset = sprintf(message_dump, "messageId=%lld, dump of message body='", _messageId);
  else
    offset = sprintf(message_dump, ", dump of message body='");

  if ( _rawMessageBody && _rawMessageBodyLen ) {
    utilx::hexdmp(message_dump + offset, sizeof(message_dump) - offset, _rawMessageBody, _rawMessageBodyLen);
    offset += static_cast<int>(_rawMessageBodyLen << 1);
  }
  snprintf(message_dump + offset, sizeof(message_dump) - offset, "'");
  return SMPPMessage::toString() + message_dump;
}

uint64_t
CancelSm::getMessageId() const
{
  if ( !_isSetMessageId )
    throw utilx::FieldNotSetException("CancelSm::getMessageId::: field is not set");
  return _messageId;
}

uint32_t
CancelSm::calculateCommandBodyLength() const
{
  return _rawMessageBodyLen;
}

}}}}
