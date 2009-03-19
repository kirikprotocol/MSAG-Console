#include <stdio.h>
#include <string.h>
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
                        size_t insize )
{
    if ( ! digits_.get() ) {
        smsc::core::synchronization::MutexGuard mg(::mtx);
        digits_ = ::getDigits();
    }
    char* digits = digits_.get();
    for ( size_t i = insize; i > 0; --i ) {
        unsigned char c = static_cast<unsigned char>(*(inbuf++));
        memcpy( outbuf, digits + unsigned(c)*3, 3 );
        outbuf += 3;
    }
    return outbuf;
}


char* HexDump::strdump( char* outbuf,
                        const char* inbuf,
                        size_t insize )
{
    for ( size_t i = insize; i > 0; --i ) {
        unsigned char c = static_cast<unsigned char>(*(inbuf++));
        if ( c < 32 || c >= 127 ) c = '.';
        *(outbuf++) = c;
    }
    return outbuf;
}

void HexDump::hexdump( std::string& out, const char* inbuf, size_t insize )
{
    const size_t pos = out.size();
    out.resize( out.size() + hexdumpsize(insize) );
    hexdump( const_cast<char*>(out.c_str()) + pos, inbuf, insize );
}

void HexDump::strdump( std::string& out, const char* inbuf, size_t insize )
{
    const size_t pos = out.size();
    out.resize( out.size() + strdumpsize(insize) );
    strdump( const_cast<char*>(out.c_str()) + pos, inbuf, insize );
}

char* HexDump::addstr( char* outbuf, const char* cstring )
{
    const size_t i = ::strlen( cstring );
    ::memcpy( outbuf, cstring, i );
    return outbuf + i;
}

}
}
