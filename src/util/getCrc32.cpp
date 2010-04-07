#include <cstdio>
#include <cstring>
#include "util/crc32.h"

/// get crc32
int main( int argc, char** argv )
{
    for ( char** p = argv+1; *p != 0; ++p )
    {
        printf("%s %llx\n",*p,int64_t(smsc::util::crc32(0,*p,std::strlen(*p))));
    }
    return 0;
}
