#include <algorithm>
#include "DiskFlusher.h"
#include "scag/counter/TimeSnapshot.h"
#include "scag/util/Time.h"
#include "util/PtrDestroy.h"

using smsc::core::synchronization::MutexGuard;

namespace scag2 {
namespace util {
namespace storage {

DiskFlusher::~DiskFlusher()
{
    stop();
    clear();
}

void DiskFlusher::clear()
{
    MutexGuard mg(mon_);
    assert(!started_);
    std::for_each( items_.rbegin(), items_.rend(), smsc::util::PtrDestroy());
    items_.clear();
}



int DiskFlusher::Execute()
{
    const int minSleepTime = 50;
    const int idleSleepTime = 100;
    unsigned written = 0;  // the number of written bytes in the last write
    unsigned curItem = 0;  // index of current processing item

    // registering a speed limiter
    counter::CounterPtr<counter::TimeSnapshot> speedLimiter;
    {
        const std::string cntName("sys.dflush." + name_);
        counter::Manager& mgr = counter::Manager::getInstance();
        counter::ObserverPtr o = mgr.getObserver(cntName.c_str());
        counter::CounterPtrAny anyPtr = mgr.registerAnyCounter
            (new counter::TimeSnapshot(cntName,5,minSleepTime));
        speedLimiter = static_cast<const counter::CounterPtr<counter::TimeSnapshot>& >(anyPtr);
    }

    smsc_log_info(log_,"started, %s", flushConfig_.toString().c_str());

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
            speedLimiter->accumulate(now*1000,written);

            int64_t totalWritten;
            unsigned writeTime;
            speedLimiter->getMaxDerivative( totalWritten, writeTime );
            writeTime *= speedLimiter->getResolution()/1000; // convert to msec

            /// speed in b/ms == kb/s
            speedKbs = unsigned(totalWritten / writeTime);
            if (written>0) {
                smsc_log_debug(log_,"accumulating %u bytes, now maxwr=%llu, dt=%ums, speed=%ukb/s",
                               written, totalWritten, writeTime, speedKbs);
            }
            written = 0;

            if ( speedKbs > flushConfig_.flushSpeed ) {
                // too fast, sleeping...
                // How much to sleep?
                //   vmax = N0 / ( t0 + tsleep )
                // we want tsleep
                sleepTime = unsigned(totalWritten / flushConfig_.flushSpeed) - writeTime + 1;
            }

        } // normal operation

        // smsc_log_debug(log_,"pass at %llums: spd=%ukb/s sleep=%d",now,speedKbs,sleepTime);

        if ( sleepTime > 0 ) {
            nextWakeTime = now + sleepTime;
            smsc_log_debug(log_,"going to sleep for %ums, nextwake=%llums",sleepTime,nextWakeTime);
            MutexGuard mg(mon_);
            mon_.wait((sleepTime < minSleepTime) ? minSleepTime:sleepTime);
            continue;
        }

        {
            MutexGuard mg(mon_);
            if ( items_.size() == 0 ) break;

            if ( curItem >= items_.size() ) curItem = 0;
            for ( unsigned i = curItem; ; ) {

                // processing all storages one by one
                written = items_[i]->flush(now);
                if ( written > 0 ) {
                    // smsc_log_debug(log_,"%u bytes written into %u",written,i);
                    curItem = i + 1;
                    break;
                } else if ( !started_ ) {
                    // erase this storage, it is finished
                    if ( curItem > i ) --curItem;
                    delete items_[i];
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


void DiskFlusher::flushIOStatistics( unsigned& pfget,
                                     unsigned& kbget,
                                     unsigned& pfset,
                                     unsigned& kbset )
{
    MutexGuard mg(mon_);
    for ( std::vector<DiskFlusherItemBase*>::const_iterator i = items_.begin();
          i != items_.end();
          ++i ) {
        (*i)->flushIOStatistics(pfget,kbget,pfset,kbset);
    }
}

}
}
}
