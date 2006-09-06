#ifndef __SMPPDMPLX_RAWMESSAGE_HPP__
# define __SMPPDMPLX_RAWMESSAGE_HPP__ 1

# include "BufferedInputStream.hpp"
# include <sys/types.h>

namespace smpp_dmplx {

/*
** The class responsibility is to grant interface for reading
** raw message in parts from network.
*/
class RawMessage {
public:
  RawMessage();
  ~RawMessage();
  bool haveReadLenght() const;
  void readDataLength(int fd);
  bool haveReadRawData() const;
  void readRawData(int fd);
  BufferedInputStream getRawData() const;
private:
  bool _haveReadlength, _haveReadRawData;

  typedef union {
    uint8_t _bufForLength[4];
    uint32_t _messageLength;
  } LenType;

  LenType _rawMessageLen;
  uint8_t* _bufForRawMessage;

  size_t _bytesWasRead;
};

}
#endif
