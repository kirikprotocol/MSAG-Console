#ifndef __SCAG_UTIL_LLTOSTR_H
# define __SCAG_UTIL_LLTOSTR_H 1

# ifdef __GNUC__
#include <string.h>
#include <stdio.h>

namespace {

  char* lltostr( long long int v, char* endptr ) 
  {
      char buf[50];
      ::snprintf(buf, 50,"%lld",v);
      const size_t len = strlen(buf);
      endptr -= len;
      ::memcpy( endptr, buf, len );
      return endptr;
  }

}
#else
#include <stdlib.h>
#endif

#endif /* __SCAG_UTIL_LLTOSTR_H */
