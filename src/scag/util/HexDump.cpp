#include <cstdio>
#include <cstring>
#include "HexDump.h"
#include "core/synchronization/Mutex.hpp"

namespace {
smsc::core::synchronization::Mutex mtx;

std::auto_ptr<char> getDigits()
{
    std::auto_ptr<char> res( new char[256*3+1]);
    for ( unsigned i = 0; i < 256; ++i ) {
        snprintf( res.get()+i*3, 4, " %02x", i );
    }
    return res;
}

}

namespace scag2 {
namespace util {

std::auto_ptr< char > HexDump::digits_;

char* HexDump::hexdump( char* outbuf,
                        const char* inbuf,
                        unsigned insize )
{
    if ( ! digits_.get() ) {
        smsc::core::synchronization::MutexGuard mg(::mtx);
        digits_ = ::getDigits();
    }
    char* digits = digits_.get();
    for ( unsigned i = insize; i > 0; --i ) {
        unsigned char c = static_cast<unsigned char>(*(inbuf++));
        memcpy( outbuf, digits + unsigned(c)*3, 3 );
        outbuf += 3;
    }
    return outbuf;
}


char* HexDump::strdump( char* outbuf,
                        const char* inbuf,
                        unsigned insize )
{
    for ( unsigned i = insize; i > 0; --i ) {
        unsigned char c = static_cast<unsigned char>(*(inbuf++));
        if ( c < 32 || c >= 127 ) c = '.';
        *(outbuf++) = c;
    }
    return outbuf;
}

char* HexDump::addstr( char* outbuf, const char* cstring )
{
    const unsigned i = ::strlen( cstring );
    ::memcpy( outbuf, cstring, i );
    return outbuf + i;
}

}
}
