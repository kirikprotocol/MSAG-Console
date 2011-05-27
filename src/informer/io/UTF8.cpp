#include <cerrno>
#include <string.h>
#include "UTF8.h"
#include "HexDump.h"
#include "InfosmeException.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

namespace {
smsc::logger::Logger* log_ = 0;
}

UTF8::UTF8() :
lock_( MTXWHEREAMI ),
conv_(iconv_t(-1))
{
    // NOTE: damn sunos places BOM in ucs2 sequence when converting utf8->ucs2
    // To avoid this we have to specify explicitly ucs-2be/ucs-2le according to the arch.
    union {
        uint8_t bytes[2];
        uint16_t words[1];
    } test;
    test.words[0] = 0xff00;
    conv_ = iconv_open( test.bytes[0] == 0xff ? "UCS-2BE" : "UCS-2LE","UTF-8" );
    if (conv_ == iconv_t(-1)) {
        throw ErrnoException(errno,"iconv_open");
    }
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("utf8");
    }
}

UTF8::~UTF8()
{
    if (conv_ != iconv_t(-1)) {
        iconv_close(conv_);
    }
}


void UTF8::convertToUcs2( const char* inptr, size_t inlen, TmpBufBase<char>& buf )
{
    static const size_t maxBytesPerChar = 4;
    int error = 0;
    // initial reservation
    if (log_ && log_->isDebugEnabled()) {
        HexDump hd;
        HexDump::string_type dump;
        const size_t dumplen = inlen < 50 ? inlen : 50;
        hd.hexdump(dump,inptr,dumplen);
        smsc_log_debug(log_,"before: %s",hd.c_str(dump));
    }
    buf.reserve(buf.GetPos()+inlen*maxBytesPerChar);
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        iconv(conv_, NULL, NULL, NULL, NULL);
        while (true) {
            size_t outbytesleft = buf.getSize() - buf.GetPos();
            char* out = buf.GetCurPtr();
#if defined(linux) || defined(__MACH__)
            char* in = const_cast<char*>(inptr); // fix for GNUC
#endif
            const size_t iconv_res = iconv(conv_,
#if defined(linux) || defined(__MACH__)
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
    if (log_ && log_->isDebugEnabled()) {
        HexDump hd;
        HexDump::string_type dump;
        const size_t dumplen = size_t(buf.GetPos()) < 50 ? size_t(buf.GetPos()) : 50;
        hd.hexdump(dump,buf.get(),dumplen);
        smsc_log_debug(log_,"after: %s",hd.c_str(dump));
    }
}

} // informer
} // eyeline
