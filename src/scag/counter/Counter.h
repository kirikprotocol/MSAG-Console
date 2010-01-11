#ifndef _SCAG_COUNTER_COUNTER_H
#define _SCAG_COUNTER_COUNTER_H

#include <string>
#include "util/int.h"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace counter {

class Counter;

typedef int32_t counttime_type;
const counttime_type counttime_max = 0x7fffffff;
const counttime_type counttime_locked = counttime_max;

/// A manager responsible for (delayed) disposing a counter
class Disposer
{
public:
    virtual void scheduleDisposal( counttime_type disposeTime, Counter& counter ) = 0;

    /// returns the next wake time.
    /// Counters shall schedule their disposal referred to this time.
    virtual counttime_type getWakeTime() const = 0;
protected:
    /// NOTE: should be invoked from registration
    inline void setDisposer( Counter& counter );
    /// return true if the counter may be destroyed.
    inline bool checkDisposal( Counter* counter, counttime_type now ) const;
};

class CounterPtrAny;

class Counter
{
    friend class CounterPtrAny;
    friend class Disposer;

protected:
    static smsc::logger::Logger* log_;
    static smsc::logger::Logger* loga_;

    Counter( const std::string& name, counttime_type disposeDelayTime = 0 );

public:
    virtual ~Counter();
    inline const std::string& getName() const { return name_; }

    // NOTE: this is unsynchronized access to disposal time.
    inline counttime_type getDisposeTime() const { return disposeTime_; }

    /// NOTE: any subclass should override this method
    virtual int getType() const = 0;

    /// return the number of times accumulate was invoked from the last reset.
    virtual int64_t getCount() const = 0;

    /// return the integral of values for given counter.
    virtual int64_t getIntegral() const = 0;

    /// reset the counter
    virtual void reset() = 0;

    /// accumulate value 'x' with weight 'w' and return current integral.
    virtual int64_t accumulate( int64_t x, int w = 1 ) = 0;

    /// advance counter position to x and return the count.
    /// for now, this operation is meaningful for 'Snapshot' counter only.
    virtual int64_t advance( int64_t ) {
        return getIntegral();
    }

private:
    inline void changeUsage( bool inc ) {
        smsc::core::synchronization::MutexGuard mg(usageMutex_);
        if ( inc ) {
            disposeTime_ = counttime_locked;
            ++usage_;
        } else if (--usage_ == 0) {
            if (!disposer_) {
                throw smsc::util::Exception("logic error: disposer is not set in counter '%s'",name_.c_str());
            }
            if ( disposeDelayTime_ == 0 ) {
                disposeTime_ = counttime_max; // not expired
            } else {
                disposeTime_ = disposer_->getWakeTime() + disposeDelayTime_;
                smsc_log_debug(loga_,"'%s': scheduling disposal at %d",name_.c_str(),int(disposeTime_));
                disposer_->scheduleDisposal(disposeTime_,*this);
            }
        }
    }
private:
    Counter( const Counter& );
    Counter& operator = ( const Counter& );

private:
    std::string name_;
    smsc::core::synchronization::Mutex usageMutex_;

    // a time to wait since release until disposal.
    counttime_type disposeDelayTime_;

    // absolute time when the counter should be disposed.
    counttime_type disposeTime_;

    // number of consumers
    unsigned       usage_;
    Disposer*      disposer_;
};


inline void Disposer::setDisposer(Counter&c)
{
    c.disposer_ = this;
}

inline bool Disposer::checkDisposal( Counter* c, counttime_type now ) const
{
    if ( !c || c->usage_ ) return false;
    {
        MutexGuard mg(c->usageMutex_);
        if ( c->usage_ || c->disposeTime_ > now ) return false;
    }
    return true;
}


class CounterPtrAny
{
public:
    inline CounterPtrAny( Counter* c = 0 ) : ptr_(c) {
        if (ptr_) ptr_->changeUsage(true);
    }
    inline ~CounterPtrAny() {
        if (ptr_) ptr_->changeUsage(false);
    }
    inline CounterPtrAny( const CounterPtrAny& c ) : ptr_(c.ptr_) {
        if (ptr_) ptr_->changeUsage(true);
    }
    inline CounterPtrAny& operator = ( const CounterPtrAny& c ) {
        if ( c.ptr_ != ptr_ ) {
            if (ptr_) ptr_->changeUsage(false);
            ptr_ = c.ptr_;
            if (ptr_) ptr_->changeUsage(true);
        }
        return *this;
    }

    inline Counter* operator -> () const {
        return ptr_;
    }
    inline Counter* get() const {
        return ptr_;
    }
    inline void reset( Counter* ptr ) {
        if ( ptr != ptr_ ) {
            if ( ptr ) ptr->changeUsage(true);
            if ( ptr_ ) ptr_->changeUsage(false);
            ptr_ = ptr;
        }
    }

    // inline T* release() {
    // if (ptr_) ptr_->changeUsage(false);
    // return ptr_;
    // }
protected:
    Counter* ptr_;
};


template < class T > class CounterPtr : protected CounterPtrAny
{
public:
    inline CounterPtr( T* ptr = 0 ) : CounterPtrAny(ptr) {}
    CounterPtr( const CounterPtr<T>& c ) : CounterPtrAny(c) {}
    CounterPtr& operator = ( const CounterPtr<T>& c ) {
        CounterPtrAny::operator = ( c );
        return *this;
    }
    inline T* operator -> () const { return static_cast<T*>(ptr_); }
    inline T* get() const { return static_cast<T*>(ptr_); }
    inline void reset( T* ptr ) { CounterPtrAny::reset(ptr); }
};

}
}

#endif
