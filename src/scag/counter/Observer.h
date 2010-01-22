#ifndef _SCAG_COUNTER_OBSERVER_H
#define _SCAG_COUNTER_OBSERVER_H

#include "util/int.h"

namespace scag2 {
namespace counter {

class Counter;

/// ActionParams
struct ActionParams
{
    enum OpType {
            GT = 1,
            LT
    };
    OpType            optype; // >, <
    int64_t           limit;  // limit
    bool              skip;   // skip other actions if this one fired
};


// ================================================================
class Observer
{
protected:
    virtual ~Observer() {}
public:
    /// notification that the value of the counter has changed
    virtual void modified( Counter& counter, int64_t value ) = 0;
    /// ref counting
    virtual void ref(bool add) = 0;
};


class ObserverPtr
{
public:
    ObserverPtr( Observer* ptr = 0 ) :ptr_(ptr) { if (ptr_) ptr_->ref(true); }
    ObserverPtr( const ObserverPtr& a ) : ptr_(a.ptr_) { if(ptr_) ptr_->ref(true); }
    ObserverPtr& operator = ( const ObserverPtr& a ) {
        if (ptr_ != a.ptr_) {
            if (ptr_) ptr_->ref(false);
            ptr_ = a.ptr_;
            if (ptr_) ptr_->ref(true);
        }
        return *this;
    }
    ~ObserverPtr() { if (ptr_) ptr_->ref(false); }
    Observer* get() const { return ptr_; }
    Observer* operator-> () const { return ptr_; }
private:
    Observer* ptr_;
};

}
}

#endif
