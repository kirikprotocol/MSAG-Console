#include "BufferedInputStream.hpp"

namespace smpp_dmplx {

BufferedInputStream::BufferedInputStream(const std::vector<uint8_t>& dataBuf) : SerializationBuffer(0), _dataBuf(dataBuf)
{
  setExternalBuffer(&_dataBuf[0],_dataBuf.size());
}

BufferedInputStream::BufferedInputStream(const BufferedInputStream& rhs) : SerializationBuffer(0)
{
  _dataBuf=rhs._dataBuf;
  setExternalBuffer(&_dataBuf[0],_dataBuf.size());
}

BufferedInputStream&
BufferedInputStream::operator=(const BufferedInputStream& rhs)
{
  if ( this != &rhs ) {
    _dataBuf=rhs._dataBuf;
    setExternalBuffer(&_dataBuf[0],_dataBuf.size());
  }
  return *this;
}

BufferedInputStream::~BufferedInputStream() {}

uint8_t
BufferedInputStream::readUInt8()
{
  return ReadByte();
}

uint16_t
BufferedInputStream::readUInt16()
{
  return ReadNetInt16();
}

uint32_t
BufferedInputStream::readUInt32()
{
  return ReadNetInt32();
}

void
BufferedInputStream::readOpaqueData(std::vector<uint8_t>& buf, size_t readSz)
{
  buf.resize(readSz);
  Read(&buf[0],readSz);
}

size_t
BufferedInputStream::size() const
{
  return _dataBuf.size();
}

}
