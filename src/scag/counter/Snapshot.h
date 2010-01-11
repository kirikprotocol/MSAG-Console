#ifndef _SCAG_COUNTER_SNAPSHOT_H
#define _SCAG_COUNTER_SNAPSHOT_H

#include <cassert>
#include "Counter.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace counter {

/**
 * Snapshot counter.
 * 
 * The values it accumulates are distributed over the 'x' position (typically time bins).
 * It supports a 'snapshot' view of the distribution
 * (only last 'nbins' bins are taken into the view).
 * 
 * The logic is taken from util/timeslotcounter.hpp.
 * 
 * NOTE: this counter does not invoke time() automatically.
 * Please, use TimeSnapshot to take automatic time.
 */
class Snapshot : public Counter
{
public:
    static int getStaticType() { return smsc::util::TypeInfo< Snapshot >::typeValue(); }

    Snapshot( const std::string& name,
              unsigned nbins,          // number of bins in the snapshot
              counttime_type disposeDelayTime = 0 ) :
    Counter(name,disposeDelayTime),
    integral_(0), lasttime_(0),
    nbins_(nbins), first_(0), last_(0)
    {
        assert(nbins > 0);
        slot_ = new int[nbins];
        slot_[0] = 0;
    }

    ~Snapshot() {
        delete [] slot_;
    }

    virtual int getType() const { return getStaticType(); }

    virtual int64_t getCount() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return integral_;
    }

    virtual int64_t getIntegral() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return integral_;
    }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        integral_ = 0;
        lasttime_ = 0;
        first_ = 0;
        last_ = 0;
        slot_[0] = 0;
    }

    virtual int64_t accumulate( int64_t x, int w = 1 );

    virtual int64_t advance( int64_t x ) {
        return accumulate(x,0);
    }

protected:
    int64_t integral_, lasttime_;
    unsigned nbins_, first_, last_;
    int* slot_;
};

}
}

#endif
