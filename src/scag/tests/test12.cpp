// a test of crc32
#include <string>
#include <cstdio>
#include "util/crc32.h"

int main()
{
    const std::string s = "hello, world";
    const uint32_t c0 = smsc::util::crc32(0,s.c_str(),s.size());
    printf( "crc32(#0) = %u\n", c0);
    const uint32_t c1 = smsc::util::crc32(c0,s.c_str(),s.size());
    printf( "crc32(#1) = %u\n", c1);
    return 0;
}
