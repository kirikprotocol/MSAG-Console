#include <cerrno>
#include <string.h>
#include "UTF8.h"
#include "InfosmeException.h"

namespace eyeline {
namespace informer {

UTF8::UTF8() :
conv_(iconv_t(-1))
{
    conv_ = iconv_open("UCS-2BE","UTF-8");
    if (conv_ == iconv_t(-1)) {
        throw ErrnoException(errno,"iconv_open");
    }
}

UTF8::~UTF8()
{
    if (conv_ != iconv_t(-1)) {
        iconv_close(conv_);
    }
}


void UTF8::convertToUcs2( const char* inptr, size_t inlen, BufType& buf )
{
    static const size_t maxBytesPerChar = 4;
    int error = 0;
    // initial reservation
    buf.reserve(buf.GetPos()+inlen*maxBytesPerChar);
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        iconv(conv_, NULL, NULL, NULL, NULL);
        while (true) {
            size_t outbytesleft = buf.getSize() - buf.GetPos();
            char* out = buf.GetCurPtr();
#ifdef linux
            char* in = const_cast<char*>(inptr); // fix for GNUC
#endif
            const size_t iconv_res = iconv(conv_,
#ifdef linux
                                           &in, 
#else
                                           &inptr,
#endif
                                           &inlen, &out, &outbytesleft);
            buf.SetPos(buf.getSize()-outbytesleft);
            if ( iconv_res == size_t(-1)) {
                if (errno == E2BIG) {
                    // buffer was too small, need to resize and try again
                    // NOTE: we do this under lock to hold state kept in conv_
                    buf.reserve(buf.GetPos()+inlen*maxBytesPerChar);
                    continue; 
                }
                // failure
                error = errno;
            }
            break;
        }
    }
    if (error) {
        throw ErrnoException(error,"invalid/incomplete utf8");
    }
}

} // informer
} // eyeline
