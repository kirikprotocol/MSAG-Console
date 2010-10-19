#ifndef _INFORMER_EXCEPTION_H
#define _INFORMER_EXCEPTION_H

#include "util/Exception.hpp"

static inline char* getStrerror(int err,char* buf,size_t buflen) {
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE>=600) && !_GNU_SOURCE
    strerror_r(err,buf,buflen);
    return buf;
#else
    return strerror_r(err,buf,buflen);
#endif
}
#define STRERROR(x,b,l) getStrerror(x,b,l)

namespace eyeline {
namespace informer {

class InfosmeException : public smsc::util::Exception
{
public:
    InfosmeException( const char* fmt, ... )
    {
        SMSC_UTIL_EX_FILL(fmt);
    }
};

}
}

#endif
