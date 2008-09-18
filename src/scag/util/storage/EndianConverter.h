#ifndef _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H
#define _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H

#include "util/int.h"

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
        cvt.words[0] = htons(i);
        return cvt.buf;
    }

    inline char* set( uint32_t i ) {
        cvt.longs[0] = htonl(i);
        return cvt.buf;
    }

    inline char* set( uint64_t i ) {
        cvt.longs[0] = htonl(uint32_t(i>>32));
        cvt.longs[1] = htonl(uint32_t(i));
        return cvt.buf;
    }

    inline uint16_t get16() const {
        return ntohs( cvt.words[0] );
    }

    inline uint32_t get32() const {
        return ntohl( cvt.longs[0] );
    }

    inline uint64_t get64() const {
        return (uint64_t(ntohl(cvt.longs[0])) << 32) + ntohl(cvt.longs[1]);
    }

    inline uint8_t* ubuf() {
        return cvt.bytes;
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

#endif /* !_SCAG_UTIL_STORAGE_ENDIANCONVERTER_H */
