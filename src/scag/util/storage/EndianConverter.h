#ifndef _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H
#define _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H

#include "util/int.h"
#include "util/byteorder.h"
#include <netinet/in.h>
#include "string.h"

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
        // register const uint8_t* p = reinterpret_cast<const uint8_t*>(buf);
        // register uint16_t tmp = uint16_t(*p);
#if BYTE_ORDER == BIG_ENDIAN
        return uint16_t(*ptr(buf)) << 8 + *move(buf,1);
#else
        return *ptr(buf) + (uint16_t(*move(buf,1)) << 8);
#endif
    }

    inline static uint32_t get32( const void* buf ) {
#if BYTE_ORDER == BIG_ENDIAN
        return uint32_t(get16(buf)) << 16 + get16(move(buf,2));
#else
        return get16(buf) + (uint32_t(get16(move(buf,2))) << 16);
#endif
    }

    inline static uint64_t get64( const void* buf ) {
        uint64_t tmp;
        memcpy(&tmp,buf,8);
#if BYTE_ORDER == BIG_ENDIAN
        return tmp;
#else
        return (uint64_t(ntohl(uint32_t(tmp)))<<32) + ntohl(uint32_t(tmp >> 32));
#endif
    }
    
    inline static void set16( void* buf, uint16_t i ) {
#if BYTE_ORDER == BIG_ENDIAN
        memcpy(buf,&i,2);
#else
        uint16_t tmp = htons(i);
        memcpy(buf,&tmp,2);
#endif
    }
    inline static void set32( void* buf, uint32_t i ) {
#if BYTE_ORDER == BIG_ENDIAN
        memcpy(buf,&i,4);
#else
        uint32_t tmp = htonl(i);
        memcpy(buf,&tmp,4);
#endif
    }
    inline static void set64( void* buf, uint64_t i ) {
#if BYTE_ORDER == BIG_ENDIAN
        memcpy(buf,&i,8);
#else
        set32(buf,uint32_t(i>>32));
        set32(reinterpret_cast<uint32_t*>(buf)+1,uint32_t(i));
#endif
    }

    union {
        char     buf[8];
        uint8_t  bytes[8];
        uint16_t words[4];
        uint32_t longs[2];
        uint64_t quads[1];
    } cvt;

private:
    inline static uint8_t* move( void* p, int x ) {
        return reinterpret_cast<uint8_t*>(p)+x;
    }
    inline static const uint8_t* move( const void* p, int x ) {
        return reinterpret_cast<const uint8_t*>(p)+x;
    }
    inline static uint8_t* ptr( void* p ) {
        return reinterpret_cast<uint8_t*>(p);
    }
    inline static const uint8_t* ptr( const void* p ) {
        return reinterpret_cast<const uint8_t*>(p);
    }
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
