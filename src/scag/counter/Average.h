#ifndef _SCAG_COUNTER_AVERAGE_H
#define _SCAG_COUNTER_AVERAGE_H

#include <cmath>
#include <list>
#include <algorithm>
#include "TimeSliceManager.h"
#include "TimeSliceGroup.h"
#include "Manager.h"

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
class Average : public Counter, public TimeSliceItem
{
public:
    static CountType getStaticType() { return TYPEAVERAGE; }

    Average( const std::string&                name,
             unsigned                          nseconds,
             Observer*                         observer = 0,
             counttime_type                    disposeDelayTime = 0 ) :
    Counter(name,observer,disposeDelayTime),
    group_(0),
    averageTime_(nseconds*usecFactor)
    {
        smsc_log_debug(loga_,"ctor %p %s '%s'",this,getTypeName(),getName().c_str());
        reset();
    }

    virtual Average* clone( const std::string& name,
                            counttime_type     disposeTime = 0 ) const
    {
        return new Average(name,unsigned(averageTime_/usecFactor),observer_.get(),disposeTime);
    }

    virtual ~Average() {
        smsc_log_debug(loga_,"dtor %p %s '%s'",this,getTypeName(),getName().c_str());
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

    virtual CountType getType() const { return getStaticType(); }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        total_.reset();
        last_.reset();
        current_.reset();
        average_ = 0;
        if ( observer_.get() ) observer_->modified(getName().c_str(),oldsev_,average_,maxval_);
    }

    /// return the total statistics
    inline const Stat& total() const { return total_; }

    /// return the last statistics
    inline const Stat& last() const { return last_; }

    /// return current data
    inline const Stat& current() const { return current_; }
    
    virtual int64_t increment( int64_t x = 1, int w = 1 ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        current_.count += w;
        current_.sum += x*w;
        current_.sum2 += x*x*w;
        return average_; // taking the previous value
    }

    virtual int64_t getValue() const {
        return average_;
    }

    virtual bool getValue( Valtype a, int64_t& value ) const
    {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        switch (a) {
        case VALUE:
        case AVERAGE: value = last_.average(); return true;
        case COUNT: value = last_.count; return true;
        case SUM: value = last_.sum; return true;
        case SIGMA: {
            double x = last_.sigma() + 0.5;
            if ( x < 0 ) x -= 1.;
            value = int64_t(x);
            return true;
        }
        default: break;
        }
        return false;
    }

    // do averaging
    virtual void advanceTime( usec_type ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        total_.merge(current_);
        last_ = current_;
        current_.reset();
        average_ = last_.average();
        if ( observer_.get() ) observer_->modified(getName().c_str(),oldsev_,average_,maxval_);
        // return average_;
    }

private:
    // virtual TimeSliceGroup* getTimeSliceGroup() const { return group_; }
    // virtual void doSetTimeSliceGroup( TimeSliceGroup* grp ) { group_ = grp; }
    virtual usec_type getTimeSliceWidth() const { return averageTime_; }
    virtual void postRegister( Manager& mgr ) {
        group_ = mgr.getTimeManager().addItem(*this,getTimeSliceWidth());
    }
    virtual void preDestroy( Manager& mgr ) {
        if (group_) { 
            group_->remItem(*this);
            group_ = 0;
        }
    }

protected:
    TimeSliceGroup* group_;
    int64_t         averageTime_;
    Stat            total_;
    Stat            last_;
    Stat            current_;
    int64_t         average_;
};

}
}

#endif
