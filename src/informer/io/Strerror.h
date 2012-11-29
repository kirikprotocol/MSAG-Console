#ifndef _INFORMER_STRERROR_H
#define _INFORMER_STRERROR_H

#include <stdio.h>
#include <string.h>

static inline char* getStrerror(int err,char* buf,size_t buflen) 
{
#if defined(linux) || defined(__MACH__)
// #if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE>=600) && !_GNU_SOURCE
    strerror_r(err,buf,buflen);
    return buf;
// #else
//     return strerror_r(err,buf,buflen);
// #endif
#else
    snprintf(buf,buflen,"errno=%d",err);
    return buf;
#endif
}

#define STRERROR(x,b,l) getStrerror(x,b,l)

#endif
