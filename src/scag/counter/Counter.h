#ifndef _SCAG_COUNTER_COUNTER_H
#define _SCAG_COUNTER_COUNTER_H

#include <string>
#include "util/int.h"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "Observer.h"

namespace scag2 {
namespace counter {

class Counter;
class Manager;

typedef int32_t counttime_type;
const counttime_type counttime_max = 0x7fffffff;
const counttime_type counttime_locked = counttime_max;

class CounterPtrAny;

// ================================================================
class Counter
{
    friend class CounterPtrAny;
    friend class Manager;

protected:
    static smsc::logger::Logger* log_;
    static smsc::logger::Logger* loga_;

    Counter( const std::string& name,
             Observer*          observer = 0,
             counttime_type     disposeDelayTime = 0 );

public:

    typedef enum Valtype {
            VALUE = 0,      // the most appropriate value for this counter
            COUNT,
            SUM,
            MIN,
            MAX,
            AVERAGE,
            SIGMA,
            TOTALVALUE,     // TOTAL* are used for averaging
            TOTALCOUNT,
            TOTALSUM,
            TOTALMIN,
            TOTALMAX,
            TOTALAVERAGE,
            TOTALSIGMA
    } Valtype;

    virtual Counter* clone( const std::string& name,
                            counttime_type     disposeTime = 0 ) const = 0;

    virtual ~Counter();
    inline const std::string& getName() const { return name_; }

    // NOTE: this is unsynchronized access to disposal time.
    inline counttime_type getDisposeTime() const { return disposeTime_; }

    /// NOTE: any subclass should override this method
    virtual int getType() const = 0;
    virtual const char* getTypeName() const = 0;

    /// reset the counter
    virtual void reset() = 0;

    /// increment the counter with value 'x' with weight 'w'.
    /// This method is for generic usage from e.g. MSAG rules.
    /// NOTE: that x and w may be ignored for some counters.
    virtual void increment( int64_t x = 1, int w = 1 ) = 0;

    /// get the most appropriate value for the counter.
    /// it may have different sense for different types of counters.
    /// NOTE: the value may be out-of-date, e.g. for TimeSliceItem.
    virtual int64_t getValue() const = 0;

    /// get the value of counter attribute.
    /// return false if the attribute is not defined.
    virtual bool getValue( Valtype a, int64_t& value ) const = 0;

protected:
    /// guaranteed to be invoked after registration
    virtual void postRegister( Manager& mgr ) = 0;

    /// guaranteed to be invoked just before destruction
    virtual void preDestroy( Manager& mgr ) = 0;

private:

    /// NOTE: invoked only from CounterPtr
    inline void changeUsage( bool inc ) {
        smsc::core::synchronization::MutexGuard mg(usageMutex_);
        if ( inc ) {
            disposeTime_ = counttime_locked;
            ++usage_;
        } else if (--usage_ == 0) {
            // if (!disposer_) {
            // throw smsc::util::Exception("logic error: disposer is not set in counter '%s'",name_.c_str());
            // }
            if ( disposeDelayTime_ == 0 ) {
                disposeTime_ = counttime_max; // not expired
            } else {
                scheduleDisposal();
                // Manager& mgr = Manager::getInstance();
                // disposeTime_ = mgr.getWakeTime() + disposeDelayTime_;
                // smsc_log_debug(loga_,"'%s': scheduling disposal at %d",name_.c_str(),int(disposeTime_));
                // mgr.scheduleDisposal(*this);
            }
        }
    }

    void scheduleDisposal();

private:
    Counter( const Counter& );
    Counter& operator = ( const Counter& );

protected:
    mutable smsc::core::synchronization::Mutex countMutex_; // a lock to be invoked from accumulate
    // Disposer*      disposer_;
    ObserverPtr       observer_;
    CntSeverity       oldsev_; // the current severity set from observer

private:
    std::string name_;
    smsc::core::synchronization::Mutex usageMutex_; // a lock for setting number of referrers

    // a time to wait since release until disposal,
    // absolute time when the counter should be disposed.
    counttime_type disposeDelayTime_, disposeTime_;

    // number of consumers
    unsigned       usage_;
};


/// A smart pointer to any counter
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


/// a pointer to a counter of given type.
template < class T > class CounterPtr : public CounterPtrAny
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


// helper class to lock a counter from changing.
/*
class CountLock
{
public:
    CountLock( const CounterPtrAny& ptr ) : ptr_(ptr) {
        if (ptr_.get()) ptr_->countMutex_.Lock();
    }
    ~CountLock() {
        if (ptr_.get()) ptr_->countMutex_.Unlock();
    }

private:
    CountLock( const CountLock& );
    CountLock& operator = ( const CountLock& );
private:
    CounterPtrAny ptr_;
};
 */

}
}

#endif
