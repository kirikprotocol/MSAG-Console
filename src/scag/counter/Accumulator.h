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
    Accumulator( const std::string& name ) : Counter(name), count_(0) {}

    virtual int getType() const { return getStaticType(); }
    virtual int64_t getCount() const {
        return count_;
    }
    virtual int64_t getIntegral() const {
        return count_;
    }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        count_ = 0;
    }
    virtual int64_t accumulate( int64_t inc ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        count_ += inc;
        return count_;
    }
    virtual int64_t accumulate( int64_t, int64_t inc ) {
        return accumulate(inc);
    }

private:
    smsc::core::synchronization::Mutex countMutex_;
    int64_t count_;
};

}
}

#endif
