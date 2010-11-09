#include <stdio.h>
#include <string>
#include "hexdmp.hpp"

namespace eyeline {
namespace utilx {

std::string
hexdmp(const uint8_t* buf, size_t bufSz)
{
  char hexBuf[65535];
  return hexdmp(hexBuf, sizeof(hexBuf), buf, bufSz);
}

char*
hexdmp(char* dumpBuf, size_t dumpBufSz, const uint8_t* buf, size_t bufSz)
{
  int offset=0;
  for (size_t i=0; i<bufSz; ++i) {
    offset += snprintf(dumpBuf + offset, dumpBufSz - offset, "%02X", buf[i]);
    if (dumpBufSz - offset == 0)
      break;
  }
  return dumpBuf;
}

}}

