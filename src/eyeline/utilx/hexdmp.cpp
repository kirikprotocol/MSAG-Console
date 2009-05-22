#include <sys/types.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "hexdmp.hpp"

namespace eyeline {
namespace utilx {

std::string
hexdmp(const uchar_t* buf, size_t bufSz)
{
  std::ostringstream hexBuf;
  hexBuf.fill('0');
  hexBuf << std::hex;
  for (size_t i=0; i<bufSz; ++i)
    hexBuf << std::setw(2) << (uint32_t) buf[i];

  return hexBuf.str();
}

char*
hexdmp(char* dumpBuf, size_t dumpBufSz, const uchar_t* buf, size_t bufSz)
{
  int offset=0;
  for (size_t i=0; i<bufSz; ++i)
    offset = snprintf(dumpBuf + offset, dumpBufSz - offset, "%02X", buf[i]);

  return dumpBuf;
}

}}
