#ifndef __CORE_SYNCHRONIZATION_ATOMICCOUNTER_HPP__
#define __CORE_SYNCHRONIZATION_ATOMICCOUNTER_HPP__

#include "Atomics.hpp"

namespace smsc {
namespace core {
namespace synchronization {

template <typename T> class AtomicCounter
{
    volatile T count_;
#ifdef ATOMICSUSEMUTEX
    // using mutex
    Mutex lock_;
public:
    AtomicCounter() : count_(0) {}
    AtomicCounter(T init) : count_(init) {}
    static const char* getType() { return "mutex"; }
    T get() { return count_; }
    void set(T val) {
        MutexGuard mg(lock_);
        count_ = val;
    }
    T inc() {
        MutexGuard mg(lock_);
        return ++count_;
    }
    T dec() {
        MutexGuard mg(lock_);
        return --count_;
    }
    T add( T val ) {
        MutexGuard mg(lock_);
        return count_ += val;
    }
    T sub( T val ) {
        MutexGuard mg(lock_);
        return count_ -= val;
    }
    T cas( T oval, T nval ) {
        MutexGuard mg(lock_);
        if (count_ == oval) {
            count_ = nval; return oval;
        } else {
            return count_;
        }
    }
#else
    // not mutex

#ifdef __GNUC__
    /// increment and return new value
public:
    AtomicCounter() : count_(0) {}
    AtomicCounter( T init ) : count_(init) {}
    static const char* getType() { return "gnuc"; }
    T get() { return count_; }
    void set(T val) { count_ = val; }
    T inc() { return __sync_add_and_fetch(&count_,1); }
    T dec() { return __sync_sub_and_fetch(&count_,1); }
    T add( T val ) { return __sync_add_and_fetch(&count_,val); }
    T sub( T val ) { return __sync_sub_and_fetch(&count_,val); }
    T cas( T oval, T nval ) { return __sync_val_compare_and_swap(&count_,oval,nval); }
#endif
#endif

};

#ifndef ATOMICSUSEMUTEX
#ifndef __GNUC__
#ifdef __sun

#define ATOMICCOUNTERDECL(tp,bits) \
template <> class AtomicCounter< tp > \
{ \
    volatile tp count_; \
public: \
    AtomicCounter() : count_(0) {} \
    AtomicCounter(tp init) : count_(init) {} \
    static const char* getType() { return "sunos"; } \
    tp get() { return count_; } \
    void set(tp val) { count_ = val; } \
    tp inc() { return atomic_inc_ ## bits ## _nv(&count_); } \
    tp dec() { return atomic_dec_ ## bits ## _nv(&count_); } \
    tp add(tp val) { return atomic_add_ ## bits ## _nv(&count_,val); } \
    tp sub(tp val) { return atomic_add_ ## bits ## _nv(&count_,-val); } \
    tp cas(tp oval, tp nval) { return atomic_cas_ ## bits ## (&count_,oval,nval); } \
}

ATOMICCOUNTERDECL(uint8_t,8);
ATOMICCOUNTERDECL(uint16_t,16);
ATOMICCOUNTERDECL(uint32_t,32);
ATOMICCOUNTERDECL(uint64_t,64);

#undef ATOMICCOUNTERDECL

#endif
#endif
#endif

}
}
}

#endif
