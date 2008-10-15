/// This test may be used to measure timing of a code of interest.
/// Simply place your code between "// -- MEASURED CODE" comments below.
/// If you need some preparation code, put it between "// -- PREPARATION CODE" below.

#include <cstdio>
#include <cmath>
#include "scag/util/HRTimer.h"

#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"

using scag2::util::HRTimer;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

std::pair< double, double > calcavgerr( unsigned nexp, hrtime_t* data )
{
    hrtime_t avg(0), sig(0);
    for ( unsigned i = 0; i < nexp; ++i ) {
        avg += data[i];
        sig += data[i]*data[i];
    }
    double davg = double(avg) / nexp;
    double dsig = double(sig) / nexp;
    dsig = std::sqrt( (dsig-davg*davg) / (nexp-1) );
    return std::make_pair(davg,dsig);
}


struct DummyThread : public Thread
{
    DummyThread( Mutex& mtx ) : mtx_(mtx) {}
    virtual int Execute() {
        mon_.Lock();
        MutexGuard mg(mtx_);
        mon_.wait();
        return 0;
    }
    void Stop() {
        mon_.notifyAll();
    }
private:
    EventMonitor mon_;
    Mutex&       mtx_;
};


int main( int argc, char** argv )
{
    const unsigned nexp = 20; // number of experiments

    unsigned passes = 1000000;
    if ( argc > 1 ) {
        passes = ::strtoul( argv[1], NULL, 10 );
        if ( passes < 100 ) passes = 100;
        printf( "total number of passes: %u\n", passes );
    }

// -- PREPARATION CODE
    EventMonitor mtx;
    // DummyThread dt(mtx);
    MutexGuard mg(mtx);
// -- END OF PREPARATION CODE

    hrtime_t res[nexp*2];
    for ( size_t measure = 0; measure < nexp*2; ++measure ) {

        size_t sum = 0;
        HRTimer hrt;
        hrt.mark();
        for ( size_t i = 0; i < passes; ++i ) {
            // hrt.get();
            if ( measure >= nexp ) {
// -- MEASURED CODE
                // 1. trylocking locked mutex
                // mtx.TryLock();

                // 2. locking/unlocking free mutex
                // MutexGuard mg(mtx);

                // 3. notifying locked monitor
                mtx.notify();

// -- END OF MEASURED CODE
            }
            // some dummy instruction to prevent optimization
            sum += i*(measure+1);
        }

        {
            const hrtime_t d = hrt.get() / 1000;
            res[measure] = d;
        }
    }

    // calculations
    printf( "total number of experiments: %u\n", nexp );
    printf( "total number of passes per experiment: %u\n", passes );
    std::pair< double, double > avgerr0 = calcavgerr( nexp, &res[0] );
    printf( "Empty loop time(us): total=%g+-%g  onepass=%g+-%g\n",
            avgerr0.first, avgerr0.second, avgerr0.first/passes, avgerr0.second/passes );
    std::pair< double, double > avgerr = calcavgerr( nexp, &res[nexp] );
    printf( "Full loop time(us): total=%g+-%g  onepass=%g+-%g\n",
            avgerr.first, avgerr.second, avgerr.first/passes, avgerr.second/passes );
    const double avg = avgerr.first - avgerr0.first;
    const double sig = std::sqrt(avgerr.second*avgerr.second + avgerr0.second*avgerr0.second);
    printf( "Corrected time (us): total=%g+-%g  onepass=%g+-%g\n",
            avg, sig, avg/passes, sig/passes );
    return 0;
}
