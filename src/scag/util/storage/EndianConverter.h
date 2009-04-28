#ifndef _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H
#define _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H

#include "util/int.h"
#include "util/byteorder.h"
#include <netinet/in.h>

namespace scag {
namespace util {
namespace storage {

struct EndianConverter
{
    template < class T > 
        inline uint8_t* uset( T i ) {
        this->set( i );
        return cvt.bytes;
    }

    inline char* set( uint16_t i ) {
        set16(cvt.buf,i);
        return cvt.buf;
    }

    inline char* set( uint32_t i ) {
        set32(cvt.buf,i);
        return cvt.buf;
    }

    inline char* set( uint64_t i ) {
        set64(cvt.buf,i);
        return cvt.buf;
    }

    inline uint16_t get16() const {
        return get16(cvt.buf);
    }

    inline uint32_t get32() const {
        return get32(cvt.buf);
    }

    inline uint64_t get64() const {
        return get64(cvt.buf);
    }

    inline uint8_t* ubuf() {
        return cvt.bytes;
    }

    // methods for inplace conversion
    inline static uint16_t get16( const void* buf ) {
#if BYTE_ORDER == BIG_ENDIAN
        return *reinterpret_cast<const uint16_t*>(buf);
#else
        return ntohs(*reinterpret_cast<const uint16_t*>(buf));
#endif
    }

    inline static uint32_t get32( const void* buf ) {
#if BYTE_ORDER == BIG_ENDIAN
        return *reinterpret_cast<const uint32_t*>(buf);
#else
        return ntohl(*reinterpret_cast<const uint32_t*>(buf));
#endif
    }

    inline static uint64_t get64( const void* buf ) {
#if BYTE_ORDER == BIG_ENDIAN
        return *reinterpret_cast<const uint64_t*>(buf);
#else
        return (uint64_t(ntohl(*reinterpret_cast<const uint32_t*>(buf))) << 32) +
            ntohl(*(reinterpret_cast<const uint32_t*>(buf)+1));
#endif
    }
    
    inline static void set16( void* buf, uint16_t i ) {
#if BYTE_ORDER == BIG_ENDIAN
        *reinterpret_cast<uint16_t*>(buf) = i;
#else
        *reinterpret_cast<uint16_t*>(buf) = htons(i);
#endif
    }
    inline static void set32( void* buf, uint32_t i ) {
#if BYTE_ORDER == BIG_ENDIAN
        *reinterpret_cast<uint32_t*>(buf) = i;
#else
        *reinterpret_cast<uint32_t*>(buf) = htonl(i);
#endif
    }
    inline static void set64( void* buf, uint64_t i ) {
#if BYTE_ORDER == BIG_ENDIAN
        *reinterpret_cast<uint64_t*>(buf) = i;
#else
        register uint32_t* ptr = reinterpret_cast<uint32_t*>(buf);
        *ptr = htonl(i >> 32);
        *++ptr = htonl(uint32_t(i));
#endif
    }

    union {
        char     buf[8];
        uint8_t  bytes[8];
        uint16_t words[4];
        uint32_t longs[2];
        uint64_t quads[1];
    } cvt;

};

}
}
}

namespace scag2 {
namespace util {
namespace storage {
using namespace scag::util::storage;
}
}
}

#endif /* !_SCAG_UTIL_STORAGE_ENDIANCONVERTER_H */
