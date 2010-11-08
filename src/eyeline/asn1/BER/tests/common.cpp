#include <sys/types.h>

#include "common.hpp"
#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

int
write_transfer_syntax(const void *buffer, size_t size, void *pattern_tr_syntax)
{
//  printf("DBG: write_transfer_syntax: size=%ld\n", size);
  if (size > MAX_PATTERN_LEN)
    return -1;
  size_t strLen= strlen((char*)pattern_tr_syntax);
  utilx::hexdmp(((char*)pattern_tr_syntax) + strLen, MAX_PATTERN_LEN - strLen,
                (const uint8_t*)buffer, size);
  return 0;
}

}}}}
