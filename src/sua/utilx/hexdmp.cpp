#include <sys/types.h>
#include <string>
#include <sstream>
#include <iomanip>

std::string
hexdmp(const uchar_t* buf, uint32_t bufSz)
{
  std::ostringstream hexBuf;
  hexBuf.fill('0');
  hexBuf << std::hex;
  for (size_t i=0; i<bufSz; ++i)
    hexBuf << std::setw(2) << (uint32_t) buf[i];

  return hexBuf.str();
}
