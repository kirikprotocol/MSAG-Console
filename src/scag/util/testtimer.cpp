#include <vector>
#include <cstdio>
#include <cstdlib>
#include "HRTimer.h"

using scag2::util::HRTimer;

int main()
{
    HRTimer t;
    std::vector< int > values;
    values.reserve( 80 );
    std::vector< hrtime_t > times;
    times.reserve( 70 );
    HRTimer tt;
    tt.mark();
    t.mark();
    for ( int i = 0; i < 100; ++i ) {
        int sum = 0;
        for ( int j = 0; j < 100; ++j ) {
            sum += i*j;
        }
        values.push_back( sum );
        hrtime_t hrt = t.get();
        times.push_back( hrt );
    }
    for ( int i = 0; i < 100; ++i ) {
        printf( "pass #%3u: time: %lld value: %d\n", i, int64_t(times[i]), values[i] );
    }
    hrtime_t hrt = tt.get();
    printf( "total time: %lld\n", int64_t(hrt));
    return 0;
}
