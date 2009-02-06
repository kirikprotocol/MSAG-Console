#include "BufferedOutputStream.hpp"

namespace smpp_dmplx {

BufferedOutputStream::BufferedOutputStream(size_t sz) : SerializationBuffer(sz) {}

BufferedOutputStream::BufferedOutputStream(const BufferedOutputStream& rhs) : SerializationBuffer(rhs.getBufferSize())
{
  Write(rhs.getBuffer(),rhs.getBufferSize());
}

BufferedOutputStream&
BufferedOutputStream::operator=(const BufferedOutputStream& rhs)
{
  if ( this != &rhs ) {
    Write(rhs.getBuffer(),rhs.getBufferSize());
  }
  return *this;
}

BufferedOutputStream::~BufferedOutputStream() {}

void
BufferedOutputStream::writeUInt8(uint8_t arg)
{
  WriteByte(arg);
}

void
BufferedOutputStream::writeUInt16(uint16_t arg)
{
  WriteNetInt16(arg);
}

void
BufferedOutputStream::writeUInt32(uint32_t arg)
{
  WriteNetInt32(arg);
}

void
BufferedOutputStream::writeOpaqueData(const std::vector<uint8_t>& inputBuf)
{
  Write(&inputBuf[0],inputBuf.size());
}

BufferedOutputStream::operator void*()
{
  return getBuffer();
}

uint32_t
BufferedOutputStream::getSize()
{
  return getBufferSize();
}

}
