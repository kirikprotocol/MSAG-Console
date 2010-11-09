#include <stdio.h>
#include <string.h>
#include "HexDump.h"
#include "core/synchronization/Mutex.hpp"

/*
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
 */

namespace eyeline {
namespace informer {

// std::auto_ptr< char > HexDump::digits_;

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


char* HexDump::addstr( char* outbuf, const char* cstring )
{
    const size_t i = ::strlen( cstring );
    ::memcpy( outbuf, cstring, i );
    return outbuf + i;
}


void HexDump::hexdump( string_type& out, const void* inBuf, size_t insize )
{
    const size_t hexsz = hexdumpsize(insize) + 1;
    size_t pos = out.size();
    if ( pos > 0 && out.back() == '\0' ) {
        --pos; // remove trailing zero
    }
    out.resize( pos + hexsz );
    *hexdump( &out[pos], inBuf, insize ) = '\0';
}

void HexDump::strdump( string_type& out, const void* inBuf, size_t insize )
{
    const size_t strsz = strdumpsize(insize) + 1;
    size_t pos= out.size();
    if ( pos > 0 && out.back() == '\0' ) {
        --pos; // remove trailing zero
    }
    out.resize( pos + strsz );
    *strdump( &out[pos], inBuf, insize ) = '\0';
}

void HexDump::utfdump( string_type& out, const void* inBuf, size_t insize )
{
    const size_t utfsz = utfdumpsize(insize) + 1;
    size_t pos= out.size();
    if ( pos > 0 && out.back() == '\0' ) {
        --pos; // remove trailing zero
    }
    out.resize( pos + utfsz );
    utfdump( &out[pos], inBuf, insize );
}

void HexDump::addstr( string_type& out, const char* cstring )
{
    size_t pos = out.size();
    if ( pos > 0 && out.back() == '\0' ) {
        --pos;
    }
    const size_t i = ::strlen(cstring) + 1;
    out.resize( pos + i );
    ::memcpy( &out[pos], cstring, i ); // will copy \0 also
}

}
}
