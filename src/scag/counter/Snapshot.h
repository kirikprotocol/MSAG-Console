#ifndef _SCAG_COUNTER_SNAPSHOT_H
#define _SCAG_COUNTER_SNAPSHOT_H

#include <cassert>
#include "Counter.h"

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
    // static CountType getStaticType() { return smsc::util::TypeInfo< Snapshot >::typeValue(); }

    Snapshot( const std::string& name,
              unsigned       nbins,        // number of bins in the snapshot
              Observer*      observer = 0,
              counttime_type disposeDelayTime = 0 ) :
    Counter(name,observer,disposeDelayTime),
    lasttime_(0),
    integral_(0),
    nbins_(nbins), first_(0), last_(0)
    {
        assert(nbins > 0);
        slot_ = new int[nbins];
        slot_[0] = 0;
    }

    ~Snapshot() {
        delete [] slot_;
    }

    // virtual int getType() const { return getStaticType(); }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        const int64_t pv = integral_;
        integral_ = 0;
        lasttime_ = 0;
        first_ = 0;
        last_ = 0;
        slot_[0] = 0;
        if ( observer_.get() && pv != integral_ ) observer_->modified(getName().c_str(),oldsev_,integral_,maxval_);
    }

    int64_t accumulate( int64_t x, int w = 1 );

    inline int64_t advanceTo( int64_t x ) {
        return accumulate(x,0);
    }

    /// NOTE: x typically must be an ordered streaming value
    virtual int64_t increment( int64_t x = 1, int w = 1 ) {
        return accumulate(x,w);
    }

    virtual int64_t getValue() const
    {
        return integral_;
    }

    virtual bool getValue( Valtype a, int64_t& value ) const
    {
        switch (a) {
        case VALUE:
        case COUNT:
        case SUM: {
            // smsc::core::synchronization::MutexGuard mg(countMutex_);
            value = integral_; return true;
        }
        default: break;
        }
        return false;
    }

    void getMaxDerivative( int64_t& value, unsigned& nbins );

protected:
    int64_t  lasttime_;
    unsigned integral_;
    unsigned nbins_, first_, last_;
    int* slot_;
};

}
}

#endif
