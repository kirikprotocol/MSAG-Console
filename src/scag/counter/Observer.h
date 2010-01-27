#ifndef _SCAG_COUNTER_OBSERVER_H
#define _SCAG_COUNTER_OBSERVER_H

#include "util/int.h"

namespace scag2 {
namespace counter {

typedef int32_t counttime_type;
const counttime_type counttime_max = 0x7fffffff;
const counttime_type counttime_locked = counttime_max;

class Counter;

/// the severity of the reporting event
typedef enum {
    NORMAL = 1,
    WARNING,
    MINOR,
    MAJOR,
    CRITICAL
} CntSeverity;


inline static const char* severityToString( CntSeverity l ) {
    switch (l) {
    case NORMAL : return "NORM";
    case WARNING : return "WARN";
    case MINOR : return "MINOR";
    case MAJOR : return "MAJOR";
    case CRITICAL : return "CRIT";
    default : return "????";
    }
}

typedef enum {
    GT = 1,
    LT
} CntOpType;

inline static const char* opTypeToString( CntOpType o ) {
    switch (o) {
    case GT : return ">";
    case LT : return "<";
    default: return "?";
    }
}


/// ---------------
struct ActionLimit
{
    ActionLimit() {}
    ActionLimit( int64_t lim, CntOpType t, CntSeverity lev ) :
    limit(lim), optype(t), severity(lev) {}

    inline bool compare( int64_t val ) const {
        return (optype == LT) ? (val < limit) : (val > limit);
    }

    int64_t           limit;   // limit
    CntOpType         optype;  // >, <
    CntSeverity       severity; // the level of the notification
};


// ================================================================
class Observer
{
protected:
    virtual ~Observer() {}
public:
    /// notification that the value of the counter has changed
    /// NOTE: the field sev is used to backtrace the severity set from the observer.
    /// it is set from the critical section, so there must be no problem with locking.
    virtual void modified( const char*   cname,
                           CntSeverity&  sev,
                           int64_t       value,
                           unsigned      maxval ) = 0;
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
