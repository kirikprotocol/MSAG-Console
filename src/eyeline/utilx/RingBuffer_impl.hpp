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
RingBuffer<BUFSZ>::deque()
{
  if ( !_count ) throw smsc::util::Exception("RingBuffer<BUFSZ>::deque::: ring buffer is empty");
  uint8_t ret = _buffer[_head]; --_count;
  _head = static_cast<uint32_t>((_head + 1) % sizeof(_buffer));
  return ret;
}

template<int BUFSZ>
uint8_t
RingBuffer<BUFSZ>::readUint8()
{
  return deque();
}

template<int BUFSZ>
uint16_t
RingBuffer<BUFSZ>::readUint16()
{
  union {
    uint16_t val;
    uint8_t valBuf[sizeof(uint16_t)];
  } buf;

  buf.valBuf[0] = deque();
  buf.valBuf[1] = deque();
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

  buf.valBuf[0] = deque();
  buf.valBuf[1] = deque();
  buf.valBuf[2] = deque();
  buf.valBuf[3] = deque();
  
  return ntohl(buf.val);
}

template<int BUFSZ>
void
RingBuffer<BUFSZ>::readArray(uint8_t* destBuffer, size_t arraySz)
{
  for(int i=0; i < arraySz; ++i)
    destBuffer[i] = deque();
}

template<int BUFSZ>
void
RingBuffer<BUFSZ>::enque(uint8_t val)
{
  _buffer[_tail] = val;
  _tail = static_cast<uint32_t>((_tail + 1) % sizeof(_buffer));
  ++_count;
}

template<int BUFSZ>
void
RingBuffer<BUFSZ>::load(corex::io::InputStream* iStream)
{
  if ( _count < sizeof(_buffer) ) {
    uint8_t tmpBuf[sizeof(_buffer)];
    size_t numOfBytesToRead = sizeof(tmpBuf) - _count;
    ssize_t sz = iStream->read(tmpBuf, numOfBytesToRead);
//     smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_usr_cm");
//     smsc_log_info(logger, "RingBuffer<BUFSZ>::load::: iStream->read return [%s]", hexdmp(tmpBuf, sz).c_str());
    for (int i=0; i < sz; ++i)
      enque(tmpBuf[i]);
  }
}

}}

