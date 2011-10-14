#include <sstream>
#include <iomanip>
#include <string>
#include <sys/types.h>

namespace smsc {
namespace mcisme {

std::string
hexdmp(const uint8_t* buf, size_t bufSz)
{
  std::ostringstream hexBuf;
  hexBuf.fill('0');
  hexBuf << std::hex;
  for (size_t i=0; i<bufSz; ++i)
    hexBuf << std::setw(2) << (uint32_t) buf[i];

  return hexBuf.str();
}

}}
