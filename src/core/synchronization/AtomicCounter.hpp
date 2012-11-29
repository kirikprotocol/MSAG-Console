#ifndef __CORE_SYNCHRONIZATION_ATOMICCOUNTER_HPP__
#define __CORE_SYNCHRONIZATION_ATOMICCOUNTER_HPP__

#ifdef __GNUC__
// using gnuc atomics

#else

#ifdef __sun

#ifdef __SunOS_5_9
// unfortunately sunos5.9 does not have atomics in libc
#ifndef ATOMICCOUNTERUSEMUTEX
#define ATOMICCOUNTERUSEMUTEX
#endif
#else
// using sunos atomics
#include <sys/atomic.h>
#include "util/int.h"
#endif

#else

// poor-man atomics using mutex
#ifndef ATOMICCOUNTERUSEMUTEX
#define ATOMICCOUNTERUSEMUTEX
#endif
#endif
#endif

#ifdef ATOMICCOUNTERUSEMUTEX
#include "Mutex.hpp"
#endif

namespace smsc {
namespace core {
namespace synchronization {

template <typename T> class AtomicCounter
{
    volatile T count_;
#ifdef ATOMICCOUNTERUSEMUTEX
    // using mutex
    Mutex lock_;
public:
    inline AtomicCounter(T init = 0) : count_(init) {}
    static inline const char* getType() { return "mutex"; }
    inline T get() { return count_; }
    inline T inc() {
        MutexGuard mg(lock_);
        return ++count_;
    }
    inline T dec() {
        MutexGuard mg(lock_);
        return --count_;
    }
#else
    // not mutex

#ifdef __GNUC__
    /// increment and return new value
public:
    inline AtomicCounter( T init = 0 ) : count_(init) {}
    static inline const char* getType() { return "gnuc"; }
    inline T get() { return count_; }
    inline T inc() { return __sync_add_and_fetch(&count_,1); }
    /// decrement and return new value
    inline T dec() { return __sync_sub_and_fetch(&count_,1); }
#endif
#endif

};

#ifndef ATOMICCOUNTERUSEMUTEX
#ifndef __GNUC__
#ifdef __sun

#define ATOMICCOUNTERDECL(tp,bits) \
template <> class AtomicCounter< tp > \
{ \
    volatile tp count_; \
public: \
    inline AtomicCounter(tp init = 0) : count_(init) {} \
    static inline const char* getType() { return "sunos"; } \
    inline tp get() { return count_; } \
    inline tp inc() { return atomic_inc_ ## bits ## _nv(&count_); } \
    inline tp dec() { return atomic_dec_ ## bits ## _nv(&count_); } \
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
