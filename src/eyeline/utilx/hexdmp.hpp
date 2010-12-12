#ifndef __EYELINECOM_UTILX_HEXDMP_HPP__
# define __EYELINECOM_UTILX_HEXDMP_HPP__

# include <inttypes.h>
# include <string>

namespace eyeline {
namespace utilx {

std::string
hexdmp(const uint8_t* buf, size_t bufSz);

char*
hexdmp(char* dumpBuf, size_t dumpBufSz, const uint8_t* buf, size_t bufSz);

size_t
hexbuf2bin(const char* hex_val, uint8_t* buf, size_t buf_sz);

}}

#endif
