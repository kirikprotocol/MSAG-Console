#include <stdio.h>
#include "eyeline/utilx/hexdmp.hpp"
#include "FastParsableSmppMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

FastParsableSmppMessage::FastParsableSmppMessage(uint32_t commandId)
  : SMPPMessage(commandId), _rawMessageBody(NULL), _rawMessageBodyLen(0)
{}

size_t
FastParsableSmppMessage::serialize(io_subsystem::Packet* packet) const
{
  SMPPMessage::serialize(packet);
  if ( !_rawMessageBodyLen || !_rawMessageBody )
    generateSerializationException();
  return packet->addValue(_rawMessageBody, _rawMessageBodyLen);
}

size_t
FastParsableSmppMessage::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);
  _rawMessageBody = packet->packet_data + offset;
  _rawMessageBodyLen = static_cast<uint32_t>(packet->packet_data_len - offset);

  return packet->packet_data_len;
}

std::string
FastParsableSmppMessage::toString() const
{
  char message_dump[32*1024];

  int offset = sprintf(message_dump, ",dump of message body='");
  if ( _rawMessageBody && _rawMessageBodyLen ) {
    utilx::hexdmp(message_dump+offset, sizeof(message_dump)-offset, _rawMessageBody, _rawMessageBodyLen);
    offset += static_cast<int>(_rawMessageBodyLen << 1);
  }
  snprintf(message_dump + offset, sizeof(message_dump) - offset, "'");
  return SMPPMessage::toString() + message_dump;
}

uint32_t
FastParsableSmppMessage::calculateCommandBodyLength() const
{
  return _rawMessageBodyLen;
}

}}}}
