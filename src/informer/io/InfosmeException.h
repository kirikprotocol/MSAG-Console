#ifndef _INFORMER_EXCEPTION_H
#define _INFORMER_EXCEPTION_H

#include "util/Exception.hpp"

static inline char* getStrerror(int err,char* buf,size_t buflen) {
#ifdef linux
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE>=600) && !_GNU_SOURCE
    strerror_r(err,buf,buflen);
    return buf;
#else
    return strerror_r(err,buf,buflen);
#endif
#else
    snprintf(buf,buflen,"errno=%d",err);
    return buf;
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
protected:
    InfosmeException() {}
};


class ErrnoException : public InfosmeException
{
public:
    ErrnoException( int err, const char* fmt, ...)
    {
        SMSC_UTIL_EX_FILL(fmt);
        char ebuf[100];
        message += ": ";
        message += STRERROR(err,ebuf,sizeof(ebuf));
    }
};


class FileDataException : public InfosmeException
{
public:
    FileDataException( size_t pos, const char* fmt, ... ) :
    pos_(pos)
    {
        SMSC_UTIL_EX_FILL(fmt);
        char ebuf[30];
        sprintf(ebuf," at %llu",static_cast<unsigned long long>(pos));
        message += ebuf;
    }
    inline size_t getPos() const { return pos_; }

private:
    size_t pos_;
};

}
}

#endif
