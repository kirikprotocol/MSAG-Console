#ifndef _SCAG_UTIL_MSECTIME_H
#define _SCAG_UTIL_MSECTIME_H

#include <time.h>

namespace scag2 {
namespace util {

class MsecTime
{
public:
    typedef int time_type;
    MsecTime() {
        ::gettimeofday(&t0_,0);
    }
    inline time_type msectime() const {
        struct timeval tv;
        ::gettimeofday( &tv, 0 );
        time_type t = time_type((tv.tv_sec - t0_.tv_sec)*1000 +
                                (int(tv.tv_usec) - int(t0_.tv_usec)) / 1000);
        return t;
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
