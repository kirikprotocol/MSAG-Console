#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include "util/Exception.hpp"
#include "TP.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

size_t
addField(TP* packet, size_t position, uint8_t val)
{
  if ( position < packet->maxPacketLen ) {
    packet->packetBody[position] = val;
    if ( position + sizeof(val) > packet->packetLen )
      packet->packetLen = position + sizeof(val);

    return position + sizeof(val);
  } else
    throw smsc::util::Exception("addField::: can't add uint8_t type value - buffer overflow [position=%d,MAX_PACKET_SIZE=%d]", position, packet->maxPacketLen);
}

size_t
addField(TP* packet, size_t position, uint16_t val)
{
  if ( position + sizeof(val) <= packet->maxPacketLen ) {
    val = htons(val);
    memcpy(packet->packetBody + position,  (uint8_t*)&val, sizeof(val));
    if ( position + sizeof(val) > packet->packetLen )
      packet->packetLen = position + sizeof(val);

    return position + sizeof(val);
  } else
    throw smsc::util::Exception("addField::: can't add uint16_t type value - buffer overflow [position=%d,MAX_PACKET_SIZE=%d]", position, packet->maxPacketLen);
}

size_t
addField(TP* packet, size_t position, uint32_t val)
{
  if ( position + sizeof(val) <= packet->maxPacketLen ) {
    val = htonl(val);
    memcpy(packet->packetBody + position, (uint8_t*)&val, sizeof(val));
    if ( position + sizeof(val) > packet->packetLen )
      packet->packetLen = position + sizeof(val);

    return position + sizeof(val);
  } else
    throw smsc::util::Exception("addField::: can't add uint32_t type value of - buffer overflow [position=%d,MAX_PACKET_SIZE=%d]", position, packet->maxPacketLen);
}

size_t
addField(TP* packet, size_t position, const uint8_t* val, size_t valSz)
{
  if ( position + valSz <= packet->maxPacketLen ) {
    memcpy(packet->packetBody + position, val, valSz);
    if ( position + valSz > packet->packetLen )
      packet->packetLen = position + valSz;

    return position + valSz;
  } else
    throw smsc::util::Exception("addField::: can't add byte array value - buffer overflow [position=%d,array size=%d,MAX_PACKET_SIZE=%d]", position, valSz, packet->maxPacketLen);
}

size_t
extractField(const TP& packet_buf, size_t position, uint8_t* field_val)
{
  if ( position + sizeof(uint8_t) <= packet_buf.packetLen ) {
    *field_val = packet_buf.packetBody[position];
    return position + sizeof(uint8_t);
  } else
    throw smsc::util::Exception("extractField::: can't extract uint8_t type value - position is out of bounds [position=%d,packetLen=%d]", position, packet_buf.packetLen);
}

size_t
extractField(const TP& packet_buf, size_t position, uint16_t* field_val)
{
  if ( position + sizeof(uint16_t) <= packet_buf.packetLen ) {
    memcpy((uint8_t*)field_val, packet_buf.packetBody + position, sizeof(uint16_t));
    *field_val = ntohs(*field_val);
    return position + sizeof(uint16_t);
  } else
    throw smsc::util::Exception("extractField::: can't extract uint16_t type value - position is out of bounds [position=%d,packetLen=%d]", position, packet_buf.packetLen);
}

size_t
extractField(const TP& packet_buf, size_t position, uint32_t* field_val)
{
  if ( position + sizeof(uint32_t) <= packet_buf.packetLen ) {
    memcpy((uint8_t*)field_val, packet_buf.packetBody + position, sizeof(uint32_t));
    *field_val = ntohl(*field_val);
    return position + sizeof(uint32_t);
  } else
    throw smsc::util::Exception("extractField::: can't extract uint32_t type value - position is out of bounds [position=%d,packetLen=%d]", position, packet_buf.packetLen);
}

size_t
extractField(const TP& packet_buf, size_t position, uint8_t* field_val, size_t field_val_sz)
{
  if ( position + field_val_sz <= packet_buf.packetLen ) {
    memcpy(field_val, packet_buf.packetBody + position, field_val_sz);
    return position + field_val_sz;
  } else
    throw smsc::util::Exception("extractField::: can't extract byte array value - position is out of bounds [position=%d,fieldValSz=%d,packetLen=%d]", position, field_val_sz, packet_buf.packetLen);
}

}}}
