#include <stdio.h>

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "SubmitSm.hpp"
#include "OptionalParameter.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

SubmitSm::SubmitSm()
  : SMPPMessage(_MSGCODE), _rawMessageBody(NULL), _rawMessageBodyLen(0),
    _isSetSarMsgRefNum(false), _isSetSarTotalSegments(false), _isSetSarSegmentSeqNum(false) {}

size_t
SubmitSm::serialize(io_subsystem::Packet* packet) const
{
  SMPPMessage::serialize(packet);
  if ( !_rawMessageBodyLen || !_rawMessageBody )
    throw utilx::SerializationException("SubmitSm::serialize::: message body wasn't set");
  return packet->addValue(_rawMessageBody, _rawMessageBodyLen);
}

size_t
SubmitSm::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);
  _rawMessageBody = packet->packet_data + offset;
  _rawMessageBodyLen = static_cast<uint32_t>(packet->packet_data_len - offset);

  char serviceType[6];
  offset = extractVariableCOctetString(packet, offset, serviceType, sizeof(serviceType));
  offset += 2; // skip source_addr_ton and source_addr_npi
  char address[21];
  offset = extractVariableCOctetString(packet, offset, address, sizeof(address)); // skip source address
  offset += 2; // skip dest_addr_ton and dest_addr_npi
  offset = extractVariableCOctetString(packet, offset, address, sizeof(address)); // skip destination address
  offset += 3; //skip esm_class, protocol_id, priority_flag
  char scheduleDeliveryTime[17];
  offset = extractVariableCOctetString(packet, offset, scheduleDeliveryTime, sizeof(scheduleDeliveryTime));
  char validityPeriod[17];
  offset = extractVariableCOctetString(packet, offset, validityPeriod, sizeof(validityPeriod));
  offset += 4; //skip registered_delivery, replace_if_present_flag, data_coding, sm_default_msg_id
  uint8_t smLength;
  offset = packet->extractValue(&smLength, offset);
  offset += smLength;

  // process optional parameters
  uint16_t tag;
  OptionalParameter<uint16_t, 0x020C> sarMsgRefNumTLV;
  OptionalParameter<uint8_t, 0x020E> sarTotalSegmentsTLV;
  OptionalParameter<uint8_t, 0x020F> sarSegmentSeqnumTLV;
  while(offset < packet->packet_data_len) {
    if ( sarMsgRefNumTLV.isSetValue() && sarTotalSegmentsTLV.isSetValue() && sarSegmentSeqnumTLV.isSetValue() )
      break;

    packet->extractValue(&tag, offset);
    if ( tag == sarMsgRefNumTLV.getTag() ) {
      offset = sarMsgRefNumTLV.deserialize(packet, offset);
      _sarMsgRefNum = sarMsgRefNumTLV.getValue(); _isSetSarMsgRefNum = true;
    } else if ( tag == sarTotalSegmentsTLV.getTag() ) {
      offset = sarTotalSegmentsTLV.deserialize(packet, offset);
      _sarTotalSegments = sarTotalSegmentsTLV.getValue(); _isSetSarTotalSegments = true;
    } else if ( tag == sarSegmentSeqnumTLV.getTag() ) {
      offset = sarSegmentSeqnumTLV.deserialize(packet, offset);
      _sarSegmentSeqNum = sarSegmentSeqnumTLV.getValue(); _isSetSarSegmentSeqNum = true;
    }
  }

  return packet->packet_data_len;
}

std::string
SubmitSm::toString() const
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

uint16_t
SubmitSm::getSarMsgRefNum() const
{
  if ( !_isSetSarMsgRefNum )
    throw utilx::FieldNotSetException("SubmitSm::getSarMsgRefNum::: field is not set");
  return _sarMsgRefNum;
}

uint8_t
SubmitSm::getSarTotalSegments() const
{
  if ( !_isSetSarTotalSegments )
    throw utilx::FieldNotSetException("SubmitSm::getSarTotalSegments::: field is not set");
  return _sarTotalSegments;
}

uint8_t
SubmitSm::getSarSegmentSeqNum() const
{
  if ( !_isSetSarSegmentSeqNum )
    throw utilx::FieldNotSetException("SubmitSm::getSarSegmentSeqNum::: field is not set");
  return _sarSegmentSeqNum;
}

SubmitSmResp*
SubmitSm::makeResponse(uint32_t command_status) const
{
  SubmitSmResp* submitSmResp = new SubmitSmResp();
  submitSmResp->setCommandStatus(command_status);
  submitSmResp->setSequenceNumber(getSequenceNumber());

  return submitSmResp;
}

uint32_t
SubmitSm::calculateCommandBodyLength() const
{
  return _rawMessageBodyLen;
}

}}}}
