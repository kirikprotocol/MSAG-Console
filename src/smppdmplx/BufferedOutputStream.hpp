#ifndef __SMPPDMPLX_BUFFEREDOUTPUTSTREAM_HPP__
# define __SMPPDMPLX_BUFFEREDOUTPUTSTREAM_HPP__

# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>

# include <vector>
# include <string.h> // need for include <util/BufferSerialization.hppw>
# include <util/BufferSerialization.hpp>

namespace smpp_dmplx {

class BufferedOutputStream : private smsc::util::SerializationBuffer {
public:
  BufferedOutputStream(size_t sz=0);
  BufferedOutputStream(const BufferedOutputStream&);
  BufferedOutputStream& operator=(const BufferedOutputStream&);

  virtual ~BufferedOutputStream();

  virtual void writeUInt8(uint8_t arg);

  virtual void writeUInt16(uint16_t arg);

  virtual void writeUInt32(uint32_t arg);

  virtual void writeOpaqueData(const std::vector<uint8_t>& buf);

  virtual operator void*();

  virtual uint32_t getSize();
};

}

#endif
