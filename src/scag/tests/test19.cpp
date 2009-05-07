//
// A test of alignment
//
#include <string>
#include "util/int.h"
#include "scag/util/HexDump.h"
#include "scag/util/storage/EndianConverter.h"

using namespace scag2::util;
using namespace scag2::util::storage;

/// get from ptr, set into temp, compare
inline uint16_t onecheck16( const char* ptr )
{
    return EndianConverter::get16(ptr);
}

inline uint32_t onecheck32( const char* ptr )
{
    return EndianConverter::get32(ptr);
}

inline uint64_t onecheck64( const char* ptr )
{
    return EndianConverter::get64(ptr);
}

/// extract data from ptr
void oneprint( const char* ptr )
{
    const uint64_t* uptr = reinterpret_cast<const uint64_t*>(ptr);
    uint64_t utmp;
    memcpy(&utmp,uptr,8);
    HexDump hd;
    std::string dump("char: ");
    hd.hexdump(dump,ptr,8);
    dump.append(" uint64: ");
    hd.hexdump(dump,uptr,8);
    printf("uint64: %llx, cvt16:%x, cvt32: %x, cvt64: %llx, dump: %s\n",
           utmp,
           EndianConverter::get16(ptr),
           EndianConverter::get32(ptr),
           EndianConverter::get64(ptr),
           dump.c_str());
}

int main()
{
    const char* test = "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef";
    for ( size_t pass = 0; pass < 10000000; ++pass ) {
        for ( size_t i = 0; i < 8; ++i ) {
            // const char* ptr = test + i;
            // oneprint( ptr );
            onecheck32(test+i);
        }
    }
    return 0;
}
