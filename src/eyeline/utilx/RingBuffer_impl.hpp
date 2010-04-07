# include <string.h>
# include <util/Exception.hpp>

namespace eyeline {
namespace utilx {

template<int BUFSZ>
RingBuffer<BUFSZ>::RingBuffer() 
  : _head(0), _tail(0), _count(0) {
  memset(_buffer, 0 , sizeof(_buffer));
}

template<int BUFSZ>
size_t
RingBuffer<BUFSZ>::getSizeOfAvailData()
{
  return _count;
}

template<int BUFSZ>
uint8_t
RingBuffer<BUFSZ>::readUint8()
{
  uint8_t valBuf[1];
  readArray(valBuf, sizeof(valBuf));
  return valBuf[0];
}

template<int BUFSZ>
uint16_t
RingBuffer<BUFSZ>::readUint16()
{
  union {
    uint16_t val;
    uint8_t valBuf[sizeof(uint16_t)];
  } buf;

  readArray(buf.valBuf, sizeof(buf.valBuf));
  return ntohs(buf.val);
}

template<int BUFSZ>
uint32_t
RingBuffer<BUFSZ>::readUint32()
{
  union {
    uint32_t val;
    uint8_t valBuf[sizeof(uint32_t)];
  } buf;

  readArray(buf.valBuf, sizeof(buf.valBuf));
  return ntohl(buf.val);
}

template<int BUFSZ>
void
RingBuffer<BUFSZ>::readArray(uint8_t* destBuffer, size_t arraySz)
{
  if ( !arraySz )
    return;
  if ( !_count )
    throw smsc::util::Exception("RingBuffer<BUFSZ>::readArray::: ring buffer is empty");
  size_t bytesToEnd = sizeof(_buffer) - _head;
  if ( arraySz < bytesToEnd ) {
    memcpy(destBuffer, _buffer + _head, arraySz);
    _head += static_cast<unsigned>(arraySz);
  } else {
    memcpy(destBuffer, _buffer + _head, bytesToEnd);
    memcpy(destBuffer, _buffer, arraySz - bytesToEnd);
    _head = static_cast<unsigned>(arraySz - bytesToEnd);
  }
  _count -= static_cast<unsigned>(arraySz);
}

template<int BUFSZ>
void
RingBuffer<BUFSZ>::load(corex::io::InputStream* iStream)
{
  if ( _count < sizeof(_buffer) ) {
    uint8_t tmpBuf[sizeof(_buffer)];
    ssize_t sz = iStream->read(tmpBuf, sizeof(tmpBuf) - _count);

    size_t bytesToEnd = sizeof(_buffer) - _tail;
    if ( bytesToEnd < sz ) {
      memcpy(_buffer + _tail, tmpBuf, bytesToEnd);
      memcpy(_buffer, tmpBuf + bytesToEnd, sz - bytesToEnd);
      _tail = static_cast<unsigned>(sz - bytesToEnd);
    } else {
      memcpy(_buffer + _tail, tmpBuf, sz);
      _tail += static_cast<unsigned>(sz);
    }
    _count += static_cast<unsigned>(sz);
  }
}

}}

