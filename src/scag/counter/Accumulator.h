#ifndef _SCAG_COUNTER_ACCUMULATOR_H
#define _SCAG_COUNTER_ACCUMULATOR_H

#include "Counter.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace counter {

/**
 * Accumulator counter.
 * 
 * It accumulates two values:
 * 1. number of calls (count);
 * 2. total accumulated value (integral).
 */
class Accumulator : public Counter
{
public:
    static int getStaticType() { return smsc::util::TypeInfo< Accumulator >::typeValue(); }

    Accumulator( const std::string& name,
                 counttime_type disposeDelayTime = 0 ) :
    Counter(name,disposeDelayTime),
    count_(0), integral_(0) {}

    virtual int getType() const { return getStaticType(); }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        count_ = 0;
        integral_ = 0;
    }

    inline int64_t getCount() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return count_;
    }

    inline int64_t getIntegral() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return integral_;
    }

    inline int64_t accumulate( int64_t x ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        ++count_;
        return integral_ += x;
    }

protected:
    int64_t count_;
    int64_t integral_;
};

}
}

#endif
