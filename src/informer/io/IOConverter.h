#ifndef _INFORMER_IOCONVERTER_H
#define _INFORMER_IOCONVERTER_H

#include <iosfwd>
#include "util/int.h"

namespace eyeline {
namespace informer {

struct FromBuf
{
    FromBuf( const void* thebuf, size_t bufLen ) :
    buf(static_cast<const unsigned char*>(thebuf)), buflen(bufLen), pos(0) {}
    uint8_t  get8();
    uint16_t get16();
    uint32_t get32();
    uint64_t get64();

    // compressed items (good for tag/length)
    uint16_t getc16();

    const unsigned char* skip( size_t bytes );
    const char* getCString();
    void setPos( size_t newpos );
    void setLen( size_t newlen );
    inline const char* getBuf() const { return reinterpret_cast<const char*>(buf); }
    inline size_t getPos() const { return pos; }
    inline size_t getLen() const { return buflen; }
    inline void setBuf( void* thebuf, size_t bufLen ) {
        buf = reinterpret_cast<unsigned char*>(thebuf);
        buflen = bufLen;
        pos = 0;
    }
private:
    FromBuf();
private:
    const unsigned char* buf;
    size_t               buflen;
    size_t               pos;
};


struct ToBuf
{
    ToBuf( void* thebuf, size_t bufLen ) :
    buf(reinterpret_cast<unsigned char*>(thebuf)), buflen(bufLen), pos(0) {}
    void set8(uint8_t);
    void set16( uint16_t );
    void set32( uint32_t );
    void set64( uint64_t );

    void setc16( uint16_t );

    void copy( size_t bytes, const void* from );
    void setCString( const char* s );
    void setHexCString( const char* s, size_t slen );
    void skip( size_t bytes );
    void setPos( size_t newpos );
    void setBuf( void* thebuf, size_t bufLen ) {
        buf = reinterpret_cast<unsigned char*>(thebuf);
        buflen = bufLen;
        pos = 0;
    }
    inline size_t getPos() const { return pos; }
    inline size_t getLen() const { return buflen; }
public:
    unsigned char* buf;    // current pointer
    size_t         buflen;
    size_t         pos;    // position since beginning
};

} // informer
} // smsc

#endif
