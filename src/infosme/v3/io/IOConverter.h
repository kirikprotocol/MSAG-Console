#ifndef _INFORMER_IOCONVERTER_H
#define _INFORMER_IOCONVERTER_H

#include <iosfwd>
#include "util/int.h"

namespace eyeline {
namespace informer {
namespace io {

struct FromBuf
{
    FromBuf( const void* thebuf ) : buf(static_cast<const unsigned char*>(thebuf)) {}
    uint8_t  get8();
    uint16_t get16();
    uint32_t get32();
    uint64_t get64();
    const unsigned char* skip( size_t bytes );
    const char* getCString();
private:
    FromBuf();
public:
    const unsigned char* buf;
};


struct ToBuf
{
    ToBuf( void* thebuf ) : buf(reinterpret_cast<unsigned char*>(thebuf)) {}
    void set8(uint8_t);
    void set16( uint16_t );
    void set32( uint32_t );
    void set64( uint64_t );
    void copy( size_t bytes, const void* from );
    void setCString( const char* s );
    unsigned char* getPtr() { return buf; }
public:
    unsigned char* buf;
};

} // io
} // informer
} // smsc

#endif
