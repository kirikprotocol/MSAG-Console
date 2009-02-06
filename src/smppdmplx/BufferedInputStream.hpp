#ifndef __SMPPDMPLX_BUFFEREDINPUTSTREAM_HPP__
# define __SMPPDMPLX_BUFFEREDINPUTSTREAM_HPP__

# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>

# include <vector>
# include <string.h> // need for include <util/BufferSerialization.hppw>
# include <util/BufferSerialization.hpp>

namespace smpp_dmplx {

class BufferedInputStream : private smsc::util::SerializationBuffer {
public:
  BufferedInputStream(const std::vector<uint8_t>& dataBuf);
  BufferedInputStream(const BufferedInputStream& rhs);
  BufferedInputStream& operator=(const BufferedInputStream& rhs);

  virtual ~BufferedInputStream();

  virtual uint8_t readUInt8();

  virtual uint16_t readUInt16();

  virtual uint32_t readUInt32();

  virtual void readOpaqueData(std::vector<uint8_t>& buf, size_t readSz);

  virtual size_t size() const;
private:
  std::vector<uint8_t> _dataBuf;
};

}

#endif
