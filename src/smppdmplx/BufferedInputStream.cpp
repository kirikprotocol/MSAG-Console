#include "BufferedInputStream.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::BufferedInputStream::BufferedInputStream(const std::vector<uint8_t>& dataBuf) : SerializationBuffer(0), _dataBuf(dataBuf)
{
  smsc_log_debug(dmplxlog,"BufferedInputStream::BufferedInputStream::: _dataBuf.size=%d",_dataBuf.size());
  setExternalBuffer(&_dataBuf[0],_dataBuf.size());
  smsc_log_debug(dmplxlog,"BufferedInputStream::BufferedInputStream::: Leave it");
}

smpp_dmplx::BufferedInputStream::BufferedInputStream(const BufferedInputStream& rhs) : SerializationBuffer(0)
{
  _dataBuf=rhs._dataBuf;
  setExternalBuffer(&_dataBuf[0],_dataBuf.size());
}

smpp_dmplx::BufferedInputStream&
smpp_dmplx::BufferedInputStream::operator=(const BufferedInputStream& rhs)
{
  if ( this != &rhs ) {
    _dataBuf=rhs._dataBuf;
    setExternalBuffer(&_dataBuf[0],_dataBuf.size());
  }
  return *this;
}

smpp_dmplx::BufferedInputStream::~BufferedInputStream() {}

uint8_t
smpp_dmplx::BufferedInputStream::readUInt8()
{
  return ReadByte();
}

uint16_t
smpp_dmplx::BufferedInputStream::readUInt16()
{
  return ReadNetInt16();
}

uint32_t
smpp_dmplx::BufferedInputStream::readUInt32()
{
  return ReadNetInt32();
}

void
smpp_dmplx::BufferedInputStream::readOpaqueData(std::vector<uint8_t>& buf, size_t readSz)
{
  buf.resize(readSz);
  Read(&buf[0],readSz);
}

size_t
smpp_dmplx::BufferedInputStream::size() const
{
  return _dataBuf.size();
}
