#include "DiskFlusher.h"
#include "scag/counter/TimeSnapshot.h"
#include "scag/util/Time.h"

namespace scag2 {
namespace util {
namespace storage {

DiskFlusher::~DiskFlusher()
{
    stop();
    clear();
}



int DiskFlusher::Execute()
{
    const int minSleepTime = 50;
    const int idleSleepTime = 100;
    unsigned written = 0;  // the number of written bytes in the last write
    unsigned curItem = 0;  // index of current processing item

    counter::TimeSnapshot speedLimiter("sys." + name_,5,minSleepTime);

    smsc_log_info(log_,"started");

    util::msectime_type nextWakeTime = 0;
    while ( true ) {

        int sleepTime = 0;     // millisec
        unsigned speedKbs = 0; // kb/s

        util::msectime_type now = util::currentTimeMillis();

        if ( !started_ ) {
            // skip speed limitation, we need to stop as soon as possible
            
        } else if ( written == 0 && now < nextWakeTime ) {
            // we have waked prematurely, continue sleeping
            sleepTime = int(nextWakeTime - now);

        } else {

            // normal operation
            speedLimiter.accumulate(now*1000,written);

            int64_t totalWritten;
            unsigned writeTime;
            speedLimiter.getMaxDerivative( totalWritten, writeTime );
            writeTime *= speedLimiter.getResolution()/1000; // convert to msec

            /// speed in b/ms == kb/s
            speedKbs = totalWritten / writeTime;
            if (written>0) {
                smsc_log_debug(log_,"accumulating %u bytes, now maxwr=%llu, dt=%ums, speed=%ukb/s",
                               written, totalWritten, writeTime, speedKbs);
            }
            written = 0;

            if ( speedKbs > maxSpeed_ ) {
                // too fast, sleeping...
                // How much to sleep?
                //   vmax = N0 / ( t0 + tsleep )
                // we want tsleep
                sleepTime = totalWritten / maxSpeed_ - writeTime + 1;
            }

        } // normal operation

        // smsc_log_debug(log_,"pass at %llums: spd=%ukb/s sleep=%d",now,speedKbs,sleepTime);

        if ( sleepTime > 0 ) {
            if (sleepTime < minSleepTime) sleepTime = minSleepTime;
            nextWakeTime = now + sleepTime;
            smsc_log_debug(log_,"going to sleep for %u msec, nextwake=%llums",sleepTime,nextWakeTime);
            MutexGuard mg(mon_);
            mon_.wait(sleepTime);
            continue;
        }

        {
            MutexGuard mg(mon_);
            if ( items_.size() == 0 ) break;

            if ( curItem >= items_.size() ) curItem = 0;
            for ( unsigned i = curItem; ; ) {

                // processing all storages one by one
                written = items_[i]->flush();
                if ( written > 0 ) {
                    // smsc_log_debug(log_,"%u bytes written into %u",written,i);
                    curItem = i + 1;
                    break;
                } else if ( !started_ ) {
                    // erase this storage, it is finished
                    if ( curItem > i ) --curItem;
                    items_.erase(items_.begin()+i);
                } else {
                    // smsc_log_debug(log_,"nothing is written into %u",i);
                    ++i;
                }
                if ( i >= items_.size() ) { i = 0; }
                if ( i == curItem ) break;

            }

            if ( written == 0 ) {

                // nothing is written and we are trying to stop
                if (!started_) break;

                // idle, we want to sleep
                // smsc_log_debug(log_,"idle, sleeping %u msec",idleSleepTime);
                mon_.wait(idleSleepTime);
            }

        }

        // NOTE: we shall accumulate written value at the next pass of the loop.

    } // while true

    smsc_log_info(log_,"stopped");
    return 0;
}

}
}
}
