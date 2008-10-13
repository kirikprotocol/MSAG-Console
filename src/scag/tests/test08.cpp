/// a test of seqlock

#include <cstdio>
#include <cmath>
#include "scag/util/HRTimer.h"
#include "core/synchronization/Mutex.hpp"

using scag2::util::HRTimer;
using namespace smsc::core::synchronization;

int main()
{
    Mutex mtx;
    HRTimer hrt0;
    hrt0.mark();
    HRTimer hrt(hrt0);
    hrt.mark();
    const unsigned passes = 100000;
    hrtime_t avg = 0;
    hrtime_t sig = 0;
    size_t x;
    for ( size_t i = 0; i < passes; ++i ) {
        hrt.get();
        {
            MutexGuard mg(mtx);
            x = i;
        }
        const hrtime_t d = hrt.get() / 1000;
        avg += d;
        sig += d*d;
    }
    unsigned total = unsigned(hrt0.get()/1000);
    double perpass = double(total)/passes;

    // calculations
    double davg = double(avg) / passes;
    double dsig = double(sig) / passes;
    dsig = std::sqrt( (dsig-davg*davg) / passes );
    printf( "total number of passes: %u\n", passes );
    printf( "total time (us): %u, time per pass (us): %g\n", total, perpass );
    printf( "time for hrt.get (us): %g+-%g\n", davg, dsig );
    return 0;
}
