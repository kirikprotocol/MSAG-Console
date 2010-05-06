#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "eyeline/utilx/Exception.hpp"
#include "Packet.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

size_t
Packet::addValue(uint8_t value)
{
  if ( packet_data_len + sizeof(value) > MAX_PACKET_LEN )
    throw utilx::SerializationException("Packet::addValue(uint8_t)::: result packet's data is too long - max packet size=%d, packet_data_len=%d, value size=%d", MAX_PACKET_LEN, packet_data_len, sizeof(value));

  packet_data[packet_data_len] = value;
  packet_data_len += sizeof(value);

  return packet_data_len;
}

size_t
Packet::addValue(uint16_t value)
{
  if ( packet_data_len + sizeof(value) > MAX_PACKET_LEN )
    throw utilx::SerializationException("Packet::addValue(uint16_t)::: result packet's data is too long - max packet size=%d, packet_data_len=%d, value size=%d", MAX_PACKET_LEN, packet_data_len, sizeof(value));

  value = htons(value);
  memcpy(packet_data + packet_data_len, reinterpret_cast<uint8_t*>(&value), sizeof(value));
  packet_data_len += sizeof(value);

  return packet_data_len;
}

size_t
Packet::addValue(uint32_t value)
{
  if ( packet_data_len + sizeof(value) > MAX_PACKET_LEN )
    throw utilx::SerializationException("Packet::addValue(uint32_t)::: result packet's data is too long - max packet size=%d, packet_data_len=%d, value size=%d", MAX_PACKET_LEN, packet_data_len, sizeof(value));

  value = htonl(value);
  memcpy(packet_data + packet_data_len, reinterpret_cast<uint8_t*>(&value), sizeof(value));
  packet_data_len += sizeof(value);

  return packet_data_len;
}

size_t
Packet::addValue(const uint8_t* value, size_t value_length)
{
  if ( !value_length )
    return packet_data_len;

  if ( packet_data_len + value_length > MAX_PACKET_LEN )
    throw utilx::SerializationException("Packet::addValue(byte array)::: result packet's data is too long - max packet size=%d, packet_data_len=%d, value size=%d", MAX_PACKET_LEN, packet_data_len, value_length);

  memcpy(packet_data + packet_data_len, value, value_length);
  packet_data_len += value_length;

  return packet_data_len;
}

size_t
Packet::extractValue(uint8_t* value, size_t offset) const
{
  if ( !isSufficientSpace(offset, sizeof(uint8_t)) )
    throw utilx::DeserializationException("Packet::extractValue(uint8_t)::: packet boundary violation - offset=%d,packet_data_len=%d", offset, packet_data_len);

  *value = packet_data[offset];
  return offset + sizeof(uint8_t);
}

size_t
Packet::extractValue(uint16_t* value, size_t offset) const
{
  if ( !isSufficientSpace(offset, sizeof(uint16_t)) )
    throw utilx::DeserializationException("Packet::extractValue(uint16_t)::: packet boundary violation - offset=%d,size of extractable field=%d,packet_data_len=%d", offset, sizeof(uint16_t), packet_data_len);

  memcpy(reinterpret_cast<uint8_t*>(value), packet_data + offset, sizeof(uint16_t));
  *value = ntohs(*value);
  return offset + sizeof(uint16_t);
}

size_t
Packet::extractValue(uint32_t* value, size_t offset) const
{
  if ( !isSufficientSpace(offset, sizeof(uint32_t)) )
    throw utilx::DeserializationException("Packet::extractValue(uint32_t)::: packet boundary violation - offset=%d,size of extractable field=%d,packet_data_len=%d", offset, sizeof(uint32_t), packet_data_len);

  memcpy(reinterpret_cast<uint8_t*>(value), packet_data + offset, sizeof(uint32_t));
  *value = ntohl(*value);
  return offset + sizeof(uint32_t);
}

bool
Packet::isSufficientSpace(size_t offset, size_t required_size) const
{
  if ( offset + required_size > packet_data_len )
    return false;
  else
    return true;
}

}}}
