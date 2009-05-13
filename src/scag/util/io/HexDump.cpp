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
        snprintf( res.get()+i*3, 4, "%02x ", i );
    }
    return res;
}

}

namespace scag2 {
namespace util {

std::auto_ptr< char > HexDump::digits_;

char* HexDump::hexdump( register char* outbuf,
                        const void* inBuf,
                        size_t insize )
{
    register const unsigned char* inbuf = reinterpret_cast<const unsigned char*>(inBuf);
    /*
    if ( ! digits_.get() ) {
        smsc::core::synchronization::MutexGuard mg(::mtx);
        digits_ = ::getDigits();
    }
     const char* digits = digits_.get();
     */
    const unsigned char* endbuf = inbuf + insize;
    while ( inbuf < endbuf ) {
        register unsigned char c = *inbuf;
        ++inbuf;
        register unsigned char d = c >> 4;
        d += (d < 0xa) ? '0' : 0x57;
        *outbuf = char(d);
        d = c & 0xf;
        d += (d < 0xa) ? '0' : 0x57;
        *++outbuf = char(d);
        *++outbuf = ' ';
        ++outbuf;
    }
    return outbuf;
}


char* HexDump::strdump( register char* outbuf,
                        const void* inBuf,
                        size_t insize )
{
    register const unsigned char* inbuf = reinterpret_cast<const unsigned char*>(inBuf);
    const unsigned char* endbuf = inbuf + insize;
    while ( inbuf < endbuf ) {
        register unsigned char c = *(inbuf++);
        if ( c < 32 || c >= 127 ) { c = '.'; }
        *(outbuf++) = char(c);
    }
    return outbuf;
}


char* HexDump::utfdump( register char* outbuf,
                        const void* inBuf,
                        size_t insize )
{
    register const unsigned char* inbuf = reinterpret_cast<const unsigned char*>(inBuf);
    const unsigned char* endbuf = inbuf + insize;
    while ( inbuf < endbuf ) {
        register unsigned char c = *(inbuf++);
        if ( c < 32 || c == 127 || c > 0xfd ) {c = '.';}
        *(outbuf++) = char(c);
    }
    return outbuf;
}


void HexDump::hexdump( std::string& out, const void* inBuf, size_t insize )
{
    const size_t pos = out.size();
    out.resize( out.size() + hexdumpsize(insize) );
    hexdump( const_cast<char*>(out.c_str()) + pos, inBuf, insize );
}

void HexDump::strdump( std::string& out, const void* inBuf, size_t insize )
{
    const size_t pos = out.size();
    out.resize( out.size() + strdumpsize(insize) );
    strdump( const_cast<char*>(out.c_str()) + pos, inBuf, insize );
}

void HexDump::utfdump( std::string& out, const void* inBuf, size_t insize )
{
    const size_t pos = out.size();
    out.resize( out.size() + utfdumpsize(insize) );
    utfdump( const_cast<char*>(out.c_str()) + pos, inBuf, insize );
}

char* HexDump::addstr( char* outbuf, const char* cstring )
{
    const size_t i = ::strlen( cstring );
    ::memcpy( outbuf, cstring, i );
    return outbuf + i;
}

}
}
