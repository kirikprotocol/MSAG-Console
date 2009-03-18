#include <stdlib.h>
#include <errno.h>
#include "strtol.hpp"

namespace eyeline {
namespace utilx {

long
strtol(const char *str, char **endptr, int base)
{
  errno = 0;
  long st = ::strtol(str, endptr, base);
  if ( !st && !errno ) {
    for (;*str != 0 || ( endptr && str < *endptr); ++str )
      if ( !( (*str >= '0' && *str <= '9') || *str == ' ') ) {
        errno = ERANGE; break;
      }
  }
  return st;
}

}}
