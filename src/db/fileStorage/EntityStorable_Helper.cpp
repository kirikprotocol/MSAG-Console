#include "EntityStorable_Helper.hpp"
#include <util/crc32.h>
#include <string.h>

EntityStorable_Helper::EntityStorable_Helper(uint8_t* serializeBuf, uint16_t serializeBufSz)
  : _serialize_buf(serializeBuf), _serialize_buf_size(serializeBufSz) {}

EntityStorable_Helper::EntityStorable_Helper(const EntityStorable_Helper& rhs)
{
  make_copy(rhs);
}

EntityStorable_Helper&
EntityStorable_Helper::operator=(const EntityStorable_Helper& rhs)
{
  if ( this != &rhs ) {
    delete [] _serialize_buf;
    make_copy(rhs);
  }
  return *this;
}

void
EntityStorable_Helper::make_copy(const EntityStorable_Helper& rhs)
{
  if ( rhs._serialize_buf_size > 0 && rhs._serialize_buf ) {
    _serialize_buf_size = rhs._serialize_buf_size;
    _serialize_buf = new uint8_t [_serialize_buf_size];
    memcpy(_serialize_buf, rhs._serialize_buf, _serialize_buf_size);
  }
}

EntityStorable_Helper::~EntityStorable_Helper() { delete [] _serialize_buf; }

uint32_t
EntityStorable_Helper::calcCrc(uint32_t crc) const {
  return smsc::util::crc32(crc, _serialize_buf, _serialize_buf_size);
}

uint16_t
EntityStorable_Helper::getSize() const {
  return _serialize_buf_size;
}
