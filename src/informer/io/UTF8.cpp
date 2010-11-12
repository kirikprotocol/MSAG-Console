#include <cerrno>
#include <string.h>
#include "UTF8.h"
#include "InfosmeException.h"

namespace eyeline {
namespace informer {

UTF8::UTF8() :
conv_(iconv_t(-1))
{
    conv_ = iconv_open("UTF-8","UCS2-BE");
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
    while (true) {
        // resize buffer
        buf.reserve(buf.GetPos()+inlen*maxBytesPerChar);
        size_t outbytesleft = buf.getSize() - buf.GetPos();
        char* out = buf.GetCurPtr();
#ifdef linux
        char* in = const_cast<char*>(inptr); // fix for GNUC
#endif

        {
            smsc::core::synchronization::MutexGuard mg(lock_);
            iconv(conv_, NULL, NULL, NULL, NULL);
            if (iconv(conv_,
#ifdef linux
                      &in, 
#else
                      &inptr,
#endif
                      &inlen, &out, &outbytesleft) == size_t(-1)) {
                buf.SetPos(buf.getSize()-outbytesleft);
                if (errno == E2BIG) {
                    continue;
                }
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
