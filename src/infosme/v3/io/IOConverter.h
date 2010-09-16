#ifndef _INFOSME_V3_IOCONVERTER_H
#define _INFOSME_V3_IOCONVERTER_H

#include <iosfwd>
#include "util/int.h"

namespace smsc {
namespace infosme {
namespace io {

struct FromBuf
{
    FromBuf( const void* thebuf ) : buf(static_cast<const unsigned char*>(thebuf)) {}
    uint8_t  get8();
    uint16_t get16();
    uint32_t get32();
    uint64_t get64();
    const unsigned char* skip( size_t bytes );
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
public:
    unsigned char* buf;
};

} // io
} // infosme
} // smsc

#endif
