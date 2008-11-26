#ifndef __SCAG_UTIL_LLTOSTR_H
# define __SCAG_UTIL_LLTOSTR_H 1

# ifdef __GNUC__
#include <string.h>
#include <sstream>
namespace {

  char* lltostr( long long int v, char* endptr ) 
  {
    std::ostringstream s;
    s << v;
    const std::string ss = s.str();
    endptr -= ss.size();
    ::memcpy( endptr, ss.c_str(), ss.size() );
    return endptr;
  }

}
#else
#include <stdlib.h>
#endif

#endif /* __SCAG_UTIL_LLTOSTR_H */
