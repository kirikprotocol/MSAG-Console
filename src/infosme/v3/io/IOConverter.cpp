#include "IOConverter.h"
#include "util/byteorder.h"

namespace smsc {
namespace infosme {
namespace io {

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

}
}
}
