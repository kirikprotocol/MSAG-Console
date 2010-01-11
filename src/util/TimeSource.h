#ifndef _SMSC_UTIL_TIMESOURCE_H
#define _SMSC_UTIL_TIMESOURCE_H

#include <ctime>

namespace smsc {
namespace util {

/// Time sources.
/// Some of them provide absolute time in seconds getSeconds(),
/// others provide high-res time via getHRTime().
/// Those that have getHRTime() should also have:
/// 1. static integer constant ticksPerSec, which specifies
///    how many ticks are per second in the returned HR time;
/// 2. typedef hrtime_type which gives the type of HR time.
///
/// Also, there is TimeSourceSetup structure which should give cheapest
/// time specification for different systems.

struct TimeSourceTime {
    inline static time_t getSeconds() {
        return time(0);
    }
};

struct TimeSourceGettimeofday {
    typedef int64_t hrtime_type;
    static const hrtime_type ticksPerSec = 1000000U;
    inline static time_t getSeconds() {
        struct timeval tv;
        gettimeofday(&tv,0);
        return tv.tv_sec;
    }
    inline static hrtime_type getHRTime() {
        struct timeval tv;
        gettimeofday(&tv,0);
        return hrtime_type(tv.tv_sec)*ticksPerSec + tv.tv_usec;
    }
};


#if defined(linux)
struct TimeSourceClockGettime {
    typedef int64_t hrtime_type;
    static const hrtime_type ticksPerSec = 1000000000U;
    inline static hrtime_type getHRTime() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC,&ts);
        return hrtime_type(ts.tv_sec)*ticksPerSec + ts.tv_nsec;
    }
};
#endif

#if defined(sun)
struct TimeSourceClockGethrtime {
    typedef hrtime_t hrtime_type;
    static const hrtime_type ticksPerSec = 1000000000U;
    inline static hrtime_type getHRTime() {
        return gethrtime();
    }
};
#endif /* sun */

/// contains cheapest selection for time sources.
struct TimeSourceSetup {
#if defined(__sparc) || defined(__sparc__)
    typedef TimeSourceGettimeofday AbsSec;
    typedef TimeSourceGethrtime    HRTime;
#else
#if defined(__i386) || defined(__i386__) || defined(__ia64) || defined(__ia64__)
    typedef TimeSourceTime         AbsSec;
    typedef TimeSourceClockGettime HRTime;
#else
#error NOT IMPLEMENTED
#endif
#endif
};


}
}

#endif
