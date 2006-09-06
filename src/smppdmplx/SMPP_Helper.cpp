#include "SMPP_Helper.hpp"
#include <util/Exception.hpp>

#include <iomanip>
#include <sstream>

void
smpp_dmplx::SMPP_Helper::readCString(smpp_dmplx::BufferedInputStream& sourceBuf, std::string& value, size_t maxSize)
{
  char ch;
  while ( (ch = sourceBuf.readUInt8()) &&
          (value.size() < maxSize) )
    value += ch;

  if ( value.size() + 1 == maxSize )
    throw  smsc::util::Exception("smpp_dmplx::SMPP_Helper::readCString::: string size exceeded max length");

  return;
}

void
smpp_dmplx::SMPP_Helper::writeCString(smpp_dmplx::BufferedOutputStream& destBuf, const std::string& value, size_t maxSize)
{
  if ( value.length() + 1 <= maxSize ) {
    std::vector<uint8_t> tmpBuf(reinterpret_cast<const uint8_t*>(value.c_str()),
                                reinterpret_cast<const uint8_t*>(value.c_str()) + value.length() + 1);
    destBuf.writeOpaqueData(tmpBuf);
  } else
    throw  smsc::util::Exception("smpp_dmplx::SMPP_Helper::writeCString::: string size exceeded max length");

  return;
}

std::string
smpp_dmplx::SMPP_Helper::hexdmp(const std::vector<uint8_t>& buf)
{
  size_t sz = buf.size();
  std::ostringstream hexBuf;
  hexBuf.fill('0');
  hexBuf << std::hex;
  for (size_t i=0; i<sz; ++i)
    hexBuf << std::setw(2) << (uint32_t) buf[i];

  return hexBuf.str();
}
