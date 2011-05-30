#ifndef __UTIL_STRLCPY_H
# define __UTIL_STRLCPY_H 1

#include <string.h>
# ifdef linux
namespace {

  size_t strlcpy( char* dst, const char* src, size_t dstsize )
  {
    const size_t lsrc = strlen(src);
    if ( dstsize > 0 ) {
      const size_t l = std::min(lsrc,dstsize-1);
      memcpy( dst, src, l );
      dst[l] = '\0';
    }
    return lsrc;
  }

}
#endif

#endif /* __UTIL_LLTOSTR_H */
