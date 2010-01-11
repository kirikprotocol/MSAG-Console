#ifndef _SCAG_COUNTER_COUNTERACCUMULATOR_H
#define _SCAG_COUNTER_COUNTERACCUMULATOR_H

#include "Counter.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace counter {

class Accumulator : public Counter
{
public:
    static int getStaticType() { return smsc::util::TypeInfo< Accumulator >::typeValue(); }

    Accumulator( const std::string& name,
                 counttime_type disposeDelayTime = 0 ) :
    Counter(name,disposeDelayTime),
    count_(0), integral_(0) {}

    virtual int getType() const { return getStaticType(); }

    virtual int64_t getCount() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return count_;
    }

    virtual int64_t getIntegral() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return integral_;
    }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        count_ = 0;
        integral_ = 0;
    }

    virtual int64_t accumulate( int64_t inc, int64_t x = 0 ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        ++count_;
        return integral_ += inc;
    }

protected:
    smsc::core::synchronization::Mutex countMutex_;
    int64_t count_;
    int64_t integral_;
};

}
}

#endif
