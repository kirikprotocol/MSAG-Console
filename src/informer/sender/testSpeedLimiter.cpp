#include "AbsSpeedLimiter.h"
#include "informer/io/SpeedControl.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/MutexReportContentionRealization.h"
#include "logger/Logger.h"

using namespace smsc::logger;
using namespace eyeline::informer;
using namespace smsc::core::synchronization;

int main( int argc, char** argv )
{
    Logger::initForTest( Logger::LEVEL_DEBUG );
    unsigned speed = 10;
    if ( argc > 1 ) {
        speed = unsigned(atoi(argv[1]));
        if (speed<=0) speed = 1;
        else if ( speed > 10000 ) speed = 10000;
    }

    Logger* mainlog = Logger::getInstance("main");
    SpeedControl<usectime_type,tuPerSec> control(speed);
    AbsSpeedLimiter                      limiter(speed,20);

    EventMonitor mon;
    MutexGuard mg(mon);
    bool proc = true;
    for ( int i = 0; i < 100000; ++i ) {

        usectime_type now = currentTimeMicro();
        unsigned delay = 0;
        usectime_type delay1, delay2 = 0;
        usectime_type udp = 0;
        do {
            delay1 = control.isReady(now % flipTimePeriod, maxSnailDelay );
            if (delay1>0) {
                delay = unsigned(delay1);
                break;
            }
            delay2 = limiter.isReady(now,&udp);
            if (delay2>0) {
                delay = unsigned(delay2);
                break;
            }
        } while (false);
        

        if (delay==0) {

            const int nchunks = 2;

            control.consumeQuant(nchunks);
            limiter.consumeQuant(now,nchunks);
            smsc_log_debug(mainlog,"processed %u udiff=%ld",nchunks,long(udp));
            proc = true;
        } else {
            if (proc) {
                smsc_log_debug(mainlog,"delay=%u delay1=%u delay2=%u udiff=%ld",
                               delay,
                               unsigned(delay1),
                               unsigned(delay2),
                               long(udp));
                proc = false;
            }
            delay = delay/1000 + 1;
            if (delay>5) {
                mon.wait(delay);
            }

            if (now % 11 == 0) {
                smsc_log_debug(mainlog,"intermediate stop");
                mon.wait(3000);
            }
        }
    }
    return 0;
}
