#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "AdaptationLayer_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

size_t
AdaptationLayer_Message::makeHeader(message_header_t message_header,
                                    msg_code_t sua_msg_code,
                                    uint32_t sua_message_body_size) const
{
  message_header[0] = PROTOCOL_VERSION;
  message_header[1] = RESERVED_FIELD;
  message_header[2] = (sua_msg_code & 0x0000FF00) >> 8; // message class
  message_header[3] = sua_msg_code & 0x000000FF; // message type

  uint32_t msgLen;
  msgLen = htonl(sua_message_body_size);
  memcpy(&message_header[4], reinterpret_cast<uint8_t*>(&msgLen), sizeof(msgLen));

  return HEADER_SIZE;
}

size_t
AdaptationLayer_Message::serialize(common::TP* result_buf) const
{
  result_buf->pkt_sctp_props.streamNo = getStreamNo();
  result_buf->pkt_sctp_props.orderingTransfer = getOrderDelivering();

  message_header_t messageHeader;
  makeHeader(messageHeader, _msgCode, getLength());
  result_buf->packetLen = 0;
  return common::addField(result_buf, 0, messageHeader, sizeof(messageHeader));
}

size_t
AdaptationLayer_Message::deserialize(const common::TP& packet_buf)
{
  uint8_t header[MESSAGE_PREAMBLE_SIZE];
  size_t offset = common::extractField(packet_buf, 0, header, sizeof(header));
  if ( header[0] != PROTOCOL_VERSION )
    throw smsc::util::Exception("AdaptationLayer_Message::deserialize::: unsupported protocol version [=%d]", header[0]);

  msg_code_t msgCode = (msg_code_t(header[2]) << 8) | header[3];
  if ( msgCode != getMsgCode() )
    throw smsc::util::Exception("AdaptationLayer_Message::deserialize::: wrong message code - expected message code[=0x%04X], has been gotten message code [=0x%04X]", getMsgCode(), msgCode);

  uint32_t len;
  offset = common::extractField(packet_buf, offset, &len);
  if ( packet_buf.packetLen != len )
    throw smsc::util::Exception("AdaptationLayer_Message::deserialize::: value of length in SUA message header [=%d] is not equal to size of transport packet [=%d]", len, packet_buf.packetLen);

  return offset;
}

std::string
AdaptationLayer_Message::toString() const
{
  char buf[64];
  msg_code_t msgcode = getMsgCode();
  
  snprintf(buf, sizeof(buf), "version=[%d],msg_class=[%d],msg_type=[%d],len=[%d]", msgcode >> 24, (msgcode >> 8) & 0xFF, msgcode & 0xFF, getLength());
  return std::string(buf);
}

const common::TLV_RoutingContext&
AdaptationLayer_Message::getRoutingContext() const
{
  if ( _routingContext.isSetValue() ) return _routingContext;
  else throw utilx::FieldNotSetException("AdaptationLayer_Message::getRoutingContext::: routingContext wasn't set");
}

}}}
