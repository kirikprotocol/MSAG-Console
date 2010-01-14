#ifndef _SCAG_COUNTER_AVERAGE_H
#define _SCAG_COUNTER_AVERAGE_H

#include <cmath>
#include <list>
#include <algorithm>
#include "AveragingGroup.h"
#include "util/TypeInfo.h"
#include "scag/util/MsecTime.h"

namespace scag2 {
namespace counter {

/**
 * Averaging counter.
 * 
 * It accumulates incoming values into current statistics set.
 * Then there is a special call average() into which the data is merged into total set.
 * 
 * Typically an averaging counter should be added into averaging group.
 */
class Average : public Counter, public AverageItem
{
public:
    static int getStaticType() { return smsc::util::TypeInfo< Average >::typeValue(); }

    Average( const std::string&                name,
             const CounterPtr<AveragingGroup>& group,
             counttime_type                    disposeDelayTime = 0 ) :
    Counter(name,disposeDelayTime),
    group_(group)
    {
        reset();
        if (group_.get()) addItem(*group_.get());
    }

    virtual ~Average() {
        if (group_.get()) remItem(*group_.get());
    }


    struct Stat {
        int64_t  sum;         // the sum of values
        int64_t  sum2;        // the sum of squares
        int64_t  count;       // total number of entries
        inline void reset() { count = 0; sum = 0; sum2 = 0; }
        inline void merge( const Stat& s ) {
            count += s.count;
            sum += s.sum;
            sum2 += s.sum2;
        }
        inline int64_t average() const {
            register const int64_t c = count;
            return c ? (sum+c/2) / c : 0;
        }
        inline double sigma() const {
            register const int64_t c = count;
            if ( c == 0 ) return 0;
            const double avg = double(sum) / c;
            const double sig2 = double(sum2) / c - avg*avg;
            return sig2 > 0 ? std::sqrt(sig2) : -std::sqrt(-sig2);
        }
    };

    virtual int getType() const { return getStaticType(); }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        total_.reset();
        last_.reset();
        current_.reset();
    }

    /// return the total statistics
    inline const Stat& total() const { return total_; }

    /// return the last statistics
    inline const Stat& last() const { return last_; }

    /// return current data
    inline const Stat& current() const { return current_; }
    
    virtual void increment( int64_t x = 1, int w = 1 ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        current_.count += w;
        current_.sum += x*w;
        current_.sum2 += x*x*w;
    }

    virtual bool getValue( Valtype a, int64_t& value )
    {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        switch (a) {
        case VALUE:
        case AVERAGE: value = current_.average(); return true;
        case COUNT: value = current_.count; return true;
        case SUM: value = current_.sum; return true;
        case SIGMA: {
            double x = current_.sigma() + 0.5;
            if ( x < 0 ) x -= 1.;
            value = int64_t(x);
            return true;
        }
        default: break;
        }
        return false;
    }

    // do averaging
    virtual void average( util::MsecTime::time_type ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        total_.merge(current_);
        last_ = current_;
        current_.reset();
    }

protected:
    CounterPtr<AveragingGroup> group_;
    Stat                       total_;
    Stat                       last_;
    Stat                       current_;
};

}
}

#endif
