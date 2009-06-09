#include "EndianConverter.h"

namespace scag {
namespace util {
namespace io {

uint16_t EndianConverter::get16( const void* buf ) {
    register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    register uint16_t tmp(*p);
    return (tmp << 8) + *++p;
}

uint32_t EndianConverter::get32( const void* buf ) {
    register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
    register uint32_t tmp(*p);
    tmp <<= 8;
    tmp += *++p;
    tmp <<= 8;
    tmp += *++p;
    tmp <<= 8;
    tmp += *++p;
    return tmp;
}

uint64_t EndianConverter::get64( const void* buf )
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
#if BYTE_ORDER == BIG_ENDIAN
    unsigned shift = unsigned(reinterpret_cast<uint64_t>(p)) & 0x7;
    p -= shift;
    shift *= 8;
    const uint64_t* pp = reinterpret_cast<const uint64_t*>(p);
    uint64_t res = (*pp) << shift;
    if ( shift != 0 ) {
        res += (*++pp) >> (64-shift);
    }
    return res;
#else
    return (uint64_t(get32(p))<<32) + get32(p+4);
#endif
    /*
    union {
        uint64_t u64;
        uint32_t u32[2];
    } tmp;
    memcpy(&tmp,buf,8);
#if BYTE_ORDER == BIG_ENDIAN
    return tmp.u64;
#else
    return (uint64_t(ntohl(tmp.u32[0])) << 32) + ntohl(tmp.u32[1]);
#endif
     */
}

void EndianConverter::set16( void* buf, uint16_t i )
{
    register uint8_t* p = reinterpret_cast<uint8_t*>(buf);
    *p = uint8_t(i >> 8);
    *++p = uint8_t(i);
}

void EndianConverter::set32( void* buf, register uint32_t i )
{
    register uint8_t* p = reinterpret_cast<uint8_t*>(buf) + 3;
    *p = uint8_t(i);
    i >>= 8;
    *--p = uint8_t(i);
    i >>= 8;
    *--p = uint8_t(i);
    i >>= 8;
    *--p = uint8_t(i);
}

void EndianConverter::set64( void* buf, uint64_t i )
{
#if BYTE_ORDER == BIG_ENDIAN
    memcpy(buf,&i,8);
#else
    set32(buf,uint32_t(i>>32));
    set32(reinterpret_cast<uint8_t*>(buf)+4,uint32_t(i));
#endif
}

}
}
}
