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
                 Observer* observer = 0,
                 counttime_type disposeDelayTime = 0 ) :
    Counter(name,observer,disposeDelayTime),
    count_(0), integral_(0) {
        smsc_log_debug(loga_,"ctor %p %s '%s'",this,getTypeName(),getName().c_str());
    }

    virtual ~Accumulator() {
        smsc_log_debug(loga_,"dtor %p %s '%s'",this,getTypeName(),getName().c_str());
    }

    virtual Accumulator* clone( const std::string& name,
                                counttime_type disposeTime = 0 ) const
    {
        return new Accumulator( name, observer_.get(), disposeTime );
    }

    virtual int getType() const { return getStaticType(); }
    virtual const char* getTypeName() const { return "accumulator"; }

    virtual void reset() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        count_ = 0;
        integral_ = 0;
        if ( observer_.get() ) observer_->modified(getName().c_str(),oldsev_,integral_);
    }

    /*
    inline int64_t getCount() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return count_;
    }

    inline int64_t getIntegral() const {
        // smsc::core::synchronization::MutexGuard mg(countMutex_);
        return integral_;
    }
     */

    virtual void increment( int64_t x = 1, int w = 1 ) {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        count_ += w;
        integral_ += x*w;
        if ( observer_.get() ) observer_->modified(getName().c_str(),oldsev_,integral_);
    }

    virtual int64_t getValue() const
    {
        return integral_;
    }

    virtual bool getValue( Valtype a, int64_t& value ) const
    {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        switch (a) {
        case SUM:
        case VALUE: value = integral_; return true;
        case COUNT: value = count_; return true;
        case AVERAGE: count_ ? value = ( integral_ + count_/2 ) / count_ : 0; return true;
        default: break;
        }
        return false;
    }

protected:
    virtual void postRegister( Manager& mgr ) {}
    virtual void preDestroy( Manager& mgr ) {}

protected:
    int64_t     count_;
    int64_t     integral_;
};

}
}

#endif
