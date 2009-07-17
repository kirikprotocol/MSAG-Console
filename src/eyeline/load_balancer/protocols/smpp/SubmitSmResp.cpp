#include <stdio.h>
#include "SubmitSmResp.hpp"
#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

size_t
SubmitSmResp::serialize(io_subsystem::Packet* packet) const
{
  size_t offset = SMPPMessage::serialize(packet);

  if ( _rawMessageBodyLen && _rawMessageBody )
    return packet->addValue(_rawMessageBody, _rawMessageBodyLen);

  return offset;
}

size_t
SubmitSmResp::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);

  if ( getCommandLength() > SMPPMessage::SMPP_HEADER_SZ ) {
    _rawMessageBody = packet->packet_data + offset;
    _rawMessageBodyLen = static_cast<uint32_t>(packet->packet_data_len - offset);
  }

  return packet->packet_data_len;
}

std::string
SubmitSmResp::toString() const
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
SubmitSmResp::calculateCommandBodyLength() const
{
  return _rawMessageBodyLen;
}

}}}}
