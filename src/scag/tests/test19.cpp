//
// A test of alignment
//
#include <string>
#include "util/int.h"
#include "scag/util/io/HexDump.h"
#include "scag/util/io/EndianConverter.h"

using namespace scag2::util;
using namespace scag2::util::io;

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
    memcpy(&utmp,ptr,8);
    HexDump hd;
    HexDump::string_type dump;
    hd.hexdump(dump,ptr,8);
    hd.addstr(dump," str: ");
    hd.strdump(dump,&utmp,8);
    printf("uint64: %llx, cvt16:%x, cvt32: %x, cvt64: %llx, dump: %s\n",
           utmp,
           EndianConverter::get16(ptr),
           EndianConverter::get32(ptr),
           EndianConverter::get64(ptr),
           hd.c_str(dump));
}

int main()
{
    const char* test = "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef";
    for ( size_t pass = 0; pass < 10000000; ++pass ) {
        for ( size_t i = 0; i < 8; ++i ) {
            // const char* ptr = test + i;
            // oneprint(test+i);
            onecheck64(test+i);
            /*
            HexDump hd;
            std::string dump;
            hd.hexdump(dump,test+i,8);
             */
        }
    }
    return 0;
}
