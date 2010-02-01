#ifndef _SCAG_COUNTER_OBSERVER_H
#define _SCAG_COUNTER_OBSERVER_H

#include <cstring>
#include "util/int.h"

namespace scag2 {
namespace counter {

typedef int32_t counttime_type;
const counttime_type counttime_max = 0x7fffffff;
const counttime_type counttime_locked = counttime_max;

class Counter;

/// the severity of the reporting event
typedef enum {
    SEVUNKNOWN = 0,
    SEVNORMAL = 1,
    SEVWARNING,
    SEVMINOR,
    SEVMAJOR,
    SEVCRITICAL
} CntSeverity;


inline static const char* severityToString( CntSeverity l ) {
    switch (l) {
    case SEVNORMAL : return "NORMAL";
    case SEVWARNING : return "WARNING";
    case SEVMINOR : return "MINOR";
    case SEVMAJOR : return "MAJOR";
    case SEVCRITICAL : return "CRITICAL";
    default : return "????";
    }
}

inline static CntSeverity stringToSeverity( const char* s ) {
    if (!s) return SEVUNKNOWN;
    if (0==strcmp(s,"NORMAL")) return SEVNORMAL;
    if (0==strcmp(s,"WARNING")) return SEVWARNING;
    if (0==strcmp(s,"MINOR")) return SEVMINOR;
    if (0==strcmp(s,"MAJOR")) return SEVMAJOR;
    if (0==strcmp(s,"CRITICAL")) return SEVCRITICAL;
    return SEVUNKNOWN;
}

typedef enum {
    OPTYPEUNKNOWN = 0,
    OPTYPEGE = 1,
    OPTYPELE
} CntOpType;

inline static const char* opTypeToString( CntOpType o ) {
    switch (o) {
    case OPTYPEGE : return "GE";
    case OPTYPELE : return "LE";
    default: return "??";
    }
}

inline static CntOpType stringToOpType( const char* optype ) {
    if ( !optype ) return OPTYPEUNKNOWN;
    if (0 == strcmp("GE",optype)) return OPTYPEGE;
    if (0 == strcmp("LE",optype)) return OPTYPELE;
    return OPTYPEUNKNOWN;
}


typedef enum CountType {
        TYPEUNKNOWN = 0,
        TYPEACCUMULATOR,
        TYPEAVERAGE,
        TYPETIMESNAPSHOT
} CountType;


inline static const char* countTypeToString(CountType c)
{
    switch (c) {
    case TYPEACCUMULATOR : return "ACCUMULATOR";
    case TYPEAVERAGE : return "AVERAGE";
    case TYPETIMESNAPSHOT : return "INTEGRAL";
    default : return "???";
    }
}
    
inline static CountType stringToCountType(const char* c)
{
    if (!c) return TYPEUNKNOWN;
    if (0==strcmp(c,"ACCUMULATOR")) {
        return TYPEACCUMULATOR;
    } else if (0==strcmp(c,"AVERAGE")) {
        return TYPEAVERAGE;
    } else if (0==strcmp(c,"INTEGRAL")) {
        return TYPETIMESNAPSHOT;
    } else {
        return TYPEUNKNOWN;
    }
}




/// ---------------
struct ActionLimit
{
    ActionLimit() {}
    ActionLimit( int64_t lim, CntOpType t, CntSeverity lev ) :
    limit(lim), optype(t), severity(lev) {}

    /// compare given value with this limit.
    /// @return true, if the value is beyond the limit.
    inline bool compare( int64_t val ) const {
        return (optype == OPTYPELE) ? (val <= limit) : (val >= limit);
    }

    /// compare two limits by their importance
    bool operator < ( const ActionLimit& lim ) const {
        if ( severity < lim.severity ) return true;
        if ( lim.severity < severity ) return false;
        // the same severity
        if ( optype == lim.optype ) {
            if ( limit == lim.limit ) return false;
            return compare(lim.limit);
        } else if ( optype == OPTYPELE ) {
            return true;
        } else return false;
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
