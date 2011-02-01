#ifndef _INFORMER_SNAPSHOTCOUNTER_H
#define _INFORMER_SNAPSHOTCOUNTER_H

#include "core/synchronization/Mutex.hpp"
#include "Typedefs.h"

namespace eyeline {
namespace informer {

class SnapshotCounter
{
public:
    SnapshotCounter( usectime_type interval,
                     unsigned nbins = 10 );


    /// accumulate the bin at current time and return the integral.
    int accumulate( usectime_type currentTime, int w = 1 );

    /// advance the time to current time.
    int advanceTime( usectime_type currentTime ) {
        return accumulate( currentTime, 0 );
    }

    /// NOTE: advanceTime() must be invoked prior to this one
    inline int getIntegral() const {
        return integral_;
    }

    inline usectime_type getInterval() const {
        return resolution_ * nbins_;
    }

private:
    smsc::core::synchronization::Mutex lock_;
    usectime_type resolution_;  // the width of a bin
    usectime_type lastTime_;    // divided by resolution
    int integral_;
    unsigned nbins_, first_, last_;
    int* slots_;
};

} // informer
} // smsc

#endif
