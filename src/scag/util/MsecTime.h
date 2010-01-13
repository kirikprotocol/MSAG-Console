#ifndef _SCAG_UTIL_MSECTIME_H
#define _SCAG_UTIL_MSECTIME_H

#include <sys/time.h>
#include "util/int.h"

namespace scag2 {
namespace util {

class MsecTime
{
public:
    // NOTE: time is signed to allow arithmetics
    typedef int64_t time_type;
    static const time_type max_time = 0x7fffffffffffffffLL;

    MsecTime() {
        ::gettimeofday(&t0_,0);
    }
    inline time_type msectime() const {
        struct timeval tv;
        ::gettimeofday(&tv,0);
        time_type t = time_type(tv.tv_sec - t0_.tv_sec)*1000 +
            (int(tv.tv_usec) - int(t0_.tv_usec)) / 1000;
        return t;
    }

    static time_type currentTimeMillis() {
        static MsecTime epoch(time_type(0)); // from epoch
        return epoch.msectime();
    }

protected:
    MsecTime( time_type t0 ) {
        t0_.tv_sec = time_t(t0/1000);
        t0_.tv_usec = suseconds_t((t0%1000)*1000);
    }
private:
    struct timeval t0_;
};

} // namespace util
} // namespace scag2

namespace scag {
namespace util {
using scag2::util::MsecTime;
} // namespace util
} // namespace scag2

#endif /* !  _SCAG_UTIL_MSECTIME_H */
