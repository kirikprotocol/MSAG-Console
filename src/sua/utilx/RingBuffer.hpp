#ifndef __SUA_UTILX_RINGBUFFER_HPP__
# define __SUA_UTILX_RINGBUFFER_HPP__ 1

# include <sys/types.h>
# include <sua/corex/io/IOStreams.hpp>

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

}

# include <sua/utilx/RingBuffer_impl.hpp>

#endif
