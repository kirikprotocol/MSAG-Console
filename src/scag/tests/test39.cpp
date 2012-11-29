// a test of atomics

#include <stdio.h>
#include "util/int.h"

// #define ATOMICCOUNTERUSEMUTEX

#include "core/synchronization/AtomicCounter.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"

using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

EventMonitor mon;
AtomicCounter< uint32_t > ac;
unsigned finished = 0;
unsigned iterations = 1;

class TestAtomic : public ThreadedTask
{
public:
    const char* taskName() { return "testatomic"; }
    int Execute()
    {
        const uint64_t maxiter = uint64_t(iterations)*1000000;
        for ( uint64_t i = 0; i < maxiter; ++i ) {
            if ( (i % 2) ) {
                ac.dec();
            } else {
                ac.inc();
            }
        }
        MutexGuard mg(mon);
        ++finished;
        mon.notify();
        return 0;
    }
};


int main( int argc, char** argv )
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_INFO );

    unsigned nthreads = 10;

    if ( argc > 1 ) {
        nthreads = unsigned(atoi(argv[1]));
        if (nthreads < 1 || nthreads > 100) {
            nthreads = 10;
        }
        if ( argc > 2 ) {
            iterations = unsigned(atoi(argv[2]));
            if ( iterations < 1 ) {
                iterations = 1;
            }
        }
    }

    printf("test atomic type=%s nthreads=%u, %u Mcycles\n",
           ac.getType(),nthreads,iterations);

    ThreadPool tp;
    for ( unsigned i = 0; i < nthreads; ++i ) {
        tp.startTask( new TestAtomic() );
    }
    while ( finished < nthreads ) {
        MutexGuard mg(mon);
        mon.wait(5000);
    }
    printf("all %u threads finished\n",nthreads);
    return 0;
}
