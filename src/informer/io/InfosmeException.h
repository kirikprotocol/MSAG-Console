#ifndef _INFORMER_EXCEPTION_H
#define _INFORMER_EXCEPTION_H

#include "util/Exception.hpp"

static inline char* getStrerror(int err,char* buf,size_t buflen) {
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

namespace eyeline {
namespace informer {

typedef enum
{
    EXC_GENERIC = 1,

    EXC_DLVLIMITEXCEED,    // number of deliveries exceeded for the user
    EXC_NOTFOUND,          // item not found by id
    EXC_ALREADYEXIST,      // item already exists

    EXC_BADNAME,           // invalid/too long  userid/smscid/etc.
    EXC_SYSTEM,            // system call/libc failure/no resources

    EXC_LOGICERROR,        // program logic error
    EXC_CONFIG,            // configuration problem
    EXC_IOERROR,           // conversion problem (text, binary data)
    EXC_BADFILE,           // corrupted file
    EXC_NOTIMPL,           // not implemented
    EXC_NOTAUTH,           // not authorized
    EXC_ACCESSDENIED,      // access denied
    EXC_BADADDRESS,        // invalid srd/dst address
    EXC_EXPIRED,           // request/resource expired
    EXC_BADFORMAT          // parsing failed (in alm)
} ErrorCode;


class InfosmeException : public smsc::util::Exception
{
public:
    InfosmeException( ErrorCode code, const char* fmt, ... ) :
    code_(code)
    {
        SMSC_UTIL_EX_FILL(fmt);
    }
    ~InfosmeException() throw() {}

    ErrorCode getCode() const { return code_; }

protected:
    InfosmeException() {}

    ErrorCode code_;
};


class ErrnoException : public InfosmeException
{
public:
    ErrnoException( int err, const char* fmt, ...)
    {
        code_ = EXC_SYSTEM;
        SMSC_UTIL_EX_FILL(fmt);
        char ebuf[100];
        message += ": ";
        message += STRERROR(err,ebuf,sizeof(ebuf));
    }
    ~ErrnoException() throw() {}
protected:
    ErrnoException() {}
};


class FileWriteException : public ErrnoException
{
public:
    FileWriteException( const char* fn,
                        int err, 
                        const char* fmt, ... ) :
    fn_(fn)
    {
        code_ = EXC_SYSTEM;
        SMSC_UTIL_EX_FILL(fmt);
        char ebuf[100];
        message += ": ";
        message += STRERROR(err,ebuf,sizeof(ebuf));
    }
    ~FileWriteException() throw() {}

    inline const char* getFileName() const { return fn_.c_str(); }

protected:
    std::string fn_;
};


class FileReadException : public ErrnoException
{
public:
    // when file cannot be opened/found
    FileReadException( const char* fn,
                       int err, size_t pos,
                       const char* fmt, ... ) :
    fn_(fn), pos_(pos)
    {
        SMSC_UTIL_EX_FILL(fmt);
        if ( err != 0 ) {
            code_ = EXC_SYSTEM;
            char ebuf[100];
            message += ": ";
            message += STRERROR(err,ebuf,sizeof(ebuf));
        } else {
            code_ = EXC_BADFILE;
        }
    }
    ~FileReadException() throw() {}

    FileReadException( const FileReadException& f, const char* newfn ) :
    fn_(newfn), pos_(f.pos_)
    {
        code_ = f.code_;
        message = f.message;
    }

    inline const char* getFileName() const { return fn_.c_str(); }
    inline size_t getPos() const { return pos_; }

private:
    std::string fn_;
    size_t      pos_;
};

}
}

#endif
