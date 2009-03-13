#include <cstdio>
#include "Drndm.h"

int main()
{
    scag2::util::Drndm rnd;
    for ( int i = 0; i < 100; ++i ) {
        printf( "%llx\n", rnd.get() );
    }
    return 0;
}
