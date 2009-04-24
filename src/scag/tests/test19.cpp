//
// A test of alignment
//
#include <string>
#include "util/int.h"
#include "scag/util/HexDump.h"

int main()
{
    const char* test = "\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef";
    for ( size_t i = 0; i < 8; ++i ) {
        const char* ptr = test + i;
        const uint64_t* uptr = reinterpret_cast<const uint64_t*>(ptr);
        scag::util::HexDump hd;
        std::string dump("char: ");
        hd.hexdump(dump,ptr,8);
        dump.append(" uint64: ");
        hd.hexdump(dump,uptr,8);
        printf("uint64:%llx, dump: %s\n", *uptr, dump.c_str());
    }
    return 0;
}
