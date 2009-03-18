#ifndef __EYELINE_UTILX_RINGBUFFER_HPP__
# define __EYELINE_UTILX_RINGBUFFER_HPP__

# include <sys/types.h>
# include <eyeline/corex/io/IOStreams.hpp>

namespace eyeline {
namespace utilx {

template <int BUFSZ>
class RingBuffer {
public:
  RingBuffer();
  size_t getSizeOfAvailData();

  uint8_t readUint8();
  uint16_t readUint16();
  uint32_t readUint32();
  void readArray(uint8_t* destBuffer, size_t arraySz);

  void load(corex::io::InputStream* buf);
private:
  uint8_t deque();
  void enque(uint8_t val);

  uint32_t _head, _tail, _count;
  uint8_t _buffer[BUFSZ];
};

}}

# include <eyeline/utilx/RingBuffer_impl.hpp>

#endif
