#ifndef __CORE_SYNCHRONIZATION_ATOMICS_HPP__
#define __CORE_SYNCHRONIZATION_ATOMICS_HPP__

#ifdef __GNUC__
// using gnuc atomics

#else

#ifdef __sun

#ifdef __SunOS_5_9
// unfortunately sunos5.9 does not have atomics in libc
#ifndef ATOMICSUSEMUTEX
#define ATOMICSUSEMUTEX
#endif
#else
// using sunos atomics
#include <sys/atomic.h>
#include "util/int.h"
#endif

#else

// poor-man atomics using mutex
#ifndef ATOMICSUSEMUTEX
#define ATOMICSUSEMUTEX
#endif
#endif
#endif

#ifdef ATOMICSUSEMUTEX
#include "Mutex.hpp"
#endif

namespace smsc {
namespace core {
namespace synchronization {
#ifdef __sun
typedef uint64_t FastestAtomic;
#else
typedef unsigned FastestAtomic;
#endif
}
}
}

#endif
