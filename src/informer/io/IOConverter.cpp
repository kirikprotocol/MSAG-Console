#include <cstring>
#include "IOConverter.h"
#include "InfosmeException.h"
#include "util/byteorder.h"

#define checksz(sz) { \
    register const size_t newpos = pos + sz; \
    if (newpos>buflen) { throw InfosmeException(EXC_IOERROR,"buffer is too small: len=%u",unsigned(buflen)); } \
    pos = newpos; \
}

namespace eyeline {
namespace informer {

uint8_t FromBuf::get8()
{
    checksz(1);
    return *(buf++);
}


uint16_t FromBuf::get16()
{
    checksz(2);
    register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    register uint16_t res(*p);
    res <<= 8;
    res += *(++p);
    buf = reinterpret_cast<const unsigned char*>(++p);
    return res;
}


uint32_t FromBuf::get32()
{
    checksz(4);
    register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    register uint32_t res(*p);
    res <<= 8;
    res += *++p;
    res <<= 8;
    res += *++p;
    res <<= 8;
    res += *++p;
    buf = reinterpret_cast<const unsigned char*>(++p);
    return res;
}


uint64_t FromBuf::get64()
{
#if BYTE_ORDER == BIG_ENDIAN
    checksz(8);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    unsigned shift = unsigned(reinterpret_cast<uint64_t>(p)) & 0x7;
    p -= shift;
    const uint64_t* pp = reinterpret_cast<const uint64_t*>(p);
    uint64_t res = (*pp);
    if ( shift != 0 ) {
        shift *= 8;
        res <<= shift;
        res += (*++pp) >> (64-shift);
    }
    buf += 8;
    return res;
#else
    uint64_t res = get32();
    res <<= 32;
    return res + get32();
#endif
}


uint16_t FromBuf::getc16()
{
    register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    if (*p > 0x7f) {
        checksz(3);
        uint16_t res(*p);
        res <<= 8;
        res += *(++p);
        buf = reinterpret_cast<const unsigned char*>(++p);
        return res;
    } else {
        checksz(1);
        return *(buf++);
    }
}


const char* FromBuf::getCString()
{
    register const unsigned char* bnew = buf;
    for ( ; *bnew != '\0'; ++bnew ) {}
    ++bnew; // to skip '\0'
    checksz((bnew-buf));
    register const char* ret = reinterpret_cast<const char*>(buf);
    buf = bnew;
    return ret;
}


const unsigned char* FromBuf::skip( size_t bytes )
{
    checksz(bytes);
    register const unsigned char* ret = buf;
    buf += bytes;
    return ret;
}


void FromBuf::setPos( size_t newpos )
{
    if (newpos>buflen) {
        throw InfosmeException(EXC_IOERROR,"buffer is too small: len=%u, pos=%u",unsigned(buflen), unsigned(newpos));
    }
    register int off = int(newpos) - int(pos);
    buf += off;
    pos = newpos;
}


void FromBuf::setLen( size_t newlen )
{
    if (newlen<pos) {
        throw InfosmeException(EXC_IOERROR,"buffer newlen is too small: len=%u, pos=%u", unsigned(newlen), unsigned(pos));
    }
    buflen = newlen;
}


void ToBuf::set8( uint8_t c )
{
    checksz(1);
    *(buf++) = c;
}


void ToBuf::set16( uint16_t c )
{
    checksz(2);
    register uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    *p = uint8_t(c >> 8);
    *++p = uint8_t(c);
    buf += 2;
}


void ToBuf::set32( uint32_t c )
{
    checksz(4);
    buf += 4;
    register uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    *--p = uint8_t(c);
    c >>= 8;
    *--p = uint8_t(c);
    c >>= 8;
    *--p = uint8_t(c);
    c >>= 8;
    *--p = uint8_t(c);
}


void ToBuf::set64( uint64_t c )
{
#if BYTE_ORDER == BIG_ENDIAN
    checksz(8);
    memcpy(buf,&c,8);
    buf += 8;
#else
    set32(uint32_t(c>>32));
    set32(uint32_t(c));
#endif
}


void ToBuf::setc16( uint16_t c )
{
    if (c>0x7f) {
        checksz(3);
        register uint8_t* p = reinterpret_cast<uint8_t*>(buf);
        *p = 0xcd;
        *++p = uint8_t(c>>8);
        *++p = uint8_t(c);
        buf += 3;
    } else {
        checksz(1);
        *(buf++) = c;
    }
}


void ToBuf::copy( size_t bytes, const void* from )
{
    checksz(bytes);
    memcpy(buf,from,bytes);
    buf += bytes;
}


void ToBuf::setCString( const char* s )
{
    register const size_t inc = strlen(s)+1;
    checksz(inc);
    memcpy(buf,s,inc);
    buf += inc;
}


void ToBuf::setHexCString( const char* s, size_t slen )
{
    register const size_t inc = slen*2+1;
    checksz(inc);
    register unsigned char* out = buf;
    for ( ; slen > 0; --slen ) {
        register uint8_t bt = uint8_t(*s);
        *out++ = bt >> 4;
        *out++ = bt & 0xf;
    }
    *out = '\0';
    buf += inc;
}


void ToBuf::skip( size_t bytes )
{
    checksz(bytes);
    buf += bytes;
}


void ToBuf::setPos( size_t newpos )
{
    if (newpos>buflen) {
        throw InfosmeException(EXC_IOERROR,"buffer is too small: len=%u, pos=%u",unsigned(buflen), unsigned(newpos));
    }
    register int off = int(newpos) - int(pos);
    buf += off;
    pos = newpos;
}

}
}
