#include "BufferedOutputStream.hpp"

smpp_dmplx::BufferedOutputStream::BufferedOutputStream(size_t sz) : SerializationBuffer(sz) {}

smpp_dmplx::BufferedOutputStream::BufferedOutputStream(const BufferedOutputStream& rhs) : SerializationBuffer(rhs.getBufferSize())
{
  Write(rhs.getBuffer(),rhs.getBufferSize());
}

smpp_dmplx::BufferedOutputStream&
smpp_dmplx::BufferedOutputStream::operator=(const BufferedOutputStream& rhs)
{
  if ( this != &rhs ) {
    Write(rhs.getBuffer(),rhs.getBufferSize());
  }
  return *this;
}

smpp_dmplx::BufferedOutputStream::~BufferedOutputStream() {}

void
smpp_dmplx::BufferedOutputStream::writeUInt8(uint8_t arg)
{
  WriteByte(arg);
}

void
smpp_dmplx::BufferedOutputStream::writeUInt16(uint16_t arg)
{
  WriteNetInt16(arg);
}

void
smpp_dmplx::BufferedOutputStream::writeUInt32(uint32_t arg)
{
  WriteNetInt32(arg);
}

void
smpp_dmplx::BufferedOutputStream::writeOpaqueData(const std::vector<uint8_t>& inputBuf)
{
  Write(&inputBuf[0],inputBuf.size());
}

smpp_dmplx::BufferedOutputStream::operator void*()
{
  return getBuffer();
}

uint32_t
smpp_dmplx::BufferedOutputStream::getSize()
{
  return getBufferSize();
}
