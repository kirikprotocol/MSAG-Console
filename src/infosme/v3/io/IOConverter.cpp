#include <cstring>
#include "IOConverter.h"
#include "util/byteorder.h"

namespace smsc {
namespace informer {
namespace io {

uint8_t FromBuf::get8()
{
    return *(buf++);
}


uint16_t FromBuf::get16()
{
    register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    register uint16_t res(*p);
    res <<= 8;
    res += *(++p);
    buf = reinterpret_cast<const unsigned char*>(++p);
    return res;
}


uint32_t FromBuf::get32()
{
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


const unsigned char* FromBuf::skip( size_t bytes )
{
    const unsigned char* ret = buf;
    buf += bytes;
    return ret;
}


void ToBuf::set8( uint8_t c )
{
    *(buf++) = c;
}


void ToBuf::set16( uint16_t c )
{
    buf += 2;
    register uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    *p = uint8_t(c >> 8);
    *++p = uint8_t(c);
    buf += 2;
}


void ToBuf::set32( uint32_t c )
{
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
    buf += 8;
    memcpy(buf,&c,8);
#else
    set32(uint32_t(c>>32));
    set32(uint32_t(c));
#endif
}


void ToBuf::copy( size_t bytes, const void* from )
{
    memcpy(buf,from,bytes);
    buf += bytes;
}

}
}
}
