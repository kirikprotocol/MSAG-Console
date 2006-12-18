#include "EntityStorable_Helper.hpp"
#include <util/crc32.h>

EntityStorable_Helper::EntityStorable_Helper(uint8_t* serializeBuf, uint16_t serializeBufSz)
  : _serialize_buf(serializeBuf), _serialize_buf_size(serializeBufSz) {}

EntityStorable_Helper::~EntityStorable_Helper() { delete [] _serialize_buf; }

uint32_t
EntityStorable_Helper::calcCrc(uint32_t crc) const {
  return smsc::util::crc32(crc, _serialize_buf, _serialize_buf_size);
}

uint16_t
EntityStorable_Helper::getSize() const {
  return _serialize_buf_size;
}
