#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <util/Exception.hpp>
#include "TP.hpp"

namespace communication {

size_t
addField(TP* packet, size_t position, uint8_t val)
{
  if ( position < TP::MAX_PACKET_SIZE ) {
    packet->packetBody[position] = val;
    if ( position + sizeof(val) > packet->packetLen )
      packet->packetLen = position + sizeof(val);

    return position + sizeof(val);
  } else
    throw smsc::util::Exception("addField::: can't add uint8_t type value - buffer overflow [position=%d,MAX_PACKET_SIZE=%d]", position, TP::MAX_PACKET_SIZE);
}

size_t
addField(TP* packet, size_t position, uint16_t val)
{
  if ( position + sizeof(val) <= TP::MAX_PACKET_SIZE ) {
    val = htons(val);
    memcpy(packet->packetBody + position,  (uint8_t*)&val, sizeof(val));
    if ( position + sizeof(val) > packet->packetLen )
      packet->packetLen = position + sizeof(val);

    return position + sizeof(val);
  } else
    throw smsc::util::Exception("addField::: can't add uint16_t type value - buffer overflow [position=%d,MAX_PACKET_SIZE=%d]", position, TP::MAX_PACKET_SIZE);
}

size_t
addField(TP* packet, size_t position, uint32_t val)
{
  if ( position + sizeof(val) <= TP::MAX_PACKET_SIZE ) {
    val = htonl(val);
    memcpy(packet->packetBody + position, (uint8_t*)&val, sizeof(val));
    if ( position + sizeof(val) > packet->packetLen )
      packet->packetLen = position + sizeof(val);

    return position + sizeof(val);
  } else
    throw smsc::util::Exception("addField::: can't add uint32_t type value of - buffer overflow [position=%d,MAX_PACKET_SIZE=%d]", position, TP::MAX_PACKET_SIZE);
}

size_t
addField(TP* packet, size_t position, const uint8_t* val, size_t valSz)
{
  if ( position + valSz <= TP::MAX_PACKET_SIZE ) {
    memcpy(packet->packetBody + position, val, valSz);
    if ( position + valSz > packet->packetLen )
      packet->packetLen = position + valSz;

    return position + valSz;
  } else
    throw smsc::util::Exception("addField::: can't add byte array value - buffer overflow [position=%d,array size=%d,MAX_PACKET_SIZE=%d]", position, valSz, TP::MAX_PACKET_SIZE);
}

size_t
extractField(const TP& packetBuf, size_t position, uint8_t* fieldVal)
{
  if ( position + sizeof(uint8_t) <= packetBuf.packetLen ) {
    *fieldVal = packetBuf.packetBody[position];
    return position + sizeof(uint8_t);
  } else
    throw smsc::util::Exception("extractField::: can't extract uint8_t type value - position is out of bounds [position=%d,packetLen=%d]", position, packetBuf.packetLen);
}

size_t
extractField(const TP& packetBuf, size_t position, uint16_t* fieldVal)
{
  if ( position + sizeof(uint16_t) <= packetBuf.packetLen ) {
    memcpy((uint8_t*)fieldVal, packetBuf.packetBody + position, sizeof(uint16_t));
    *fieldVal = ntohs(*fieldVal);
    return position + sizeof(uint16_t);
  } else
    throw smsc::util::Exception("extractField::: can't extract uint16_t type value - position is out of bounds [position=%d,packetLen=%d]", position, packetBuf.packetLen);
}

size_t
extractField(const TP& packetBuf, size_t position, uint32_t* fieldVal)
{
  if ( position + sizeof(uint32_t) <= packetBuf.packetLen ) {
    memcpy((uint8_t*)fieldVal, packetBuf.packetBody + position, sizeof(uint32_t));
    *fieldVal = ntohl(*fieldVal);
    return position + sizeof(uint32_t);
  } else
    throw smsc::util::Exception("extractField::: can't extract uint32_t type value - position is out of bounds [position=%d,packetLen=%d]", position, packetBuf.packetLen);
}

size_t
extractField(const TP& packetBuf, size_t position, uint8_t* fieldVal, size_t fieldValSz)
{
  if ( position + fieldValSz <= packetBuf.packetLen ) {
    memcpy(fieldVal, packetBuf.packetBody + position, fieldValSz);
    return position + fieldValSz;
  } else
    throw smsc::util::Exception("extractField::: can't extract byte array value - position is out of bounds [position=%d,fieldValSz=%d,packetLen=%d]", position, fieldValSz, packetBuf.packetLen);
}

}
